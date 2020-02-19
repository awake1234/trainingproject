#include "uploadtask.h"
#include <QFileInfo>
#include<QDebug>

//初始化实例
uploadtask * uploadtask::uploadtaskinstance= new uploadtask;



//返回这个实例
uploadtask *uploadtask::get_uploadtask_instance()
{
    return uploadtaskinstance;
}



//将文件添加到列表中
//返回结果：0：成功
//-1:文件打开失败
//-2:文件已经存在列表中
//-3:获取上传文件的布局失败
int uploadtask::appendtolist(QString filepath)
{
    int ret = 0;
    //先得到文件的相关信息
    QFileInfo fileinfo(filepath);
    //获得文件的名字
    QString Fileinfo_filename = fileinfo.fileName();

    // 尝试打开文件
    QFile file(filepath);
    bool flg = file.open(QIODevice::ReadOnly);
    if(flg==false)
    {
        qDebug()<<"打开文件失败";
        ret = -1;
        return ret;
    }

    //遍历filelist查看是否已经在文件列表中
    for(int i = 0;i<uploadfile_list.size();++i)
    {
        //如果已经存在了
        if((uploadfile_list.at(i))->filename==Fileinfo_filename)
        {
            ret = -2;
            qDebug()<<"文件已经存在在队列中";
            file.close();
            return ret;
        }
    }




    //正常上传操作，设置上传文件的相关信息
    //创建一个临时的上传文件信息结构体
    uploadfileinfo * tempfileinfo = new uploadfileinfo;

    //设置结构体中的相关的信息
    tempfileinfo->filename = Fileinfo_filename;
    tempfileinfo->md5 = m_common.getfilemd5(filepath);
    tempfileinfo->filesize = fileinfo.size();
    tempfileinfo->isupload = false;
    tempfileinfo->filepoint = &file;
    tempfileinfo->dp = nullptr;    //这边暂时先设为空指针
    tempfileinfo->filepath = filepath;

    qDebug()<<Fileinfo_filename<<":添加到队列成功";

    //加锁
    mutex.lock();
    uploadfile_list.append(tempfileinfo);  //加入到文件列表的数据结构list中
    //解锁
    mutex.unlock();

    //发送信号给主线程
    emit emtfilename(Fileinfo_filename);

    return 0;

}

//将文件添加到上传列表中
void uploadtask::adduploadfiles()
{ //这里面做上传文件到文件列表中的操作

    for(int i = 0;i<filepath.size();i++)
    {
        int ret = appendtolist(filepath.at(i));
        if(ret==0)
        {
           qDebug()<<"文件已经成功添加到上传列表";

        }else {
           qDebug()<<"文件添加到上传列表失败";
           return;
        }
    }
    emit finished();  //发送线程结束信号
    return;
}

uploadtask::uploadtask()
{

}
