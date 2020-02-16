#ifndef UPLOADTASK_H
#define UPLOADTASK_H


#include<QString>
#include<QFile>
#include <QList>
#include "common.h"
#include "dataprocess.h"
#include "uploadlayout.h"
#include<QFileDialog>

//上传文件任务的队列的类，这是一个单例模式的类，保证整个程序只有这一个实例

//上传文件的一些信息
struct  uploadfileinfo
{
    QString md5;  //上传文件的md5码
    QString filepath; //文件路径
    qint64  filesize;  //文件的大小
    QFile * filepoint;      //文件指针
    bool   isupload;   //文件是否在上传
    QString filename;  //文件的名称
    dataprocess *dp;   //上传进度控件
};





class uploadtask
{
public:

   //对外提供一个得到实例的接口
   static uploadtask *  get_uploadtask_instance();

   //将文件添加到uploadfilelist中
   int appendtolist(QString filepath);



private:
    uploadtask();   //构造函数私有化

    common m_common;

    //要在外面初始化
    static uploadtask * uploadtaskinstance;

    //上传的文件的列表
    QList<uploadfileinfo*> uploadfile_list;

};

#endif // UPLOADTASK_H
