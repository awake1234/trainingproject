#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QMouseEvent>

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

private:
    Ui::MainWindow *ui;

    //用于记录移动坐标
    QPoint dragposition;




protected slots:
    void mousePressEvent(QMouseEvent * ev);
    void mouseMoveEvent(QMouseEvent *event);
};

#endif // MAINWINDOW_H
