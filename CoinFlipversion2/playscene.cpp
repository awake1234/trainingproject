#include "playscene.h"
#include<QMenu>
#include<QMenuBar>
#include<QPixmap>
#include<QPainter>
#include "mypushbutton.h"
#include<QTimer>
#include<QLabel>
#include<QFont>
#include<QString>
#include<QDebug>
#include <synchapi.h>
playscene::playscene(QWidget *parent) : QMainWindow(parent)
{

}


//场景的构造函数
playscene::playscene(int index):levelindex(index)
{

    this->setFixedSize(450,588);

    this->setWindowIcon(QPixmap(":/picture/res/Coin0001.png"));

    this->setWindowTitle("翻翻翻");

    //加载返回按钮声音
    backsound = new QSound(":/picture/res/BackButtonSound.wav");

    //加载计时结束的按钮
    timersound = new QSound(":/picture/res/countdown.wav");

    //加载游戏结束音效
    gameoversound = new QSound(":/picture/res/gameover.wav");

    //创建菜单栏
    QMenuBar * bar = this->menuBar();
    this->setMenuBar(bar);

    //创建菜单
    QMenu * menu = bar->addMenu("菜单");

    //创建退出项
    QAction * quitAction = menu->addAction("退出");

    //监听退出选项
    connect(quitAction,&QAction::triggered,[=](){
        this->close();
    });

     //创建返回按钮
     MyPushButton * backBtn = new MyPushButton(":/picture/res/BackButton.png",":/picture/res/BackButtonSelected.png");

     backBtn->setParent(this);

     backBtn->move(this->width()-backBtn->width(),this->height()-backBtn->height());

     //点击返回发送一个自定义的返回信号
     connect(backBtn,&MyPushButton::clicked,[=]()
     {
        QTimer::singleShot(500,this,[=]()
        {
            this->hide();
            backsound->play();             //播放按钮返回的声音
            emit this->chooseSceneBack();
        }
                           );
     });


     //在页面下方显示当前关卡
     QLabel *label = new QLabel(this);
     QFont font;
     font.setFamily("华文新魏");
     font.setPointSize(20);
     label->setFont(font);
     QString str = QString("Level:%1").arg(this->levelindex);
     label->setText(str);
     label->setGeometry(QRect(30,this->height()-50,120,50));   //设置位置和大小


     //创建金币的背景图片
     for(int i = 0;i<4;i++)
     {
         for(int j =0;j<4;j++)
         {
             //绘制背景图片
             QLabel *label = new QLabel;
             label->setGeometry(0,0,60,60);
             //金币的背景
             label->setPixmap(QPixmap(":/picture/res/BoardNode.png"));
             label->setParent(this);
             label->move(115+i*50,200+j*50);

         }
    }


     //初始化游戏数组
     dataConfig config;
     gameArray = config.mData[this->levelindex];

     //初始化胜利的图片
     WinLabel = new QLabel;
     QPixmap temppix;
     temppix.load(":/picture/res/LevelCompletedDialogBg.png");
     WinLabel->setGeometry(0,0,temppix.width(),temppix.height());
     WinLabel->setPixmap(temppix);
     WinLabel->setParent(this);
     //居中显示 放在界面的上方
     WinLabel->move((this->width()-temppix.width())*0.5,-temppix.height());



     //初始化失败的图片
     Faillabel = new QLabel;
     QPixmap failpix;
     failpix.load(":/picture/res/failed.png");
     Faillabel->setGeometry(0,0,failpix.width(),failpix.height());
     Faillabel->setPixmap(failpix);
     Faillabel->setParent(this);
     //居中显示 放在界面的上方
     Faillabel->move((this->width()-failpix.width())*0.5,-failpix.height());


     //加载声音文件
     winsound = new QSound(":/picture/res/LevelWinSound.wav");

     //初始化金币
     this->initCoin();

    //给计时器的图片设置一个标签
     MyPushButton * timerbutton = new MyPushButton(":/picture/res/LevelIcon.png");
     timerbutton->setParent(this);
     //将按钮移动到右上角
     timerbutton->move(this->width()-2*timerbutton->width(),timerbutton->height());

     //创建一个标签显示数字
     QLabel * timerlabel = new QLabel;


     //设置字体的颜色
     QPalette pa;
     pa.setColor(QPalette::WindowText,Qt::red);
     timerlabel->setPalette(pa);

     timerlabel->setParent(timerbutton);

     //每升一关加五秒
     timerlabel->setText(QString::number(10+(levelindex-1)*5));
     timerlabel->setFixedSize(QSize(timerbutton->width(),timerbutton->height()));
     timerlabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);   //居中显示

     //设置字体格式，1，字体，2，大小，3，75代表粗体
     QFont timerfont ("华文新魏", 10, 75);
     timerlabel->setFont(timerfont);


     countdown = new QTimer(this);
     //只要一初始化就要开始启动定时器
     countdown->start(1000);    //每隔一秒发送一次信号


     //监听计时器
     connect(countdown,&QTimer::timeout,[=]()
     {
        //将当前时间减一秒
        int curtime = timerlabel->text().toInt()-1;
        //如果计时结束或者已经胜利
        if(curtime==0)
        {

            //结束计时
            countdown->stop();

            //显示失败图片
            QPropertyAnimation * animation = new QPropertyAnimation(Faillabel,"geometry");
            animation->setDuration(1000);    //设置动画播放时长
            animation->setStartValue(QRect(Faillabel->x(),Faillabel->y(),Faillabel->width(),
                                           Faillabel->height()));
            animation->setEndValue(QRect(Faillabel->x(),Faillabel->y()+114,Faillabel->width(),
                                         Faillabel->height()));

            animation->setEasingCurve(QEasingCurve::OutBounce);   //设置弹跳效果
            animation->start();

            //改变所有按钮的判断游戏是否已经结束的标志位
            for(int i =0;i<4;++i)
            {
                for(int j = 0;j<4;++j)
                {
                   CoinBtn[i][j]->isover = true;
                }
            }

            //播放音效
            timersound->play();

            //播放失败音效
            gameoversound->play();


            //设置播放时间为2秒
            QTimer::singleShot(2000,this,[=](){timersound->stop();});
        }
        //如果胜利了
        if(isWin==true)
        {
            //停止计时
            countdown->stop();
        }

        timerlabel->setText(QString::number(curtime));
        //刷新一下界面,
        this->update();

      });


}

