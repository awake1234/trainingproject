#include "fcgi_config.h"
#include "fcgi_stdio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "make_log.h"
#include "util_cgi.h"
#include "deal_mysql.h"
#include "cfg.h"
#include "cJSON.h"
#include <sys/time.h>
#include <pthread.h>



#define MD5_LOG_MODULE  "cgi"
#define MD5_LOG_PROC    "md5"


static char mysql_user[128] = {0};
static char mysql_pwd[128] = {0};
static char mysql_db[128] = {0};

void read_cfg()
{
    //读取mysql数据库配置信息
    get_cfg_value(CFG_PATH, "mysql", "user", mysql_user);
    get_cfg_value(CFG_PATH, "mysql", "password", mysql_pwd);
    get_cfg_value(CFG_PATH, "mysql", "database", mysql_db);
    LOG(MD5_LOG_MODULE, MD5_LOG_PROC, "mysql:[user=%s,pwd=%s,database=%s]", mysql_user, mysql_pwd, mysql_db);
}

//解析秒传信息的json包
int get_md5_info(char *buf,char *user,char *token,char *md5,char *filename)
{
	int ret = 0;

	cJSON *root = cJSON_Parse(buf);
	if(NULL==root)
	{
		LOG(MD5_LOG_MODULE,MD5_LOG_PROC,"cJSON_Parse err\n");
		ret = -1;
		goto END;
	}

	//返回指定字符串对应的json对象
	cJSON * child1 = cJSON_GetObjectItem(root,"user");
	if(NULL==child1)
	{
	    LOG(MD5_LOG_MODULE, MD5_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
    }


    char tempuser[128] = {0};
    strcpy(tempuser,child1->valuestring);  //拷贝内容
	//对username进行加密
    ret = encode_message(tempuser,user);
	if(ret!=0)
	{
        LOG(MD5_LOG_PROC,MD5_LOG_PROC,"encode reg_username fail\n");
	    return;
	}


    //MD5
    cJSON * child2 = cJSON_GetObjectItem(root,"md5");
    if(NULL==child2)
    {

        LOG(MD5_LOG_MODULE, MD5_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
    }
    strcpy(md5, child2->valuestring); //拷贝内容

    //文件名字
    cJSON *child3 = cJSON_GetObjectItem(root, "fileName");
    if(NULL == child3)
    {
        LOG(MD5_LOG_MODULE, MD5_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
    }
    strcpy(filename, child3->valuestring); //拷贝内容

    //token
    cJSON *child4 = cJSON_GetObjectItem(root, "token");
    if(NULL == child4)
    {
        LOG(MD5_LOG_MODULE, MD5_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
    }

    strcpy(token, child4->valuestring); //拷贝内容
END:
    if(root!=NULL)
    {
    	cJSON_Delete(root);   //删除json对象
    	root = NULL;
    }
    return ret;
}


//秒传处理
int deal_md5(char * user,char * md5,char *filename)
{
    int ret = 0;
    MYSQL *conn = NULL;
    int ret2 = 0;
    char tmp[512] = {0};
    char sql_cmd[SQL_MAX_LEN] = {0};
    char * out = NULL;


    //连接数据库
    conn = msql_conn(mysql_user,mysql_pwd,mysql_db);
    if(conn==NULL)
    {
    	LOG(MD5_LOG_MODULE, MD5_LOG_PROC, "msql_conn err\n");
        ret = -1;
        goto END;
    }

    //设置数据库编码
    mysql_query(conn,"set names utf8");

    //获得md5值文件的文件计数值count
    sprintf(sql_cmd,"select count from file_info where md5='%s'",md5);
    ret2 = process_result_one(conn,sql_cmd,tmp);
    //说明服务器已经有这个文件
    if(ret2==0) 
    {
    	int count = atoi(tmp);

        sprintf(sql_cmd,"select * from user_file_list where user='%s' and md5='%s' and filename='%s'",user,md5,filename);
        ret2 = process_result_one(conn,sql_cmd,NULL);
        if(ret2==2)
        {
        	LOG(MD5_LOG_MODULE, MD5_LOG_PROC, "%s[filename:%s, md5:%s]已存在\n", user, filename, md5);
            ret = -2; //0秒传成功，-1出错，-2此用户已拥有此文件， -3秒传失败
            goto END;
        }
        
        //修改file_info中的count字段，+1
        sprintf(sql_cmd,"update file_info set count=%d where md5='%s'",++count,md5);
        if(mysql_query(conn, sql_cmd) != 0)
        {
            LOG(MD5_LOG_MODULE, MD5_LOG_PROC, "%s 操作失败： %s\n", sql_cmd, mysql_error(conn));
            ret = -1;
            goto END;
        }

        //user_file_list,用户文件列表中插入一条数据
        struct timeval tv;
        struct tm* ptm;
        char time_str[128];

        gettimeofday(&tv,NULL);
        ptm = localtime(&tv.tv_sec);
        strftime(time_str,sizeof(time_str),"%Y-%m-%d %H:%M:%S", ptm);
         

        //构造sql语句
        sprintf(sql_cmd, "insert into user_file_list(user, md5, createtime, filename, shared_status, pv) values ('%s', '%s', '%s', '%s', %d, %d)", user, md5, time_str, filename, 0, 0);
        if(mysql_query(conn, sql_cmd) != 0)
        {
            LOG(MD5_LOG_MODULE, MD5_LOG_PROC, "%s 操作失败： %s\n", sql_cmd, mysql_error(conn));
            ret = -1;
            goto END;
        }

        //查询用户文件数量
        sprintf(sql_cmd,"select count from user_file_count where user='%s'",user);
        count = 0;


        ret2 = process_result_one(conn,sql_cmd,tmp);
        if(ret2==1)  //没有记录
        {
           sprintf(sql_cmd,"insert into user_file_count (user,count) values('%s',%d)",user,1);
        }else if(ret2==0)
        {
        	//更新用户文件数量count字段
        	count = atoi(tmp);
            sprintf(sql_cmd,"update user_file_count set count=%d  where user = '%s'",count+1,user);
        }


        if(mysql_query(conn,sql_cmd)!=0)
        {
        	LOG(MD5_LOG_MODULE, MD5_LOG_PROC, "%s 操作失败： %s\n", sql_cmd, mysql_error(conn));
            ret = -1;
            goto END;
        }
    }else if(ret2==1)  //服务器中没有这个文件，秒传失败
    {
    	ret = -3;
        goto END;
    }
 END:
   if(ret==0)
   {
   	out = return_status("006");   //秒传成功
   }else if(ret==-2)
   {
   	out = return_status("005");   //用户文件已经存在
   }else
   {
   	out = return_status("007");  //秒传失败
   }

   if(out!=NULL)
   {
   	printf(out);  //给前端返回信息
   	free(out);    //释放out
   }

   if(conn!=NULL)
   {
   	mysql_close(conn);  //断开数据库连接
   }

   return ret;
}


//线程处理函数
void *deal_quickupload(void *arg)
{
   int len = 0;
   char *contentlength = (char *)arg;
   if(contentlength==NULL)
   { 
       len = 0;
   }else
   {
   	  len = atoi(contentlength);
   }

   if(len<=0)
   {
   	  LOG(MD5_LOG_MODULE,MD5_LOG_PROC,"len=0 no data received\n");
   }else   //获取登录的用户信息
   {
   	  char buf[1024*4] = {0};
   	  int ret = 0;
   	  ret = fread(buf,1,len,stdin);
   	  if(ret==0)
   	  {
   	  	LOG(MD5_LOG_MODULE,MD5_LOG_PROC,"fread(buf,1,len,stdin) error\n");
   	  	return;
   	  }

      LOG(MD5_LOG_MODULE, MD5_LOG_PROC, "buf = %s\n", buf);
       //解析json中信息
        /*
          {
            user:xxxx,
            token: xxxx,
            md5:xxx,
            fileName: xxx
           }
        */

      char user[128] = {0};
      char md5[256] = {0};
      char token[256] = {0};
      char filename[128] = {0};
      ret = get_md5_info(buf,user,token,md5,filename); //解析json中的数据
      if(ret!=0)
      {
      	LOG(MD5_LOG_MODULE,MD5_LOG_PROC,"get_md5_info() err\n");
      	return;
      }

      LOG(MD5_LOG_MODULE, MD5_LOG_PROC, "user = %s, token = %s, md5 = %s, filename = %s\n", user, token, md5, filename);



      //验证登录token,成功返回0 失败 -1
      ret = verify_token("user_token",user,token);
      if(ret==0)
      {
      	deal_md5(user,md5,filename);   //秒传处理
      }else
      {
      	char * out = return_status("111");  //token验证失败
      	if(out!=NULL)
      	{
      		printf(out);  //给前台反馈错误码
      		free(out);
      	}
      }


   }
   return;
}
     




int main()
{
	int ret = 0;
	//读取配置文件信息
	//read_cfg();

	while(FCGI_Accept()>=0)
	{
        read_cfg();
		char *contentlength = getenv("CONTENT_LENGTH");
        
        printf("Content-type: text/html\r\n\r\n");

        pthread_t  pid;
        ret = pthread_create(&pid,NULL,(void *)deal_quickupload,(void *)contentlength);

        if(ret!=0)
        {
         LOG(MD5_LOG_MODULE,MD5_LOG_PROC,"pthread_create fail\n");
         goto END; 
        }
    

        //回收线程
        pthread_join(pid,NULL);
    }

END:
	return ret;
}
