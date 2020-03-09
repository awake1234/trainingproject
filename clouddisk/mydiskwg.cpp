#include "mydiskwg.h"
#include "ui_mydiskwg.h"

extern QMutex mutex;   //加锁文件列表资源
extern QWaitCondition notempty;  //条件变量文件列表不为空

mydiskwg::mydiskwg(QWidget *parent):QWidget(parent),
    ui(new Ui::mydiskwg)
{
    ui->setupUi(this);

    //添加右键菜单的内容
    this->AddMenuAction();


    //获取上传文件的QNetworkAcessmanager
    m_manager = m_common.getmanager();

    m_common.getFileTypeList(); //得到文件类型列表



    for(int i = 0;i<thread_num;++i)
    {
       QThread * thread = new QThread;
       consumerthread_file * temp = new consumerthread_file();
       //加入到数组中
       thread_consume_uploadfiles.insert(i,thread);
       consumer_thread.insert(i,temp);
       //将当前对象放到消费者线程中处理
       consumer_thread[i]->moveToThread(thread_consume_uploadfiles[i]);
    }

    //thread_consumer = new QThread();
    //consumerthread_file *  consumer = new consumerthread_file;
    //consumer->moveToThread(thread_consumer);



    //消费者线程启动后执行相应的操作
    for(int i = 0;i<thread_num;++i)
    {
      connect(this,&mydiskwg::startconsumer,consumer_thread[i],&consumerthread_file::uploadfilesAction);
      connect(consumer_thread[i],SIGNAL(sig_progressvalue(qint64,qint64,dataprocess *)),this,SLOT(update_progress_value(qint64, qint64 , dataprocess *)),Qt::BlockingQueuedConnection);
      connect(consumer_thread[i],SIGNAL(sig_deleteprogress()),this,SLOT(delete_finishedfile()),Qt::BlockingQueuedConnection);
   }



    //通过自定义信号来启动消费者线程
    //connect(this,&mydiskwg::startconsumer,consumer,&consumerthread_file::uploadfilesAction);
    //connect(consumer,SIGNAL(sig_progressvalue(qint64,qint64,dataprocess *)),this,SLOT(update_progress_value(qint64, qint64 , dataprocess *)));
    //connect(consumer,SIGNAL(sig_deleteprogress()),this,SLOT(delete_finishedfile()));


    for(int i = 0;i<thread_num;i++)
    {
     //启动线程
       thread_consume_uploadfiles[i]->start();
       qDebug()<<"thread"<<i<<"has been started";
    }


    //thread_consumer->start();
    //qDebug()<<"thread has been started";



  }

mydiskwg::~mydiskwg()
{
    delete ui;
}

//初始化filelistwidget的相关属性
void mydiskwg::initlistwidget()
{
    ui->filelistWidget->setViewMode(QListView::IconMode);  //从左往右排图标,文字在下面图标在上面
    ui->filelistWidget->setIconSize(QSize(80,80));   //设置图标的大小
    ui->filelistWidget->setGridSize(QSize(100,120));  //设置栅格的大小
    ui->filelistWidget->setResizeMode(QListView::Adjust); //大小变化后重新布局

    ui->filelistWidget->setMovement(QListView::Static);  //用户不能随便移动图标
    ui->filelistWidget->setSpacing(30);  //表示各个控件之间的上下间距

    ui->filelistWidget->setContextMenuPolicy(Qt::CustomContextMenu); //设置成右键菜单策略

    //发送右键菜单请求策略
    connect(ui->filelistWidget,&QListWidget::customContextMenuRequested,this,&mydiskwg::rightMenu);


    //监听所有的item点击事件
    connect(ui->filelistWidget,&QListWidget::itemClicked,[=](QListWidgetItem * selected)
    {
       if(selected->text()=="上传文件")
       {
           //执行上传文件操作
           adduploadfiles();
           emit startconsumer();
       }
    });

}

