#include "consumerthread_file.h"
#include <synchapi.h>
extern QMutex mutex;   //加锁文件列表资源
extern QWaitCondition notempty;  //条件变量文件列表不为空

static int index = 0;  //定义一个变量用来记录已经被处理的文件的下标
static QMutex mutex_index;     //用来你锁住下标，一次只能允许一个线程修改

consumerthread_file:: consumerthread_file(QObject *parent) : QObject(parent)
{
    uploadtask = uploadtask::get_uploadtask_instance();

  //  connect(this,SIGNAL(clear_filelist()),this,SLOT(refreshindex()));



}


//文件上传的真正的操作
void consumerthread_file::uploadfilesAction()
{

while(1){

    mutex.lock();

    //当上传队列为空或者没有任务可取时阻塞等待
    while(uploadtask->isEmpty())
    {
        //阻塞等待
        qDebug()<<"consumer thread is waiting because listsize="<<uploadtask->uploadfile_list.size();
        notempty.wait(&mutex);
    }

    //如果所有的认为已经处理完成
    while(index==uploadtask->uploadfile_list.size())
    {
        qDebug()<<"all task has been uploaded";
        emit clear_filelist();
        notempty.wait(&mutex);
        continue;
    }

    qDebug()<<"uploadlist size:"<<uploadtask->uploadfile_list.size();

    qDebug()<<"thread has been waken up";

    qDebug()<<"index= "<<index;


    //处理任务
    uploadfileinfo *uploadfileinfo = uploadtask->takeTask(index);
    qDebug()<<"consumer thread has taketask"<<index;
    mutex.unlock();

    //对index进行加锁
    mutex_index.lock();
    index++;    //将下标加一 供下一个线程读取
    mutex_index.unlock();

    qDebug()<<"current index = "<<index;

    //获取保存用户登录信息的实例
    logininfoinstance *login = logininfoinstance::getinstance();
    QByteArray filejson = setMD5Json(login->getuser(),login->gettoken(),uploadfileinfo->md5,uploadfileinfo->filename);

    //创建一个新的发送的manager
    manager = new QNetworkAccessManager();

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader,filejson.size());  //设置长度

    QString url = QString("http://%1:%2/md5").arg(login->getip()).arg(login->getport());
    qDebug()<<"md5 url is"<<url;
    request.setUrl(url);



    QNetworkReply * reply = manager->post(request,filejson);
    qDebug()<<"manager has post filejson:"<<filejson;


    connect(reply,&QNetworkReply::readyRead,[=]()
    {
       QByteArray data = reply->readAll();
       qDebug()<<"reply read"<<data;

       QString status = m_common.getcode(data);
       if("007"==status)
       {
         qDebug()<<uploadfileinfo->filename<<"秒传失败";
         //调用真正的上传文件的函数
         this->uploadfileslow(uploadfileinfo);
       }else if("006"==status)
       {
         qDebug()<<uploadfileinfo->filename<<"秒传成功";
         //发送信号删除进度条
         emit sig_deleteprogress();

       }else if("005"==status)
       {
          qDebug()<<uploadfileinfo->filename<<"文件已经存在";
          //发送信号删除进度条
          emit sig_deleteprogress();
       }else {
           //发送一个信号
          emit sig_loginAgain();
       }
        reply->deleteLater();

         //退出事件循环
         loop.exit();
     });
         loop.exec();

}

}

//真正的文件上传操作
void consumerthread_file::uploadfileslow(uploadfileinfo *info)
{

    logininfoinstance * login = logininfoinstance::getinstance();
    QString boundry = m_common.getBoundry();

    //组织要发送的数据
    QByteArray data;

    data.append(boundry);
    data.append("\r\n");

    //添加一些要传输的信息
    data.append("Content-Disposition:form-data;");
    data.append(QString("user=\"%1\"").arg(login->getuser()));
    data.append(QString("filename=\"%1\"").arg(info->filename));
    data.append(QString("md5=\"%1\"").arg(info->md5));
    data.append(QString("size=%1").arg(info->filesize));
    data.append("\r\n");

    data.append("Content-Type: multipart/form-data");
    data.append("\r\n");
    data.append("\r\n");

    //这里是真正的文件的内容
    data.append(info->filepoint->readAll());
    data.append("\r\n");

    data.append(boundry);

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader,"multipart/form-data"); //传输文件要使用form-data格式
    QString url = QString("http://%1:%2/upload").arg(login->getip()).arg(login->getport());
    request.setUrl(QUrl(url));

    qDebug()<<"send data size"<<data.size();
    qDebug()<<"send data:"<<data.data();          //输出发送的信息
    QNetworkReply *reply  = manager->post(request,data);


    //返回一些上传过程中的传输的字节大小
    connect(reply,&QNetworkReply::uploadProgress,[=](qint64 bytesent,qint64 byteTotal)
    {
        if(byteTotal!=0)
        {
            emit sig_progressvalue(bytesent,byteTotal,info->dp);
            qDebug()<<"emit sig_progressvalue";
        }
     });

    connect(reply,&QNetworkReply::readyRead,[=]()
    {
        if(reply->error()!=QNetworkReply::NoError)
        {
            qDebug()<<reply->errorString();
            reply->deleteLater();
            return;
        }

        QByteArray data = reply->readAll();
        reply->deleteLater();


        if("008"==m_common.getcode(data))
        {
            qDebug()<<info->filename<<"上传成功";
            m_common.writeRecord(login->getuser(),info->filename,"008");
        }if("009" == m_common.getcode(data) )
        {
            qDebug()<<info->filename<<"上传失败";
            m_common.writeRecord(login->getuser(),info->filename,"009");

        }


        uploadtask=uploadtask::get_uploadtask_instance();
        if(uploadtask==nullptr)
        {
            qDebug()<<"get uploadtask fail";
            return;
        }
        //发送信号删除进度条
        emit sig_deleteprogress();
        loop1.exit();
    });
    loop1.exec();
}



//打包MD5的json包
QByteArray consumerthread_file::setMD5Json(QString user, QString token, QString md5, QString filename)
{
    QMap<QString,QVariant> filejson;
    filejson.insert("user",user);
    filejson.insert("token",token);
    filejson.insert("md5",md5);
    filejson.insert("filename",filename);

    QJsonDocument doc = QJsonDocument::fromVariant(filejson);
    return doc.toJson();
}

//将index 重新设为0，清除上传列表
void consumerthread_file::refreshindex()
{
    mutex_index.lock();
    //uploadtask->clearlist();
    //qDebug()<<"all task has been cleared";
    index=0;
    mutex_index.unlock();

}



