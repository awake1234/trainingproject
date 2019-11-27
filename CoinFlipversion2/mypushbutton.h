#ifndef MYPUSHBUTTON_H
#define MYPUSHBUTTON_H

#include <QWidget>
#include<QPushButton>
#include<QPropertyAnimation>
#include<QMouseEvent>


class MyPushButton : public QPushButton
{
    Q_OBJECT
public:
    explicit MyPushButton(QWidget *parent = nullptr);

    //重载构造函数
    //normalImg代表正常显示的图片
    //按下后的图片默认为空
    MyPushButton(QString normalImg,QString pressImg="");

    //重写鼠标事件
    void mousePressEvent(QMouseEvent * e);
    void mouseReleaseEvent(QMouseEvent * e);

    void  zoom1(); //向下跳跃
    void  zoom2();  //向上跳跃

    //设置一个是否可以按下的标志位，用来设置选关的按钮
    bool  isclicked = true;


private:
    QString normalImgPath;
    QString pressedImgPath;


signals:

public slots:
};

#endif // MYPUSHBUTTON_H
