#ifndef _UTIL_CGI_H_
#define _UTIL_CGI_H_

#define   FILE_NAME_LEN      (256)    //文件名字长度
#define   TEMP_BUF_MAX_LEN   (1024) //临时缓冲区的最大的长度
#define   FILE_URL_LEN       (512)    //文件存放的URL的最大的长度
#define   HOST_NAME_LEN      (30)     //主机的IP地址长度
#define   USER_NAME_LEN      (128)   //用户名字长度
#define   TOKEN_LEN          (128)     //登录TOKEN的长度
#define   MD5_LEN            (256)      //文件MD5的长度
#define   PWD_LEN            (256)     //密码长度
#define   TIME_STRING_LEN    (25)   //时间戳长度
#define   SUFFIX_LEN         (8)   //后缀名长度
#define   PIC_NAME_LEN       (10)   //图片资源名字长度
#define   PIC_URL_LEN        (256)  //图片资源URL名字的长度
  

#define UTIL_LOG_MODULE "cgi"
#define UTIL_LOG_PROC  "util"

//定义一个给得到的用户信息进行加密的函数
//使用des加密并转换成base64编码格式
int  encode_message(char * before_info,char *after_info);

//返回前端情况，NULL代表失败
char *return_status(char *status_num);

//去除字符串两边的空格
int  trim_space(char * inbuf);

//通过文件名，得到文件后缀字符串
int  get_file_suffix(const char *file_name,char * suffix);

//验证登录token,成功返回0 失败返回-1
int verify_token(char *hashtable,char * field,char *token);

//解析url query 类似xxx=123&&yyy=456之类的字符串,传入key值，得到相应的value值
int query_parse_key_value(const char * query,const char *key,char *value,int *value_len_p);


//在一段内存中查找一个字符串第一次出现的位置
char * memstr(char * fulldata,int full_data_len,char *substr);
#endif
