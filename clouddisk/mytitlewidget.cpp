#include "mytitlewidget.h"
#include "ui_mytitlewidget.h"

mytitlewidget::mytitlewidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::mytitlewidget)
{
    ui->setupUi(this);

    QPixmap pixmap(":/ico/images/njitlogo.jpg");
    //设置logo图片,缩放图片的大小scaled函数
    ui->logolabel->setPixmap(pixmap.scaled(40,40));


    //监听设置，最小化，和关闭按钮
    connect(ui->minButton,&QToolButton::clicked,[=]()
    {
        emit minisizesignal();
    });

    connect(ui->setButton,&QToolButton::clicked,[=]()
    {
        emit setsignal();
    });

    connect(ui->closeButton,&QToolButton::clicked,[=]()
    {
        emit closesignal();
    });

}

mytitlewidget::~mytitlewidget()
{
    delete ui;
}
