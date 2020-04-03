#include "createlink.h"
#include "ui_createlink.h"

createlink::createlink(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::createlink)
{
    ui->setupUi(this);

    connect(ui->radioButton_hascode,&QRadioButton::toggled,[=](bool checked)
    {
        //如果被选中了
        if(checked)
        {
            emit buttonchecked(true);
        }else{
            emit buttonchecked(false);
        }

    });

}

createlink::~createlink()
{
    delete ui;
}

//在界面上设置连接
void createlink::setsharelink_code(QString link,QString code)
{
    //如果不为空
    if(link!=nullptr)
    {
       ui->lineEdit_link->setText(link);
       ui->textEdit_code->setAlignment(Qt::AlignVCenter|Qt::AlignHCenter);   //水平垂直居中
       ui->textEdit_code->setText(code);
    }else{
        qDebug()<<"LINK IS NULL";
    }

    return;
}



//关闭窗口
void createlink::on_pushButton_close_clicked()
{
    this->close();
}

//点击复制到剪切板
void createlink::on_pushButton_confirm_clicked()
{
    QClipboard * clipboard = QApplication::clipboard(); //获取剪切板指针

    QString  filelink = QString("link:%1\npassword:%2\n").arg(ui->lineEdit_link->text()).arg(ui->textEdit_code->toPlainText());

    clipboard->setText(filelink);    //将内容写入到剪切板

    QMessageBox::information(this,"分享结果","你已经成功复制到剪切板，快去分享吧！！！");

}
