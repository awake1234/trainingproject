//展示用户文件列表的CGI程序
#include "fcgi_config.h"
#include "fcgi_stdio.h"
#include <stdlib.h>
#include <stdio.h>
#include  <string.h>
#include <make_log.h>
#include <util_cgi.h>
#include <deal_mysql.h>
#include "cfg.h"
#include "cJSON.h"
#include <sys/time.h>



#define MYFILES_LOG_MODULE "cgi"
#define MYFILES_LOG_PROC  "myfiles"



//mysql 数据库配置信息 用户名， 密码， 数据库名称
static char mysql_user[128] = {0};
static char mysql_pwd[128] = {0};
static char mysql_db[128] = {0};//读取配置信息


//读取配置文件信息
void read_cfg()
{
    //读取mysql数据库配置信息
    get_cfg_value(CFG_PATH, "mysql", "user", mysql_user);
    get_cfg_value(CFG_PATH, "mysql", "password", mysql_pwd);
    get_cfg_value(CFG_PATH, "mysql", "database", mysql_db);
    LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "mysql:[user=%s,pwd=%s,database=%s]", mysql_user, mysql_pwd, mysql_db);
}

 //通过json包获取用户名和toen 
int get_count_json_info(char *buf,char * user,char * token)
{
    
    int ret = 0;
    //1.得到CJSON格式的根结构体
    cJSON * root = cJSON_Parse(buf);
    if(NULL==root)
    {
    	LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "cJSON_Parse err\n");
        ret = -1;
        goto END;
    }

    //2.返回指定的json对象
    cJSON * child1 = cJSON_GetObjectItem(root,"user");
    if(NULL == child1)
    {
        LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
    }

    char tempuser[128]={0};
    //进行加密
    strcpy(tempuser,child1->valuestring);
    ret = encode_message(tempuser,user);
    if(ret!=0)
    {
        LOG(MYFILES_LOG_MODULE,MYFILES_LOG_PROC,"encode user error\n");
        goto END;
    }

    cJSON *  child2 = cJSON_GetObjectItem(root,"token");
    if(NULL==child2)
    {
        LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
    }

    strcpy(token,child2->valuestring);

END:
     if(root!=NULL)
     {
     	cJSON_Delete(root);
     	root=NULL;
     }
 
     return ret;
}
     
 //获取用户文件的数量
long get_user_files_count(char *user)
{
	long  num = 0;

  	char sql_cmd[SQL_MAX_LEN] = {0};
  	MYSQL * conn = NULL;


  	conn = msql_conn(mysql_user,mysql_pwd,mysql_db);
  	if(conn==NULL)
  	{
  		LOG(MYFILES_LOG_MODULE,MYFILES_LOG_PROC,"mysql_conn fail\n");
  		num = -1;
  		goto END;
  	}  

    //设置数据库的编码，处理中文字符问题
    mysql_query(conn,"set names utf8");

    sprintf(sql_cmd,"select count from user_file_count where user = '%s'",user);
    char tmp[512] = {0};

    int ret = process_result_one(conn,sql_cmd,tmp);
    if(ret!=0)
    {
    	LOG(MYFILES_LOG_MODULE,MYFILES_LOG_PROC,"%s 操作失败\n",sql_cmd);
    	num = -1;
    	goto END;
    }

    num = atol(tmp);  //字符型转长整型
END:
    if(conn!=NULL)
    {
    	mysql_close(conn);
    }

     //返回文件的数量
     return num;
 }


//返回给前端结果,参数1：用户文件的数量，参数2：口令验证的结果
void return_count_status(long num,int result)
{

   char num_buf[128]={0};  //保存用户文件数量
   char * out = NULL;   
   char * token_response;
   if(result==0)
   {
      token_response = "110";   //口令验证成功
   }else
   {
   	 token_response = "111";   //口令验证失败
   }

  sprintf(num_buf,"%ld",num);

  cJSON * root = cJSON_CreateObject();  //创建json项目
  cJSON_AddStringToObject(root,"num",num_buf);
  cJSON_AddStringToObject(root,"code",token_response);
  out = cJSON_Print(root);   //转换成字符串

  cJSON_Delete(root);

  if(out!=NULL)
  {
  	printf(out);
  	free(out);
  }
}


 //解析json包获取信息
