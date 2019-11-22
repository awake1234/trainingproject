#ifndef CHOOSELEVELSCENE_H
#define CHOOSELEVELSCENE_H

#include <QMainWindow>
#include<QPaintEvent>
#include<QPainter>
#include"playscene.h"
#include "mypushbutton.h"
#include<QSound>

class chooselevelscene : public QMainWindow
{
    Q_OBJECT
public:
    explicit chooselevelscene(QWidget *parent = nullptr);

    //重写绘图事件
    void paintEvent(QPaintEvent *);

    //创建关卡选择按钮
    void CreateLevel();

signals:
    void  chooseSceneBack();
private:
     //关卡显示类的指针
     playscene * pscene;
     QSound * choosesound;
     QSound * backsound;

public slots:
};

#endif // CHOOSELEVELSCENE_H
