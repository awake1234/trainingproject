#include "ruledescribe.h"
#include<QLabel>
#include<QString>
#include<QIcon>
ruledescribe::ruledescribe(QWidget *parent) : QWidget(parent)
{

    this->setWindowTitle("规则介绍");

    //设置一个窗口图标
    QIcon icon(QPixmap(":/picture/res/Coin0001.png"));
    this->setWindowIcon(icon);


    // 添加一个标签写上规则介绍
   QLabel * ruleLabel = new QLabel(this);

   ruleLabel->setText("      规则介绍      \n1.点击钱币将会发生翻转\n以其为中中心的上下左右的钱币都会发生翻转\n2.当画面所有的钱币都变成金币时视为闯关成功\n");
   //设置字体大小
   QFont ft;
   ft.setPointSize(10);
   ft.setBold(true);
   ruleLabel->setFont(ft);

   //设置字体颜色
   QPalette pa;
   pa.setColor(QPalette::WindowText,Qt::red);    //参数1：一般的前景色：插入的颜色和背景色区分
   ruleLabel->setPalette(pa);

   ruleLabel->move(0,(this->height()-ruleLabel->height())*0.5);
}

//重写绘图事件
void ruledescribe::paintEvent(QPaintEvent *)
{
    //加载背景图
    QPixmap pix;
    pix.load(":/picture/res/PlayLevelSceneBg.png");
    QPainter painter(this);

    pix = pix.scaled(pix.width(),pix.height()*0.7);

    //固定界面的大小为图片的大小
    this->setFixedSize(pix.width(),pix.height());
    painter.drawPixmap(0,0,pix.width(),pix.height(),pix);


    //加载一个标题
    pix.load(":/picture/res/Title.png");
    painter.drawPixmap(0,0,pix.width(),pix.height(),pix);

}
