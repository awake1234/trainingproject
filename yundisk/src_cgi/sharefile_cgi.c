//共享列表展示程序
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

#define SHAREFILES_LOG_MODULE "cgi"
#define SHAREFILES_LOG_PROC   "sharefiles"


//数据库的信息
static char mysql_user[128]={0};
static char mysql_pwd[128]={0};
static char mysql_db[128]={0};

//redis 服务器ip 端口
static char redis_ip[30]={0};
static char redis_port[10]={0};


void read_cfg()
{
	 //读取mysql数据库配置信息
    get_cfg_value(CFG_PATH, "mysql", "user", mysql_user);
    get_cfg_value(CFG_PATH, "mysql", "password", mysql_pwd);
    get_cfg_value(CFG_PATH, "mysql", "database", mysql_db);
    LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "mysql:[user=%s,pwd=%s,database=%s]", mysql_user, mysql_pwd, mysql_db);

    //读取redis配置信息
    get_cfg_value(CFG_PATH, "redis", "ip", redis_ip);
    get_cfg_value(CFG_PATH, "redis", "port", redis_port);
    LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "redis:[ip=%s,port=%s]\n", redis_ip, redis_port);
}

void get_share_files_count()
{
	char sql_cmd[SQL_MAX_LEN]={0};
	MYSQL *conn = NULL;
	long line = 0;   //保存得到的个数

    //连接数据库
    conn = msql_conn(mysql_user,mysql_pwd,mysql_db);
    if(conn==NULL)
    {
    	LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "msql_conn err\n");
        goto END;
    }

    //设置数据库编码，主要处理中文编码问题
    mysql_query(conn,"set names utf8");

    sprintf(sql_cmd,"select count from user_file_count where user='%s'","sharefile_account");
    char tmp[64]={0};
    int ret2 = process_result_one(conn,sql_cmd,tmp);
    if(ret2!=0)
    {  
    	LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "%s 操作失败\n", sql_cmd);
        goto END;
    }
   
     line = atol(tmp);

END:
    if(conn!=NULL)
    {
    	mysql_close(conn);
    } 
    LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "line = %ld\n", line);
    printf("%ld", line); //给前端反馈的信息
}


//解析json包
int  get_fileslist_json_info(char * buf,int *p_start,int *p_count)
{
	int ret = 0;


	cJSON * root = cJSON_Parse(buf);
	if(NULL==root)
	{
		LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "cJSON_Parse err\n");
        ret = -1;
        goto END;
	}


	//文件起点
	cJSON * child1 = cJSON_GetObjectItem(root,"start");
	if(NULL==child1)
	{
		LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
    }

    *p_start = child1->valueint;  //拷贝内容

    cJSON * child2 = cJSON_GetObjectItem(root,"count");
    if(NULL==child2)
    {
        LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "cJSON_GetObjectItem err\n");
        ret = -1;
        goto END;
    }

    *p_count = child2->valueint;

END:
    if(root!=NULL)
    {
    	cJSON_Delete(root);  //删除json对象
    	root=NULL;
    }

     return ret;
}


