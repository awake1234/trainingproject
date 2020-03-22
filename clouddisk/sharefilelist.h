#ifndef SHAREFILELIST_H
#define SHAREFILELIST_H

#include <QWidget>
#include "mymenu.h"
#include "common.h"
#include "fileproperty.h"
#include "logininfoinstance.h"
#include <QNetworkReply>
#include <QJsonArray>
#include "fileproperty.h"
#include "des.h"
#include "downloadtask.h"
#include "downloadlayout.h"
#include <QFileDialog>
#include<QTimer>

namespace Ui {
class sharefilelist;
}

class sharefilelist : public QWidget
{
    Q_OBJECT

public:
    explicit sharefilelist(QWidget *parent = nullptr);
    ~sharefilelist();

    enum CMD{property,cancel,sharefilesdownload,save};   //鼠标右键文件可以做的操作

    /*******界面操作*******/
    void initlistwidget();   //初始化listwidget的属性
    void AddMenuAction();    //添加右键菜单的按钮
    void clearItems();      //清除所有的items
    void refreshItems();    //刷新所有的items


    /*******业务操作*******/
    void refreshFiles();         //刷新文件
    void clearshareFileList();  //清除共享文件列表
    void getuserFileList();     //获得用户的文件列表
    void getfilejsoninfo(QByteArray data);  //解析得到的文件信息


    /****鼠标右键文件可以使用的操作*****/
    void DealSelectedFile(CMD cmd);   //抽象的处理函数
    void showproperty(FileInfo * info);  //显示属性
    void cancelshare(FileInfo * info);  //取消分享
    void savefiletolist(FileInfo *info); //转存功能
    void adddownloadFiles(FileInfo * info);  //下载功能
    void downloadFilesAction();           //处理下载任务的实际操作
    void dealFilePv(QString md5,QString filename);
    /********设置json文件包******/
    QByteArray SetShaerfilejson(int m_start,int m_count);
    QByteArray SetCancelfilejson(QString user,QString md5,QString filename);
signals:
    void gototransfer(uistatus  status);   //发送一个信号转到下载页面
private:
    mymenu * m_menuEmpty;       //右键空白处的菜单
    QAction * m_refreshAction;  //刷新


    mymenu  *  m_menuItem;       //右键item的菜单
    QAction *  m_downloadAction;  //下载
    QAction *  m_propertyAction;   //属性
    QAction *  m_cancleAction;     //取消分享
    QAction *  m_saveAction;       //转存文件

    QNetworkAccessManager * m_manager;
    common m_common;

    int m_start;     //每次请求文件的开始位置
    int m_count;     //每次请求文件的数量
    long m_userFileCount;   //用户文件的数量
    QList<FileInfo *> m_shareFileList;  //保存共享文件信息的列表

    QTimer m_downloadTimer;   //定时检查下载队列是否有任务下载
    Ui::sharefilelist *ui;
};

#endif // SHAREFILELIST_H
