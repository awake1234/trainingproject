#ifndef MYCOIN_H
#define MYCOIN_H

#include <QWidget>
#include<QPushButton>
#include<QTimer>
#include<QMouseEvent>
#include<QSound>


class MyCoin : public QPushButton
{
    Q_OBJECT
public:
    //explicit表示只能显示调用构造函数，不能隐式转换
  // explicit MyCoin(QWidget *parent = nullptr);
   MyCoin(QString butImg);

   //创建改变flg函数执行翻转效果
   void changeFlg();
   //重写鼠标事件
   void mousePressEvent(QMouseEvent * e);


   QTimer * timer1;    //计时器1用来记录正面翻转
   QTimer * timer2;     //计时器2用来记录反面翻转

   int min = 1;    //记录最初的一个图标的下标
   int max = 8;     //记录最后面的一个图标的下标

   int posx;  //横坐标
   int posy;  //纵坐标
   bool flg; //正反标志

   bool isAnimation  = false;   //用来表示金币是否正在翻转中

   bool  isover = false;   //判断是否已经结束游戏
   //翻转的声音
   QSound * coinflip;



signals:

public slots:
};

#endif // MYCOIN_H
