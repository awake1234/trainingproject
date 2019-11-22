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


     //加载声音文件
     winsound = new QSound(":/picture/res/LevelWinSound.wav");

     //初始化金币
     this->initCoin();





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


