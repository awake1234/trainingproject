//处理文件操作的cgi程序
#include "fcgi_config.h"
#include "fcgi_stdio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "make_log.h" //日志头文件
#include "util_cgi.h"
#include "deal_mysql.h"
#include "redis_keys.h"
#include "redis_op.h"
#include "cfg.h"
#include "cJSON.h"
#include <sys/time.h>
#include <unistd.h>
#define DEALFILE_LOG_MODULE "cgi"
#define DEALFILE_LOG_PROC  "dealfile"

//保存查询mysql的用信息等
static char mysql_user[128] = {0};
static char mysql_pwd[128] = {0};
static char mysql_db[128] = {0};


//保存redis的配置信息
static char redis_ip[30] ={0};
static char redis_port[10] ={0};

//读取配置文件信息
void read_cfg()
{
   get_cfg_value(CFG_PATH,"mysql","user",mysql_user);
   get_cfg_value(CFG_PATH,"mysql","password",mysql_pwd);
   get_cfg_value(CFG_PATH,"mysql","database",mysql_db);
   LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "mysql:[user=%s,pwd=%s,database=%s]", mysql_user, mysql_pwd, mysql_db);

    //读取redis配置信息
   get_cfg_value(CFG_PATH, "redis", "ip", redis_ip);
   get_cfg_value(CFG_PATH, "redis", "port", redis_port);
   LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "redis:[ip=%s,port=%s]\n", redis_ip, redis_port);
 }


//解析得到的json字符串
int get_json_info(char * buf,char *user,char * token,char * md5,char * filename)
{
	int ret = 0;


	//解析json包
	cJSON * root = cJSON_Parse(buf);
	if(NULL==root)
	{ 
		LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "cJSON_Parse err\n");
        ret = -1;
        goto END;
    }


    //用户名
    cJSON *  child1 = cJSON_GetObjectItem(root,"user");
    if(NULL == child1)
    {
        LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
    }

    char tempuser[128] = {0};

    strcpy(tempuser,child1->valuestring);  //拷贝内容
    
    //加密
    ret = encode_message(tempuser,user);
    if(ret!=0)
    {
        LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "encode username err\n");
        ret = -1;
        goto END;
    }

    LOG(DEALFILE_LOG_MODULE,DEALFILE_LOG_PROC,"user='%s'\n",user);
    //文件md5
    cJSON * child2 = cJSON_GetObjectItem(root,"token");
    if(child2==NULL)
    {
        LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
   }

    strcpy(token,child2->valuestring);
    LOG(DEALFILE_LOG_MODULE,DEALFILE_LOG_PROC,"token='%s'\n",token);

     //文件名字
    cJSON *child3 = cJSON_GetObjectItem(root, "filename");
    if(NULL == child3)
    {
        LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
    }
    
    strcpy(filename, child3->valuestring); //拷贝内容
     
    //md5
    cJSON *child4 = cJSON_GetObjectItem(root, "md5");
    if(NULL == child4)
    {
        LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
    }

    strcpy(md5, child4->valuestring); //拷贝内容

END:
    if(root != NULL)
    {
        cJSON_Delete(root);//删除json对象
        root = NULL;
    }

    return ret;

}

