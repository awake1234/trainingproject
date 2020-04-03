#ifndef CONSUMERTHREAD_FILE_H
#define CONSUMERTHREAD_FILE_H

#include <QObject>
#include "uploadtask.h"
#include "logininfoinstance.h"
#include <QJsonDocument>
#include "common.h"
#include <QNetworkReply>
#include<QNetworkAccessManager>
#include<QEventLoop>



//用于处理真正的文件上传操作
class consumerthread_file : public QObject
{
    Q_OBJECT
public:
    explicit consumerthread_file(QObject *parent = nullptr);

    //处理秒传的操作
    void uploadfilesAction();


    //设置MD5信息的JSON包
    QByteArray setMD5Json(QString  user,QString token,QString md5,QString filename);
    //手动添加一个事件循环
    QEventLoop loop;


private:
    common m_common;
    QNetworkAccessManager * manager;
    uploadtask * uploadtask;
signals:
     void sig_loginAgain();  //重新登录信号
     void sig_uploadfinished();  //上传的任务全部完成的信号
     void clear_filelist();      //发送一个清理上传文件的信号
     void sig_progressvalue(qint64,qint64, dataprocess*);      //发送进度条的值的信号
     void sig_deleteprogress();    //删除进度条的信号


public slots:
     void refreshindex();
     //处理真正的文件上传的操作
     void uploadfileslow(uploadfileinfo *);

};

#endif // CONSUMERTHREAD_FILE_H
