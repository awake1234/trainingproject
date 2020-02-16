#include "buttongroup.h"
#include "ui_buttongroup.h"

#include <QPainter>
Buttongroup::Buttongroup(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Buttongroup)
{
    ui->setupUi(this);


    //检测窗口关闭按钮
    connect(ui->toolButton_close,&QToolButton::clicked,[=]()
    {
       //发送一个关闭信号
        emit(this->closewindow());
    });


    //监测窗口最小化的按钮
    connect(ui->toolButton_minsize,&QToolButton::clicked,[=]()
    {
        //发送窗口最小化信号
        emit(this->minsizewindow());
    });

    //检测最大化按钮
    connect(ui->toolButton_maxsize,&QToolButton::clicked,[=]()
    {
       //设置一个标志位
        static bool ismax = false;
        if(ismax==false)
        {
            //设置成缩小到正常的图标
            ui->toolButton_maxsize->setIcon(QIcon(":/ico/images/title_normal.png"));
            ismax = true;
        }else {
           ui->toolButton_maxsize->setIcon(QIcon(":/ico/images/title_max.png"));
           ismax = false;
        }
        //发送最大化窗口的命令
        emit(this->maxsizewindow());

     });


}

Buttongroup::~Buttongroup()
{
    delete ui;
}

//设置父对象
void Buttongroup::setParent(QWidget *parent)
{
    m_parent = parent;
}

//重写绘图事件
void Buttongroup::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);   //一个宏表示没有使用event

    QPixmap map(":/ico/images/title_bk.jpg");
    QPainter p(this);  //要指定绘图对象

    p.drawPixmap(0,0,this->width(),this->height(),map);

}
