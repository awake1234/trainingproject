#ifndef RANKLIST_H
#define RANKLIST_H

#include <QWidget>
#include "common.h"
#include <QNetworkAccessManager>
#include "logininfoinstance.h"
#include <QNetworkReply>
#include <QJsonArray>
#include <QLabel>
#include <QMovie>
namespace Ui {
class ranklist;
}

struct  RankFileInfo{
    QString filename;
    int pv;            //下载量
};

class ranklist : public QWidget
{
    Q_OBJECT

public:
    explicit ranklist(QWidget *parent = nullptr);
    ~ranklist();

    //设置tabwidget属性
    void initTabwidget();


    //显示共享文件列表
    void   refreshFiles();        //显示共享的文件列表
    void   clearshareFileList();  //清空共享文件列表
    void   getUserFileList();     //获取用户的文件列表
    void   refreshlist();         //更新图形界面列表
    QByteArray setFileListJson(int m_start,int m_count);//设置要发送的json格式包
    void getFileJsonInfo(QByteArray data);//解析文件列表json信息，存放在文件列表中

private:
    Ui::ranklist *ui;

    QNetworkAccessManager * m_manager;

    common m_common;

    int m_start;
    int m_count;
    long m_userFilesCount;

    QList<RankFileInfo *> m_list;   //文件列表
};

#endif // RANKLIST_H
