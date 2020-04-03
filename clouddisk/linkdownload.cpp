#include "linkdownload.h"
#include "ui_linkdownload.h"

linkdownload::linkdownload(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::linkdownload)
{
    ui->setupUi(this);


    //点击下载按钮时
    connect(ui->pushButton_download,&QPushButton::clicked,[=]()
    {
        emit sig_download();
    });

    connect(ui->pushButton_save,&QPushButton::clicked,[=]()
    {
        emit sigSave();
    });
}


//给界面设置内容
void linkdownload::setcontent(QString url, QString username)
{
    ui->label_shareuser->setText(username);
    ui->lineEdit_downloadlink->setText(url);
}

void linkdownload::hidecode(bool flg)
{
    //如果为true
    if(flg==true)
    {
        //隐藏提取码框
        ui->label_code->hide();
        ui->lineEdit_code->hide();
    }
    return;
}

//得到用户输入的密码
QString linkdownload::getcode()
{
    return ui->lineEdit_code->text();
}

linkdownload::~linkdownload()
{
    delete ui;
}
