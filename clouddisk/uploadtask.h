#ifndef UPLOADTASK_H
#define UPLOADTASK_H


#include<QString>
#include<QFile>
#include <QList>
#include "common.h"
#include "dataprocess.h"
#include "uploadlayout.h"
#include<QFileDialog>
#include <QMutex>
#include<QWaitCondition>


static  QMutex mutex;   //加锁文件列表资源
static QWaitCondition notempty;  //条件变量文件列表不为空
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





class uploadtask:public QObject
{
    Q_OBJECT
public:   

   //对外提供一个得到实例的接口
   static uploadtask *  get_uploadtask_instance();



   //上传的文件的列表
   QList<uploadfileinfo*> uploadfile_list;

   //判断上传列表是否为空
   bool isEmpty();

   //取任务
   uploadfileinfo * takeTask(int index);

   void clearlist();

   int appendtolist(QString filepath);

   //删除掉已经完成的任务
   void delete_uploadtask();

protected :

signals:





private:
    uploadtask();   //构造函数私有化
    //要在外面初始化
    static uploadtask * uploadtaskinstance;




};

#endif // UPLOADTASK_H
