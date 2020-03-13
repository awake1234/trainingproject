#ifndef COMMON_H
#define COMMON_H
#include <QRegExp>
#include<QJsonDocument>
#include<QJsonObject>
#include<QJsonValue>
#include<QFile>
#include<QMessageBox>
#include<QMap>
#include<QVariant>
#include<QDebug>
#include<QNetworkAccessManager>
#include "des.h"
#include "base64.h"
#include<QCryptographicHash>
#include <qdatetime.h>
#include <QDir>
#include <QListWidgetItem>

//定义几个用于正则表达式匹配的宏
#define USER_MATCH      "^[a-zA-Z][a-zA-Z0-9_]{7,15}$"
#define Nickname_MATCH  "^[a-zA-Z][a-zA-Z0-9_]{4,15}$"
#define PassWord_MATCH  "^[a-zA-Z]\\w{5,17}$"
#define Phone_MATCH     "^(13[0-9]|14[5|7]|15[0|1|2|3|5|6|7|8|9]|18[0|1|2|3|5|6|7|8|9])\\d{8}$"
#define EMail_MATCH     "^\\w+([-+.]\\w+)*@\\w+([-.]\\w+)*\\.\\w+([-.]\\w+)*$"
#define IP_MATCH        "((?:(?:25[0-5]|2[0-4]\\d|[01]?\\d?\\d)\\.){3}(?:25[0-5]|2[0-4]\\d|[01]?\\d?\\d))"
#define PORT_MATCH      "^[1-9]$|(^[1-9][0-9]$)|(^[1-9][0-9][0-9]$)|(^[1-9][0-9][0-9][0-9]$)|(^[1-6][0-5][0-5][0-3][0-5]$)"


#define CONFILE_PATH  "conf/config.json"    //定义保存配置文件的路径
#define FILETYPE_DIR  "conf/filetype"  //存放文件类型图片的目录
#define RECORD_DIR    "conf/record/"     //保存用户上传下载文件记录的目录

//文件信息结构体
struct FileInfo
{
    QString md5;       //文件md5
    QString filename; //文件名
    QString user;    //用户
    QString time;    //上传时间
    QString url;     //文件url
    QString type;    //文件类型
    qint64  size;    //文件大小
    int shareStatus; //是否共享 1共享 0不共享
    int pv;          //下载量

    QListWidgetItem *item;  //filelistwidget的item
};





//定义一个枚举来表示操作的状态
enum uistatus{upload,download,record,share};

class common
{
public:
    common();

    //定义一个得到配置文件当中指定的key值的value值的函数，给定参数三一个默认值
    QString getcfgValue(QString title,QString key,QString cfg_path=CONFILE_PATH);

    //封装一个正则匹配字符串的函数，参数1表示要匹配的类型，参数2 ：要匹配的字符串
    bool isMatch(QString pattern,QString src);

    //封装一个保存服务器信息的函数,参数3：保存的配置文件的路径
    void  savewebinfo(QString ip,QString port,QString conf_file_path);
    //提供一个将用户名和密码加密之后写入到json文件的函数
    void  savelogininfo(QString username,QString password,bool isremember,QString conf_file_path);

    //将要发送的数据按分装成JSON格式的字符串
    QByteArray packinfo(QString username,QString nickname,QString password,QString phonenum,QString emailaddress);

    //提供一个获得静态变量manager的接口
    QNetworkAccessManager * getmanager();

    //得到文件的md5值的函数
    QString getfilemd5(QString filepath);

    //得到服务器回复的状态码
    QString getcode(QByteArray json);

    //制作分隔线
    QString getBoundry();

    //保存传输记录到本地文件
    void writeRecord(QString user,QString name,QString code,QString path=RECORD_DIR);

    //得到文件后缀，去conf/filetype中寻找对应的图片，如果没有，使用other.png
    QString getFileType(QString filetype);

    //得到文件类型列表
    void getFileTypeList();

    static QStringList  m_typelist;  //可以找的文件类型列表
 private:
    static QNetworkAccessManager * m_manager;   //保证只有一个manager对象

    static QString m_typepath;  //需要返回的文件类型的路径

};

#endif // COMMON_H