//分享文件的相关操作
int  share_file(char * user,char * md5,char * filename)
{
   
    int ret = 0;
    int  ret2 = 0;
    redisContext *redis_conn = NULL;
    MYSQL * conn = NULL;
	char * out = NULL;
    char fileid[256]={0};
    char sql_cmd[SQL_MAX_LEN]={0};
    char tmp[64] = {0};   //保存查询出来的结果
    //连接redis
    redis_conn = rop_connectdb_nopwd(redis_ip,redis_port);
    if(redis_conn==NULL)
    {    	
        LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "redis connected error");
        ret = -1;
        goto END;
    }


    //连接mysql数据库
    conn = msql_conn(mysql_user,mysql_pwd,mysql_db);
    if(conn==NULL)
    {
    	LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "msql_conn err\n");
        ret = -1;
        goto END;
    }

    //设置数据库编码问题，处理中文字符 
    mysql_query(conn,"set  names utf8");

    //文件标识，md5+文件名
    sprintf(fileid,"%s%s",md5,filename);
    
    //1.先判断文件是否已经分享，判断Redis的集合中有没有这个文件，如果有，说明别人已经分享过这个文件
    ret2 = rop_zset_exit(redis_conn,FILE_PUBLIC_ZSET,fileid);
    if(ret2==1) //存在
    {
    	LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "别人已经分享此文件\n");
        ret = -2;
        goto END;
    }
    else if(ret2==0)  //不存在
    {
    	//查看此文件别人是否已经分享过了，只是没有在redis中更新
    	sprintf(sql_cmd,"select * from share_file_list where md5='%s' and filename = '%s'",md5,filename);


    	ret2 = process_result_one(conn,sql_cmd,NULL); //执行sql语句
    	if(ret2==2)   //有结果，别人已经分享
    	{
           rop_zset_add(redis_conn,FILE_PUBLIC_ZSET,0,fileid);
           LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "别人已经分享此文件\n");
    	   ret = -2;
    	   goto  END;
    	}
    }else   
    {
        ret =-1;
        goto  END;
    }

    //如果此文件没有被分享,mysql中保存一份记录,更新user_file_list中文件的分享状态
    sprintf(sql_cmd,"update user_file_list set shared_status = 1 where user='%s' and md5 = '%s' and filename = '%s'",user,md5,filename);

    if(mysql_query(conn,sql_cmd)!=0)
    {  	
        LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "%s 操作失败: %s\n", sql_cmd, mysql_error(conn));
        ret = -1;
        goto END;
    }

    time_t now;
    char create_time[TIME_STRING_LEN];

    //获取当前时间
    now = time(NULL);
    strftime(create_time, TIME_STRING_LEN-1, "%Y-%m-%d %H:%M:%S", localtime(&now));


    //将分享的文件信息 保存到share_file_list中
    sprintf(sql_cmd,"insert into share_file_list(user,md5,createtime,filename,pv) values('%s','%s','%s','%s',%d)",user,md5,create_time,filename,0);
    if(mysql_query(conn,sql_cmd)!=0)
    {
    	LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "%s 操作失败: %s\n", sql_cmd, mysql_error(conn));
        ret = -1;
        goto END;
    }

    //查询共享文件数量,这里设置一个虚假的用户名，专门用来保存共享文件的数量
    sprintf(sql_cmd,"select count from user_file_count where user='%s'","sharefile_account");
    int  count = 0;

    ret2 = process_result_one(conn,sql_cmd,tmp);
    if(ret2==1)   //没有记录
    {
       sprintf(sql_cmd,"insert into user_file_count (user,count) values ('%s',%d)","sharefile_account",1);
    }else if(ret2==0)  //有记录
    {
      count = atoi(tmp);
      sprintf(sql_cmd,"update user_file_count set count = %d where user='%s'",count+1,"sharefile_account");
    }

    if(mysql_query(conn,sql_cmd)!=0)
    {
        LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "%s 操作失败: %s\n", sql_cmd, mysql_error(conn));
        ret = -1;
        goto END;
    }

    //redis集合中增加一个元素
    rop_zset_add(redis_conn,FILE_PUBLIC_ZSET,0,fileid);
    
    //redis对应的分享文件的hash也需要发生变化
    rop_hash_set(redis_conn,FILE_NAME_HASH,fileid,filename);

END:
    out = NULL;
    if(ret==0)
    {
    	out = return_status("010"); //成功
    }else if(ret==-1)
    {
    	out = return_status("011");  //失败
    }else if(ret==-2)
    {
    	out = return_status("012");  //别人已经分享
    }

    if (out!=NULL)
    {
    	printf(out); //向前端返回信息
    	free(out);
    }

    if(redis_conn!=NULL)
    {
    	rop_disconnect(redis_conn);
    }

    if(conn!=NULL)
    {
    	mysql_close(conn);
    }
    return ret;
}


int remove_file_from_storage(char * fileid)
{
	int ret = 0;

	//读取fdfs client的配置文件的路径
	char fdfs_cli_conf_path[256]={0};
	get_cfg_value(CFG_PATH,"dfs_path","client",fdfs_cli_conf_path);

	char cmd[1024*2]={0};
	sprintf(cmd,"/usr/bin/fdfs_delete_file %s %s",fdfs_cli_conf_path,fileid);
	ret = system(cmd);
   
	LOG(DEALFILE_LOG_MODULE,DEALFILE_LOG_PROC,"remove file from storage ret=%d\n",ret);
	return ret;
}