//获取共享文件列表
int get_share_filelist(int start,int count)
{
	int ret = 0;
	char sql_cmd[SQL_MAX_LEN]={0};
	MYSQL * conn = NULL;
	cJSON * root = NULL;
	cJSON * array = NULL;
	char * out = NULL;
	char * out2 = NULL;
	MYSQL_RES * res_set = NULL;


	//连接到数据库
	conn = msql_conn(mysql_user,mysql_pwd,mysql_db);
	if(conn==NULL)
	{
		LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "msql_conn err\n");
        ret = -1;
        goto END;
    }

    mysql_query(conn,"set names utf8");


    //sql语句
    sprintf(sql_cmd,"select share_file_list.*,file_info.url,file_info.size,file_info.type  from file_info,share_file_list where file_info.md5 = share_file_list.md5 limit %d,%d",start,count);
    
    LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "%s 在操作\n", sql_cmd);

    if(mysql_query(conn,sql_cmd)!=0)
    {
    	LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "%s 操作失败: %s\n", sql_cmd, mysql_error(conn));
        ret = -1;
        goto END;
    }


    res_set = mysql_store_result(conn);  //生成结果集
    if(res_set==NULL)
    {
    	LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "smysql_store_result error!\n");
        ret = -1;
        goto END;
    }


    ulong line = 0;

    //mysql_num_rows接收由mysql_store_result 返回结果集的行数
    line = mysql_num_rows(res_set); 
    if(line==0)
    {
    	LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "mysql_num_rows(res_set) failed\n");
        ret = -1;
        goto END;
    }

    MYSQL_ROW row;

    root = cJSON_CreateObject();
    array = cJSON_CreateArray();
    
    //当数据用完或发生错误时返回NULL
    while((row = mysql_fetch_row(res_set))!=NULL)
    {
        cJSON * item = cJSON_CreateObject();
    
        if(row[0]!=NULL)
        {
        	cJSON_AddStringToObject(item,"user",row[0]);
        }  
 
          //-- md5 文件md5
        if(row[1] != NULL)
        {
            cJSON_AddStringToObject(item, "md5", row[1]);
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
        cJSON_AddNumberToObject(item, "sharestatus", 1);


        //-- pv 文件下载量，默认值为0，下载一次加1
        if(row[4] != NULL)
        {
            cJSON_AddNumberToObject(item, "pv", atol( row[4] ));
        }

        //-- url 文件url
        if(row[5] != NULL)
        {
            cJSON_AddStringToObject(item, "url", row[5]);
        }

        //-- size 文件大小, 以字节为单位
        if(row[6] != NULL)
        {
            cJSON_AddNumberToObject(item, "size", atol( row[6] ));
        }

        //-- type 文件类型： png, zip, mp4……
        if(row[7] != NULL)
        {
            cJSON_AddStringToObject(item, "type", row[7]);
        }
    
		cJSON_AddItemToArray(array, item);
	}
        cJSON_AddItemToObject(root,"files",array);
   
        out = cJSON_Print(root);

        LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "%s\n", out);

END:
    if(ret==0)
    {
    	printf("%s",out);  //给前端返回信息
    }else{
    	out2 = NULL;
        out2 = return_status("015");
    }

    if(out2!=NULL)
    {
    	printf(out2);    //给前端返回错误码
        free(out2);
    }

    if(res_set!=NULL)
    {
    	mysql_free_result(res_set);
    }

    if(conn!=NULL)
    {
    	mysql_close(conn);
    }

    if(root!=NULL)
    {
    	cJSON_Delete(root);
    }

    if(out!=NULL)
    {
    	free(out);
    }
     return ret;
}