//初始化金币的图像
 void playscene::initCoin()
 {
     QString img;

     for(int i =0;i<4;i++)
     {
         for(int j =0;j<4;j++)
         {
             //如果是1则显示金币
            if(gameArray[i][j]==1)
            {
                img = ":/picture/res/Coin0001.png";
            }else {
                //否则加载银币
                img = ":/picture/res/Coin0008.png";
             }

            MyCoin * coin = new MyCoin(img);
            coin->setParent(this);
            coin->move(115+i*50,200+j*50);
            coin->posx = i;
            coin->posy = j;
            coin->flg = gameArray[i][j];
            CoinBtn[i][j] = coin;    //将这个值添加到数组中去

            //监听点击事件
            connect(coin,&QPushButton::clicked,[=]()
            {
                coin->changeFlg();
                gameArray[coin->posx][coin->posy]=gameArray[coin->posx][coin->posy]==0?1:0;

                //设置一个定时器,这个时候应该是已经全部创建好了
                QTimer::singleShot(30,this,[=]()
                {
                    if(coin->posx-1>=0)
                    {
                        CoinBtn[coin->posx-1][coin->posy]->changeFlg();
                        gameArray[coin->posx-1][coin->posy]=gameArray[coin->posx-1][coin->posy]==0?1:0;
                    }

                    if(coin->posx+1<=3)
                    {
                        CoinBtn[coin->posx+1][coin->posy]->changeFlg();
                        gameArray[coin->posx+1][coin->posy]=gameArray[coin->posx+1][coin->posy]==0?1:0;
                    }
                    if(coin->posy-1>=0)
                    {
                        CoinBtn[coin->posx][coin->posy-1]->changeFlg();
                        gameArray[coin->posx][coin->posy-1]=gameArray[coin->posx][coin->posy-1]==0?1:0;
                    }
                    if(coin->posy+1<=3)
                    {
                        CoinBtn[coin->posx][coin->posy+1]->changeFlg();
                        gameArray[coin->posx][coin->posy+1]=gameArray[coin->posx][coin->posy+1]==0?1:0;
                    }

                    //默认胜利标志为true;
                    this->isWin = true;

                    //判断是否已经胜利
                    for(int i = 0;i<4;i++)
                    {
                        for(int j = 0;j<4;j++)
                        {
                            if(CoinBtn[i][j]->flg==false)
                            {
                                this->isWin = false;
                                break;
                            }
                        }
                     }

                    //如果胜利了
                    if(this->isWin)
                    {
                        //改变所有按钮的判断游戏是否已经结束的标志位
                        for(int i =0;i<4;++i)
                        {
                            for(int j = 0;j<4;++j)
                            {
                               CoinBtn[i][j]->isover = true;
                            }
                        }

                        QPropertyAnimation * animation = new QPropertyAnimation(WinLabel,"geometry");
                        animation->setDuration(1000);    //设置动画播放时长
                        animation->setStartValue(QRect(WinLabel->x(),WinLabel->y(),WinLabel->width(),
                                                       WinLabel->height()));
                        animation->setEndValue(QRect(WinLabel->x(),WinLabel->y()+114,WinLabel->width(),
                                                     WinLabel->height()));
                        animation->setEasingCurve(QEasingCurve::OutBounce);   //设置弹跳效果
                        animation->start();

                        //播放声音
                        winsound->play();


                        //发送一个游戏胜利的信号
                        emit  this->gamewin();


                        //延迟三秒显示下一个界面
                        QTimer::singleShot(3000,this,[=]()
                        {
                            playscene * newscene = new playscene(levelindex+1);
                            //显示下一个场景
                            newscene->show();

                            this->hide();           //隐藏当前界面

                            //每一个新的场景发送返回消息给上一个场景 以次类推直到返回到最初的界面
                            connect(newscene,&playscene::chooseSceneBack,[=]() mutable
                            {
                                emit this->chooseSceneBack();

                                delete  newscene;
                                newscene = nullptr;
                            });

                        });




                    }
                });

          });
       }
     }
}



//设置场景
void playscene::paintEvent(QPaintEvent *)
{

  QPainter painter(this);
  QPixmap pix;
  pix.load(":/picture/res/PlayLevelSceneBg.png");
  painter.drawPixmap(0,0,this->width(),this->height(),pix);

  //加载标题
  pix.load(":/picture/res/Title.png");
  pix = pix.scaled(pix.width()*0.5,pix.height()*0.5);   //将标题的宽和高各缩小一半
  painter.drawPixmap(10,30,pix.width(),pix.height(),pix);


 }


