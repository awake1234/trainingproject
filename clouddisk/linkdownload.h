#ifndef LINKDOWNLOAD_H
#define LINKDOWNLOAD_H

#include <QWidget>

namespace Ui {
class linkdownload;
}

class linkdownload : public QWidget
{
    Q_OBJECT

public:
    explicit linkdownload(QWidget *parent = nullptr);

    //界面设置内容
    void setcontent(QString url,QString username);
    void hidecode(bool flg);
    QString getcode();

    ~linkdownload();
signals:
    void sig_download();  //发送一个下载的信号
    void sigSave();          //转存信号
private:
    Ui::linkdownload *ui;
};

#endif // LINKDOWNLOAD_H
