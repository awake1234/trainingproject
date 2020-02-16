#ifndef MYDISKWG_H
#define MYDISKWG_H

#include <QWidget>
#include "mymenu.h"
#include <QAction>
#include <QListWidget>
#include <QPoint>
#include<QFileDialog>
#include "uploadtask.h"

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

    void adduploadfiles();

public   slots:
    void rightMenu(const QPoint pos);

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

};

#endif // MYDISKWG_H
