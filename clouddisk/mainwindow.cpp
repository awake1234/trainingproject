#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLabel>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //去除边框
    this->setWindowFlags(Qt::FramelessWindowHint | windowFlags());
    //将title_widget的父对象设置为mainwindow
    ui->title_widget->setParent(this);

    managesignals();

}

MainWindow::~MainWindow()
{
    delete ui;
}


//集中管理信号
void MainWindow::managesignals()
{
    //关闭窗口
    connect(ui->title_widget,&Buttongroup::closewindow,[=]()
    {
       //关闭窗口
        this->close();
    });

    //窗口最小化
    connect(ui->title_widget,&Buttongroup::minsizewindow,[=]()
    {
       //窗口最小化
       this->showMinimized();
    });

    //窗口最大化
    connect(ui->title_widget,&Buttongroup::maxsizewindow,[=]()
    {
       //如果当前是最大化的
       if(this->isMaximized())
       {
           //显示正常大小
          this->showNormal();
       }else {

          //显示最大化
          this->showMaximized();
       }
    });


    //切换窗口
    //我的文件界面
    connect(ui->title_widget,&Buttongroup::sigmydisk,[=]()
    {
        ui->stackedWidget->setCurrentIndex(0);
    });

    //传输界面
    connect(ui->title_widget,&Buttongroup::sigtransfer,[=]()
    {
        ui->stackedWidget->setCurrentIndex(1);
    });

    //分型页面按钮
    connect(ui->title_widget,&Buttongroup::sigshare,[=]()
    {
        ui->page_sharelist->refreshFiles();   //刷新文件内容
        ui->stackedWidget->setCurrentIndex(2);

    });

    //下载排行榜界面
    connect(ui->title_widget,&Buttongroup::sigdownloadrank,[=]()
    {
        ui->page_downloadlist->refreshFiles();  //刷新文件列表
        ui->stackedWidget->setCurrentIndex(3);
    });


    //切换用户
    connect(ui->title_widget,&Buttongroup::sigswitchuser,[=]()
    {
           //重新登录
           this->loginagain();
    });


    //其他操作失败重新登录的信号
    connect(ui->page_mydisk,&mydiskwg::loginAgainSignal,[=]()
    {
         //重新登录
         this->loginagain();
    });



    //检测是否要切换到传输界面
    connect(ui->page_mydisk,&mydiskwg::switchto_ui,[=](uistatus status)
    {
       ui->title_widget->slotButtonClick_page(Page::TRANSFER);  //触发转换到传输界面的函数

       //上传页面
       if(status==upload)
       {
          ui->page_transfer_record->showuploadtask();  //切换到上传列表的界面
       }

       //下载页面
       if(status==download)
       {
           ui->page_transfer_record->showdownloadtask(); //切换到下载任务界面
       }

       //分享界面
       if(status==share)
       {
           //初始化界面
           ui->page_sharelist->refreshFiles();
           ui->stackedWidget->setCurrentIndex(2);  //切换到分享界面
       }

    });

    connect(ui->tabWidget,SIGNAL(tabBarClicked(int)),this,SLOT(setmytabwig(int)));


}

//显示窗口
void MainWindow::ShowMainWindow()
{
    //显示用户的容量大小
    ui->page_mydisk->getuserspace();

    //得到信号以后 进行UI设置
    connect(ui->page_mydisk,&mydiskwg::getusedspace,[=]()
    {
    //拼接一个字符串显示用户容量大小
    qint64 size = ui->page_mydisk->usedspace;
    QString text = QString("%1MB/2048MB").arg(size/(1024*1024*2));
    //设置标签的值
    QLabel *  labeluserspace = ui->title_widget->getlabelsize();
    labeluserspace->setText(text);

    //设置当前值
    QProgressBar * userspace = ui->title_widget->getprogress();
    userspace->setRange(0,2048);
    userspace->setValue(static_cast<int>(size/(1024*1024*2))+1);

    });

   //显示用户的文件列表
   ui->page_mydisk->getuserfilecount();
   //获取设置用户名信息的label的地址
   QLabel * temp = ui->title_widget->getlabelusername();
   logininfoinstance * info = logininfoinstance::getinstance();
   temp->setText(info->getuser());

   this->show();


}

//重新登录
void MainWindow::loginagain()
{

    emit changeuser();
    ui->page_mydisk->clearitems();
    ui->page_mydisk->clearfilelist();
    //清除所有的传输任务
    ui->page_mydisk->clearAllTask();
}




//重写鼠标点击事件
void MainWindow::mousePressEvent(QMouseEvent *ev)
{
    //只允许鼠标左建点击
    if(ev->button()==Qt::LeftButton)
    {
        //减去左上角
        dragposition = ev->globalPos()-frameGeometry().topLeft();
    }


}

