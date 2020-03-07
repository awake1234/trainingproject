/*************************************************************************
	> File Name: uploadcgi.c
	> Author: lp
	> Mail: lp86282176 
	> Created Time: 2019年12月25日 星期三 17时14分58秒
 ************************************************************************/
#include "deal_mysql.h"
#include "make_log.h"
#include "fileupload.h"
#include "fcgi_stdio.h"
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<stdio.h>
#include "util_cgi.h"   //cgi后台通用接口 
#include <pthread.h>
#include "cfg.h"

#define  UPLOAD_LOG_MODULE "cgi"
#define  UPLOAD_LOG_PROC  "upload"
//配置文件的路径
#define  CONFFILE   "../conf/client.conf"


//mysql 数据库的配置信息，用户名，密码，数据库名称
static char mysql_user[128]={0};
static char mysql_pwd[128] = {0};
static char mysql_db[128] = {0};

//读取配置信息
void read_cfg()
{
  get_cfg_value(CFG_PATH,"mysql","user",mysql_user);
  get_cfg_value(CFG_PATH,"mysql","password",mysql_pwd);
  get_cfg_value(CFG_PATH,"mysql","database",mysql_db);
  LOG(UPLOAD_LOG_MODULE,UPLOAD_LOG_PROC,"mysql:user=%s,pwd=%s,database=%s\n",mysql_user,mysql_pwd,mysql_db);
}


//解析post数据，保存到本地临时路径
int recvfile_save(long len,char *user,char *filename,char * md5,long *p_size)
{
    int ret = 0;
    char * filebuf =NULL;
    char * begin = NULL;
    char *p, *q,*k;
	char tempuser[128]={0};

    char content_text[TEMP_BUF_MAX_LEN] = {0};  //文件头部信息
    char boundary[TEMP_BUF_MAX_LEN]={0};   //分界线信息

    //开辟存放文件的内存
    filebuf = (char *)malloc(len);
    if(filebuf==NULL)
    {
      LOG(UPLOAD_LOG_MODULE,UPLOAD_LOG_PROC,"malloc file buf fail\n");
      return -1;
    }

    int ret2 = fread(filebuf,1,len,stdin);  //从标准输入读入数据
    if(ret2==0)
    {
      LOG(UPLOAD_LOG_MODULE,UPLOAD_LOG_PROC,"fread(filebuf,1,len,stdin) error\n");
      ret = -1;
      goto  END;
    }
	LOG(UPLOAD_LOG_MODULE,UPLOAD_LOG_PROC,"buf = %s\n",filebuf);


    //处理前端发来的post数据格式
    begin  = filebuf;
    p = begin;

    /*
       ------WebKitFormBoundary88asdgewtgewx\r\n
       Content-Disposition: form-data; user="lp"; filename="xxx.jpg"; md5="xxxx"; size=10240\r\n
       Content-Type: application/octet-stream\r\n
       \r\n
       真正的文件内容\r\n
       ------WebKitFormBoundary88asdgewtgewx
    */

    //得到分隔线
    p = strstr(begin,"\r\n");  //匹配到\r\n
    if(p==NULL)
    {
      LOG(UPLOAD_LOG_MODULE,UPLOAD_LOG_PROC,"no boundary\n");
      ret  = -1;
      goto END;
    }

    //拷贝分隔线
    strncpy(boundary,begin,p-begin);    
    boundary[p-begin] = '\0';   //加上字符串结束符
    LOG(UPLOAD_LOG_MODULE,UPLOAD_LOG_PROC,"boundary:[%s]\n",boundary);
     //移动到下一行
     p+=2;
     len-=(p-begin);  //更新要处理的数据长度

     begin =p;
     p = strstr(begin,"\r\n");
     if(p==NULL)
     {
      LOG(UPLOAD_LOG_MODULE,UPLOAD_LOG_PROC,"error:get content_text\n");
      ret = -1;
      goto END;
     }

     strncpy(content_text,begin,p-begin);
     content_text[p-begin] = '\0';
     LOG(UPLOAD_LOG_MODULE,UPLOAD_LOG_PROC,"content_text:[%s]\n",content_text);
     p+=2;
     len-=(p-begin);

      //获取相关的文件信息
      q = begin;
      q = strstr(begin,"user=");

      q+=strlen("user=");
      q++; //跳过第一个引号

      k=strchr(q,'"');   //找到下引号
      strncpy(tempuser,q,k-q);   //拷贝用户名
      tempuser[k-q]='\0'; 
      //去掉第一个字符串两边的空白字符
      LOG(UPLOAD_LOG_MODULE,UPLOAD_LOG_PROC,"trim_space user before\n");
      trim_space(tempuser);

	  //给username 进行加密处理
	  encode_message(tempuser,user);

      begin = k;
      q = begin;
      q = strstr(begin,"filename=");
      q+=strlen("filename=");
      q++;

      k = strchr(q,'"');
      strncpy(filename,q,k-q);
      filename[k-q]='\0';
      trim_space(filename);

      begin = k;
      q = begin;
      q = strstr(begin,"md5=");
      q+=strlen("md5=");
      q++;

      k = strchr(q,'"');
      strncpy(md5,q,k-q);
      md5[k-q]='\0';
  
      trim_space(md5);

      begin = k;
      q = begin;
      q = strstr(begin,"size=");
      q+=strlen("size=");

      k = strstr(q,"\r\n");
      char tmp[256]={0};
      strncpy(tmp,q,k-q);
      tmp[k-q]='\0';

      *p_size = strtol(tmp,NULL,10); // 字符串转LONG

      begin = p;
      p = strstr(begin,"\r\n");
      p+=4;   //跳过\r\n\r\n
      len-=(p-begin);
      //真正的文件内容
      begin = p;
      p = memstr(begin,len,boundary);  //找到文件结束的位置
      if(p==NULL)
      {
        LOG(UPLOAD_LOG_MODULE,UPLOAD_LOG_PROC,"memstr(begin,len,boundary) error\n");
        ret = -1;
        goto END;
      }else
      {
        p = p-2;
      }

	  LOG(UPLOAD_LOG_MODULE,UPLOAD_LOG_PROC,"trim_spcae user after\n");
      //保存文件
      int fd = 0;
      fd = open(filename,O_CREAT|O_WRONLY,0644);
      if(fd<0)
      {
        LOG(UPLOAD_LOG_MODULE,UPLOAD_LOG_PROC,"open %s error\n",filename);
        ret = -1;
        goto END;
      }
	  LOG(UPLOAD_LOG_MODULE,UPLOAD_LOG_PROC,"open file %s successfully\n",filename);
      
      //ftruncate会将参数fd指定的文件大小改为参数length指定的文件大小
      ftruncate(fd,(p-begin));
      write(fd,begin,(p-begin));
      close(fd);
END:
    free(filebuf);
    return ret;
}


