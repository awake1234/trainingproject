#include "common.h"




//给manager申请空间
QNetworkAccessManager * common::m_manager = new QNetworkAccessManager();

common::common()
{

}

//得到配置文件中指定的键所对应的值
QString common::getcfgValue(QString title, QString key, QString cfg_path)
{
    QFile file(cfg_path);
    file.open(QIODevice::ReadOnly);   //以只读方式打开
    QString value = nullptr;
    QByteArray Content_Array = file.readAll();   //读出所有内容保存到array中
    file.close();
    //将array转化成QjsonDocumet对象
    QJsonDocument root = QJsonDocument::fromJson(Content_Array);

    //解析root
    //如果root不为空
    if(root.isObject()==true)
    {
         QJsonObject par = root.object();

         QJsonObject sub = par.value(title).toObject();


         //得到值
         value = sub.value(key).toString();

    }

    return  value;
}

//判断输入内容是否匹配
bool common::isMatch(QString pattern, QString src)
{
    //判空
    if(pattern==nullptr||src==nullptr)
    {
        return false;
    }

    //创建一个正则匹配的对象
    QRegExp reg;
    //设置要匹配的类型
    reg.setPattern(pattern);
    if(reg.exactMatch(src)==true)
    {
        return true;
    }

    return false;
}

//保存web信息文件
void common::savewebinfo(QString ip, QString port, QString conf_file_path)
{
    //读入配置文件
    QFile file(conf_file_path);
    bool ret = file.open(QIODevice::ReadOnly);
    if(ret==false)
    {
         QMessageBox::warning(nullptr,"warning","open conf file error");
    }

    //读到内存中进行处理
    QByteArray data = file.readAll();
    //关闭文件
    file.close();
    //转换成QJsonDocument文件
    QJsonDocument doc = QJsonDocument::fromJson(data);

    //判断是否是object文件，即判断里面是否有内容
    //如果没有内容
    if(doc.isObject()==false)
    {

        //先保存到内存容器中
        QMap<QString,QVariant> mylogin;
        mylogin.insert("isremember","");
        mylogin.insert("pwd","");
        mylogin.insert("user","");


        //保存到容器中
        QMap<QString,QVariant> myfiletype_path;
        myfiletype_path.insert("path","");

        //创建一个保存web服务器的文件的信息
        QMap<QString,QVariant> mywebinfo;
        mywebinfo.insert("ip",QVariant(ip));
        mywebinfo.insert("port",QVariant(port));

        //全部合并成一个大的map
        QMap<QString,QVariant> total;
        total.insert("login",mylogin);
        total.insert("filetype_path",myfiletype_path);
        total.insert("web_server",mywebinfo);

        //打开文件，重新写入配置文件
        file.open(QIODevice::WriteOnly);

        //可以直接用map生成json文件
        doc = QJsonDocument::fromVariant(total);
        data = doc.toJson();
        file.write(data);

        //关闭文件
        file.close();

    }else{
        //如果有内容
        //读取文件内容
        //最大的一个object文件
        QJsonObject par = doc.object();

        //小的login的object
        QJsonObject login = par.value("login").toObject();
        QString  user = login.value("usr").toString();
        QString  isremember = login.value("isremember").toString();
        QString  pwd= login.value("pwd").toString();


        //先保存到内存容器中
        QMap<QString,QVariant> mylogin;
        mylogin.insert("isremember",QVariant(isremember));
        mylogin.insert("pwd",QVariant(pwd));
        mylogin.insert("user",QVariant(user));

        //保存可以上传的文件内容的文件的路径
        QJsonObject filetype_path = par.value("filetype_path").toObject();
        QString  path = filetype_path.value("path").toString();

        //保存到容器中
        QMap<QString,QVariant> myfiletype_path;
        myfiletype_path.insert("path",QVariant(path));

        //创建一个保存web服务器的文件的信息
        QMap<QString,QVariant> mywebinfo;
        mywebinfo.insert("ip",QVariant(ip));
        mywebinfo.insert("port",QVariant(port));

        //全部合并成一个大的map
        QMap<QString,QVariant> total;
        total.insert("login",mylogin);
        total.insert("filetype_path",myfiletype_path);
        total.insert("web_server",mywebinfo);

        //打开文件，重新写入配置文件
        file.open(QIODevice::WriteOnly);

        //可以直接用map生成json文件
        doc = QJsonDocument::fromVariant(total);
        data = doc.toJson();
        file.write(data);

        //关闭文件
        file.close();
 }




}

