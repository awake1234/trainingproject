#ifndef DOWNLOADTASK_H
#define DOWNLOADTASK_H

#include<QVBoxLayout>
#include<QUrl>
#include<QFile>
#include "dataprocess.h"
#include <QList>
#include "common.h"
#include "downloadlayout.h"


//下载文件信息
struct DownloadInfo
{
    QFile *file;        //文件指针
    QString user;       //下载用户
    QString filename;   //文件名字
    QString md5;        //文件md5
    QUrl url;           //下载网址
    bool isDownload;    //是否已经在下载
    dataprocess *dp;   //下载进度控件
    bool isShare;       //是否为共享文件下载
};


class DownloadTask
{
public:

    static DownloadTask * getInstance();

    void clearlist(); //请除上传列表
    bool isEmpty();  //判断上传队列是否为空

    DownloadInfo * takeTask();  //取出任务
    void delDownloadTask();  //删除下载完成的任务
    bool isdownload();        //是否有文件在下载
    bool isshared();   //判断是否是被分享的文件

    int appendDownloadList(FileInfo *info,QString filePath,bool isShare=false);


signals:



private:
    DownloadTask();

    QList<DownloadInfo *> list;
    static DownloadTask * instance;
};

#endif // DOWNLOADTASK_H
