#########################################################################
# File Name: fastdfs.sh
# Author: lp
# mail: 1620811656.com
# Created Time: 2020年03月25日 星期三 14时45分44秒
#########################################################################
#!/bin/bash

#关闭tracker和storage服务器
tracker_start()
{
	ps aux | grep fdfs_trackerd | grep -v grep > /dev/null
	if [ $? -eq 0 ];then
		echo "fdfs_trackered already running"
	else
		sudo /usr/bin/fdfs_trackerd /etc/fdfs/tracker.conf
		if [ $? -ne 0 ];then
			echo "tracker start failed"
		else
			echo "tracker start successfully"
		fi
	fi
}


storage_start()
{
	ps aux | grep fdfs_storaged | grep -v grep > /dev/null
	if [ $? -eq 0 ];then
		echo "fdfs_storaged already running"
	else
		sudo /usr/bin/fdfs_storaged /etc/fdfs/storage.conf
		if [ $? -ne 0 ];then
			echo "storage start failed"
		else
			echo "storage start successfully"
		fi
	fi
}



#参宿个数为０通知用户输入参数
if [ $# -eq 0 ];then
	echo "operation menu"
	echo "start storage please input argument: storage"
	echo "start tracker please input argument: tracker"
	echo "start storage  and  tracker please input argument all"
	echo "stop  storage and tracker input argument: stop "
	exit 0
fi


case $1 in
	storage)
		storage_start
		;;
	tracker)
		tracker_start
		;;
	all)
		tracker_start
		storage_start
		;;
	stop)
		sudo /usr/bin/fdfs_trackerd /etc/fdfs/tracker.conf stop
		sudo /usr/bin/fdfs_storaged /etc/fdfs/storage.conf stop
		;;
	*)
		echo "nothing to do"
esac