//添加右键菜单的条目
void mydiskwg::AddMenuAction()
{
    m_menu1 = new mymenu(this);

    m_download = new QAction("下载",this);
    m_delete = new QAction("删除",this);
    m_share = new QAction("分享",this);
    m_property = new QAction("属性",this);
    m_menu1->addAction(m_download);
    m_menu1->addAction(m_delete);
    m_menu1->addAction(m_share);
    m_menu1->addAction(m_property);


    m_empty = new mymenu(this);
    m_upload = new QAction("上传文件",this);
    m_refresh = new QAction("刷新",this);
    m_Download_ASC = new QAction("按文件大小升序",this);
    m_Download_Des = new QAction("按文件大小降序",this);
    m_empty->addAction(m_upload);
    m_empty->addAction(m_refresh);
    m_empty->addAction(m_Download_ASC);
    m_empty->addAction(m_Download_Des);



    //设置具体的事件实现
    connect(m_upload,&QAction::triggered,[=]()
    {
         //执行上传文件操作
        adduploadfiles();
        emit startconsumer();
    });

    //刷新界面
    connect(m_refresh,&QAction::triggered,[=]()
    {
        this->getuserfilecount();
    });

    //按文件大小升序排序
    connect(m_Download_ASC,&QAction::triggered,[=]()
    {
       this->getuserfilecount(pvASC);
    });


    //按文件大小降序
    connect(m_Download_Des,&QAction::triggered,[=]()
    {
       this->getuserfilecount(pvDES);
    });


    //处理选中文件所做的操作
    connect(m_property,&QAction::triggered,[=]()
    {
        this->dealselectedfile("属性");
    });

    //处理下载操作
    connect(m_download,&QAction::triggered,[=]()
    {
       this->dealselectedfile("下载");
    });

    //删除操作
    connect(m_delete,&QAction::triggered,[=]()
    {
       this->dealselectedfile("删除");
    });

    //分享操作
    connect(m_share,&QAction::triggered,[=]()
    {
        this->dealselectedfile("分享");
    });
}


//添加上传文件的item
void mydiskwg::Adduploaditem(QString iconpath, QString text)
{
    QListWidgetItem * uploaditem = new QListWidgetItem(QIcon(iconpath),text);
    ui->filelistWidget->addItem(uploaditem);
}


//自定义文件打开的窗口
QStringList mydiskwg::fileopendialog()
{
    QStringList ltFilePath;
    QFileDialog dialog(this, tr("Open Files"), tr("./"),"files(*.*)");
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setModal(QFileDialog::ExistingFiles);
    dialog.setOption(QFileDialog::DontUseNativeDialog);
    dialog.exec();
    ltFilePath = dialog.selectedFiles();

    return  ltFilePath;
}

//清除items
void mydiskwg::clearitems()
{
    int n = ui->filelistWidget->count();  //获得item的数量

    for(int i = 0;i<n;i++)
    {
        QListWidgetItem * item = ui->filelistWidget->takeItem(0);
        delete item;
    }

}

void mydiskwg::dealselectedfile(QString cmd)
{
    if(cmd==nullptr)
    {
        return;
    }

    FileInfo * info = nullptr;
    //获取当前的item指针
    QListWidgetItem * curitem = ui->filelistWidget->currentItem();

    //在文件列表中查找到对应的文件
    for(int i = 0;i<m_fileList.size();i++)
    {
        if(m_fileList.at(i)->item==curitem)
        {
            info = m_fileList.at(i);
            break;
        }
    }

    if(cmd=="属性")
    {
        //处理显示属性的操作
        deal_property(info);
    }


}

//显示属性
void mydiskwg::deal_property(FileInfo *info)
{
    fileproperty * filepro = new fileproperty();
    filepro->setfileinfo(info);
    filepro->show();

    connect(filepro,&fileproperty::close,[=]()
    {
        delete  filepro;
    });

}





//右键菜单槽函数
void mydiskwg::rightMenu(const QPoint pos)
{
    QListWidgetItem * item = ui->filelistWidget->itemAt(pos);  //得到当前位置的条目

    //如果在空白处
    if(item==nullptr)
    {
        m_empty->exec(QCursor::pos());   //在鼠标所在的位置显示
    }else{
       ui->filelistWidget->setCurrentItem(item);  //设置所选择的条目
       if(item->text()=="上传文件")
       {
          return;
       }
       m_menu1->exec(QCursor::pos());
    }
}


