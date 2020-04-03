#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QMouseEvent>
#include <QClipboard>
#include "linkdownload.h"
#include "common.h"
#include <QString>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
      Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    //集中管理监听信号的函数
    void managesignals();
    void ShowMainWindow();
    void loginagain();  //重新登录函数
signals:
    void changeuser();

private:
    Ui::MainWindow *ui;

    //用于记录移动坐标
    QPoint dragposition;




protected slots:
    void mousePressEvent(QMouseEvent * ev);
    void mouseMoveEvent(QMouseEvent *event);
    //设置tabwidget界面
    void setmytabwig(int index);

};

#endif // MAINWINDOW_H
