#include "sharefilelist.h"
#include "ui_sharefilelist.h"

sharefilelist::sharefilelist(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::sharefilelist)
{
    ui->setupUi(this);

    m_manager = m_common.getmanager();
    //初始化listwidget的格式
    initlistwidget();
    //添加右键按钮
    AddMenuAction();

    m_downloadTimer.start(500);   //0.5秒检查一次

    connect(&m_downloadTimer,&QTimer::timeout,[=]()
    {
       downloadFilesAction();
    });
}

sharefilelist::~sharefilelist()
{
    delete ui;
}

void sharefilelist::initlistwidget()
{

    ui->listWidget->setViewMode(QListView::IconMode);
    ui->listWidget->setIconSize(QSize(80,80));
    ui->listWidget->setGridSize(QSize(100,120));

    ui->listWidget->setResizeMode(QListView::Adjust);
    ui->listWidget->setMovement(QListView::Static);  //设置为不可移动

    ui->listWidget->setSpacing(30);  //设置间隔
    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);  //CustomContextMenu时会发出信号 customContextMenuRequested()。通过该信号，可以获得当前鼠标的位置

    //获得触发时鼠标的位置
    connect(ui->listWidget,&QListWidget::customContextMenuRequested,[=](const QPoint pos)
    {
        QListWidgetItem * item =ui->listWidget->itemAt(pos);

        //空白处
        if(item==nullptr)
        {
           m_menuEmpty->exec(QCursor::pos());  //在鼠标所在的位置显示
        }else{
            ui->listWidget->setCurrentItem(item);
            m_menuItem->exec(QCursor::pos());
        }

    });


}

//添加右键菜单的按钮
void sharefilelist::AddMenuAction()
{
    m_menuItem = new mymenu(this);
    m_propertyAction = new QAction("属性",this);
    m_cancleAction = new QAction("取消分享",this);
    m_saveAction = new QAction("转存文件",this);
    m_downloadAction = new QAction("下载",this);

    //添加到菜单上
    m_menuItem->addAction(m_downloadAction);
    m_menuItem->addAction(m_saveAction);
    m_menuItem->addAction(m_cancleAction);
    m_menuItem->addAction(m_propertyAction);


    m_menuEmpty = new mymenu(this);
    m_refreshAction = new QAction("刷新",this);
    m_menuEmpty->addAction(m_refreshAction);


    //刷新按钮
    connect(m_refreshAction,&QAction::triggered,[=]()
    {
        refreshFiles();   //刷新列表
    });


    //属性按钮
    connect(m_propertyAction,&QAction::triggered,[=]()
    {
       this->DealSelectedFile(property);
    });

    //取消分享
    connect(m_cancleAction,&QAction::triggered,[=]()
    {
       this->DealSelectedFile(cancel);
    });

    //转存
    connect(m_saveAction,&QAction::triggered,[=]()
    {
        this->DealSelectedFile(save);
    });


    //下载
    connect(m_downloadAction,&QAction::triggered,[=]()
    {
        this->DealSelectedFile(sharefilesdownload);
    });

}

//清除所有的items
void sharefilelist::clearItems()
{
    int n = ui->listWidget->count();

    for(int i = 0;i<n;i++)
    {
        //takeItem(index) 删除某个位置的item,返回这个item的地址，但是需要手动delete删除
        QListWidgetItem * item = ui->listWidget->takeItem(0);
        delete  item;
    }

}

//刷新items
void sharefilelist::refreshItems()
{
    clearItems();

    for(int i=0;i<m_shareFileList.size();i++)
    {
        ui->listWidget->addItem(m_shareFileList.at(i)->item);
    }

}

//刷新文件
void sharefilelist::refreshFiles()
{
    logininfoinstance *instance = logininfoinstance::getinstance();

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QString url = QString("http://%1:%2/sharefiles?cmd=count").arg(instance->getip()).arg(instance->getport());
    request.setUrl(QUrl(url));

    QNetworkReply *reply = m_manager->post(request,"");
    connect(reply,&QNetworkReply::readyRead,[=]()
    {
        QByteArray data = reply->readAll();

        reply->deleteLater();

        m_userFileCount = data.toInt();

        clearshareFileList();

        if(m_userFileCount>0)
        {
            m_start = 0;
            m_count = 10;

            getuserFileList(); //请求用户的文件列表
        }


    });




}

//清除共享文件列表
void sharefilelist::clearshareFileList()
{
    int n = m_shareFileList.size();

    for(int i = 0;i<n;i++)
    {

        FileInfo * temp = m_shareFileList.takeAt(0);
        delete  temp;
    }
}


