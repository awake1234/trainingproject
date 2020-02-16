#include "logininfoinstance.h"


//类外定义静态成员
logininfoinstance *  logininfoinstance::instance = new logininfoinstance();

logininfoinstance::logininfoinstance()
{
    
    
    
}

//得到这个实例
logininfoinstance *logininfoinstance::getinstance()
{
    return instance;
}
//设值注册信息
void logininfoinstance::setlogininfo(QString tempuser, QString tempip, QString tempport, QString temptoken)
{
    user = tempuser;
    ip = tempip;
    port = tempport;
    token = temptoken;
}

QString logininfoinstance::getuser() const
{
    return user;
}

QString logininfoinstance::gettoken() const
{
    return token;
}

QString logininfoinstance::getip() const
{
    return ip;
}

QString logininfoinstance::getport() const
{
    return port;
}


logininfoinstance::logininfoinstance(const logininfoinstance &)
{
    
}

//重载等号操作符
logininfoinstance logininfoinstance::operator=(const logininfoinstance &)
{
    //*this返回的还是当前的对象，可以避免被复制
    return *this;

}


