#include "ranklist.h"
#include "ui_ranklist.h"

ranklist::ranklist(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ranklist)
{
    ui->setupUi(this);

    m_manager = m_common.getmanager();

    initTabwidget();
}

ranklist::~ranklist()
{
    delete ui;
}


//设置tabwidget属性
void ranklist::initTabwidget()
{
     ui->tableWidget->setColumnCount(4);  //设置列数
     ui->tableWidget->horizontalHeader()->setDefaultSectionSize(300);  //设置列的宽度
     ui->tableWidget->horizontalHeader()->setSectionsClickable(false);  //设置表头不可点击

     QStringList header;
     header.append("排名");
     header.append("文件名");
     header.append("下载量");
     header.append("热度");
     ui->tableWidget->setHorizontalHeaderLabels(header);  //设置表头标签

     QFont font = ui->tableWidget->horizontalHeader()->font();  //获取表头原来的字体
     font.setBold(true);  //字体设置粗体
     ui->tableWidget->horizontalHeader()->setFont(font);  //重新设置字体

     ui->tableWidget->verticalHeader()->setDefaultSectionSize(40); //设置垂直方向的高度
     ui->tableWidget->setShowGrid(false);  //设置不显示格子线
     ui->tableWidget->verticalHeader()->setVisible(false);   //设置垂直头不可见，不自动显示行号
     ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);  //设置单行选择
     ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
     ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);   //设置不可编辑

     // 通过样式表，设置表头背景色
     ui->tableWidget->horizontalHeader()->setStyleSheet(
                   "QHeaderView::section{"
                   "background: lightyellow;"
                   "font: 13pt \"新宋体\";"
                   "height: 35px;"
                   "border:1px solid #c7f0ea;"
                   "}");

     //设置第0列的宽度
     ui->tableWidget->setColumnWidth(0,100);
     ui->tableWidget->setColumnWidth(3,100);

     //设置列宽策略使列自适应宽度，所设置列平均分填充空白部分
     ui->tableWidget->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);
     ui->tableWidget->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Stretch);

}


//显示共享的文件列表
void ranklist::refreshFiles()
{
    logininfoinstance * login = logininfoinstance::getinstance();

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    QString url=QString("http://%1:%2/sharefiles?cmd=count").arg(login->getip()).arg(login->getport());
    request.setUrl(QUrl(url));

    QNetworkReply * reply = m_manager->post(request,"");

    connect(reply,&QNetworkReply::readyRead,[=]()
    {
        QByteArray data = reply->readAll();
        reply->deleteLater();

        m_userFilesCount = data.toInt();

        clearshareFileList();

        if(m_userFilesCount>0)
        {
           m_start = 0;
           m_count = 10;
           getUserFileList();
        }else{
          this->refreshlist();
        }
    });


}

//清空共享文件列表
void ranklist::clearshareFileList()
{
    int n = m_list.size();
    for(int i=0;i<n;i++)
    {
        RankFileInfo *info = m_list.takeFirst();
        delete  info;
    }
}

void ranklist::getUserFileList()
{
   if(m_userFilesCount==0)
   {
     qDebug()<<"获取下载排行榜文件列表结束";
     refreshlist();
     return;
   }

   if(m_userFilesCount<m_count)
   {
       m_count = m_userFilesCount;
   }

   logininfoinstance *login = logininfoinstance::getinstance();

   QByteArray filelistjson = setFileListJson(m_start,m_count);

   m_start+=m_count;
   m_userFilesCount-=m_count;


   QNetworkRequest request;
   request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
   request.setHeader(QNetworkRequest::ContentLengthHeader,filelistjson.size());

   QString url=QString("http://%1:%2/sharefiles?cmd=pvdesc").arg(login->getip()).arg(login->getport());
   request.setUrl(QUrl(url));

   QNetworkReply *reply=m_manager->post(request,filelistjson);
   if(reply==nullptr)
   {
       qDebug()<<"reply==nullptr";
       return;
   }

   connect(reply,&QNetworkReply::finished,[=]()
           {
               QByteArray data=reply->readAll();

               reply->deleteLater();

               if("015" != m_common.getcode(data) ) //common.h
               {
                   getFileJsonInfo(data);//解析文件列表json信息，存放在文件列表中

                   //继续获取下载榜列表
                   getUserFileList();

               }
               else
               {
                   qDebug()<<"下载榜列表获取失败";
                   return;
               }

           });
}



//更新图形界面
void ranklist::refreshlist()
{
    int rowcount = ui->tableWidget->rowCount();
    for(int i = 0;i<rowcount;i++)
    {
        ui->tableWidget->removeRow(0);
    }


    for(int i = 0;i<m_list.size();i++)
    {
        QTableWidgetItem * item1 = new QTableWidgetItem;
        QTableWidgetItem * item2 = new QTableWidgetItem;
        QTableWidgetItem * item3 = new QTableWidgetItem;

        ui->tableWidget->insertRow(i);

        item1->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);  //水平垂直居中
        item2->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        item3->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

        item1->setText(QString::number(i+1));
        item2->setText(m_list.at(i)->filename);
        item3->setText(QString::number(m_list.at(i)->pv));

         ui->tableWidget->setItem(i,0,item1);
         ui->tableWidget->setItem(i,1,item2);
         ui->tableWidget->setItem(i,2,item3);

         //给最后一列前三名添加热度标志
         if(i<3)
         {
             QTableWidgetItem * item4 = new QTableWidgetItem;
             item4->setIcon(QIcon(":/ico/images/fire.gif"));
             ui->tableWidget->setItem(i,3,item4);
         }else{
             QTableWidgetItem * item4 = new QTableWidgetItem;
             item4->setIcon(QIcon(":/ico/images/firegray.jpg"));
             ui->tableWidget->setItem(i,3,item4);
         }



    }
}

QByteArray ranklist::setFileListJson(int m_start, int m_count)
{
    QMap<QString,QVariant> FilesListJson;
    FilesListJson.insert("start",m_start);
    FilesListJson.insert("count",m_count);

    QJsonDocument doc=QJsonDocument::fromVariant(FilesListJson);
    return doc.toJson();
}

//解析json包
void ranklist::getFileJsonInfo(QByteArray data)
{
    QJsonDocument doc=QJsonDocument::fromJson(data);

    if(doc.isObject())
    {
        QJsonObject obj=doc.object();
        QJsonArray  fileArray=obj.value("files").toArray();
        for(int i=0;i<fileArray.size();i++)
        {
            RankFileInfo *fileinfo=new RankFileInfo;

            fileinfo->filename=fileArray[i].toObject().value("filename").toString();
            fileinfo->pv=fileArray[i].toObject().value("pv").toInt();

            qDebug()<<fileinfo->filename<<" "<<fileinfo->pv;
            m_list.append(fileinfo);

        }
    }
}