int  get_fileslist_json_info(char * buf,char *user,char * token,int  *p_start,int *p_count)
{

	int ret = 0;
   /*json数据如下
    {
        "user": "yyyyy"
        "token": xxxx
        "start": 0
        "count": 10
    }
  */
   cJSON *root = cJSON_Parse(buf);
   if(root==NULL)
   {
   	 LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "cJSON_Parse err\n");
     ret = -1;
     goto END;
   }

   //返回指定字符对应的json对象
   cJSON *child1 = cJSON_GetObjectItem(root,"user");
   if(NULL == child1)
   {
        LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
    }
    
    char tempuser[128]={0};

    strcpy(tempuser,child1->valuestring);

    //加密
    ret = encode_message(tempuser,user);
    if(ret!=0)
    {
        LOG(MYFILES_LOG_MODULE,MYFILES_LOG_PROC,"encode user error\n");
        goto END;
    }

     //token
    cJSON *child2 = cJSON_GetObjectItem(root, "token");
    if(NULL == child2)
    {
        LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
    }

    strcpy(token, child2->valuestring); //拷贝内容

    //文件起点
    cJSON *child3 = cJSON_GetObjectItem(root, "start");
    if(NULL == child3)
    {
        LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
    }

    *p_start = child3->valueint;

    //文件请求个数
    cJSON *child4 = cJSON_GetObjectItem(root, "count");
    if(NULL == child4)
    {
        LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
    }

    *p_count = child4->valueint;

END:
    if(root != NULL)
    {
        cJSON_Delete(root);//删除json对象
        root = NULL;
    }

    return ret;
}

 //获取用户文件列表
int get_user_filelist(char *cmd,char * user,int start,int count)
{
   int ret = 0;
   char sql_cmd[SQL_MAX_LEN]={0};
   MYSQL * conn = NULL;
   MYSQL_RES * res_ret = NULL;
   cJSON * root = NULL;
   cJSON * array = NULL;
   char * out = NULL;
   char * out2 = NULL;

   conn = msql_conn(mysql_user,mysql_pwd,mysql_db);
   if (conn == NULL)
   {
        LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "msql_conn err\n");
        ret = -1;
        goto END;
    }
  
    //设置数据库编码，主要处理中文编码问题
    mysql_query(conn, "set names utf8");

    //普通用户视图
    if(strcmp(cmd,"normal")==0)
    {
      sprintf(sql_cmd,"select user_file_list.*,file_info.url,file_info.size,file_info.type from file_info,user_file_list where user='%s' and file_info.md5=user_file_list.md5 limit %d,%d",user,start,count);
    }else if(strcmp(cmd,"pvASC")==0) //按文件大小升序
    {
         //sql语句
        sprintf(sql_cmd, "select user_file_list.*, file_info.url, file_info.size, file_info.type from file_info, user_file_list where user = '%s' and file_info.md5 = user_file_list.md5  order by file_info.size  asc limit %d, %d", user, start, count);
    }else if(strcmp(cmd,"pvDES")==0)  //按文件大小降序
    {
       //sql语句
       sprintf(sql_cmd, "select user_file_list.*, file_info.url, file_info.size, file_info.type from file_info, user_file_list where user = '%s' and file_info.md5 = user_file_list.md5 order by file_info.size  desc limit %d, %d", user, start, count);
    }
    LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "%s 在操作\n", sql_cmd);
    
    if (mysql_query(conn, sql_cmd) != 0)
    {
        LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "%s 操作失败：%s\n", sql_cmd, mysql_error(conn));
        ret = -1;
        goto END;
    }

    res_ret = mysql_store_result(conn);  //生成结果集
    if(res_ret==NULL)
    { 
    	LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "smysql_store_result error: %s!\n", mysql_error(conn));
        ret = -1;
        goto END;
    }
   
    ulong line = 0;
    //mysql_num_rows接收由mysql_store_result返回的结果结构集，并返回结构集中的行数
    line = mysql_num_rows(res_ret);
    if (line == 0)//没有结果
    {
        LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "mysql_num_rows(res_set) failed：%s\n", mysql_error(conn));
        ret = -1;
        goto END;
    }

    MYSQL_ROW row;

    root = cJSON_CreateObject();
    array = cJSON_CreateArray();
    //mysql_fetch_row从mysql_store_result得到的结果集中取一行，放到一个行结构体中
    //当数据取完或发生错误时跳出循环
    while((row=mysql_fetch_row(res_ret))!=NULL)
    {
    	cJSON * item = cJSON_CreateObject();

        //user
        if(row[0]!=NULL)
        {
        	cJSON_AddStringToObject(item,"user",row[0]);
        }
        
        //md5
        if(row[1]!=NULL)
        {
           cJSON_AddStringToObject(item,"md5",row[1]);
        }
    
       //-- createtime 文件创建时间
        if(row[2] != NULL)
        {
            cJSON_AddStringToObject(item, "time", row[2]);
        }

        //-- filename 文件名字
        if(row[3] != NULL)
        {
            cJSON_AddStringToObject(item, "filename", row[3]);
        }


        //-- shared_status 共享状态, 0为没有共享， 1为共享
        if(row[4] != NULL)
        {
            cJSON_AddNumberToObject(item, "shareStatus", atoi( row[4] ));
        }

        //-- pv 文件下载量，默认值为0，下载一次加1
        if(row[5] != NULL)
        {
            cJSON_AddNumberToObject(item, "pv", atol( row[5] ));
        }

        //-- url 文件url
        if(row[6] != NULL)
        {
            cJSON_AddStringToObject(item, "url", row[6]);
        }

        //-- size 文件大小, 以字节为单位
        if(row[7] != NULL)
        {
            cJSON_AddNumberToObject(item, "size", atol( row[7] ));
        }

        //-- type 文件类型： png, zip, mp4……
        if(row[8] != NULL)
        {
            cJSON_AddStringToObject(item, "type", row[8]);
        }
         

        cJSON_AddItemToArray(array,item);

    }
  
    cJSON_AddItemToObject(root, "files", array);

    out = cJSON_Print(root);

// LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "%s\n", out);

END:
    if(ret==0)
    {
    	printf("%s",out);
    }else
    {
		out2 =NULL;
    	out2 = return_status("015"); 
    }

    if(out2!=NULL)
    {
    	printf(out2); //给前端返回失败结果code:015
    }

    if(res_ret!=NULL)
    {
    	//释放得到的结果集
    	mysql_free_result(res_ret);
    }

    if(conn != NULL)
    {
        mysql_close(conn);
    }

    if(root != NULL)
    {
        cJSON_Delete(root);
    }
    return ret;
}


int main()
{
   char cmd[20];  //保存用户的命令
   
   
   //1.读取配置文件信息
	read_cfg();

   while(FCGI_Accept()>=0)
   {
   	//获取url地址?后面的内容
   	char *query = getenv("QUERY_STRING");
    
    //解析命令获取命令的内容
    query_parse_key_value(query,"cmd",cmd,NULL);
    LOG(MYFILES_LOG_MODULE,MYFILES_LOG_PROC,"query cmd=%s\n",cmd);
  
    char *contentlength = getenv("CONTENT_LENGTH");
    int len;

    printf("Content-type: text/html\r\n\r\n");


    if(contentlength==NULL)
    {
    	len = 0;
    }else{
    	len = atoi(contentlength);
    }


    if(len<=0)
    {
    	printf("No data from standard input.<p>\n");
    	LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "len = 0, No data from standard input\n");
    }else
    {
    	char buf[1024*4] = {0};
    	int ret = 0;
    	ret = fread(buf,1,len,stdin);
		char user[128]={0};
		char token[20]={0};

    	if(ret==0)
    	{
    		LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "fread(buf, 1, len, stdin) err\n");
            continue;
    	}
    	LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "buf = %s\n", buf);
        
        //命令是获取用户文件的数量
        if(strcmp(cmd,"count")==0)
        {
           get_count_json_info(buf,user,token);   //通过json包获取用户名和toen 
         
           //验证token是否正确
           ret = verify_token("user_token",user,token);
           if(ret!=0)
           {
             LOG(MYFILES_LOG_MODULE,MYFILES_LOG_PROC,"verify token fail\n");
           }


           //获取用户文件的数量
           long num = get_user_files_count(user);
           if(num<0)
           {
              LOG(MYFILES_LOG_MODULE,MYFILES_LOG_PROC,"get_user_file_count error\n");
              return -1;
           }


            //给前端返回结果
           return_count_status(num,ret);
         }else
         {  //获取用户文件信息 127.0.0.1:80/myfiles&cmd=normal
            //按下载量升序 127.0.0.1:80/myfiles?cmd=pvasc
            //按下载量降序127.0.0.1:80/myfiles?cmd=pvdesc
             
             int start;  //文件起点
             int count;  //请求的文件数量
             get_fileslist_json_info(buf,user,token,&start,&count);  //解析json包获取信息

             LOG(MYFILES_LOG_MODULE, MYFILES_LOG_PROC, "user = %s, token = %s, start = %d, count = %d\n", user, token, start, count);

             //验证登录口令
             ret = verify_token("user_token",user,token);
             if(ret==0)
             {
             	get_user_filelist(cmd,user,start,count);  //获取用户文件列表
             }else
             {
             	char * out = return_status("111");  //token验证失败
                if(out!=NULL)
                {
                	printf(out);  //给前端反馈错误码
                    free(out);
                }
              }
            }
      }
}
    return 0;
}
