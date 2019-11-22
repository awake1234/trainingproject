#include "chooselevelscene.h"
#include<QPixmap>
#include<QMenuBar>

#include<QTimer>
#include<QLabel>

chooselevelscene::chooselevelscene(QWidget *parent) : QMainWindow(parent)
{
    //场景指针初始化为空
    pscene=nullptr;
    //设置一个固定的大小
    this->setFixedSize(450,588);
    //设置标题
    this->setWindowTitle("关卡选择");
    //加一个图标
    this->setWindowIcon(QPixmap(":/picture/res/Coin0001.png"));
    //加载返回按钮声音
    backsound = new QSound(":/picture/res/BackButtonSound.wav");
    //创建一个菜单栏可供退出程序
    QMenuBar * bar = this->menuBar();
    this->setMenuBar(bar);
     //创建菜单
     QMenu *menu = bar->addMenu("菜单");
    //创建按钮菜单项
     QAction * quitAction = menu->addAction("退出");
     //点击关闭窗口
     connect(quitAction,&QAction::triggered,[=]()
     {
        this->close();
     });

     //注意这里不能写到绘图事件函数中
     //创建一个back按钮
     MyPushButton * backBtn = new MyPushButton(":/picture/res/BackButton.png",":/picture/res/BackButtonSelected.png");

     //将按钮添加到背景上面
     backBtn->setParent(this);

     //将按钮放到右下角
     backBtn->move(this->width()-backBtn->width(),this->height()-backBtn->height());


     //给返回按钮添加功能
     connect(backBtn,&QPushButton::clicked,[=]()
            {
             QTimer::singleShot(500,this,[=]()
             {
                 this->hide();
                 //发送一个自定义的信号

                 //播放声音
                 backsound->play();

                 emit this->chooseSceneBack();
             });
           });


     //创建选关按钮
     CreateLevel();

     //加载按按钮声音
     choosesound = new QSound(":/picture/res/TapButtonSound.wav");



}

//设置背景
void chooselevelscene::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    //加载一张图片
    QPixmap chooseback;

    chooseback.load(":/picture/res/PlayLevelSceneBg.png");

     //画上这个图
    painter.drawPixmap(0,0,this->width(),this->height(),chooseback);

    //添加一个标题ICON
    QPixmap titlepix;
    titlepix.load(":/picture/res/Title.png");

    //把标题滑道中间
    painter.drawPixmap((this->width()-titlepix.width())*0.5,30,titlepix.width(),titlepix.height(),titlepix);

}



//创建关卡函数
void chooselevelscene::CreateLevel()
{

    //共创建20个关卡
    for(int i = 0;i<20;i++)
    {
        MyPushButton * levelbtn = new MyPushButton(":/picture/res/LevelIcon.png");

        levelbtn->setParent(this);

        //四行五列
        levelbtn->move(50+(i%4)*90,130+(i/4)*80);

        //显示按钮上的文字
        QLabel * label = new QLabel;
        label->setParent(levelbtn);
        label->setFixedSize(levelbtn->width(),levelbtn->height());
        label->setText(QString::number(i+1));
        label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);   //居中显示
        //label->move(50+(i%4)*90,130+(i/4)*80);
        label->setAttribute(Qt::WA_TransparentForMouseEvents,true);  //鼠标事件穿透


        //监听每一个选择关卡的按钮
        connect(levelbtn,&MyPushButton::clicked,[=](){
             //游戏场景不要复用
             if(pscene==nullptr)
             {
                this->hide();

                //播放选择关卡的声音
                choosesound->play();

                //将选择的关卡号传入场景构造函数中
                pscene = new playscene(i+1);
                pscene->show();

                //监听关卡返回按钮
               connect(pscene,&playscene::chooseSceneBack,[=]()
               {
                   //显示场景选择界面
                   this->show();
                   //播放返回按钮声音
                   backsound->play();
                   //删除场景展现界面指针
                   delete  pscene;
                   //置为空指针
                   pscene = nullptr;
               }
             );

               }


         });

    }

}

