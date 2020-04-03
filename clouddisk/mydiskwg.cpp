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

    //消费者线程启动后执行相应的操作
    for(int i = 0;i<thread_num;++i)
    {
      connect(this,&mydiskwg::startconsumer,consumer_thread[i],&consumerthread_file::uploadfilesAction);
      connect(consumer_thread[i],SIGNAL(sig_progressvalue(qint64,qint64,dataprocess *)),this,SLOT(update_progress_value(qint64, qint64 , dataprocess *)),Qt::BlockingQueuedConnection);
      connect(consumer_thread[i],SIGNAL(sig_deleteprogress()),this,SLOT(delete_finishedfile()),Qt::BlockingQueuedConnection);
    }

    for(int i = 0;i<thread_num;i++)
    {
       //启动线程
      thread_consume_uploadfiles[i]->start();
      qDebug()<<"thread"<<i<<"has been started";
    }

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
           emit startconsumer();
           //执行上传文件操作
           adduploadfiles();
           //唤醒所有的线程
           mutex.lock();
           notempty.wakeAll();
           qDebug()<<"producer thread has waken up all consumer threads";
           mutex.unlock();
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
    m_sharelink = new QAction("分享链接",this);
    m_property = new QAction("属性",this);

    m_menu1->addAction(m_download);
    m_menu1->addAction(m_delete);
    m_menu1->addAction(m_share);
    m_menu1->addAction(m_sharelink);
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
        emit startconsumer();
        adduploadfiles();
        //唤醒所有的线程
        mutex.lock();
        notempty.wakeAll();
        qDebug()<<"producer thread has waken up all consumer threads";
        mutex.unlock();

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

    connect(m_sharelink,&QAction::triggered,[=]()
    {
       this->dealselectedfile("分享链接");
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

    //处理属性
    if(cmd=="属性")
    {
        //处理显示属性的操作
        deal_property(info);
    }

    //处理分享操作
    if(cmd=="分享")
    {
       deal_share(info);
    }


    if(cmd=="删除")
    {
       deal_delete(info);
    }


    if(cmd=="下载")
    {
        deal_pv(info);
    }

    if(cmd=="分享链接")
    {
        deal_sharelink(info);
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


//分享文件
void mydiskwg::deal_share(FileInfo *info)
{
   //如果已经分享就没有必要再分享
    if(info->shareStatus==1)
    {
        QMessageBox::warning(this,"分享状态","此文件已被分享");
        return;
    }
   logininfoinstance * logininfo = logininfoinstance::getinstance();
   QByteArray data = this->setdealfilejson(logininfo->getuser(),logininfo->gettoken(),info->md5,info->filename);

   QNetworkRequest request;
   request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
   request.setHeader(QNetworkRequest::ContentLengthHeader,data.size());

  //设置url
  QString urL = QString("http://%1:%2/dealfile?cmd=share").arg(logininfo->getip()).arg(logininfo->getport());
  request.setUrl(QUrl(urL));

  QNetworkReply  * reply = m_manager->post(request,data);

   //读取数据
   connect(reply,&QNetworkReply::readyRead,[=]()
   {

       QByteArray ret = reply->readAll();

       if(ret==nullptr)
       {
           qDebug()<<"recvive nothing";
           reply->deleteLater();
           return;
       }

       //成功
       if("010"==m_common.getcode(ret))
       {
           QMessageBox::information(this,tr("分享状态"),tr("您已经成功分享"));

           //修改文件列表中该文件的状态
           for(int i = 0;i<m_fileList.size();i++)
           {
               if(m_fileList.at(i)->filename==info->filename&&m_fileList.at(i)->md5==info->md5)
               {
                   m_fileList.at(i)->shareStatus=1;
                   break;
               }
           }

           //发送一个信号转到共享文件列表
           emit switchto_ui(share);

       }
       else if("011" == m_common.getcode(ret) )
       {
           QMessageBox::information(this,tr("分享文件"),tr("文件分享失败"));

       }
       else if("012" == m_common.getcode(ret) )
       {
           QMessageBox::information(this,tr("分享文件"),tr("文件已经被其他用户分享在云盘"));
       }
       else
       {
           QMessageBox::information(this,tr("用户"),tr("用户登陆验证码失效,请重新登录"));
           emit loginAgainSignal();
       }
   });



}


//删除操作
void mydiskwg::deal_delete(FileInfo *info)
{
    logininfoinstance *login = logininfoinstance::getinstance();
    QByteArray data = setdealfilejson(login->getuser(),login->gettoken(),info->md5,info->filename);

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader,data.size());


    QString  url = QString("http://%1:%2/dealfile?cmd=del").arg(login->getip()).arg(login->getport());
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

        if("013"==m_common.getcode(data))
        {
           QMessageBox::information(this,tr("删除结果"),tr("删除成功"));

            for(int i = 0;i<m_fileList.size();i++)
            {
                if(m_fileList.at(i)->md5==info->md5 && m_fileList.at(i)->filename==info->filename)
                {
                    FileInfo * info = m_fileList.takeAt(i);
                    QListWidgetItem * item = info->item;
                    ui->filelistWidget->removeItemWidget(item);   //删除图标
                    delete  item;
                    delete  info;
                    break;
                }
            }
        }else if("014"==m_common.getcode(data))
        {
            qDebug()<<info->filename<<"delete fail";
        }else{
              QMessageBox::information(this,tr("用户"),tr("用户登录验证码失效，请重新登录"));
              emit loginAgainSignal();   //重新登录信号
        }
    });
    return;
}


