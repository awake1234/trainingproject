#include "login.h"
#include "ui_login.h"


login::login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::login)
{
    /*UI方面的操作*/
    ui->setupUi(this);
    //设置title颜色
    ui->titlewidget->setStyleSheet("color:red");

    //去掉边框,并且用或来保存原来的一些FLAGS
    this->setWindowFlags(Qt::FramelessWindowHint | windowFlags());

    //设置全局字体
    this->setFont(QFont("Times", 10, QFont::Bold));

    //设置固定大小
    this->setFixedSize(QSize(641,541));

    //设置注册按钮最初为不可选中
   // ui->toolButton_reg->setDisabled(true);
    //设置手机号的输入框只能输入11位
    ui->lineEdit_phonenum->setMaxLength(11);
    //设置密文输入
    ui->lineEdit_password->setEchoMode(QLineEdit::Password);
    ui->lineEdit_confirmpass->setEchoMode(QLineEdit::Password);
    ui->lineEdit_confirmpass->setEchoMode(QLineEdit::Password);
    ui->lineEdit_login_pass->setEchoMode(QLineEdit::Password);
    //设置提示文字
    ui->lineEdit_set_IP->setPlaceholderText(QString("服务器的IP地址回车键键入"));
    ui->lineEdit_set_port->setPlaceholderText(QString("服务器的端口回车键键入"));

    /*其他操作*/

    //进行一些初始化
    m_common = new common();
    //初始化界面
    bool ret = this->init_ui(CONFILE_PATH);
    if(ret==false)
    {
        qDebug()<<"init ui error";
    }

    //信号与槽的设置
    //点击注册按钮 跳转到用户注册界面
    connect(ui->reg_Button,&QToolButton::clicked,[=]()
    {
        ui->stackedWidget->setCurrentWidget(ui->register_page);
    });

    //监听返回登录界面的按钮
    connect(ui->allright_Button,&QToolButton::clicked,[=]()
    {
       //返回登录界面
       //清空注册界面
       this->clearregister();
       ui->stackedWidget->setCurrentIndex(0);
    });

    //监听最小化的信号
    connect(ui->titlewidget,&mytitlewidget::minisizesignal,[=]()
    {
        //窗口最小化
        this->showMinimized();
    });


    //监听转到设置界面的信号
    connect(ui->titlewidget,&mytitlewidget::setsignal,[=]()
    {
       //转到设置页面
       ui->stackedWidget->setCurrentIndex(2);
       //将界面重新初始化
       ui->lineEdit_set_IP->setPlaceholderText(QString("服务器的IP地址回车键键入"));
       ui->lineEdit_set_port->setPlaceholderText(QString("服务器的端口回车键键入"));
    });
    //监听关闭信号
    connect(ui->titlewidget,&mytitlewidget::closesignal,[=]()
    {
          //判断当前的页面
          //如果当前页面是登录界面
          if(ui->stackedWidget->currentIndex()==0)
          {
               this->close();
          }
          //如果当前界面时注册界面
          else if(ui->stackedWidget->currentIndex()==1)
          {
              //返回登录界面
              this->clearregister();
              ui->stackedWidget->setCurrentIndex(0);
          }else {
              //返回登录界面
              ui->stackedWidget->setCurrentIndex(0);
          }
      });



      //设置悬浮消息
      this->settips();

      this->MatchMsg();

     //注册确认按钮
     connect(ui->toolButton_reg,&QToolButton::clicked,this,[=]()
     {
         //所有的errorlabel全为空则正确
         if((ui->username_error_label->text().toUtf8()==""&&
             ui->Nickname_error_label->text().toUtf8()==""&&
              ui->password_error_label->text().toUtf8()==""&&
                ui->passconfirm_error_label->text().toUtf8()==""&&
                 ui->phonenum_error_label->text().toUtf8()==""&&
                 ui->email_error_label->text().toUtf8()=="")||this->is_all_item_right()==true)
         {
               //发送注册消息
               this->reg_senddata();
         }else {
             QMessageBox::critical(this,"result","注册失败");
         }


     });

     //监听网络的QLINEDIT
     connect(ui->lineEdit_set_IP,&QLineEdit::editingFinished,[=]()mutable
     {
         //用正则进行匹配
         bool ret = m_common->isMatch(IP_MATCH,ui->lineEdit_set_IP->text().toUtf8());
         if(ret==false)
         {
             //显示错误消息
             ui->IP_error_label->setText(QString("输入的IP地址不符合标准，请重新输入"));
         }else {
              //清除LABEL的错误消息
              ui->IP_error_label->clear();
              this->m_netflg[0] = true;
         }

     });

     //监听网络的QLINEDIT
     connect(ui->lineEdit_set_port,&QLineEdit::editingFinished,[=]() mutable
     {
         //用正则进行匹配
         bool ret = m_common->isMatch(PORT_MATCH,ui->lineEdit_set_port->text().toUtf8());
         if(ret==false)
         {
              //显示错误消息
              ui->port_error_label->setText(QString("输入的端口不符合标准，请重新输入"));

         }else {
              //清除LABEL的错误消息
              ui->port_error_label->clear();
              this->m_netflg[1] = true;

          }

      });


     //监听网络设置的确认按钮
     connect(ui->toolButton_net_confirm,&QToolButton::clicked,[=]()
     {
       bool ret = true;
       for(int i=0;i<2;i++)
       {
           if(this->m_netflg[i]==false)
           {
               ret = false;
               break;
           }
       }

       //如果填写的结果是正确的
       if(ret==true)
       {
           QMessageBox::information(this,"result","设置成功");
       }else{
           QMessageBox::critical(this,"result","设置失败");
       }
      });






}

