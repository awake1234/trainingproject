#########################################################################
# File Name: fastdfs_start.sh
# Author: lp
# mail: 1620811656.com
# Created Time: 2019年12月27日 星期五 14时45分36秒
#########################################################################
#!/bin/bash

/usr/bin/fdfs_trackerd  ./conf/tracker.conf 
/usr/bin/fdfs_storaged  ./conf/storage.conf 
/usr/bin/fdfs_monitor   ./conf/client.conf 

