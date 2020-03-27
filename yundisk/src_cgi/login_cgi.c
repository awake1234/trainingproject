#include "fcgi_config.h"
#include "fcgi_stdio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "make_log.h"
#include "util_cgi.h"
#include "deal_mysql.h"
#include "redis_op.h"
#include "cfg.h"
#include "cJSON.h"
#include "des.h"
#include "base64.h"
#include "md5.h"
#include <time.h>
#include "login_cgi.h"

//写入到日志中的宏
#define LOGIN_LOG_MODULE "cgi"
#define LOGIN_LOG_PROC "login"


//定义得到用户信息的函数,获得的都是加密后的数据
int get_user_info(char *buf,char *user,char *pwd)
{
    //将char * 转换成cJSON对象
    int ret = 0;
    cJSON * root = cJSON_Parse(buf);
    if(root==NULL)
    {
       LOG(LOGIN_LOG_MODULE,LOGIN_LOG_PROC,"cJSON_Parse() error\n");
       ret = -1;
       goto END;
    }

    //用来保存得到的字符串的值
    char tempuser[128] = {0};
    char temppwd[128] = {0};
    //得到用户名的值
    cJSON *userinfo = cJSON_GetObjectItem(root,"user");
    if(userinfo==NULL)
    {
    	LOG(LOGIN_LOG_MODULE,LOGIN_LOG_PROC,"cJSON_GetObjectItem(user) error\n");
    	ret = -1;
    	goto END;
    } 
    //拷贝进tempuser中
    strcpy(tempuser,userinfo->valuestring);
   
    //加密之后传出

	ret = encode_message(tempuser,user);
	if(ret!=0)
	{
    	LOG(LOGIN_LOG_MODULE,LOGIN_LOG_PROC,"encode_message(username) error\n");
    	ret = -1;
    	goto END;
	}
    //得到密码的值
    cJSON *pwdinfo = cJSON_GetObjectItem(root,"pwd");
    if(pwdinfo==NULL)
    {
    	LOG(LOGIN_LOG_MODULE,LOGIN_LOG_PROC,"cJSON_GetObjectItem(pwd) error\n");
    	ret = -1;
    	goto END;
    } 
    //拷贝进temppwd中
    strcpy(temppwd,pwdinfo->valuestring);
	//加密之后传出
	ret = encode_message(temppwd,pwd);
	if(ret!=0)
	{
    
    	LOG(LOGIN_LOG_MODULE,LOGIN_LOG_PROC,"encode_message(pwd) error\n");
    	ret = -1;
    	goto END;
	}

END:
   if(root!=NULL)
   {
      //删除当前的实体
      cJSON_Delete(root);
      root==NULL;
   }
   
   return ret;
}

//检测用户信息是否正确
int check_user_info(char *username,char *pwd)
{
    //连接数据库
    MYSQL * conn = NULL;  //创建一个数据库句柄
	int ret = 0;
    char sql_cmd[SQL_MAX_LEN] = {0};
    //得到数据库的用户，密码，数据库标识等信息
    char mysql_user[256] = {0};
    char mysql_pwd[256] = {0};
    char mysql_db[256] = {0};
    get_mysql_info(mysql_user,mysql_pwd,mysql_db);
    LOG(LOGIN_LOG_MODULE,LOGIN_LOG_PROC,"mysql_user=%s,mysql_pwd=%s,mysql_db=%s\n",mysql_user,mysql_pwd,mysql_db);
    //连接数据库
    conn = msql_conn(mysql_user,mysql_pwd,mysql_db);
    if(conn==NULL)
    {
    	LOG(LOGIN_LOG_MODULE,LOGIN_LOG_PROC,"mysql_conn err\n");
         return -1;
    }

    //设置数据库编码处理中文编码问题
    mysql_query(conn,"set name utf8");

    sprintf(sql_cmd,"select password from user where name=\"%s\"",username);

     char temp[PWD_LEN]={0};

     process_result_one(conn,sql_cmd,temp); //执行sql语句,结果集保存在temp中
     if(strcmp(temp,pwd)==0)
     {
     	ret = 0;
     }else
     {
     	ret=-1;
     }

     mysql_close(conn);

     return ret;
}


