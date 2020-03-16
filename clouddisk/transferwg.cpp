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

    connect(ui->tabWidget,&QTabWidget::currentChanged,[=](int index)
    {
        if(index==2)
        {
          showdatarecord(); //显示传输记录
        }
    });

}

transferwg::~transferwg()
{
    delete ui;
}

void transferwg::showuploadtask()
{
        ui->tabWidget->setCurrentIndex(0);
}

void transferwg::showdatarecord(QString path)
{
      logininfoinstance * login = logininfoinstance::getinstance();

      QFile file(path+login->getuser());
      if(!file.open(QIODevice::ReadOnly))
      {
          qDebug()<<"display data record error";
          return;
      }

      QByteArray data = file.readAll();
      ui->textEdit->setText(QString::fromLocal8Bit(data));


}

//点击按钮清除记录
void transferwg::on_toolButton_clearrecord_clicked()
{
    logininfoinstance * login = logininfoinstance::getinstance();
    QString filepath = RECORD_DIR+login->getuser();
    if(QFile::exists((filepath)))
    {
        QFile::remove(filepath);  //删除文件
        ui->textEdit->clear();
    }


}