//删除文件
int del_file(char * user,char * md5,char * filename)
{
    //１.先判断此文件是否已经被分享
	//２.判断集合中是否有这个文件，如果有说明别人已经分享过此文件    //3.如果集合中没有此文件，可能是redis中没有记录，再从mysql中查询，如果mysql中没有记录，就是真的没有被分享
	//4.如果mysql中有记录,而redis中没有记录，只需要处理mysql中的记录
	//５.如果redis中有记录，redis和mysql都要处理
   
	int ret = 0;
	char sql_cmd[SQL_MAX_LEN] = {0};
	MYSQL * conn = NULL;
	redisContext * redis_conn = NULL;
	char * out = NULL;
	char tmp[512] = {0};
    char fileid[512]={0};
    int ret2 = 0;
	int count = 0;
	int share = 0;  //分享状态
    int flag = 0;   //表示redis中是否有记录


	//链接redis
    redis_conn = rop_connectdb_nopwd(redis_ip,redis_port);
	if(redis_conn==NULL)
	{
        LOG(DEALFILE_LOG_MODULE,DEALFILE_LOG_PROC,"redis connected error\n ");
        ret = -1;
		goto END;
	}

   //链接mysql
   conn = msql_conn(mysql_user,mysql_pwd,mysql_db);
   if(conn==NULL)
   {
	   LOG(DEALFILE_LOG_MODULE,DEALFILE_LOG_PROC,"msql_conn error\n");
       ret = -1;
	   goto END;
   }

   mysql_query(conn,"set names utf8");

   //文件标示　md5+文件名
   sprintf(fileid,"%s%s",md5,filename);


   //判断集合中有没有这个文件
   ret2 = rop_zset_exit(redis_conn,FILE_PUBLIC_ZSET,fileid);
   if(ret2==1)  //存在
   {
       share = 1;  //共享标志
	   flag = 1;   //redis中有记录
   }else if(ret2==0)
   {
      //在mysql中查询
	  sprintf(sql_cmd,"select shared_status from user_file_list where user='%s' and md5='%s' and filename = '%s'",user,md5,filename);
	  ret2 = process_result_one(conn,sql_cmd,tmp);
	  if(ret2==0)
	  {
		  share = atoi(tmp);  //shared_status字段
	  }else if(ret2==-1)  //失败
	  {
		  LOG(DEALFILE_LOG_MODULE,DEALFILE_LOG_PROC,"%s 操作失败\n",sql_cmd);
	      ret = -1;
		  goto END;
	  }
	   
   }else{//出错
	   ret = -1;
	   goto END;
   }
   //从文件已经被分享删除share_file_list的数据
   if(share==1)
   {
       //如果mysql中有记录删除相关分享记录
	   //删除在共享列表中的数据
	   sprintf(sql_cmd,"delete from share_file_list where user='%s' and md5='%s'and filename='%s'",user,md5,filename);
	   if(mysql_query(conn,sql_cmd)!=0)
	   {
          LOG(DEALFILE_LOG_MODULE,DEALFILE_LOG_PROC,"%s操作失败\n",sql_cmd);
          ret = -1;
		  goto END;
	   }

	   //共享文件数量-1
	   //查询共享文件的数量
	   sprintf(sql_cmd,"select count from user_file_count where user='%s'","sharefile_account");

	   ret2 = process_result_one(conn,sql_cmd,tmp);
	   if(ret2!=0)
	   {
		   LOG(DEALFILE_LOG_MODULE,DEALFILE_LOG_PROC,"%s操作失败\n",sql_cmd);
		   ret = -1;
		   goto END;
	   }

	   count = atoi(tmp);

	   //更新count数量
	   sprintf(sql_cmd,"update user_file_count set count=%d where user='%s'",count-1,"sharefile_account");

	   if(mysql_query(conn,sql_cmd)!=0)
	   {
		   LOG(DEALFILE_LOG_MODULE,DEALFILE_LOG_PROC,"%s操作失败\n",sql_cmd);
		   ret = -1;
		   goto END;
	   }


	   //如果redis中有记录，需要在redis中删除相关的记录
	   if(1==flag)
	   {
		   //有序集合中删除指定成员
		   rop_zset_zrem(redis_conn,FILE_PUBLIC_ZSET,fileid);
		   //从hash移除相关的记录
		   rop_hash_del(redis_conn,FILE_NAME_HASH,fileid);
	   }
   }

   //用户文件数量-1
   //查询用户数量文件
   sprintf(sql_cmd,"select count from user_file_count where user = '%s'",user);
   ret2 = process_result_one(conn,sql_cmd,tmp);
   if(ret2!=0)
   {
	   LOG(DEALFILE_LOG_MODULE,DEALFILE_LOG_PROC,"%s操作失败\n",sql_cmd);
	   ret=-1;
	   goto END;
   }

   count = atoi(tmp);
   if(count==1)
   {
     sprintf(sql_cmd,"delete from user_file_count where user='%s'",user);//删除用户拥有此文件的记录
   }else{
	   //否则更新用户的文件数量
	   sprintf(sql_cmd,"update user_file_count set count=%d where user='%s'",count-1,user);
   }

   if(mysql_query(conn,sql_cmd)!=0)
   {
	   LOG(DEALFILE_LOG_MODULE,DEALFILE_LOG_PROC,"%s操作失败\n",sql_cmd);
	   ret = -1;
	   goto END;
   }

   //删除用户文件列表数据
   sprintf(sql_cmd,"delete from user_file_list where user='%s'and md5='%s'and filename='%s'",user,md5,filename);

   if(mysql_query(conn,sql_cmd)!=0)
   {
	   LOG(DEALFILE_LOG_MODULE,DEALFILE_LOG_PROC,"%s操作失败\n",sql_cmd);
	   ret = -1;
	   goto END;
   }

   //文件信息表file_info的文件引用计数-1
   sprintf(sql_cmd,"select count from file_info where md5='%s'",md5);
   ret2 = process_result_one(conn,sql_cmd,tmp);  //执行sql语句
   if(ret2==0)
   {
	   count = atoi(tmp);
   }else{
	   LOG(DEALFILE_LOG_MODULE,DEALFILE_LOG_PROC,"%s操作失败\n");
	   ret = -1;
	   goto END;
   }

   count--;

   //更新操作
   sprintf(sql_cmd,"update file_info set count=%d where md5='%s'",count,md5);
   if(mysql_query(conn,sql_cmd)!=0)
   {
     LOG(DEALFILE_LOG_MODULE,DEALFILE_LOG_PROC,"%s操作失败\n",sql_cmd);
	 ret = -1;
	 goto END;
   }

   //删除之后没有人拥有这个文件了
   if(count==0)
   {
      sprintf(sql_cmd,"select file_id from file_info where md5='%s'",md5);
	  ret2 = process_result_one(conn,sql_cmd,tmp);
	  if(ret2!=0)
	  {
		  LOG(DEALFILE_LOG_MODULE,DEALFILE_LOG_PROC,"%s操作失败\n");
		  ret = -1;
		  goto END;
	  }

      //删除文件信息表中该文件的信息
	  sprintf(sql_cmd,"delete from file_info where md5='%s'",md5);
      if(mysql_query(conn,sql_cmd)!=0)
	  {
		  LOG(DEALFILE_LOG_MODULE,DEALFILE_LOG_PROC,"%s操作失败\n");
		  ret = -1;
		  goto END;
	  }

	  //从storage服务器中删除此文件
	  ret2 = remove_file_from_storage(tmp);
      if(ret2!=0)
	  {
		  LOG(DEALFILE_LOG_MODULE,DEALFILE_LOG_PROC,"remove file form storage error\n");
		  ret = -1;
		  goto END;
	  }
   }

END:
   out = NULL;
   if(ret==0)
   {
	   out = return_status("013");  //成功	   
   }else{
	   out = return_status("014");  //失败
   }
   
   if(out!=NULL)
   {
	   printf(out);
	   free(out);
   }

   if(redis_conn!=NULL)
   {
      rop_disconnect(redis_conn);
   }
   if(conn!=NULL)
   {
	   mysql_close(conn);
   }

   return ret;
}