//处理下载操作
void mydiskwg::deal_pv(FileInfo *info)
{
    QString filepath =  QFileDialog::getSaveFileName(this,tr("select one dictory to save"),info->filename);

    emit switchto_ui(download);   //切换到下载页面

    DownloadTask * downloadtask = DownloadTask::getInstance();


    downloadtask->appendDownloadList(info,filepath);


    this->downloadfileAction();

}

void mydiskwg::downloadfileAction()
{
    DownloadTask * downloadtask = DownloadTask::getInstance();


    if(downloadtask->isEmpty())
    {
        return;
    }

    //如果改文件是被分享的不作处理
    if(downloadtask->isshared())
    {
        return;
    }
    //取任务
    DownloadInfo * downloadfileinfo = downloadtask->takeTask();

    logininfoinstance *login = logininfoinstance::getinstance();

    QNetworkReply * reply = m_manager->get(QNetworkRequest(downloadfileinfo->url));


    connect(reply,&QNetworkReply::finished,[=]()
    {
        QByteArray data = reply->readAll();
        downloadfileinfo->file->write(data);

        qDebug()<<downloadfileinfo->filename<<"下载成功";
        m_common.writeRecord(login->getuser(),downloadfileinfo->filename,"010");

        dealFilepv(downloadfileinfo->md5,downloadfileinfo->filename);

        downloadtask->delDownloadTask();

        reply->deleteLater();
    });

    connect(reply,&QNetworkReply::downloadProgress,[=](qint64 bytesReceived,qint64 bytesTotal){
        if(bytesTotal!=0)
        {
            downloadfileinfo->dp->setprogress(bytesReceived/1024,bytesTotal/1024);
        }

    });

}

void mydiskwg::dealFilepv(QString md5, QString filename)
{

       logininfoinstance * login = logininfoinstance::getinstance();

       QByteArray data=setdealfilejson(login->getuser(), login->gettoken() ,md5, filename);

       QNetworkRequest request;
       request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
       request.setHeader(QNetworkRequest::QNetworkRequest::ContentLengthHeader,data.size());

       QString url=QString("http://%1:%2/dealfile?cmd=pv").arg(login->getip()).arg(login->getport());
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

                       for(int i=0;i<m_fileList.size();i++)
                       {
                           if(m_fileList.at(i)->md5 == md5 && m_fileList.at(i)->filename == filename)
                           {
                               m_fileList.at(i)->pv+=1;
                               break;
                           }
                       }
                   }
                   else if("017" == m_common.getcode(data) )
                   {
                       qDebug()<<filename<<"dealFilePv失败";
                   }
                   else
                   {
                       QMessageBox::information(this,"用户","用户登陆验证码失效,请重新登录");
                       emit loginAgainSignal();
                   }

               });

         return;

}


