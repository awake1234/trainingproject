#ifndef _LOGIN_CGI_H_
#define _LOGIN_CGI_H_

#define PWD_LEN (20)

//定义得到用户信息的函数
int get_user_info(char *buf,char *user,char *pwd);

//检测用户信息是否正确
int check_user_info(char *username,char *pwd);



#endif
