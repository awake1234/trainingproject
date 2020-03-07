#include "dataprocess.h"
#include "ui_dataprocess.h"

dataprocess::dataprocess(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::dataprocess)
{
    ui->setupUi(this);
}

dataprocess::~dataprocess()
{
    delete ui;
}


//设置文件的名字
void dataprocess::setfilename(QString name)
{
    ui->label_filename->setText(name+":");
}

//设置进度条当前的值
void dataprocess::setprogress(qint64 value, qint64 max)
{
    ui->progressBar->setRange(0,max);
    ui->progressBar->setValue(value);
}