//分享链接给好友的操作
void mydiskwg::deal_sharelink(FileInfo *info)
{
    //显示窗口
    createlink * mylink = new createlink();
    mylink->show();

    //默认只显示连接没有提取码
    //生成不带提取码的链接
    QString filepath = info->url;
    logininfoinstance * login = logininfoinstance::getinstance();
    //拼接连接
    QString sharelink = QString("%1?shareuser=%2&md5=%3").arg(filepath).arg(login->getuser()).arg(info->md5);
    //对连接进行加密
    QByteArray encode_link(sharelink.toLocal8Bit());
    QByteArray link_base64 = encode_link.toBase64(QByteArray::Base64UrlEncoding|QByteArray::OmitTrailingEquals);
    QString link(link_base64);
    //设置到界面上
    mylink->setsharelink_code(link);

    //查看是否选择了带提取码
    //被选中了
    connect(mylink,&createlink::buttonchecked,[=](bool ischecked)
    {
        if(ischecked==true)
        {
            //生成带提取码的链接
            QString filepath = info->url;

            //生成随机的四位的密码
            qsrand(QTime::currentTime().second() * 1000 + QTime::currentTime().msec());
            QString sharecode = nullptr;
            for(int i = 0;i<4;i++)
            {
                int flg = qrand()%4;
                switch (flg) {
                case 0: sharecode+='0'+qrand()%10;break;   //数字
                case 1: sharecode+='a'+qrand()%26;break;   //小写字母
                case 2: sharecode+='A'+qrand()%26;break;    //大写字母
                default:sharecode+='z';
              }
            }
            logininfoinstance * login = logininfoinstance::getinstance();
            //拼接连接
            QString sharelink = QString("%1?sharecode=%2&shareuser=%3&md5=%4").arg(filepath).arg(sharecode).arg(login->getuser()).arg(info->md5);
            //对连接进行加密
            QByteArray encode_link(sharelink.toLocal8Bit());
            QByteArray link_base64 = encode_link.toBase64(QByteArray::Base64UrlEncoding|QByteArray::OmitTrailingEquals);
            QString link(link_base64);
            //设置到界面上
            mylink->setsharelink_code(link,sharecode);
         }else{
            //生成不带提取码的链接
            QString filepath = info->url;
            logininfoinstance * login = logininfoinstance::getinstance();
            //拼接连接
            QString sharelink = QString("%1?shareuser=%2&md5=%3").arg(filepath).arg(login->getuser()).arg(info->md5);
            qDebug()<<"sharelink"<<sharelink;
            //对连接进行加密
            QByteArray encode_link(sharelink.toLocal8Bit());
            QByteArray link_base64 = encode_link.toBase64(QByteArray::Base64UrlEncoding|QByteArray::OmitTrailingEquals);
            QString link(link_base64);
            //设置到界面上
            mylink->setsharelink_code(link);
        }
    });
}

