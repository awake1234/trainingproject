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
#include "consumerthread_file.h"
#include<QJsonArray>
#include "fileproperty.h"
#include "downloadtask.h"




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

    /********item相关*****/
    //初始化filelistwidget文件列表
    void initlistwidget();
    //添加右键菜单的action
    void AddMenuAction();
    //自定义文件打开窗口 不使用windows系统界面
    QStringList  fileopendialog();
    //清除所有的item
     void clearitems();
    //处理选中文件item所做的操作
    void dealselectedfile(QString cmd);


    /******细分到每个功能的具体操作*******/
    void deal_property(FileInfo * info);   //属性操作
    void deal_share(FileInfo  *info);      //分享操作
    void deal_delete(FileInfo * info);     //删除操作
    void deal_pv(FileInfo *info);          //下载操作
    void downloadfileAction();             //下载文件的过程
    void dealFilepv(QString md5,QString filename);  //处理服务器端数据库相关数据的修改
    void deal_sharelink(FileInfo * info);    //分享链接的操作
    //设置处理文件的发送的json包
    QByteArray setdealfilejson(QString username,QString token,QString md5,QString filename);



    /******上传文件的操作******/
    //添加上传文件的item
    void Adduploaditem(QString iconpath=":/ico/images/upload.png",QString text = "上传文件");
    void adduploadfiles();
    int  appendtolist(QString filepath);


    /*****显示用户的文件列表******/
    //用来展示文件排列的枚举类型
    //normal:普通用户列表
    //pvASC按下载量升序排序
    //pvDES按下载量降序
    enum DISPLAY{normal,pvASC,pvDES};
    void getuserfilelist(DISPLAY cmd);                  //获取用户的文件列表
    void refreshFileItems();//显示文件Item
    QByteArray setfileslistjson(QString user,QString token,int start,int count);
    void getFileJsonInfo(QByteArray data);    //获取返回的文件信息
    void getuserfilecount(DISPLAY cmd=normal);       //先查询用户文件的数量并且去获得用户文件信息
    QByteArray setcountjson(QString user,QString token);  //设置查询用户文件数量的json包
    QStringList getcountstatus(QByteArray json);    //得到服务器返回的json文件
    void clearfilelist();               //清空文件列表
    void getuserspace();                //获取用户容量

    //清除所有的任务
    void clearAllTask();


    //维护图片列表
    QList<FileInfo *>  m_fileList;                  //维护一个保存文件信息的列表
    QList<FileInfo * > m_picList;                  //维护一个图片信息的列表
    //公有成员变量
    QAction *m_upload;   //上传文件
    qint64 usedspace = 0;      //用户容量的大小

public   slots:
    void rightMenu(const QPoint pos);
    //定义一个槽函数来刷新进度条的值
    void update_progress_value(qint64,qint64,dataprocess *);
    //定义槽函数删除已经上传的文件的进度条
    void delete_finishedfile();

    //得到filelistwidget的指针
    QListWidget * getfilelistwidget();

signals:
   //定义一个信号来切换界面
   //transferstatus为枚举类型在common.h中定义
   void switchto_ui(uistatus status);
   void startproducer();
   void startconsumer();
   void loginAgainSignal();
   void getusedspace();

private:


    Ui::mydiskwg *ui;

    //创建菜单
    mymenu * m_menu1; //点击文件时产生的菜单
    QAction * m_download;  //下载
    QAction * m_share;     //分享
    QAction * m_delete;    //删除
    QAction * m_property;  //属性


    mymenu * m_empty;    //点击空白处生成的菜单

    QAction *m_refresh;  //刷新
    QAction *m_Download_ASC;//按下载量升序
    QAction *m_Download_Des; //按下载量降序
    QAction *m_sharelink;    //分享文件链接给别人

    /*线程*/
    QThread * thread_adduploadfiles;  //生产者线程
    QVector <QThread *> thread_consume_uploadfiles; //消费者线程数组
    QVector <consumerthread_file *> consumer_thread;  //消费者线程处理工作的对象指针数组
    QThread * thread_consumer;

    //上传任务的实例
    uploadtask * uploadtask;
    dataprocess *m_process;



    //消费者
    int thread_num = 3;  //定义一个消费者线程数，默认为3，后期加上UI操作

    //用户文件数量
    long m_userfilecount;   //用户文件数量
    int  m_start;    //文件位置起点
    int  m_count;    //每次请求文件的数量

    QNetworkAccessManager *m_manager;
    common m_common;




};

#endif // MYDISKWG_H
