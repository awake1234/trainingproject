#ifndef MYDISKWG_H
#define MYDISKWG_H

#include <QWidget>
#include "mymenu.h"
#include <QAction>
#include <QListWidget>
#include <QPoint>
#include<QFileDialog>
#include "uploadtask.h"
#include <QThread>


//我的网盘的widget

namespace Ui {
class mydiskwg;
}


class mydiskwg : public QWidget
{
    Q_OBJECT

public:
    explicit mydiskwg(QWidget *parent = nullptr);
    ~mydiskwg();

    //初始化filelistwidget文件列表
    void initlistwidget();

    //添加右键菜单的action
    void AddMenuAction();

    //添加上传文件的item
    void Adduploaditem(QString iconpath=":/ico/images/upload.png",QString text = "上传文件");

    //自定义文件打开窗口 不使用windows系统界面
    QStringList  fileopendialog();

public   slots:
    void rightMenu(const QPoint pos);

    //定义一个槽函数在主线程中插入进度条
    void addprogress(QString filename);

signals:
   //定义一个信号来切换传输的任务的界面
   //transferstatus为枚举类型在common.h中定义
   void switchto_transferui(transferstatus status);

private:
    Ui::mydiskwg *ui;

    //创建菜单
    mymenu * m_menu1; //点击文件时产生的菜单

    QAction * m_download;  //下载
    QAction * m_share;     //分享
    QAction * m_delete;    //删除
    QAction * m_property;  //属性


    mymenu * m_empty;    //点击空白处生成的菜单

    QAction *m_upload;   //上传文件
    QAction *m_refresh;  //刷新
    QAction *m_Download_ASC;//按下载量升序
    QAction *m_Download_Des; //按下载量降序

    QThread thread_adduploadfiles;  //生产者线程


    uploadtask * uploadtask;   //上传任务的实例
};

#endif // MYDISKWG_H
