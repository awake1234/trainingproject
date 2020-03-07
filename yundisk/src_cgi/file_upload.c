/*************************************************************************
	> File Name: file_upload.c
	> Author: lp
	> Mail: lp86282176 
	> Created Time: 2019年12月10日 星期二 20时15分46秒
 ************************************************************************/
#include<errno.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/types.h>
#include "logger.h"
#include<stdio.h>
#include "fileupload.h"
#include<string.h>
//#include "fileupload.h"
#include "fdfs_client.h"

//文件上传api
int fileupload(const char *conffilename,const char *filename,char * file_id)
{

    char group_name[FDFS_GROUP_NAME_MAX_LEN + 1];
	ConnectionInfo * pTrackerServer;
	int result;
	int store_path_index;
	ConnectionInfo storageServer;

    //初始化日志
	log_init();
    g_log_context.log_level = LOG_ERR;
    ignore_signal_pipe();
    
	//初始化client失败
	if((result=fdfs_client_init(conffilename))!=0)
	{
        return result;
	}

	//连接trackerserver失败
	pTrackerServer = tracker_get_connection();
	if(pTrackerServer == NULL)
	{
         fdfs_client_destroy();
		 return errno!=0?errno:ECONNREFUSED;
    }

     *group_name = '\0';

	//tracker查询存储结点
    if ((result=tracker_query_storage_store(pTrackerServer, \
                 &storageServer, group_name, &store_path_index)) != 0)
	 {
               fdfs_client_destroy();
			   fprintf(stderr, "tracker_query_storage fail, " \
					   "error no: %d, error info: %s\n", \
					   result, STRERROR(result));
		       return result;
	 }


    //根据文件名来存储文件（猜测）	
	result = storage_upload_by_filename1(pTrackerServer, \
			    &storageServer, store_path_index, \
				filename, NULL, \
	         NULL, 0, group_name, file_id);
	//执行成功
	if(result==0)
	{
		 //将fileid打印出来
		 printf("%s\n",file_id);
	}else
	{

		 fprintf(stderr, "upload file fail, " \
				 "error no: %d, error info: %s\n", \
				 result, STRERROR(result));
	}

	tracker_disconnect_server_ex(pTrackerServer, true);
    fdfs_client_destroy();

    return result;

}