//鼠标移动
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
        if(event->buttons() & Qt::LeftButton)
        {
           move(event->globalPos()-dragposition);
        }
}

//设置mypic页面
void MainWindow::setmytabwig(int index)
{
    //我的文档界面
    if(index==1)
    {
        //从总的文件页面去取item
        QListWidget * tempwidget = ui->page_mydisk->getfilelistwidget();

        for(int i = 0;i<tempwidget->count();i++)
        {
            QString filename = tempwidget->item(i)->text();
            //解析文件名得到后缀名
            int n = filename.indexOf(QChar('.'),0);
            //截取后缀
            QString suffix = filename.mid(n+1);

            if(suffix=="css"||suffix=="docx"||suffix=="docx_mac"||suffix=="pdf"||suffix=="pptx_win"||suffix=="pptx_mac"||suffix=="xlsx"||suffix=="text")
            {
                QListWidgetItem * tempitem = new QListWidgetItem(*tempwidget->item(i));
                ui->page_mydoc->m_piclist.append(tempitem);
            }
        }

        ui->page_mydoc->refreshpicItems();
    }

    //我的图片界面
    if(index==2)
    {
        //从总的文件页面去取item
        QListWidget * tempwidget = ui->page_mydisk->getfilelistwidget();

        for(int i = 0;i<tempwidget->count();i++)
        {
            QString filename = tempwidget->item(i)->text();
            //解析文件名得到后缀名
            int n = filename.indexOf(QChar('.'),0);

            //截取后缀
            QString suffix = filename.mid(n+1);

            if(suffix=="jpeg"||suffix=="jpg"||suffix=="png"||suffix=="bmp"||suffix=="gif")
            {
                QListWidgetItem * tempitem = new QListWidgetItem(*tempwidget->item(i));
                ui->page_mypics->m_piclist.append(tempitem);
            }
        }

        ui->page_mypics->refreshpicItems();

    }

    //音频
    if(index==3)
    {
        //从总的文件页面去取item
        QListWidget * tempwidget = ui->page_mydisk->getfilelistwidget();

        for(int i = 0;i<tempwidget->count();i++)
        {
            QString filename = tempwidget->item(i)->text();
            //解析文件名得到后缀名
            int n = filename.indexOf(QChar('.'),0);

            //截取后缀
            QString suffix = filename.mid(n+1);

            if(suffix=="mp3"||suffix=="wav"||suffix=="wmv"||suffix=="wma")
            {
                QListWidgetItem * tempitem = new QListWidgetItem(*tempwidget->item(i));
                ui->page_mysound->m_piclist.append(tempitem);
            }
        }

        ui->page_mysound->refreshpicItems();

     }
     //视频界面
     if(index==4)
     {
         //从总的文件页面去取item
         QListWidget * tempwidget = ui->page_mydisk->getfilelistwidget();

         for(int i = 0;i<tempwidget->count();i++)
         {
             QString filename = tempwidget->item(i)->text();
             //解析文件名得到后缀名
             int n = filename.indexOf(QChar('.'),0);

             //截取后缀
             QString suffix = filename.mid(n+1);

             if(suffix=="mp4"||suffix=="avi"||suffix=="mov")
             {
                 QListWidgetItem * tempitem = new QListWidgetItem(*tempwidget->item(i));
                 ui->page_myvideo->m_piclist.append(tempitem);
             }
         }

         ui->page_myvideo->refreshpicItems();

      }

     //其他文件
     if(index==5)
     {
         //从总的文件页面去取item
         QListWidget * tempwidget = ui->page_mydisk->getfilelistwidget();

         for(int i = 0;i<tempwidget->count();i++)
         {
             QString filename = tempwidget->item(i)->text();
             //去除上传文件按钮
             if(filename=="上传文件")
             {
                 continue;
             }

             //解析文件名得到后缀名
             int n = filename.indexOf(QChar('.'),0);

             //截取后缀
             QString suffix = filename.mid(n+1);

             if(!(suffix=="mp4"||suffix=="avi"||suffix=="mov"||suffix=="css"||
               suffix=="docx"||suffix=="docx_mac"||suffix=="pdf"||suffix=="pptx_win"||
               suffix=="pptx_mac"||suffix=="xlsx"||suffix=="text"||suffix=="jpeg"||
               suffix=="jpg"||suffix=="png"||suffix=="bmp"||suffix=="gif" ||
               suffix=="mp3"||suffix=="wav"||suffix=="wmv"||suffix=="wma"))
             {
                 QListWidgetItem * tempitem = new QListWidgetItem(*tempwidget->item(i));
                 ui->page_other->m_piclist.append(tempitem);
             }
         }

         ui->page_other->refreshpicItems();

      }

}
