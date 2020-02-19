#include "mainwindow.h"
#include <QApplication>
#include "login.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.show();


    //显示一下登录的界面
     //login h;
     //h.show();

    return a.exec();
}
