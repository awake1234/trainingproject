#include "mycoin.h"
#include<QDebug>



//金币类继承于按钮
 MyCoin::MyCoin(QString butImg)
 {
    QPixmap pix;
    bool ret = pix.load(butImg);
    if(ret==false)
    {
        qDebug()<<"加载图片失败";
    }


    //设置固定大小
    this->setFixedSize(pix.width(),pix.height());
    this->setStyleSheet("QPushButton{border:0px}");
    this->setIcon(pix);
    this->setIconSize(QSize(pix.width(),pix.height()));

    //初始化两个定时器
    timer1 = new QTimer(this);
    timer2 = new QTimer(this);

    //加载一个金币翻转的声音
    coinflip = new QSound(":/picture/res/ConFlipSound.wav");


    //监听计时器
    //时间一到就会切换下一张图片
    connect(timer1,&QTimer::timeout,[=]()
    {
       QPixmap  coinpic;
       //拼接一个路径
       QString str = QString(":/picture/res/Coin000%1.png").arg(this->min++);

       //加载图片
       coinpic.load(str);

       this->setFixedSize(coinpic.width(),coinpic.height());
       this->setStyleSheet("QPushButton{border:0px}");
       this->setIcon(coinpic);

       this->setIconSize(QSize(coinpic.width(),coinpic.height()));
       //如果超出了最大值，重置为1
       if(this->min>this->max)
       {
           this->min = 1;
           this->isAnimation = false;
           timer1->stop(); //停止定时器
       }

    });


   //监听定时器2
    connect(timer2,&QTimer::timeout,[=]()
    {
       QPixmap  coinpix;

       QString path = QString(":/picture/res/Coin000%1.png").arg(max--);

       coinpix.load(path);


       //设置图标
       this->setFixedSize(coinpix.width(),coinpix.height());
       this->setStyleSheet("QPushButton{border:0px}");
       this->setIcon(coinpix);

       //设置图标大小
       this->setIconSize(QSize(coinpix.width(),coinpix.height()));

       if(this->max<this->min)
       {
           this->max = 8;
           this->isAnimation = false;
           timer2->stop();
       }


      });



 }


 //重写鼠标点击事件
 void MyCoin::mousePressEvent(QMouseEvent * e)
 {
     //如果正在翻转或者游戏已经结束则点击无效,
     if(this->isAnimation==true||this->isover==true)
     {
         return;
     }

     //播放声音
     coinflip->play();

     //返回原来的鼠标点击事件
     return QPushButton::mousePressEvent(e);


 }

 //改变flg标志位
 void  MyCoin::changeFlg()
 {
     //如果是正面
     if(flg==true)
     {
         //启动定时器
         timer1->start(100);

         //将isAnimation改为true
         this->isAnimation = true;
         flg = false;
     }else {
         timer2->start(100);
         this->isAnimation = true;
         flg = true;

     }

 }