login::~login()
{
    delete ui;
}

//清空注册界面
void login::clearregister()
{
    //清空所有的输入框
    /*
    ui->lineEdit_username->clear();
    ui->lineEdit_nickname->clear();
    ui->lineEdit_password->clear();
    ui->lineEdit_confirmpass->clear();
    ui->lineEdit_phonenum->clear();
    ui->lineEdit_email->clear();
    */
    this->settips();
    //清空所有的错误的标签
    ui->username_error_label->clear();
    ui->Nickname_error_label->clear();
    ui->password_error_label->clear();
    ui->passconfirm_error_label->clear();
    ui->phonenum_error_label->clear();
    ui->email_error_label->clear();

}

//设置悬浮tips
void login::settips()
{

    //设置悬浮消息
    ui->lineEdit_username->setToolTip(QString("请输入字母开头，8-16个字符，只能包含字母数字下划线"));
    ui->lineEdit_username->setToolTipDuration(0);
    ui->lineEdit_nickname->setToolTip(QString("请输入字母开头，5-16个字符，只能包含字母数字下划线"));
    ui->lineEdit_nickname->setToolTipDuration(0);
    ui->lineEdit_password->setToolTip(QString("请输入以字母开头，6~18字符，只能包含字母、数字和下划线"));
    ui->lineEdit_password->setToolTipDuration(0);
    ui->lineEdit_confirmpass->setToolTip(QString("请重新输入你的密码"));
    ui->lineEdit_confirmpass->setToolTipDuration(0);
    ui->lineEdit_phonenum->setToolTip(QString("请输入你的手机号"));
    ui->lineEdit_phonenum->setToolTipDuration(0);
    ui->lineEdit_email->setToolTip(QString("请输入你的邮箱"));
    ui->lineEdit_email->setToolTipDuration(0);


    ui->lineEdit_username->setPlaceholderText(QString("字母开头8-16个字符仅字母数字下划线回车键键入"));
    ui->lineEdit_nickname->setPlaceholderText(QString("字母开头5-16个字符仅字母数字下划线回车键键入"));
    ui->lineEdit_password->setPlaceholderText(QString("字母开头6-18个字符仅字母数字下划线回车键键入"));
    ui->lineEdit_confirmpass->setPlaceholderText(QString("重新输入密码,回车键键入"));
    ui->lineEdit_phonenum->setPlaceholderText(QString("请输入你的手机号,回车键键入"));
    ui->lineEdit_email->setPlaceholderText(QString("请输入你的邮箱,回车键键入"));

  }

