#ifndef MAINSCENE_H
#define MAINSCENE_H

#include <QMainWindow>
#include<QPixmap>
#include<QPaintEvent>
#include<QPainter>
#include<QDebug>
#include"mypushbutton.h"
#include "chooselevelscene.h"
#include<QTimer>
#include<QSound>
#include "ruledescribe.h"

namespace Ui {
class MainScene;
}

class MainScene : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainScene(QWidget *parent = nullptr);
    ~MainScene();

    //重写绘图事件
    void paintEvent(QPaintEvent *);

private:
    Ui::MainScene *ui;

    //创建一个选择场景的对象
    chooselevelscene * choosescene;
};

#endif // MAINSCENE_H
