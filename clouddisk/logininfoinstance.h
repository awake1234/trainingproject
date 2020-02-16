#ifndef LOGININFOINSTANCE_H
#define LOGININFOINSTANCE_H

#include <QString>
#include "common.h"
class logininfoinstance
{
public:
    static logininfoinstance *getinstance();  //对外提供一个获得实例的方法，要设置成静态方法 
    
    //设置注册信息
    void setlogininfo(QString tempuser,QString tempip,QString tempport,QString temptoken="");
    
    //加const 表示该函数不会修改类中的成员变量，只是做一个返回成员的作用
    QString getuser() const;
    QString gettoken() const;
    QString getip() const;
    QString getport() const;
    
    
    
private:
    //构造函数和析构函数为私有
    logininfoinstance();
    ~logininfoinstance();
    
    //复制构造函数和=号操作符也设置为私有防止被复制
    logininfoinstance(const logininfoinstance &);
    logininfoinstance operator=(const logininfoinstance &);
    
    //定义一个静态数据成员，类中声明，类外必须定义
    static logininfoinstance *instance;
    
    QString user;
    QString token;
    QString ip;
    QString port;
    
};

#endif // LOGININFOINSTANCE_H