//定义一个生成token的函数
int set_toekn(char *user,char *token)
{
     int ret = 0;
     redisContext *redis_conn = NULL; //连接redis数据库句柄

     //redis服务器的ip 和端口
     char redis_ip[30] = {0};
     char redis_port[30] = {0};

     //读取redis配置信息
     get_cfg_value(CFG_PATH,"redis","ip",redis_ip);
     get_cfg_value(CFG_PATH,"redis","port",redis_port);
     LOG(LOGIN_LOG_MODULE,LOGIN_LOG_PROC,"redis:ip:%s,port:%s",redis_ip,redis_port);

     //连接redis数据库,无密码连接
     redis_conn = rop_connectdb_nopwd(redis_ip,redis_port);
     if(redis_conn==NULL)
     {
      LOG(LOGIN_LOG_MODULE,LOGIN_LOG_PROC,"redis connect error");
      ret = -1;
      goto END;
     }

     //产生四个小于1000的随机数
     int rand_num[4]={0};
     srand((unsigned int)time(NULL));

     int i = 0;
     for(i=0;i<4;i++)
     {
        rand_num[i] = rand()%1000;
     }


     char tmpchar[1024] = {0};
     sprintf(tmpchar,"%s%d%d%d%d",user,rand_num[0],rand_num[1],rand_num[2],rand_num[3]); //使用用户名和密码拼接成一个字符串
     LOG(LOGIN_LOG_MODULE,LOGIN_LOG_PROC,"tmpchar:%s",tmpchar);

     //加密
     char enc_tmp[1024*2] = {0};
     int  enc_len = 0;

     ret = DesEnc((unsigned char *)tmpchar,strlen(tmpchar),(unsigned char *)enc_tmp,&enc_len);
     if(ret!=0)
     {
      LOG(LOGIN_LOG_MODULE,LOGIN_LOG_PROC,"DesEnc error\n");
      ret = -1;
      goto END;
     }
     
     //转换成base64编码
     char char_base64[1024*3] = {0};
     base64_encode((const unsigned char *)enc_tmp,enc_len,char_base64); //base64编码
     LOG(LOGIN_LOG_MODULE,LOGIN_LOG_PROC,"char_base64=%s\n",char_base64);


     //用md5加密
     MD5_CTX token_md5;
     MD5Init(&token_md5);
     unsigned char Final_token[16];   //用来保存最终生成的md5串
     MD5Update(&token_md5,(unsigned char *)char_base64,strlen(char_base64));
     MD5Final(&token_md5,Final_token); 

     //转成16进制
     char str[100] = {0};
     for(i=0;i<16;i++)
     {
      sprintf(str,"%02x",Final_token[i]);
      strcpy(token,str);   //拷贝进token参数传出
     }
     
     //采用hash表的方式来保存token
     char * title = "user_token"; //表名
      //一个user对应一个token;
     ret =  rop_hash_set(redis_conn, title, user, token);
     if(ret!=0)
     {
        LOG(LOGIN_LOG_MODULE,LOGIN_LOG_PROC,"rop_hash_set() error");
        goto END;
     }

END:
    if(redis_conn!=NULL)
    {
      rop_disconnect(redis_conn);
    }
    return ret;
}

//返回登录验证的状态
void return_login_status(char * status_num ,char * token)
{
      char *out =NULL;
      cJSON * root = cJSON_CreateObject();  //创建json对象
      cJSON_AddStringToObject(root,"code",status_num);
      cJSON_AddStringToObject(root,"token",token);
      out = cJSON_Print(root);   //转化成字符串

      cJSON_Delete(root);
      if(out!=0)
      {
          printf(out); //给前端反馈消息
          free(out); 
      }

}

int main()
{
   //阻塞等待用户连接
   while(FCGI_Accept()>=0)
   {
   	   //从环境变量中读取数据的长度
   	   char * contentlength = getenv("CONTENT_LENGTH");
   	   int len;
       char token[128]={0};   //用来保存生成的令牌

	   //首先要输出http头
	   printf("Content-type: text/html\r\n\r\n");
       if(contentlength==NULL)
       {
       	 len = 0;
       }else
       {
       	len = atoi(contentlength);
       }
       
       //没有登录用户的信息
       if(len<=0)
       {
       	 LOG(LOGIN_LOG_MODULE,LOGIN_LOG_PROC,"len = 0,no login info received\n");
       }else
       {
       	 //读取用户的信息
         //1.创建一块内存来保存读取的内容
         char buf[1024*4]={0};
         int ret = 0;
         ret = fread(buf,1,len,stdin); 
         if(ret==0)
         {
         	LOG(LOGIN_LOG_MODULE,LOGIN_LOG_PROC,"fread() error\n");
         	continue;
         } 
   
         //将读取的内存地址写入
         LOG(LOGIN_LOG_MODULE,LOGIN_LOG_PROC,"buf=%s，len = %d\n",buf,ret);

         //得到用户的信息
         char user[512] = {0};
         char pwd[512] = {0};
         ret = get_user_info(buf,user,pwd);
         if(ret!=0)
         {
         	LOG(LOGIN_LOG_MODULE,LOGIN_LOG_PROC,"get_user_info() fail\n");
         	return ret;
         }

         
         LOG(LOGIN_LOG_MODULE,LOGIN_LOG_PROC,"get_user_info:user:%s,pwd:%s\n",user,pwd);

         //检测用户名和密码是否正确
         ret = check_user_info(user,pwd);
         //登录成功
         if(ret==0)
         {
            memset(token,0,sizeof(token));
            //生成token;
            ret = set_toekn(user,token);
            if(ret==0)
            {
              //发送成功结果
               return_login_status("000",token);
            }else
			{

              return_login_status("001","fail");
			}
		 }
			else
            {
              return_login_status("001","fail");
            }
        

       }
       
        
   }

     return 0;
}
