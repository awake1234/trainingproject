#include "mainscene.h"
#include "ui_mainscene.h"
#include<QSound>

MainScene::MainScene(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainScene)
{
    ui->setupUi(this);

    //初始化场景对象
    choosescene = new chooselevelscene();

    //设置固定大小
    this->setFixedSize(450,588);
    //设置应用图片
    this->setWindowIcon(QPixmap(":/picture/res/Coin0001.png"));

    //设置窗口标题
    this->setWindowTitle("天天翻翻乐");

    //实现退出的功能
    connect(ui->actionexit,&QAction::triggered,
            [=]()
     {
         this->close();
     }
    );

    //添加一个规则介绍的下拉项
    QAction * describe = new QAction("规则介绍",this);

    ui->menu->addAction(describe);
    //创建一个规则介绍的对象
    ruledescribe * rule = new ruledescribe();

    //监听规则介绍按钮
    connect(describe,&QAction::triggered,[=]()
    {
       rule->show();
    }
    );




    //创建开始按钮
    MyPushButton * startBtn = new MyPushButton(":/picture/res/MenuSceneStartButton.png");
    //将按钮添加到这个窗口上
    startBtn->setParent(this);

   // startBtn->isclicked=true;

    //移动到一个合适位置
    startBtn->move(this->width()*0.5-startBtn->width()*0.5,this->height()*0.7);


    //加载开始按钮音效
    QSound *startsound = new QSound(":/picture/res/TapButtonSound.wav",this);
    //加载背景音乐
    QSound *backsound = new QSound(":/picture/res/backgroundsound.wav",this);

    //播放背景音乐,循环播放
    backsound->setLoops(-1);
    backsound->play();


    //按钮跳跃特效的实现
    connect(startBtn,&QPushButton::clicked,
            [=]()
            {
             //播放声音
              startsound->play();
              startBtn->zoom1();    //向上跳跃
              startBtn->zoom2();    //向下跳跃

              //将背景音乐关闭
              backsound->stop();

              //设置延时进入新的场景
              QTimer::singleShot(500,this,[=]()
              {
                 //隐藏当前窗口
                 this->hide();
                 //显示新的窗口
                 choosescene->show();

              });
            }
            );

    //当收到选择菜单的返回信号时显示主菜单界面
    connect(choosescene,&chooselevelscene::chooseSceneBack,[=](){this->show();backsound->play();});


}

//重写绘图事件
void MainScene::paintEvent(QPaintEvent *)
{
    //创建一个painter指定绘图设备
    QPainter painter(this);

    //加载图片
    QPixmap background(":/picture/res/PlayLevelSceneBg.png");

    //绘制背景图
    painter.drawPixmap(0,0,width(),height(),background);

    //创建一个标题的图片对象
    QPixmap titlepix;

    //加载标题
    titlepix.load(":/picture/res/Title.png");

    //缩放标题
    titlepix = titlepix.scaled(titlepix.width()*0.8,titlepix.height()*0.8);

    //绘制标题
    painter.drawPixmap(10,30,titlepix.width(),titlepix.height(),titlepix);


}

MainScene::~MainScene()
{
    delete ui;
    delete choosescene;
    choosescene = nullptr;
}
