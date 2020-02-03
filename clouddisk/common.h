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
//定义几个用于正则表达式匹配的宏
#define USER_MATCH      "^[a-zA-Z][a-zA-Z0-9_]{7,15}$"
#define Nickname_MATCH  "^[a-zA-Z][a-zA-Z0-9_]{4,15}$"
#define PassWord_MATCH  "^[a-zA-Z]\\w{5,17}$"
#define Phone_MATCH     "^(13[0-9]|14[5|7]|15[0|1|2|3|5|6|7|8|9]|18[0|1|2|3|5|6|7|8|9])\\d{8}$"
#define EMail_MATCH     "^\\w+([-+.]\\w+)*@\\w+([-.]\\w+)*\\.\\w+([-.]\\w+)*$"
#define IP_MATCH        "((?:(?:25[0-5]|2[0-4]\\d|[01]?\\d?\\d)\\.){3}(?:25[0-5]|2[0-4]\\d|[01]?\\d?\\d))"
#define PORT_MATCH      "^[1-9]$|(^[1-9][0-9]$)|(^[1-9][0-9][0-9]$)|(^[1-9][0-9][0-9][0-9]$)|(^[1-6][0-5][0-5][0-3][0-5]$)"


#define CONFILE_PATH "conf/config.json"    //定义保存配置文件的路径
#define FILETYPE_DIR    "conf/filetype"  //存放文件类型图片的目录
#define RECORD_DIR    "conf/record/"     //保存用户上传下载文件记录的目录



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



 private:
    static QNetworkAccessManager * m_manager;   //保证只有一个manager对象


};

#endif // COMMON_H