//将本地文件上传到后台分布式文件系统中
int upload_to_storage(char * filename,char *fileid)
{
    int ret = 0;

    pid_t pid;
    int fd[2];

    //创建无名管道
    if(pipe(fd)<0)
    {
      LOG(UPLOAD_LOG_MODULE,UPLOAD_LOG_PROC,"pipe  error\n");
      ret = -1;
      goto END;
    }


    //创建进程
    pid = fork();
    if(pid<0)
    {
      LOG(UPLOAD_LOG_MODULE,UPLOAD_LOG_PROC,"fork error\n");
      ret = -1;
      goto END;
    }

   //子进程
   if(pid==0)
   {
       //关闭读端
      close(fd[0]);
      //将标准输出 重定向 写管道
      dup2(fd[1],STDOUT_FILENO);  //宏：STDOUT_FILENO是在unistd.h中定义

      LOG(UPLOAD_LOG_MODULE,UPLOAD_LOG_PROC,"dup2() success\n");    
      //读取fdfs_client的配置文件的路径
      char fdfs_cliconf_path[256] = {0};
      get_cfg_value(CFG_PATH,"dfs_path","client",fdfs_cliconf_path);

      //通过execlp在环境变量PATH找到 fdfs_upload_file 并执行，后面是参数argv[0],argv[1],最后一个参数必须是NULL
      execlp("fdfs_upload_file","fdfs_upload_file",fdfs_cliconf_path,filename,NULL); 
      //执行失败该函数才会返回，执行成功不会返回
      LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "execlp fdfs_upload_file error\n");
      close(fd[1]);
   }else
   {
       //关闭写端
       close(fd[1]);

       //从管道中读数据,当子进程执行文件上传之后 fastDfs 会返回一个fileid 这时候标准输出已经被重定向到管道中去了
       read(fd[0],fileid,TEMP_BUF_MAX_LEN);

       //去掉字符串两边的空白字符
       trim_space(fileid);

        if(strlen(fileid)==0)
        {
          LOG(UPLOAD_LOG_MODULE,UPLOAD_LOG_PROC,"upload failed\n");
          ret = -1;
          goto END;
        }
  
        LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "get [%s] succ!\n", fileid);
        wait(NULL);   //等待子进程结束，回收其资源
        close(fd[0]);
   }
END:
   return ret;
}


