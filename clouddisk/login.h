#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include<QPaintEvent>
#include<QPainter>
#include<QMouseEvent>
#include "common.h"
#include <QString>
#include <QMessageBox>
#include <QTimer>
#include<QNetworkAccessManager>
#include<QNetworkReply>
#include<QNetworkRequest>



namespace Ui {
class login;
}

class login : public QDialog
{
    Q_OBJECT

public:
    explicit login(QWidget *parent = nullptr);
    ~login();

    //定义一个清空注册界面的函数
    void clearregister();
    //设置悬浮tips
    void settips();
    //匹配注册界面输入是否符合标准的函数
    void MatchMsg();
    //检测是否所有的信息都已经填写正确
    bool is_all_item_right();

    //初始化界面的操作，将一些账号或者密码等通过配置文件来放到界面上
    bool init_ui(QString conf_path);

protected slots:
    //重写绘图事件
   void paintEvent(QPaintEvent *event);
   //重写鼠标事件
   void mouseMoveEvent(QMouseEvent * event);
   void mousePressEvent(QMouseEvent * ev);




private slots:
   void on_toolButton_net_confirm_clicked();

   void reg_senddata();

   void on_toolButton_login_clicked();

private:
    Ui::login *ui;

    //定义一个用于界面移动的坐标变量
    QPoint dragposition;

    //创建一个common对象
    common * m_common;

    //定义一个标志数组来判断注册信息是否满足标准
    bool m_flg[6]={false};
    //定义一个标志数组来判断网络设置是否正确
    bool m_netflg[2]={false};





};

#endif // LOGIN_H