//匹配注册界面是否符合输入标准的函数
void  login::MatchMsg()
{
    //用户名
    connect(ui->lineEdit_username,&QLineEdit::editingFinished,[=]() mutable{
    QString src = ui->lineEdit_username->text().toUtf8();
    if(m_common->isMatch(USER_MATCH,src)==false)
    {
       ui->username_error_label->setText("您输入的用户名不符合标准，请重新输入");
    }else {
        //清空错误提示
        ui->username_error_label->clear();
        m_flg[0] = true;
    }});
    //昵称
    connect(ui->lineEdit_nickname,&QLineEdit::editingFinished,[=]() mutable{
    QString src = ui->lineEdit_nickname->text().toUtf8();
    if(m_common->isMatch(Nickname_MATCH,src)==false)
    {
       ui->Nickname_error_label->setText("您输入的昵称不符合标准，请重新输入");
    }else {
       ui->Nickname_error_label->clear();
       m_flg[1] = true;
    }
    });
    //密码
    connect(ui->lineEdit_password,&QLineEdit::editingFinished,[=]()mutable{
    QString src = ui->lineEdit_password->text().toUtf8();
    if(m_common->isMatch(PassWord_MATCH,src)==false)
    {
       ui->password_error_label->setText("您输入的密码不符合标准，请重新输入");
    }else {
       ui->password_error_label->clear();
       m_flg[2] = true;
     }
    });

    //再次输入密码
    connect(ui->lineEdit_confirmpass,&QLineEdit::editingFinished,[=]()mutable{
    QString src = ui->lineEdit_confirmpass->text().toUtf8();
    if(m_common->isMatch(PassWord_MATCH,src)==false||src!=ui->lineEdit_password->text().toUtf8())
    {
       ui->passconfirm_error_label->setText("您输入的密码与初始密码不符，请重新输入");
    }else {
        ui->passconfirm_error_label->clear();
        m_flg[3] = true;
     }
    });

    //电话
    connect(ui->lineEdit_phonenum,&QLineEdit::editingFinished,[=]() mutable{
    QString src = ui->lineEdit_phonenum->text().toUtf8();
    if(m_common->isMatch(Phone_MATCH,src)==false)
    {
       ui->phonenum_error_label->setText("您输入的电话不符合标准，请重新输入");
    }else {
       ui->phonenum_error_label->clear();
       m_flg[4] = true;
    }
    });

    //邮箱
    connect(ui->lineEdit_email,&QLineEdit::editingFinished,[=]() mutable{
    QString src = ui->lineEdit_email->text().toUtf8();
    if(m_common->isMatch(EMail_MATCH,src)==false)
    {
       ui->email_error_label->setText("您输入的邮箱不符合标准，请重新输入");
    }else {
       ui->email_error_label->clear();
       m_flg[5] = true;
    }
    });
}

//定义是否所有的信息都已经填写正确
bool login::is_all_item_right()
{
    for(int i = 0;i<6;i++)
    {
        if(m_flg[i]==false)
        {
            return false;
        }
    }
    return true;

}