//获得用户的共享文件列表
void sharefilelist::getuserFileList()
{
    if(m_userFileCount==0)
    {
        qDebug()<<"获取共享文件列表结束";
        refreshItems();
        return;
    }


    if(m_userFileCount<m_count)
    {
        m_count = m_userFileCount;
    }

    logininfoinstance *login = logininfoinstance::getinstance();
    QByteArray countjson = SetShaerfilejson(m_start,m_count);

    m_start+=m_count;
    m_userFileCount-=m_count;

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader,countjson.size());


    QString url = QString("http://%1:%2/sharefiles?cmd=normal").arg(login->getip()).arg(login->getport());
    request.setUrl(url);



    QNetworkReply *reply = m_manager->post(request,countjson);
    if(reply==nullptr)
    {
        qDebug()<<"reply is nullptr";
        return;
    }

    connect(reply,&QNetworkReply::readyRead,[=](){

        QByteArray data = reply->readAll();

        reply->deleteLater();

        if("015"!=m_common.getcode(data))
        {
            getfilejsoninfo(data);

            //继续获得共享列表
            getuserFileList();
        }else{
            qDebug()<<"共享列表获取失败";
            return;
        }

    });

}

void sharefilelist::getfilejsoninfo(QByteArray data)
{
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if(doc.isObject())
    {
        QJsonObject obj = doc.object();
        QJsonArray fileArray = obj.value("files").toArray();
        for(int i = 0;i<fileArray.size();i++)
        {
            FileInfo * fileinfo = new FileInfo;

            fileinfo->user=fileArray[i].toObject().value("user").toString();
            fileinfo->md5=fileArray[i].toObject().value("md5").toString();
            fileinfo->time=fileArray[i].toObject().value("time").toString();
            fileinfo->filename=fileArray[i].toObject().value("filename").toString();
            fileinfo->shareStatus=fileArray[i].toObject().value("sharestatus").toInt();
            fileinfo->pv=fileArray[i].toObject().value("pv").toInt();
            fileinfo->url=fileArray[i].toObject().value("url").toString();
            fileinfo->size=qint64(fileArray[i].toObject().value("size").toDouble());
            fileinfo->type=fileArray[i].toObject().value("type").toString();

            QString filepath = m_common.getFileType(fileinfo->type+".png");
            fileinfo->item = new QListWidgetItem(QIcon(filepath),fileinfo->filename);

            m_shareFileList.append(fileinfo);
        }

    }


}

//鼠标右建文件的具体操作
void sharefilelist::DealSelectedFile(sharefilelist::CMD cmd)
{
     //获取当前的item
    QListWidgetItem * curitem = ui->listWidget->currentItem();
    FileInfo * temp = NULL;

    for(int i = 0;i<m_shareFileList.size();i++)
    {
        if(m_shareFileList.at(i)->item==curitem)
        {
            temp = m_shareFileList.at(i);
            break;
        }
    }

    //属性
    if(cmd==property)
    {
        showproperty(temp);
    }
    //取消分享
    if(cmd==cancel)
    {
        cancelshare(temp);
    }

    if(cmd==save)
    {
        savefiletolist(temp);
    }

    if(cmd==sharefilesdownload)
    {
        adddownloadFiles(temp);
    }

}


//显示选中的文件的属性
void sharefilelist::showproperty(FileInfo *info)
{
    fileproperty * filepro  = new fileproperty();
    filepro->setfileinfo(info);
    filepro->show();
    return;
}


//取消分享
void sharefilelist::cancelshare(FileInfo *info)
{
    logininfoinstance * login  = logininfoinstance::getinstance();
    //别人分享的文件无法取消分享
    //将加密的用户名解密

    QByteArray data = SetCancelfilejson(login->getuser(),info->md5,info->filename);

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader,data.size());

    QString url = QString("http://%1:%2/dealsharefile?cmd=cancel").arg(login->getip()).arg(login->getport());
    request.setUrl(QUrl(url));

    QNetworkReply * reply = m_manager->post(request,data);


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

        if("018"==m_common.getcode(data))
        {
            QMessageBox::information(this,"操作成功","此文件已取消分享");

            for(int i=0;i<m_shareFileList.size();i++)
            {
                if(m_shareFileList.at(i)->md5==info->md5 &&m_shareFileList.at(i)->filename==info->filename)
                {
                    FileInfo * tempinfo = m_shareFileList.takeAt(i);
                    QListWidgetItem * item = tempinfo->item;
                    ui->listWidget->removeItemWidget(item);
                    delete  item;
                    delete  info;
                    break;
                }

            }
         }else if("019"==m_common.getcode(data))
        {
            QMessageBox::warning(this,"操作失败","取消分享文件操作失败");
        }else if("020"==m_common.getcode(data))
        {
            QMessageBox::warning(this,"操作失败","该文件不属于当前用户");
        }


    });

    return;
}

