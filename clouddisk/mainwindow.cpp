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
    connect(ui->title_widget,&Buttongroup::sigmydisk,[=]()
    {
        ui->stackedWidget->setCurrentIndex(0);
    });


    connect(ui->title_widget,&Buttongroup::sigtransfer,[=]()
    {
        ui->stackedWidget->setCurrentIndex(1);
    });

    connect(ui->title_widget,&Buttongroup::sigshare,[=]()
    {
        ui->stackedWidget->setCurrentIndex(2);
    });

    connect(ui->title_widget,&Buttongroup::sigdownloadrank,[=]()
    {
        ui->stackedWidget->setCurrentIndex(3);
    });

    //检测是否要切换到传输界面
    connect(ui->page_mydisk,&mydiskwg::switchto_transferui,[=](transferstatus status)
    {
       ui->title_widget->slotButtonClick_page(Page::TRANSFER);  //触发转换到传输界面的函数

       if(status==upload)
       {
          ui->page_transfer_record->showuploadtask();  //切换到上传列表的界面
       }

    });





}

//显示窗口
void MainWindow::ShowMainWindow()
{
   this->show();

   //显示用户的文件列表
   ui->page_mydisk->getuserfilecount();

   //获取设置用户名信息的label的地址
   QLabel * temp = ui->title_widget->getlabelusername();
   logininfoinstance * info = logininfoinstance::getinstance();
   temp->setText(info->getuser());

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
