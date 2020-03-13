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

    if(cmd==property)
    {
        showproperty(temp);
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



QByteArray  sharefilelist::SetShaerfilejson(int m_start, int m_count)
{
    QMap<QString,QVariant> countjson;
    countjson.insert("start",m_start);
    countjson.insert("count",m_count);

    QJsonDocument doc = QJsonDocument::fromVariant(countjson);
    return doc.toJson();
}
