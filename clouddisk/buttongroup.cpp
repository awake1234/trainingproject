#include "buttongroup.h"
#include "ui_buttongroup.h"
#include <QDebug>
#include <QPainter>
Buttongroup::Buttongroup(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Buttongroup)
{
    ui->setupUi(this);

    //分配空间
    m_mapper = new QSignalMapper(this);
    m_curButton = ui->toolButton_myfile;
    m_curButton->setStyleSheet("color:black");  //设置字体颜色

    labelusername = ui->label_nickname;



    //建立按钮和按钮内容的映射关系
    m_btns.insert(ui->toolButton_myfile->text(),ui->toolButton_myfile);
    m_btns.insert(ui->toolButton_changeruser->text(),ui->toolButton_changeruser);
    m_btns.insert(ui->toolButton_download_rank->text(),ui->toolButton_download_rank);
    m_btns.insert(ui->toolButton_sharefilelist->text(),ui->toolButton_sharefilelist);
    m_btns.insert(ui->toolButton_transfer_record->text(),ui->toolButton_transfer_record);


    //建立页面与text的映射关系
    m_pages.insert(Page::MYDISK,ui->toolButton_myfile->text());
    m_pages.insert(Page::SHARE,ui->toolButton_sharefilelist->text());
    m_pages.insert(Page::TRANSFER,ui->toolButton_transfer_record->text());
    m_pages.insert(Page::SWITCHUSER,ui->toolButton_changeruser->text());
    m_pages.insert(Page::DOWNLOADRANK,ui->toolButton_download_rank->text());

    //设置按钮信号映射
    for(QMap<QString,QToolButton *>::iterator it = m_btns.begin();it!=m_btns.end();++it)
    {
        m_mapper->setMapping(it.value(),it.value()->text());  //按钮--》按钮上的文字
        connect(it.value(),SIGNAL(clicked(bool)),m_mapper,SLOT(map()));
    }

    connect(m_mapper,SIGNAL(mapped(QString)),this,SLOT(slotButtonClick_str(QString)));

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


QLabel *Buttongroup::getlabelusername()
{
    return   labelusername;
}


QLabel *Buttongroup::getlabelsize()
{
    return ui->label_userspace;
}


QProgressBar *Buttongroup::getprogress()
{
    return ui->userspace;
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


void Buttongroup::slotButtonClick_str(QString text)
{
    QToolButton * btn = m_btns[text];
    //等于当前按钮时不做处理
    if(btn==m_curButton&&btn!=ui->toolButton_changeruser)
    {
        return;
    }

    m_curButton->setStyleSheet("color:black");
    btn->setStyleSheet("color:red");
    m_curButton = btn;

    //发信号
    if(text==ui->toolButton_myfile->text())
    {
        emit sigmydisk();
    }else if(text==ui->toolButton_download_rank->text())
    {
        emit sigdownloadrank();
    }else if(text==ui->toolButton_sharefilelist->text())
    {
        emit sigshare();
    }else if(text==ui->toolButton_transfer_record->text())
    {
        emit sigtransfer();
    }else if(text==ui->toolButton_changeruser->text())
    {
        emit sigswitchuser();
    }
}




void Buttongroup::slotButtonClick_page(Page cur)
{
    QString text = m_pages[cur];
    slotButtonClick_str(text);
}

//重写绘图事件
void Buttongroup::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);   //一个宏表示没有使用event

    QPixmap map(":/ico/images/title_bk.jpg");
    QPainter p(this);  //要指定绘图对象

    p.drawPixmap(0,0,this->width(),this->height(),map);

}
