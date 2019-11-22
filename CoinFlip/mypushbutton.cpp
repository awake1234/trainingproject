#include "mypushbutton.h"
#include<QDebug>



MyPushButton::MyPushButton(QString normalImg,QString pressImg)
{
    normalImgPath = normalImg;
    pressedImgPath = pressImg;


    //创建QpixMap对象
    QPixmap normalpix;

    bool ret = normalpix.load(normalImgPath);
    if(ret==false)
    {
        qDebug()<<normalImg<<"图片加载失败";
    }

    //设置图片的固定尺寸
    this->setFixedSize(normalpix.width(),normalpix.height());

    //设置不规则图片的样式表
    this->setStyleSheet("QPushButton{border:0px;}");

    //设置图标
    this->setIcon(normalpix);

    //设置图标大小
    this->setIconSize(QSize(normalpix.width(),normalpix.height()));


}


//向下跳跃
void  MyPushButton::zoom1()
{
    //创建动画对象
    QPropertyAnimation * animational = new QPropertyAnimation(this,"geometry");

    //设置时间间隔，单位毫秒
    animational->setDuration(100);

    //创建起始位置
    animational->setStartValue(QRect(this->x(),this->y(),this->width(),this->height()));

    //创建结束位置
    animational->setEndValue(QRect(this->x(),this->y()+10,this->width(),this->height()));

    //设置缓和曲线,参数为弹跳效果
    animational->setEasingCurve(QEasingCurve::OutBounce);

    animational->start();
}


//向上跳跃
void  MyPushButton::zoom2()
{
    //创建动画对象
    QPropertyAnimation * animational = new QPropertyAnimation(this,"geometry");

    //设置时间间隔，单位毫秒
    animational->setDuration(100);

    //创建起始位置
    animational->setStartValue(QRect(this->x(),this->y()+10,this->width(),this->height()));

    //创建结束位置
    animational->setEndValue(QRect(this->x(),this->y(),this->width(),this->height()));

    //设置缓和曲线,参数为弹跳效果
    animational->setEasingCurve(QEasingCurve::OutBounce);

    animational->start();
}

//鼠标点击
void MyPushButton::mousePressEvent(QMouseEvent * e)
{
    if(pressedImgPath!="")
    {
        QPixmap pressmap;
        bool ret = pressmap.load(pressedImgPath);
        if(ret==false)
        {
            qDebug()<<"load press image error";
        }

        this->setFixedSize(pressmap.width(),pressmap.height());
        this->setStyleSheet("QpushButton{border:0px}");
        this->setIcon(pressmap);
        //这边的参数是QSize注意
        this->setIconSize(QSize(pressmap.width(),pressmap.height()));
    }

     //交给父类执行按下其他内容
    return QPushButton::mousePressEvent(e);


}


void MyPushButton::mouseReleaseEvent(QMouseEvent * e)
{
    if(pressedImgPath!="")
    {
        QPixmap normalmap;
        bool ret =normalmap.load(normalImgPath);
        if(ret==false)
        {
            qDebug()<<"load release image error";
        }

        this->setFixedSize(normalmap.width(),normalmap.height());
        this->setStyleSheet("QpushButton{border:0px}");
        this->setIcon(normalmap);
        //这边的参数是QSize注意
        this->setIconSize(QSize(normalmap.width(),normalmap.height()));
     }

    //这个非常重要,让父类执行其他内容
    return QPushButton::mouseReleaseEvent(e);
}