//获取共享文件按下载量排行榜
int get_ranking_filelist(int start,int count)
{
   //１.mysql共享文件数量和redis共享文件数量对比　判断是否相等,
   //2.如果不相等　清空redis数据,从mysql中的导入数据到redis中
   //3.从redis中读取数据，给前端反馈信息
   int ret = 0;
   char sql_cmd[SQL_MAX_LEN]={0};
   MYSQL * conn = NULL;
   cJSON * root = NULL;
   RVALUES value = NULL;
   cJSON * array = NULL;
   char * out = NULL;
   char * out2 = NULL;
   char tmp[512]={0};
   int ret2 = 0;
   MYSQL_RES * res_set = NULL;
   redisContext * redis_conn = NULL;

  //连接redis数据库
  redis_conn = rop_connectdb_nopwd(redis_ip,redis_port);
  if(redis_conn==NULL)
  {
	  LOG(SHAREFILES_LOG_MODULE,SHAREFILES_LOG_PROC,"redis_conn error\n");
	  ret = -1;
	  goto END;
  }

  //连接数据库
  conn = msql_conn(mysql_user,mysql_pwd,mysql_db);
  if(conn==NULL)
  {
	  LOG(SHAREFILES_LOG_MODULE,SHAREFILES_LOG_PROC,"msql_conn error\n");
	  ret = -1;
	  goto END;
  }

  //设置数库编码
  mysql_query(conn,"set names utf8");


  //1.查询mysql共享文件数量
  sprintf(sql_cmd,"select count from user_file_count where user='%s'","sharefile_account");
  ret2 = process_result_one(conn,sql_cmd,tmp);
  if(ret2!=0)
  {
	  LOG(SHAREFILES_LOG_MODULE,SHAREFILES_LOG_PROC,"%s操作失败\n",sql_cmd);
	  ret = -1;
	  goto END;
  }

  int sql_num = atoi(tmp);
   

  //redis共享文件数量
 int  redis_num = rop_zset_zcard(redis_conn,FILE_PUBLIC_ZSET);
 if(redis_num==-1)
 {
	 LOG(SHAREFILES_LOG_MODULE,SHAREFILES_LOG_PROC,"rop_zset_zcard error\n");
	 ret = -1;
	 goto END;
 }

 LOG(SHAREFILES_LOG_MODULE,SHAREFILES_LOG_PROC,"sql_num=%d,redis_num=%d\n",sql_num,redis_num);

 //如果两者不相等
 if(redis_num!=sql_num)
 {

	  //清空redis数据
	  rop_del_key(redis_conn,FILE_PUBLIC_ZSET);
	  rop_del_key(redis_conn,FILE_NAME_HASH);

	  //从mysql中导入数据到redis
	  strcpy(sql_cmd,"select md5,filename,pv from share_file_list order by pv desc");

	  LOG(SHAREFILES_LOG_MODULE,SHAREFILES_LOG_PROC,"%s在操作\n",sql_cmd);

	  if(mysql_query(conn,sql_cmd)!=0)
	  {
		  LOG(SHAREFILES_LOG_MODULE,SHAREFILES_LOG_PROC,"%s操作失败\n",sql_cmd);
	      ret = -1;
		  goto END;
	  }


	  res_set = mysql_store_result(conn);  //生成结果集
	  if(res_set==NULL)
	  {
		  LOG(SHAREFILES_LOG_MODULE,SHAREFILES_LOG_PROC,"mysql_store_result error\n");
		  ret = -1;
		  goto END;
	  }

	  ulong line = 0;
	  line = mysql_num_rows(res_set);  //得到结果集的行数
      if(line==0)
	  {
		  LOG(SHAREFILES_LOG_MODULE,SHAREFILES_LOG_PROC,"mysql_num_rows error\n");
		  ret = -1;
		  goto END;
	  }

	  MYSQL_ROW row;
	  while((row=mysql_fetch_row(res_set))!=NULL)
	  {
		  if(row[0]==NULL||row[1]==NULL||row[2]==NULL)
		  {
			  LOG(SHAREFILES_LOG_MODULE,SHAREFILES_LOG_PROC,"mysql_fetch_row failed\n");
			  ret = -1;
			  goto END;
		  }
	  }


	  char fileid[1024]={0};
	  sprintf(fileid,"%s%s",row[0],row[1]);    //redis中zset中的文件标示

	  rop_zset_add(redis_conn,FILE_PUBLIC_ZSET,atoi(row[2]),fileid); //下载量作为权重
      //增加hash记录
	  rop_hash_set(redis_conn,FILE_NAME_HASH,fileid,row[1]);
 }


     //从redis读取数据给前端反馈消息
	 value = (RVALUES)calloc(count,VALUES_ID_SIZE);  //堆区请求空间
     if(value==NULL)
	 {
		 ret = -1;
		 goto END;
	 }

	 int n = 0;
	 int end = start+count-1;  //加载资源结束位置
     //降序获得有序结合的元素
	 ret = rop_zset_zrevrange(redis_conn,FILE_PUBLIC_ZSET,start,end,value,&n);
	 if(ret!=0)
	 {
       LOG(SHAREFILES_LOG_MODULE,SHAREFILES_LOG_PROC,"rop_zset_zreverange error\n");
	   goto END;
	 }

	 root = cJSON_CreateObject();
	 array = cJSON_CreateArray();

	 int i;
	 for(i = 0;i<n;++i)
	 {
		 cJSON * item = cJSON_CreateObject();

		 char filename[1024]={0};
		 ret = rop_hash_get(redis_conn,FILE_NAME_HASH,value[i],filename);
		 if(ret!=0)
		 {
			 LOG(SHAREFILES_LOG_MODULE,SHAREFILES_LOG_PROC,"rop_hash_get error\n");
			 ret = -1;
			 goto END;
		 }

		 cJSON_AddStringToObject(item,"filename",filename);


		 //下载文件数
		 int score = rop_zset_get_score(redis_conn,FILE_PUBLIC_ZSET,value[i]);
		 if(score==-1)
		 {
            LOG(SHAREFILES_LOG_MODULE,SHAREFILES_LOG_PROC,"rop_zset_get_score error\n");
			ret = -1;
			goto END;
		 }

		 cJSON_AddNumberToObject(item,"pv",score);

		 cJSON_AddItemToArray(array,item);
	 }

	 cJSON_AddItemToObject(root,"files",array);
	 out = cJSON_Print(root);

	 LOG(SHAREFILES_LOG_MODULE,SHAREFILES_LOG_PROC,"out:%s\n",out);

END:
	 if(ret==0)
	 {
        printf("%s",out);
	 }else{
		 out2 = NULL;
		 out2 = return_status("015");
	 }

	 if(out2!=NULL)
	 {
		 printf(out2);
		 free(out2);
	 }

	 if(res_set!=NULL)
	 {
		 mysql_free_result(res_set);
	 }

	 if(redis_conn!=NULL)
	 {
		 rop_disconnect(redis_conn);
	 }
	 if(conn!=NULL)
	 {
		 mysql_close(conn);
	 }
	 if(value!=NULL)
	 {
		 free(value);
	 }
	 if(out!=NULL)
	 {
		 free(out);
	 }
	 if(root!=NULL)
	 {
		 cJSON_Delete(root);
	 }
   
	 return ret;
}

