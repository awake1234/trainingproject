#include "transferwg.h"
#include "ui_transferwg.h"

transferwg::transferwg(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::transferwg)
{
    ui->setupUi(this);


    //设置上传列表的布局
    UploadLayout * upload = UploadLayout::getInstance();
    upload->setUploadLayout(ui->scroll_contents_upload);  //在这个widget里加入加载条的布局

    ui->tabWidget->setCurrentIndex(0);  //显示为第一页

}

transferwg::~transferwg()
{
    delete ui;
}

void transferwg::showuploadtask()
{
    ui->tabWidget->setCurrentIndex(0);
}