//将用户名和密码加密后写入到配置文件中的函数
void common::savelogininfo(QString username, QString password, bool isremember,QString conf_file_path)
{
    //得到配置文件中的IP和PORT信息
    QString IP = getcfgValue("web_server","ip");
    QString port = getcfgValue("web_server","port");

    //创建一个服务器信息的Map
    QMap<QString,QVariant> webinfo;
    webinfo.insert("ip",QVariant(IP));
    webinfo.insert("port",QVariant(port));

    //用来保存加密后的用户名和密码
    unsigned char Encode_username[128] = {0};
    unsigned char Encode_password[128] = {0};
    //用来保存加密后的数据长度
    int Encode_username_len;
    int Encode_password_len;

    int ret = 0;

    //首先要用des加密
    //将用户名和密码加密,参数1要进行数据类型转换
    ret = DesEnc((unsigned char *)username.toUtf8().data(),username.toUtf8().length(),Encode_username,&Encode_username_len);
    if(ret!=0)
    {
        qDebug()<<"des加密用户名失败";
    }


    ret = DesEnc((unsigned char *)password.toUtf8().data(),password.toUtf8().length(),Encode_password,&Encode_password_len);
    if(ret!=0)
    {
        qDebug()<<"des加密密码失败";
    }

    qDebug()<<"desenc username"<<Encode_username;
    qDebug()<<"desenc_username_len"<<Encode_username_len;
    qDebug()<<"desenc pwd"<<Encode_password;
    qDebug()<<"desenc_pwd_len"<<Encode_password_len;


    //然后再使用base64进行加密,将加密后的二进制转化成base64字符串
   // QByteArray username_base64 = QByteArray((char *)Encode_username,Encode_username_len).toBase64();
    //QByteArray password_base64 = QByteArray((char *)Encode_password,Encode_password_len).toBase64();
    //base64加密
    char username_base64[128] ={0};
    char password_base64[128] = {0};
    base64_encode(static_cast<const unsigned char *>(Encode_username), Encode_username_len, username_base64);
    base64_encode(static_cast<const unsigned char *>(Encode_password),Encode_password_len,password_base64);


    //创建用户信息的map
    QMap<QString,QVariant> userinfo;
    userinfo.insert("user",username_base64);
    userinfo.insert("pwd",password_base64);

    //判断是否记住密码
    if(isremember==true)
    {
        userinfo.insert("isremember","yes");
    }else {
        userinfo.insert("isremember","no");
    }


     //写入文件类型图片的目录
    QMap<QString,QVariant> file_type;
    file_type.insert("path",FILETYPE_DIR);


    //组成一个大的QMap
    QMap<QString,QVariant> root;
    root.insert("web_server", webinfo);
    root.insert("filetype_path", file_type);
    root.insert("login", userinfo);

    //转换成QJSONDocument对象
    QJsonDocument doc = QJsonDocument::fromVariant(root);
    QByteArray  jsonarray = doc.toJson();
    //打开配置文件
    QFile file(conf_file_path);
     if(false==file.open(QIODevice::WriteOnly))
     {
         qDebug()<<"write into file fail,open conf file fail";
     }

    //写入配置文件
    file.write(jsonarray);


    file.close();


}

//将数据封装成Json字符串
QByteArray common::packinfo(QString username, QString nickname, QString password, QString phonenum, QString emailaddress)
{

    QJsonObject postdata
    {
        {"username",username},
        {"nickname",nickname},
        {"password",password},
        {"phonenum",phonenum},
        {"emailaddress",emailaddress}
    };

   QJsonDocument doc(postdata);
   QByteArray send_data = doc.toJson();


   return send_data;

}

//获得网络管理的句柄
QNetworkAccessManager *common::getmanager()
{

    return m_manager;
}


//读写文件的内容并得到该文件的md5值
QString common::getfilemd5(QString filepath)
{

    quint64 totalfilesize = 0;   //文件的总大小
    quint64 byteswritten = 0;    //已经写入的字节数
    quint64 bytestowrite = 0;    //还要写的文件的字节数
    quint64 loadsize = 1024*4;   //一次从文件中读取四个字节
    QByteArray buf;              //存放文件内容的缓冲区

    QFile m_openfile(filepath);
    bool flg = m_openfile.open(QIODevice::ReadOnly);
    if(flg==false)
    {
       qDebug()<<"getfilemd5:打开文件失败";
       return nullptr;
    }

    totalfilesize = m_openfile.size(); //总的文件的大小
    bytestowrite = totalfilesize;    //要写入的文件的大小

    //创建一个转换工具的指针
    QCryptographicHash * ch = new QCryptographicHash(QCryptographicHash::Md5);


    while(1)
   {
       //如果要写的文件大于0
       if(bytestowrite>0)
       {
          buf = m_openfile.read(qMin(bytestowrite,loadsize));    //如果要写的文件的大小比单次加载的字节数还要小，就加载总的文件
          ch->addData(buf);
          byteswritten+=static_cast<quint64>(buf.size());
          bytestowrite-=static_cast<quint64>(buf.size());
       }
       else  if(byteswritten==totalfilesize)   //全部已经写完了
       {
            break;
       }

   }

   //返回的结果
   QByteArray file_md5 = ch->result();

   m_openfile.close();   //关闭这个文件
   delete  ch;           //删除指针

   return file_md5.toHex();

}