//初始化界面的函数
bool login::init_ui(QString conf_path)
{
    //读取json文件
    QFile file(conf_path);
    bool ret = file.open(QIODevice::ReadOnly);
    if(ret==false)
    {
        qDebug()<<"init_ui:open conf file error";
        file.close();
        return false;
    }
    //全部读取出来
    QByteArray array = file.readAll();

    //转化成QJsondocument
    QJsonDocument doc = QJsonDocument::fromJson(array);
    if(doc.isObject())
    {
        //得到最大的object
        QJsonObject total = doc.object();

        QJsonValue login = total.value("login");
        QJsonValue webinfo = total.value("web_server");
        //解析login模块
        if(login.isObject())
        {
            QJsonObject sublogin = login.toObject();
            //得到用户名，密码，和是否记住密码的值
            QString username_str = sublogin.value("user").toString();
            QString password_str = sublogin.value("pwd").toString();

            //对加密的用户名和密码进行解密
            //先转换成bytearray
            QByteArray username_byte = username_str.toUtf8();
            QByteArray password_byte = password_str.toUtf8();

            //解密base64编码
            QByteArray username_des = QByteArray::fromBase64(username_byte);
            QByteArray password_des = QByteArray::fromBase64(password_byte);


            //使用des解密方法
            //定义保存解密后的用户名和密码的内存空间
            unsigned char username[128] = {0};
            unsigned char password[128] = {0};
            int  username_len = 0;
            int  password_len = 0;

            DesDec((unsigned char *)username_des.data(),username_des.length(),username,&username_len);
            DesDec((unsigned char *)password_des.data(),password_des.length(),password,&password_len);

            qDebug()<<"init ui: username"<<username<<"length:"<<username_len;


            QString isremember = sublogin.value("isremember").toString();

            //判断是否记住用户名
            if(isremember=="yes")
            {
                ui->lineEdit_login_pass->setText(QString((char *)password));
                ui->lineEdit_login_username->setText(QString((char *)username));
                ui->checkBox_login_remember->setCheckState(Qt::Checked);
            }else{
                //否则只设置用户名
                ui->lineEdit_login_username->setText(QString((char *)username));
            }
        }

        //解析出web_server模块
        if(webinfo.isObject())
        {
            QJsonObject subwebinfo = webinfo.toObject();

            //得到IP和端口
            QString ip = subwebinfo.value("ip").toString();
            QString  port = subwebinfo.value("port").toString();

            //将信息写到输入框中
            ui->lineEdit_set_IP->setText(ip);
            ui->lineEdit_set_port->setText(port);
        }
    }
    return true;
}


//发送登录界面用户信息的函数,参数三和四作为传出参数
int login::send_logininfo(QString ip,QString port, QString code,QString  token)
{
    int ret = 0;
    //首先获取配置文件中加密后的用户名和密码的信息
 //   QString username_base64 = m_common->getcfgValue("login","user");
   // QString pwd_base64 = m_common->getcfgValue("login","pwd");
    
    //从界面获取字符串
    QString username = ui->lineEdit_login_username->text();
    QString pwd = ui->lineEdit_login_pass->text();

    
    //组织一个发送的json字符串
    QMap<QString,QVariant> logininfo;
    logininfo.insert("user",username);
    logininfo.insert("pwd",pwd);
    
    //转换成json对象
    QJsonDocument doc = QJsonDocument::fromVariant(logininfo);
    //转换成QByteaerray
    QByteArray  loginArray = doc.toJson();
    
    //发送json包
    QNetworkAccessManager * manager = m_common->getmanager();
    
    QNetworkRequest req;
    
    //设置请求头
    req.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    req.setHeader(QNetworkRequest::ContentLengthHeader,loginArray.size());  //发送的数据的长度
    
    req.setUrl(QUrl(QString("http://%1:%2/login").arg(ip).arg(port)));
    
    //发送数据
    QNetworkReply * reply = manager->post(req,loginArray);
    
    
    //监听接收的数据
    connect(reply,&QNetworkReply::readyRead,[=]()mutable
    {
        //读取数据
        QByteArray  result = reply->readAll();
        if(result==nullptr)
        {
            ret = -1;
            qDebug()<<"receive data error";
        }
        //解析json对象
        /*成功
        {"code": "000",
        "token": "xxx"
        }
        */
        /*失败
         * {"code":"001",
         *  "token":"failed"
         * }
        */
       
        QJsonDocument root = QJsonDocument::fromJson(result);
        if(root.isObject())
        {
            QJsonObject par = root.object();
            //得到相应的数据
            code = par.value("code").toString();
            token = par.value("token").toString();
            //如果成功
            if(code=="000")
            {
                logininfoinstance * userlogininfo = logininfoinstance::getinstance();
                userlogininfo->setlogininfo(username,ip,port,token);

                w = new MainWindow;   //分配空间
                //进入主界面
                this->hide();
                w->ShowMainWindow();
                //监听切换用户的信号
                connect(w,&MainWindow::changeuser,[=]()
                {
                    //主界面隐藏
                    w->hide();
                    this->show(); //登录界面显示
                    w->deleteLater();
                });
            }else if(code=="001"){
               QMessageBox::warning(this,"result","登录失败，用户名或密码错误");
           }
        }
       });
      return ret;
}