//更新进度条的值
void mydiskwg::update_progress_value(qint64 bytesent, qint64 bytetotal, dataprocess * dataprogress)
{
    m_process = dataprogress;
    m_process->setprogress(bytesent,bytetotal);
}


//删除掉已经完成的进度条
void mydiskwg::delete_finishedfile()
{
    uploadtask = uploadtask::get_uploadtask_instance();
    uploadtask->delete_uploadtask();

}


//得到filelistwidget指针
QListWidget *mydiskwg::getfilelistwidget()
{
    return ui->filelistWidget;
}



//将文件添加到上传列表中
void mydiskwg::adduploadfiles()
{ //这里面做上传文件到文件列表中的操作
    QStringList filepath=QFileDialog::getOpenFileNames(this, tr("Select one or more files to upload"),"./", "file(*.*)");

    emit switchto_transferui(upload);
    uploadtask = uploadtask::get_uploadtask_instance();



    for(int i = 0;i<filepath.size();i++)
    {
        int ret = uploadtask->appendtolist(filepath.at(i));
        if(ret==0)
        {
           qDebug()<<"文件已经成功添加到上传列表";

        }else {
           qDebug()<<"文件添加到上传列表失败";
        }
    }


    return;
}

//获取用户文件列表
void mydiskwg::getuserfilelist(mydiskwg::DISPLAY cmd)
{
    if(m_userfilecount==0)
    {
        qDebug()<<"获取用户文件列表结束";
        refreshFileItems();   //重新刷新界面
        return;
    }

    //每次请求文件的数量不得超过文件数量的最大值
    if(m_userfilecount<m_count)
    {
        m_count=m_userfilecount;
    }

    logininfoinstance * logininfo = logininfoinstance::getinstance();
    QByteArray Countjson  = setfileslistjson(logininfo->getuser(),logininfo->gettoken(),m_start,m_count);


    m_start+=m_count;
    m_userfilecount-=m_count;

    QString temp;

    if(cmd==normal)
    {
        temp = "normal";
    }else if(cmd==pvASC)
    {
        temp = "pvASC";
    }else {
       temp = "pvDES";
    }


    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader,Countjson.size());

    QString url = QString("http://%1:%2/myfiles?cmd=%3").arg(logininfo->getip()).arg(logininfo->getport()).arg(temp);
    request.setUrl(QUrl(url));

    QNetworkReply * reply = m_manager->post(request,Countjson);
    if(reply==nullptr)
    {
        qDebug()<<"reply is null in getuserfilelist";
        return;
    }

    connect(reply,&QNetworkReply::readyRead,[=]()
    {
        QByteArray data = reply->readAll();

        if("111"==m_common.getcode(data))
        {
            QMessageBox::warning(this,"账户异常","请重新登录");
            emit  loginAgainSignal();   //发送重新登录的信号

            return;
        }

        //不是错误码就处理文件列表json信息
        if("015"!=m_common.getcode(data))
        {
            getFileJsonInfo(data);  //解析文件列表json信息，放入文件列表中

            //继续获取用户文件列表
            getuserfilelist(cmd);

            qDebug()<<"getuserfilelist:"<<m_fileList.size();
        }
        reply->deleteLater();
});
}

//显示文件Item
void mydiskwg::refreshFileItems()
{
    clearitems();

    //如果文件列表不为空
    if(!m_fileList.isEmpty())
    {
        qDebug()<<"m_fileList size:"<<m_fileList.size();
        for(int i = 0;i<m_fileList.size();i++)
        {
            //重新添加到widget中
            ui->filelistWidget->addItem(m_fileList.at(i)->item);
        }
    }
    //重新添加上传的按钮
    this->Adduploaditem();

}