//生成文件的完整的URL
int make_file_url(char * fileid,char * fdfs_file_url)
{
    int ret = 0;

    char * p = NULL;
    char * q = NULL;
    char * k = NULL;

    char fdfs_file_stat_buf[TEMP_BUF_MAX_LEN] = {0};
    char fdfs_file_host_name[TEMP_BUF_MAX_LEN] = {0};

    pid_t pid;
    int fd[2];

    if(pipe(fd)<0)
    {
      LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "pip error\n");
      ret = -1;
      goto END;
   }

   pid = fork();
   if(pid==0)   //子进程
   {
    //关闭读端
    close(fd[0]);

    //标准输出重定向
    dup2(fd[1],STDOUT_FILENO);

    //读取配置文件信息
    char fdfs_cliconf_path[256] = {0};
    get_cfg_value(CFG_PATH,"dfs_path","client",fdfs_cliconf_path);

	LOG(UPLOAD_LOG_MODULE,UPLOAD_LOG_PROC,"client conf:%s\n",fdfs_cliconf_path);
    execlp("fdfs_file_info","fdfs_file_info",fdfs_cliconf_path,fileid,NULL);

    //执行失败
    LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "execlp fdfs_file_info error\n");

    close(fd[1]);
   }else  //父进程
   {
      //关闭写端
      close(fd[1]);

      read(fd[0],fdfs_file_stat_buf,TEMP_BUF_MAX_LEN);

      wait(NULL);
      close(fd[0]);

	  LOG(UPLOAD_LOG_MODULE,UPLOAD_LOG_PROC,"uploadfile stat:%s\n",fdfs_file_stat_buf);
      p = strstr(fdfs_file_stat_buf, "source ip address: ");
     

	  if(p==NULL)
	  {
         LOG(UPLOAD_LOG_MODULE,UPLOAD_LOG_PROC,"strstr ip address fail\n");
		 ret = -1;
		 return ret;
	  }
      q = p+strlen("source ip address:");
      k = strstr(q,"\n");


      strncpy(fdfs_file_host_name,q,k-q);
      fdfs_file_host_name[k-q] = '\0';

      //读取storage_web_server的服务器的端口
      char storage_web_server_port[20] = {0};
      get_cfg_value(CFG_PATH,"storage_web_server","port",storage_web_server_port);

      //拼接完整的url
      strcat(fdfs_file_url,"http://");
      strcat(fdfs_file_url,fdfs_file_host_name);
      strcat(fdfs_file_url,":");
      strcat(fdfs_file_url,storage_web_server_port);
      strcat(fdfs_file_url,"/");
      strcat(fdfs_file_url,fileid);

     LOG(UPLOAD_LOG_MODULE,UPLOAD_LOG_PROC,"file url is：%s\n",fdfs_file_url);
   
END:
    return ret;
   }

}

//将文件信息保存到数据库
int  storefileinfo_tomysql(char * user,char * filename,char *md5,long size,char * fileid,char * fdfs_file_url)
{
    int ret = 0;
    MYSQL * conn = NULL;  //数据库连接句柄

    time_t now;
    char create_time[TIME_STRING_LEN];
    char suffix[SUFFIX_LEN];
    char sql_cmd[SQL_MAX_LEN]={0};

    //连接mysql数据库
    conn = msql_conn(mysql_user,mysql_pwd,mysql_db);
    if(conn==NULL)
    {
       LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "msql_conn connect err\n");
       ret = -1;
       goto END;
    }

    //设置数据库编码
    mysql_query(conn,"set names utf8");

    //得到文件后缀字符串，如果非法文件后缀，返回“null"
    get_file_suffix(filename,suffix); 
     //sql 语句
       /*
       -- =============================================== 文件信息表
       -- md5 文件md5
       -- file_id 文件id
       -- url 文件url
       -- size 文件大小, 以字节为单位
       -- type 文件类型： png, zip, mp4……
       -- count 文件引用计数， 默认为1， 每增加一个用户拥有此文件，此计数器+1
       */
    sprintf(sql_cmd, "insert into file_info (md5, file_id, url, size, type, count) values ('%s', '%s', '%s', '%ld', '%s', %d)",
            md5, fileid, fdfs_file_url, size, suffix, 1);

    if(mysql_query(conn,sql_cmd)!=0)
    {
       LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "%s 插入失败: %s\n", sql_cmd, mysql_error(conn));
       ret = -1;
       goto END;
    }

    LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "%s 文件信息插入成功\n", sql_cmd);


    now = time(NULL); //获取当前时间
    //以字符串的形式保存到create_time中
    strftime(create_time,TIME_STRING_LEN-1,"%Y-%m-%d %H:%M:%S", localtime(&now));

   /*
    -- =============================================== 用户文件列表
       -- user 文件所属用户
       -- md5 文件md5
       -- createtime 文件创建时间
       -- filename 文件名字
       -- shared_status 共享状态, 0为没有共享， 1为共享
       -- pv 文件下载量，默认值为0，下载一次加1
  */
  sprintf(sql_cmd,"insert into user_file_list(user,md5,createtime,filename,shared_status,pv) values ('%s','%s','%s','%s',%d,%d)",user,md5,create_time,filename,0,0);
  if(mysql_query(conn, sql_cmd) != 0)
  {
        LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "%s 操作失败: %s\n", sql_cmd, mysql_error(conn));
        ret = -1;
        goto END;
  }

  //查询用户文件数量
  sprintf(sql_cmd,"select count from user_file_count where user = '%s'",user);
  int ret2 = 0;
  char tmp[512] = {0};
  int count = 0;
  //返回值： 0成功并保存记录集到tmp中，1没有记录集，2有记录集但是没有保存，-1失败
  ret2 = process_result_one(conn,sql_cmd,tmp);
  if(ret2==1)
  {
      //插入一条记录
      sprintf(sql_cmd,"insert into user_file_count(user,count) values('%s','%d')",user,1);
  }else if(ret2==0)
  {
    //更新用户文件数量 count字段
    count = atoi(tmp);
     sprintf(sql_cmd, "update user_file_count set count = %d where user = '%s'", count+1, user);
  }
  
  if(mysql_query(conn,sql_cmd)!=0)
  {
    LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "%s 操作失败: %s\n", sql_cmd, mysql_error(conn));
    ret = -1;
    goto END;
  }