int main()
{

    char cmd[20];
    
    //读取配置信息
    read_cfg();

   while(FCGI_Accept()>=0)
   {
        char *query = getenv("QUERY_STRING");


        //解析命令
        query_parse_key_value(query,"cmd",cmd,NULL);
        LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "cmd = %s\n", cmd);

        printf("Content-type: text/html\r\n\r\n");

        
        if(strcmp(cmd,"count")==0)
        {
        	get_share_files_count(); //获取共享文件个数
        }else
        {
        	char * contentlength = getenv("CONTENT_LENGTH");
        	int len;

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
                LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "len = 0, No data from standard input\n");
            }else
            {
            	char buf[4*1024]={0};
            	int ret = 0;
            	ret = fread(buf,1,len,stdin);
                if(ret==0)
                {
                	LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "fread(buf, 1, len, stdin) err\n");
                    continue;
                }
               
                 LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "buf = %s\n", buf);

                 int start = 0;
                 int count = 0;

                 get_fileslist_json_info(buf,&start,&count);   //解析json包
                 LOG(SHAREFILES_LOG_MODULE, SHAREFILES_LOG_PROC, "start = %d, count = %d\n", start, count);
                

                 //普通视图
                 if(strcmp(cmd,"normal")==0)
                 {
                      get_share_filelist(start,count);
                 }else if(strcmp(cmd,"pvdesc")==0)   //按下载量降序排序
                 {
                      get_ranking_filelist(start,count);  //获取共享文件按下载量排行榜
                 }

            }

        }

   }




}