//设置获取上传文件列表的json包
QByteArray mydiskwg::setfileslistjson(QString user, QString token, int start, int count)
{
    QMap<QString,QVariant> countjson;
    countjson.insert("user",user);
    countjson.insert("token",token);
    countjson.insert("start",start);
    countjson.insert("count",count);

    QJsonDocument doc = QJsonDocument::fromVariant(countjson);
    return doc.toJson();
}

//解析获取的文件列表的json包
void mydiskwg::getFileJsonInfo(QByteArray data)
{
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if(doc.isObject())
    {
        QJsonObject obj = doc.object();
        QJsonArray fileArray = obj.value("files").toArray();
        for(int i = 0;i<fileArray.size();i++)
        {
            FileInfo *info = new FileInfo;
            //输出文件的大小
            qDebug()<<fileArray[i].toObject().value("size");

            info->user = fileArray[i].toObject().value("user").toString();
            info->md5 = fileArray[i].toObject().value("md5").toString();
            info->time = fileArray[i].toObject().value("time").toString();
            info->filename = fileArray[i].toObject().value("filename").toString();
            info->shareStatus = fileArray[i].toObject().value("shareStatus").toInt();
            info->pv = fileArray[i].toObject().value("pv").toInt();
            info->url = fileArray[i].toObject().value("url").toString();
            info->size = qint64(fileArray[i].toObject().value("size").toDouble());
            info->type = fileArray[i].toObject().value("type").toString();

            QString filepath = m_common.getFileType(info->type+".png");  //图片都是以文件类型.png的格式保存的
            qDebug()<<"filepath"<<filepath;
            info->item = new QListWidgetItem(QIcon(filepath),info->filename);  //文件名+图标

            qDebug()<<info->user<<":"<<info->md5<<":"<<info->time<<":"<<info->url<<":"<<info->type<<":"<<info->size;
            m_fileList.append(info);    //添加到文件列表中
        }

    }




}

//获取用户文件的数量
void mydiskwg::getuserfilecount(mydiskwg::DISPLAY cmd)
{
    //初始化文件列表
    this->initlistwidget();

    logininfoinstance * info = logininfoinstance::getinstance();
    QByteArray CountJson = setcountjson(info->getuser(),info->gettoken());

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader,CountJson.size());

    QString url = QString("http://%1:%2/myfiles?cmd=count").arg(info->getip()).arg(info->getport());
    request.setUrl(QUrl(url));

    QNetworkReply *reply = m_manager->post(request,CountJson);

    connect(reply,&QNetworkReply::readyRead,[=]()
    {
        QByteArray data = reply->readAll();

        reply->deleteLater();

        QStringList status = getcountstatus(data);

        qDebug()<<"status:"<<status;

        clearfilelist();  //清空文件列表

        if("110"==status.at(0))
        {
            //得到用户的文件数量
            m_userfilecount = status.at(1).toLong();

            //用户有文件
            if(m_userfilecount>0)
            {
                m_start = 0;
                m_count = 10;
                this->getuserfilelist(cmd);
            }else{
                //重新更新文件item
                this->refreshFileItems();
          }
        }else{
            QMessageBox::information(this,"user","用户登录验证码失效，请重新登录");
            emit loginAgainSignal();  //重新登录
        }
 });
}

QByteArray mydiskwg::setcountjson(QString user, QString token)
{
    QMap<QString,QVariant> map;
    map.insert("user",user);
    map.insert("token",token);

    QJsonDocument doc = QJsonDocument::fromVariant(map);
    return doc.toJson();

}

//得到服务返回的json文件
QStringList mydiskwg::getcountstatus(QByteArray json)
{
    QStringList list;

    QJsonDocument doc = QJsonDocument::fromJson(json);
    if(doc.isObject())
    {
        QJsonObject obj = doc.object();
        QString num = obj.value("num").toString();
        QString code = obj.value("code").toString();

        list.append(code);
        list.append(num);
    }
    return list;
}

void mydiskwg::clearfilelist()
{

    int n = m_fileList.size();

    for(int i = 0;i<n;i++)
    {
        FileInfo * info = m_fileList.takeFirst();
        delete  info;
    }

}

