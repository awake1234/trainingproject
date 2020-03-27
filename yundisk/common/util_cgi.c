//cgi通用工具接口

#include<ctype.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include "make_log.h"
#include "cJSON.h"
#include "cfg.h"
#include "util_cgi.h"
#include "redis_op.h"
#include "des.h"
#include "base64.h"



//定义一个给得到的用户信息进行加密的函数
//使用des加密并转换成base64编码格式
//传出参数afterinfo
int encode_message(char * before_info,char * afterinfo)
{
	int ret = 0;
    //使用des加密
    
    //des加密后的数据长度
    unsigned char tempinfo[512] = {0};
    int OutDataLen;

    //使用des加密
    ret =  DesEnc((unsigned char *)before_info,strlen(before_info),tempinfo,&OutDataLen);
    if(ret!=0)
    {
       LOG(UTIL_LOG_MODULE,UTIL_LOG_PROC,"desenc(info) fail\n");
       return ret;	
    }

    //使用base64编码格式,通过传出参数返回加密后的字符串
    char * baseinfo = base64_encode((const unsigned char *)tempinfo,strlen(tempinfo), afterinfo);
    if(baseinfo==NULL)
    {
    	LOG(UTIL_LOG_MODULE,UTIL_LOG_PROC,"base64_encode(info) fail");
    	ret = -1;
    }
    return ret;
}






//返回给前端结果
char * return_status(char * status_num)
{
	char *out = NULL;
	cJSON * root = cJSON_CreateObject();  //创建一个json
	cJSON_AddStringToObject(root,"code",status_num); //{"code","000"}
	out = cJSON_Print(root);  //cJSON 格式转化为字符串
  
    //删除这个root
    cJSON_Delete(root);
    
    return out;
}


int  trim_space(char * inbuf)
{
   if(inbuf==NULL)
   {
      LOG(UTIL_LOG_MODULE,UTIL_LOG_PROC,"input string is NULL\n");
      return -1;
   }
  
   int i = 0;
   int j = strlen(inbuf)-1;

   char * str = inbuf;

   int count = 0;
   

   while(isspace(str[i])&&str[i]!='\0')
   {
	   i++;
   }

   while(isspace(str[j])&&j>i)
   {
	   j--;
   }

   count = j-i+1;

   strncpy(inbuf,str+i,count);
   inbuf[count]='\0';

   return 0;
}


//得到文件后缀
int  get_file_suffix(const char *file_name,char * suffix)
{
    const char * p = file_name;
    int len = 0;
    const char *k= NULL;
    const char *q = NULL;

    if(p==NULL)
    {
      return -1;
    }

    q = p;

    while(*q!='\0')
    {
        q++;
    }
    k = q;
    while(*k!='.'&&k!=p)
    {
        k--;
    }
    if(*k=='.')
    {
        k++;
        len = q-k;
        if(len!=0)
        {
            strncpy(suffix,k,len);
            suffix[len] = '\0';
        }else{
            strncpy(suffix,"null",5);
        }
    }else{
        strncpy(suffix,"null",5);
    }
   return 0;
}

//验证登录token
//表名，hash键值
int verify_token(char *hashtable,char * field,char *token)
{
   int ret = 0;
   redisContext * redis_conn = NULL;
   char tmp_token[128] = {0};

   //redis 服务器ip,端口
   char redis_ip[30] = {0};
   char redis_port[10] = {0};

   //读取redis配置信息
   get_cfg_value(CFG_PATH,"redis","ip",redis_ip);
   get_cfg_value(CFG_PATH,"redis","port",redis_port);


   //连接redis数据库
   redis_conn = rop_connectdb_nopwd(redis_ip,redis_port);
   if(redis_conn==NULL)
   {
     LOG(UTIL_LOG_MODULE, UTIL_LOG_PROC, "redis connected error\n");
     ret = -1;
     goto END;
   }

    ret = rop_hash_get(redis_conn,hashtable, field, tmp_token);
    if(ret==0)
    {
      if(strcmp(token,tmp_token)!=0) //token不相等
      {
        ret = -1;
      }
    }
END:
   if(redis_conn!=NULL)
   {
    rop_disconnect(redis_conn);
   }

    return ret;
}


//在字符串中查找字符串第一次出现的位置
char * memstr(char * fulldata,int full_data_len,char *substr)
{
   if(fulldata==NULL || full_data_len<=0 || substr==NULL)
   {
	   return NULL;
   }

   if(*substr=='\0')
   {
	   return NULL;
   }

   //先得到匹配的字符串的长度
   int sublen = strlen(substr);

   int i;
   char * cur =fulldata;
   int last_possible = full_data_len-sublen+1;
   for(i=0;i<last_possible;++i)
   {
      if(*cur == *substr)
	  {
		  if(memcmp(cur,substr,sublen)==0)
		  {
			  //找到了
			  return cur;
		  }
	  }
      cur++;
   }

   return NULL;

}


int query_parse_key_value(const char * query,const char *key,char *value,int *value_len_p)
{

     char * cur = NULL;
     char * end = NULL;
     int value_len = 0;

     //匹配相应的key值
     cur = strstr(query,key);
     if(cur==NULL)
     {
      LOG(UTIL_LOG_MODULE,UTIL_LOG_PROC,"not find the cmd %s\n",key);
      return -1;
     }
  

     //匹配到了
     int len = strlen(key); 
     cur+=(len+1);  //跳过等号,指向value的首地址

     end = cur;

     //循环用来找到第一个参数的结束位置
     while('\0'!=*end && '#'!=*end && '&'!=*end)
     {
        end++;        
     }

     value_len=end-cur;

     strncpy(value,cur,value_len);
     value[value_len] = '\0';  //添加文件结束标志

     if(value_len_p!=NULL)
     {
      *value_len_p = value_len;
     }
     
     return 0;
}
