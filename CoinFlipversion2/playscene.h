#ifndef PLAYSCENE_H
#define PLAYSCENE_H

#include <QMainWindow>
#include<QPaintEvent>
#include "mycoin.h"
#include "dataconfig.h"
#include <QVector>
#include<QLabel>
#include<QSound>
//每关关卡显示类
class playscene : public QMainWindow
{
    Q_OBJECT
public:
    explicit playscene(QWidget *parent = nullptr);

    //重载构造函数,关卡下标作为参数
    playscene(int index);

    void paintEvent(QPaintEvent *);

    //初始化金币的图像
    void initCoin();

    int levelindex;

private:

    QVector<QVector<int>>gameArray;   //二维数组保存关卡数据


    //实现周围金币翻转功能
    MyCoin * CoinBtn[4][4];
    bool isWin=false;            //是否胜利

    QLabel * WinLabel;
    QLabel * Faillabel;

    //加载胜利声音
    QSound * winsound;
    QSound * backsound;            //返回按钮的声音
    QSound * timersound;           //倒计时结束的声音
    QSound * gameoversound;

    //设置一个倒计时器
    QTimer * countdown;    //倒计时

signals:
    void chooseSceneBack();

    void  gamewin();

public slots:
};

#endif // PLAYSCENE_H