//设置分享的json包
QByteArray mydiskwg::setdealfilejson(QString username, QString token, QString md5, QString filename)
{
    QMap<QString,QVariant> map;
    map.insert("user",username);
    map.insert("token",token);
    map.insert("md5",md5);
    map.insert("filename",filename);


    QJsonDocument doc = QJsonDocument::fromVariant(map);

    return doc.toJson();
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


//检测到鼠标进入到界面
void mydiskwg::enterEvent(QEvent *)
{

        //检测剪切板中是否有内容
        QClipboard *clipboard = QApplication::clipboard();
        //如果为空
        if(clipboard->text()==nullptr)
        {
            qDebug()<<"clipbard is empty";
            return;
        }
        //"link:%1\npassword:%2\n"
        //取出分享的连接
        QString text = clipboard->text();
        //查找链接所在的位置
        int index = text.indexOf("link",0);
        index+=5;

        //查找链接结束的位置
        int end = text.indexOf("password",index);
        end--;    //此时指向/n的位置

        //得到整体的url
        QString url = nullptr;
        for(int i = index;i<end;i++)
        {
            url.append(text.at(i));
        }

        //解析url
        QByteArray filelink = QByteArray::fromBase64(url.toUtf8());

        QString filelink_str(filelink);
        qDebug()<<"完整的url:"<<filelink_str;

        //如果是个链接
        if(m_common.isMatch(LINK_MATCH,filelink_str))
        {
            linkdownload * mylinkdownload = new linkdownload();

            //获取用户名
            int index_username = filelink_str.indexOf("shareuser",0);
            index_username+=10;
            int index_md5 = filelink_str.indexOf("md5",index_username);
            //解析得到md5
            QString md5=nullptr;
            int tempindex = index_md5+4;
            for(int i = tempindex;i<filelink_str.size();i++)
            {
                md5.append(filelink_str.at(i));
            }
            qDebug()<<"md5:"<<md5;

            //解析得到用户名
            index_md5--;
            QString shareuser = nullptr;
            for(int i = index_username;i<index_md5;i++)
            {
                shareuser.append(filelink_str.at(i));
            }
            qDebug()<<"shaeruser:"<<shareuser;

            //如果分享的用户和当前用户一样就返回
            logininfoinstance * login = logininfoinstance::getinstance();
            if(shareuser==login->getuser())
            {
                return;
            }

            //检测是否带有提取码
            QString code = nullptr;
            int index_code = filelink_str.indexOf("sharecode",0);
            //如果没有code
            int index_fileurl_end=0;
            if(index_code==-1)
            {
              index_fileurl_end = filelink_str.indexOf("shareuser",0);
            }else{
              index_fileurl_end = filelink_str.indexOf("sharecode",0);
            }
            //提取出get请求的文件链接，不带参数
            QString  getfileurl = nullptr;
            for(int i = 0;i<index_fileurl_end-1;i++)
            {
                getfileurl.append(filelink_str.at(i));
            }
            qDebug()<<"不带参数的url"<<getfileurl;
            //解析的得到文件名

            //找到最后一个'/'
            int index_filename=getfileurl.lastIndexOf("/");
            index_filename++;
            qDebug()<<"index_filename"<<index_filename;
            QString filename = nullptr;
            for(int i = index_filename;i<getfileurl.size();i++)
            {
                filename.append(getfileurl.at(i));
            }
            qDebug()<<"filename"<<filename;


            shareuser.append("给你分享了链接");
            //设置界面
            mylinkdownload->setcontent(url,shareuser);


            //没有提取码
            if(index_code==-1)
            {
                //隐藏提取码输入框
                mylinkdownload->hidecode(true);

                //执行下载操作
                //检测用户输入的密码是否正确
                connect(mylinkdownload,&linkdownload::sig_download,[=]()
                {
                       //打开文件存储位置
                       QString filepath =  QFileDialog::getSaveFileName(this,tr("select one dictory to save"));
                       //执行下载操作
                       QFile * file = new QFile(filepath);  //文件指针分配空间
                       if(!file->open(QIODevice::WriteOnly))
                       { //如果打开文件失败，则删除 file，并使 file 指针为 NULL，然后返回
                          qDebug() << "file open error";
                           delete file;
                           file = nullptr;
                           return -2;
                       }

                      QNetworkReply * reply = m_manager->get(QNetworkRequest(getfileurl));
                      connect(reply,&QNetworkReply::finished,[=]()
                      {
                          QByteArray data = reply->readAll();
                          file->write(data);
                          QMessageBox::information(this,"下载结果","恭喜你下载成功");
                      });
                });

                //没有提取码 文件进行转存
                //转存处理
                connect(mylinkdownload,&linkdownload::sigSave,[=]()
                {
                     //文件转存
                    this->SavefileLink(login->getuser(),filename,md5);
                });

            }else{
                //提取出code
                index_code+=10;
                for(int i =index_code;i<(index_code+4);i++)
                {
                    code.append(filelink_str.at(i));
                }
                 //显示操作
                qDebug()<<"code"<<code;


            //检测用户输入的密码是否正确
            connect(mylinkdownload,&linkdownload::sig_download,[=]()
            {
               QString inputcode = mylinkdownload->getcode();
               if(inputcode!=code)
               {
                   QMessageBox::warning(this,"code error","对不起，您输入的提取码错误请重新输入");
               }else{
                   //打开文件存储位置
                   QString filepath =  QFileDialog::getSaveFileName(this,tr("select one dictory to save"));
                   //执行下载操作
                   QFile * file = new QFile(filepath);  //文件指针分配空间
                   if(!file->open(QIODevice::WriteOnly))
                   { //如果打开文件失败，则删除 file，并使 file 指针为 NULL，然后返回
                      qDebug() << "file open error";

                       delete file;
                       file = nullptr;
                       return -2;
                   }

                  QNetworkReply * reply = m_manager->get(QNetworkRequest(getfileurl));
                  connect(reply,&QNetworkReply::finished,[=]()
                  {
                      QByteArray data = reply->readAll();
                      file->write(data);
                      QMessageBox::information(this,"下载结果","恭喜你下载成功");

                  });
              }
           });
            //转存处理
            connect(mylinkdownload,&linkdownload::sigSave,[=]()
            {
                //检查密码是否正确
                QString inputcode = mylinkdownload->getcode();
                if(inputcode!=code)
                {
                    QMessageBox::warning(this,"code error","对不起，您输入的提取码错误请重新输入");
                }else{
                    //文件转存
                    this->SavefileLink(login->getuser(),filename,md5);

                 }

            });

        }

            mylinkdownload->show();
            //清除剪切板
            clipboard->clear();

      }

}



//将文件添加到上传列表中
void mydiskwg::adduploadfiles()
{ //这里面做上传文件到文件列表中的操作
    QStringList filepath=QFileDialog::getOpenFileNames(this, tr("Select one or more files to upload"),"./", "file(*.*)");

    emit switchto_ui(upload);
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

    connect(reply,&QNetworkReply::readyRead,[=](){
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

            //qDebug()<<info->user<<":"<<info->md5<<":"<<info->time<<":"<<info->url<<":"<<info->type<<":"<<info->size;
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

        //重新获取用户容量
        this->getuserspace();
        //发送设置用户容量的信号
        emit getusedspace();
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


//用来获取用户的容量
void  mydiskwg::getuserspace()
{
    //组织需要发送的json字符串
    logininfoinstance  * login = logininfoinstance::getinstance();

    QByteArray data = setcountjson(login->getuser(),login->gettoken());

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader,data.size());

    //cmd=size获取用户已使用文件的大小
    QString url = QString("http://%1:%2/myfiles?cmd=size").arg(login->getip()).arg(login->getport());
    request.setUrl(QUrl(url));

    QNetworkReply * reply = m_manager->post(request,data);


    connect(reply,&QNetworkReply::readyRead,[=]() mutable
    {
       if(reply->error()!=QNetworkReply::NoError)
       {
           qDebug()<<"reply get an error";
           return;
       }

       QByteArray ret = reply->readAll();
       /*解析json
        * 结果正确：110
        * 口令验证错误：111
        * 其他错误：12
       {
         "code":10
         "size":xxx
       }
       */
       QJsonDocument doc = QJsonDocument::fromJson(ret);
       if(doc.isObject())
       {
           QString code = doc.object().value("code").toString();
           if(code=="110")
           {
              usedspace = doc.object().value("size").toInt();
              emit getusedspace();
              qDebug()<<"获取用户容量成功";
           }else if (code=="111")
           {
               qDebug()<<"口令验证失败，请重新登录";
               emit loginAgainSignal();  //发送重新登录信号
           }else{
               qDebug()<<"获取用户容量失败";
           }
       }
    });
}


//通过链接转存文件
void mydiskwg::SavefileLink(QString user, QString filename, QString md5)
{
    logininfoinstance * login  = logininfoinstance::getinstance();

     QByteArray data = setfilejson(user,filename,md5);
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

QByteArray  mydiskwg::setfilejson(QString user, QString filename, QString md5)
{
    QMap<QString,QVariant> filejson;
    filejson.insert("user",user);
    filejson.insert("md5",md5);
    filejson.insert("filename",filename);

    QJsonDocument doc = QJsonDocument::fromVariant(filejson);
    return doc.toJson();

}



//清除所有的任务
void mydiskwg::clearAllTask()
{
   uploadtask = uploadtask::get_uploadtask_instance();

   uploadtask->clearlist();

   DownloadTask * downloadtask = DownloadTask::getInstance();
   downloadtask->clearlist();
}