END:
   if(conn!=NULL)
   {
    mysql_close(conn);  //断开数据库连接
   }
  
   return ret;
}  



  

//线程处理函数
void *deal_upload(void *arg)
{
   int ret = 0;
   char * contentlength  = (char *)arg;
   char filename[FILE_NAME_LEN] = {0};
   char user[USER_NAME_LEN] = {0};
   char md5[MD5_LEN] = {0};
   long size;  //文件的大小
   char fileid[TEMP_BUF_MAX_LEN] = {0};  //文件上传到fastdfs后的文件id
   char fdfs_file_url[FILE_URL_LEN] = {0};  //文件所存放的storage的host_name   
   long len;
   

   if(contentlength!=NULL)
   {
     len = strtol(contentlength,NULL,10);
   }else
   {
     len = 0;
   }

   if(len<=0)
   {
    printf("No data from standard input\n");
    LOG(UPLOAD_LOG_MODULE,UPLOAD_LOG_PROC,"len = 0,no data recvived\n");
    goto  END;
   }else
   {
	LOG(UPLOAD_LOG_MODULE,UPLOAD_LOG_PROC,"len = %ld\n",len);
    //得到上传文件
    if(recvfile_save(len,user,filename,md5,&size)<0)
    {
       ret = -1;
       goto  END;
    }

    LOG(UPLOAD_LOG_MODULE, UPLOAD_LOG_PROC, "%s成功上传[%s, 大小：%ld, md5码：%s]到本地\n", user, filename, size, md5);

     //将文件存入fastDFS中，并得到文件的file_id
    if(upload_to_storage(filename,fileid)<0)
    {
      ret = -1;
       goto END;
    }

    //删除本地临时保存的上传文件
    unlink(filename);

    //得到文件所存放的storage的host_name
    if(make_file_url(fileid,fdfs_file_url)<0)
    {
      ret = -1;
      goto END;
    }
    
    //将文件的fastDFS信息存入mysql中
    if(storefileinfo_tomysql(user,filename,md5,size,fileid,fdfs_file_url)<0)
    {
      ret = -1;
      goto END;
    }

    }
END:
     memset(filename, 0, FILE_NAME_LEN);
     memset(user, 0, USER_NAME_LEN);
     memset(md5, 0, MD5_LEN);
     memset(fileid, 0, TEMP_BUF_MAX_LEN);
     memset(fdfs_file_url, 0, FILE_URL_LEN);

     char * out = NULL;
     if(ret==0)   //上传成功
     {
        out = return_status("008");
     }else{
        out = return_status("009");
     }

     if(out!=NULL)
     {
        printf(out);  //给前端反馈信息
        free(out);
     }

     return;

}



int main()
{
 
 
   int ret = 0;
   //读文件配置
   read_cfg();



  //使用多线程
  while(FCGI_Accept()>=0)
  {
    char * contentlength  = getenv("CONTENT_LENGTH");
    
    printf("Content-type: text/html\r\n\r\n");


    pthread_t  pid;
    ret = pthread_create(&pid,NULL,(void *)deal_upload,(void *)contentlength);
    if(ret!=0)
    {
        LOG(UPLOAD_LOG_MODULE,UPLOAD_LOG_PROC,"pthread_create fail\n");
        goto END; 
    }
    

     //回收线程
    pthread_join(pid,NULL);
//	deal_upload((void *)contentlength);

  }

END:
   
     return ret;
}