//重写绘图事件
void login::paintEvent(QPaintEvent * event)
{
    Q_UNUSED(event);   //一个宏表示没有使用event
    QPixmap pixmap(":/ico/images/login_bk.jpg");
    QPainter painter(this);

    painter.drawPixmap(0,0,this->width(),this->height(),pixmap);
}


//重写鼠标事件，只允许左键拖动
void login::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        move(event->globalPos()-dragposition);
    }

}

//得到相对差值
void login::mousePressEvent(QMouseEvent *ev)
{
    if(ev->button()==Qt::LeftButton)
    {
        dragposition = ev->globalPos()-frameGeometry().topLeft();
    }

}

//点击网络设置确定按钮触发
void login::on_toolButton_net_confirm_clicked()
{
    //获得IP 和 port值
    QString ip = ui->lineEdit_set_IP->text();
    QString port = ui->lineEdit_set_port->text();

    //调用保存配置文件信息的函数
    m_common->savewebinfo(ip,port,CONFILE_PATH);

}

//注册界面确认按钮槽函数
void login::reg_senddata()
{
    //1.获取界面上的数据
    QString username = ui->lineEdit_username->text();
    QString nickname = ui->lineEdit_nickname->text();
    QString password = ui->lineEdit_password->text();
    QString phonenum = ui->lineEdit_phonenum->text();
    QString emailaddress = ui->lineEdit_email->text();
    //通过读配置文件，将一些信息初始化到界面上面得到IP 和 port
    QString ip = ui->lineEdit_set_IP->text();
    QString port = ui->lineEdit_set_port->text();


    //2.按照格式封装要发送的协议的格式
    QByteArray senddata = m_common->packinfo(username,nickname,password,phonenum,emailaddress);

    //qDebug()<<"send_data"<<senddata;

    //发送数据,都使用同一个manager
    QNetworkAccessManager * manager = m_common->getmanager();

    QNetworkRequest request;
    //设置请求头
    //发送的内容的格式必须设置
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader,senddata.size());  //发送的数据的长度

    //拼接一个url,并向服务器发送注册命令
    QString  url = QString("http://%1:%2/reg").arg(ip).arg(port);
    request.setUrl(QUrl(url));

    //发送http请求
    QNetworkReply * reply = manager->post(request,senddata);

    //监听返回的结果
    connect(reply,&QNetworkReply::readyRead,[=]()
    {
        QByteArray result = reply->readAll();
      //  qDebug()<<"reply result:"<<result;

        //转换成Json格式
        QJsonDocument doc =QJsonDocument::fromJson(result);

        //如果是object对象
        if(doc.isObject())
        {
            //取出返回结果
             QJsonObject obj = doc.object();
             QString status = obj.value("code").toString();

             //根据状态码做出相应的反应
             if("002"==status)
             {
                 QMessageBox::information(this,"ret","恭喜你注册成功");
                 //跳转到登录界面
                 ui->stackedWidget->setCurrentIndex(0);
             }else if("003"==status)
             {
                 QMessageBox::warning(this,"ret","该用户已经存在");
             }else
             {
                  QMessageBox::critical(this,"ret","注册失败");
             }

        }

    });
}


//登录界面确认按钮
void login::on_toolButton_login_clicked()
{
    //获取所有控件的值
    QString username = ui->lineEdit_login_username->text();
    QString password = ui->lineEdit_login_pass->text();
    bool isremember = ui->checkBox_login_remember->isChecked(); //记录是否选择了记录密码

    //服务器的IP和端口
    QString IP = ui->lineEdit_set_IP->text();
    QString port = ui->lineEdit_set_port->text();
   
    //保存接收到的结果
    QString code;
    QString token;
    
    //将用户的登录信息写入到配置文件中
    m_common->savelogininfo(username,password,isremember,CONFILE_PATH);
    
    //将用户信息发送到服务器端，并检测是否正确
    send_logininfo(IP,port,code,token);


}