int main()
{

   char cmd[20] = {0};
   char user[USER_NAME_LEN] = {0};
   char token[TOKEN_LEN] = {0};
   char md5[MD5_LEN] = {0};
   char filename[FILE_NAME_LEN]={0};
   char *contentlength = NULL;
   int len;

   //读取配置信息
   read_cfg();

   while(FCGI_Accept()>=0)
   {
      //获取url参数的内容
   	  char * query = getenv("QUERY_STRING");

      query_parse_key_value(query,"cmd",cmd,NULL);  //util_cgi.h
      LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "cmd = %s\n", cmd);

   	  contentlength = getenv("CONTENT_LENGTH");
   	  
      printf("Content-type: text/html\r\n\r\n");

   	  if(contentlength==NULL)
   	  {
   	  	 len = 0;
   	  }else
   	  {
   	  	len = atoi(contentlength);
   	  }

      if(len<=0)
      {
        printf("No data from standard input.<p>\n");
        LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "len = 0, No data from standard input\n");
      }else
      {
           char buf[4*1024] = {0};
           int ret = 0;
           ret = fread(buf,1,len,stdin);
           if(ret==0)
           {
           	 LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "fread(buf, 1, len, stdin) err\n");
             continue;
           }

           LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "buf = %s\n", buf);

           //解析数据
           get_json_info(buf,user,token,md5,filename);
           LOG(DEALFILE_LOG_MODULE, DEALFILE_LOG_PROC, "user = %s, token = %s, md5 = %s, filename = %s\n", user, token, md5, filename);
      
           //验证token
           ret = verify_token("user_token",user,token);
           //验证失败
           if(ret!=0)
           {
           	  char * out =  return_status("111"); 
              if(out!=NULL)
              {
              	printf(out);  //给前端返回错误码
                free(out);
              }
              continue;
           }

           //文件分享
           if(strcmp(cmd,"share")==0)
           {
               share_file(user,md5,filename);
           }else if(strcmp(cmd,"del")==0)    //删除文件
		   {
              del_file(user,md5,filename);
		   }

      }



   }
   

    return 0;

}
