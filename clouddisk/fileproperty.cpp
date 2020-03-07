#include "fileproperty.h"
#include "ui_fileproperty.h"

fileproperty::fileproperty(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::fileproperty)
{
    ui->setupUi(this);
    this->setWindowTitle(QString("文件属性"));
}

fileproperty::~fileproperty()
{
    delete ui;
}


//设置文件信息
void fileproperty::setfileinfo(FileInfo *info)
{
    ui->filename_str->setText(info->filename);

    QString finalsize;
    int size = 0;
    //如果文件大小大于1kb并且小于1MB
    if(info->size>=1024&&info->size<1024*1024)
    {
        size = info->size/1024.0;
        finalsize = QString("%1KB").arg(size);
    }else {
        size = info->size/(1024.0*1024.0);
        finalsize = QString("%1MB").arg(size);
   }
   ui->filesize_str->setText(finalsize);
   ui->filetype_str->setText(info->type);
   ui->uploader_str->setText(info->user);
   ui->uploadtime_str->setText(info->time);
   ui->downloadurl_str->setText(info->url);
   if(info->shareStatus==1)
   {
       ui->isshared_str->setText("已分享");
   }else{
       ui->isshared_str->setText("未分享");
   }
   ui->pv_str->setText(QString("%1次").arg(info->pv));

}
