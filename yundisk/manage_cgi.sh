#########################################################################
# File Name: manage_cgi.sh
# Author: lp
# mail: 1620811656.com
# Created Time: 2020年03月23日 星期一 21时32分09秒
#########################################################################
#!/bin/bash

start=1
stop=1


case $1 in
	start)
		start=1
		stop=0
		;;
	stop)
		start=0
		stop=1
		;;
	"")
		stop=1
		start=1
		;;
	*)
		stop=0
		start=0
		;;
esac


#杀死正在运行的cgi进程
if [ "$stop" -eq 1 ];then
	kill -9 $(ps aux|grep "./bin_cgi/login.cgi"|grep -v grep|awk '{print $2}') > /dev/null 2>&1
	kill -9 $(ps aux|grep "./bin_cgi/register.cgi"|grep -v grep|awk '{print $2}') > /dev/null 2>&1
	kill -9 $(ps aux|grep "./bin_cgi/upload.cgi"|grep -v grep|awk '{print $2}') > /dev/null 2>&1
	kill -9 $(ps aux|grep "./bin_cgi/mydisk.cgi"|grep -v grep|awk '{print $2}') > /dev/null 2>&1
	kill -9 $(ps aux|grep "./bin_cgi/dealfile.cgi"|grep -v grep|awk '{print $2}') > /dev/null 2>&1
	kill -9 $(ps aux|grep "./bin_cgi/sharefile.cgi"|grep -v grep|awk '{print $2}') > /dev/null 2>&1
	kill -9 $(ps aux|grep "./bin_cgi/dealsharefile.cgi"|grep -v grep|awk '{print $2}') > /dev/null 2>&1
	kill -9 $(ps aux|grep "./bin_cgi/md5.cgi"|grep -v grep|awk '{print $2}') > /dev/null 2>&1
    echo "CGI 程序已经全部关闭,bye-bye"
fi



#启动cgi进程
if [ "$start" -eq 1 ];then
	echo -n "上传:"
	spawn-fcgi -a 127.0.0.1 -p 8849 -f ./bin_cgi/upload.cgi
	echo -n "注册:"
	spawn-fcgi -a 127.0.0.1 -p 8850 -f ./bin_cgi/register.cgi
	echo -n "登录:"
	spawn-fcgi -a 127.0.0.1 -p 8851 -f ./bin_cgi/login.cgi
	echo -n "秒传:"
	spawn-fcgi -a 127.0.0.1 -p 8852 -f ./bin_cgi/md5.cgi
	echo -n "我的网盘:"
	spawn-fcgi -a 127.0.0.1 -p 8853 -f ./bin_cgi/mydisk.cgi
	echo -n "处理文件:"
	spawn-fcgi -a 127.0.0.1 -p 8854 -f ./bin_cgi/dealfile.cgi
	echo -n "共享文件列表:"
	spawn-fcgi -a 127.0.0.1 -p 8855 -f ./bin_cgi/sharefile.cgi
	echo -n "处理共享文件:"
	spawn-fcgi -a 127.0.0.1 -p 8856 -f ./bin_cgi/dealsharefile.cgi
    echo "CGI 程序已经全部启动,welcome"
fi