//转存
void sharefilelist::savefiletolist(FileInfo *info)
{
    logininfoinstance * login  = logininfoinstance::getinstance();

     QByteArray data = SetCancelfilejson(login->getuser(),info->md5,info->filename);
     QNetworkRequest request;
     request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
     request.setHeader(QNetworkRequest::ContentLengthHeader,data.size());

     QString url = QString("http://%1:%2/dealsharefile?cmd=save").arg(login->getip()).arg(login->getport());
     request.setUrl(QUrl(url));

     QNetworkReply * reply = m_manager->post(request,data);
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

         if("020"==m_common.getcode(data))
         {
            QMessageBox::information(this,"操作成功","此文件转存成功");
         }else if("021"==m_common.getcode(data))
         {
             QMessageBox::information(this,"操作失败","此文件已经在您的列表中");
         }else if ("022"==m_common.getcode(data)) {
             QMessageBox::warning(this,"操作失败","转存文件操作失败");
         }
     });
     return;
}


//添加下载任务到队列中去
void sharefilelist::adddownloadFiles(FileInfo *info)
{
    QString path;
    emit gototransfer(download);

    DownloadTask * downloadtask = DownloadTask::getInstance();

    path = QFileDialog::getSaveFileName(this,tr("select one dirctory to save"),info->filename);

    downloadtask->appendDownloadList(info,path,true);  //这里要将分享的标志设为true

    return;

}



void sharefilelist::downloadFilesAction()
{

    DownloadTask * downloadtask = DownloadTask::getInstance();
    if(downloadtask==nullptr)
    {
        qDebug()<<"downloadtask get instance fail";
        return;
    }

    if(downloadtask->isEmpty())
    {
        return;
    }

    if(downloadtask->isdownload())
    {
        return;
    }

    //不是被分享的文件返回
    if(!downloadtask->isshared())
    {
        return;
    }


    DownloadInfo * downloadfileinfo = downloadtask->takeTask();


    logininfoinstance *login = logininfoinstance::getinstance();


    QNetworkReply *reply = m_manager->get(QNetworkRequest(downloadfileinfo->url));

    connect(reply,&QNetworkReply::finished,[=]()
                {
                    QByteArray data=reply->readAll();

                    downloadfileinfo->file->write(data);

                    qDebug()<<downloadfileinfo->filename<<"下载成功"<<endl;
                    m_common.writeRecord(login->getuser(), downloadfileinfo->filename, "010");

                    dealFilePv(downloadfileinfo->md5,downloadfileinfo->filename);

                    downloadtask->delDownloadTask();

                    reply->deleteLater();
                });

        connect(reply,&QNetworkReply::downloadProgress,[=](qint64 bytesReceived, qint64 bytesTotal)
                {
                    if(bytesTotal!=0)
                        downloadfileinfo->dp->setprogress(bytesReceived/1024,bytesTotal/1024);

                });

}


//处理服务器数据库的相关参数的操作
void sharefilelist::dealFilePv(QString md5, QString filename)
{
    logininfoinstance * login = logininfoinstance::getinstance();

    QByteArray data = SetCancelfilejson(login->getuser(),md5,filename);

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    request.setHeader(QNetworkRequest::QNetworkRequest::ContentLengthHeader,data.size());

    QString url=QString("http://%1:%2/dealsharefile?cmd=pv").arg(login->getip()).arg(login->getport());
    request.setUrl(QUrl(url));

    QNetworkReply *reply=m_manager->post(request,data);

    connect(reply,&QNetworkReply::readyRead,[=]()
               {

                   if(reply->error()!=QNetworkReply::NoError)
                   {
                       qDebug()<<reply->errorString();
                       reply->deleteLater();
                       return;
                   }

                   QByteArray data=reply->readAll();
                   reply->deleteLater();

                   if("016" == m_common.getcode(data) )
                   {
                       qDebug()<<filename<<"dealFilePv成功";

                       for(int i=0;i<m_shareFileList.size();i++)
                       {
                           if(m_shareFileList.at(i)->md5 == md5 && m_shareFileList.at(i)->filename == filename)
                           {
                               m_shareFileList.at(i)->pv+=1;

                               break;
                           }
                       }
                   }
                   else if("017" == m_common.getcode(data) )
                   {
                       qDebug()<<filename<<"dealFilePv失败";
                   }
               });

       return;


}



QByteArray  sharefilelist::SetShaerfilejson(int m_start, int m_count)
{
    QMap<QString,QVariant> countjson;
    countjson.insert("start",m_start);
    countjson.insert("count",m_count);

    QJsonDocument doc = QJsonDocument::fromVariant(countjson);
    return doc.toJson();
}

//设置取消文件分享要发送的json格式包
QByteArray sharefilelist::SetCancelfilejson(QString user, QString md5, QString filename)
{
    QMap<QString,QVariant> canclefilejson;
    canclefilejson.insert("user",user);
    canclefilejson.insert("md5",md5);
    canclefilejson.insert("filename",filename);

    QJsonDocument doc = QJsonDocument::fromVariant(canclefilejson);
    return doc.toJson();
}
