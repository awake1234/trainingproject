#ifndef CREATELINK_H
#define CREATELINK_H

#include <QWidget>
#include <QDebug>
#include <QClipboard>
#include <QMessageBox>
namespace Ui {
class createlink;
}

class createlink : public QWidget
{
    Q_OBJECT

public:
    explicit createlink(QWidget *parent = nullptr);
    ~createlink();

    //将连接显示到界面上
    void setsharelink_code(QString link,QString code=nullptr);

signals:
    void buttonchecked(bool);   //发送一个选中的信号

private slots:
    void on_pushButton_close_clicked();

    void on_pushButton_confirm_clicked();

private:
    Ui::createlink *ui;
};

#endif // CREATELINK_H
