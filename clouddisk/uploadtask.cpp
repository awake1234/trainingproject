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



//判断上传队列是否为空
bool uploadtask::isEmpty()
{
    return uploadfile_list.isEmpty();
}

//取指定的任务
uploadfileinfo *uploadtask::takeTask(int index)
{
    //如果当前任务没有被上传
    if(uploadfile_list.at(index)->isupload==false)
    {
        //改为正在上传
        uploadfile_list.at(index)->isupload = true;
    }
    return uploadfile_list.at(index);
}


//在UI界面中删除掉已经完成的任务
void uploadtask::delete_uploadtask()
{

    for (int i=0;i<uploadfile_list.size();i++)
    {
        if(uploadfile_list.at(i)->isupload)
        {
             uploadfileinfo *tmp=uploadfile_list.takeAt(i);
             UploadLayout *uploadlayout=UploadLayout::getInstance();
             QLayout* layout=uploadlayout->getUploadLayout();
             layout->removeWidget(tmp->dp);

             tmp->filepoint->close();

             delete tmp->dp;
             delete tmp->filepoint;
             delete tmp;

             return;
        }
    }


}
//追加上传文件到上传列表中
//参数：path 上传文件路径
//返回值：成功为0
//失败：
//  -1: 打开文件失败
//  -2：上传的文件是否已经在上传队列中
//  -3: 获取布局失败

int uploadtask::appendtolist(QString filepath)
{
    int ret = 0;
    //先得到文件的相关信息
    QFileInfo fileinfo(filepath);
    //获得文件的名字
    QString Fileinfo_filename = fileinfo.fileName();

    // 尝试打开文件
    QFile *file=new QFile(filepath);
    bool flg = file->open(QIODevice::ReadOnly);
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
            file->close();
            return ret;
        }
     }


    //正常上传操作，设置上传文件的相关信息
    //创建一个临时的上传文件信息结构体
    uploadfileinfo * tempfileinfo = new uploadfileinfo;

    common  m_common;



    //加载一个上传的进度条
    dataprocess * fileprogress = new dataprocess();
    fileprogress->setfilename(Fileinfo_filename);

    //获取放置进度条的布局实例
    UploadLayout * uploadlayout_instance = UploadLayout::getInstance();
    if(uploadlayout_instance==nullptr)
    {
        qDebug()<<"获取上传文件布局实例失败";
        return  -3;
    }

    //获取布局
    QVBoxLayout *vlayout = static_cast<QVBoxLayout * >(uploadlayout_instance->getUploadLayout());
    vlayout->insertWidget(vlayout->count()-1,fileprogress);    //因为下标是从0开始计算的，加入进度条

    //设置结构体中的相关的信息
    tempfileinfo->filename = Fileinfo_filename;
    tempfileinfo->md5 = m_common.getfilemd5(filepath);
    tempfileinfo->filesize = fileinfo.size();
    tempfileinfo->isupload = false;
    tempfileinfo->filepoint = file;
    tempfileinfo->dp = fileprogress;    //这边暂时先设为空指针
    tempfileinfo->filepath = filepath;

    qDebug()<<Fileinfo_filename<<":添加到队列成功";

    //加锁
    mutex.lock();
    uploadfile_list.append(tempfileinfo);  //加入到文件列表的数据结构list中
    //解锁
    mutex.unlock();

    //唤醒所有的线程
    mutex.lock();
    notempty.wakeAll();
    qDebug()<<"producer thread has waken up all consumer threads";
    mutex.unlock();
    return 0;
}


//清空上传文件的列表
void uploadtask::clearlist()
{
    int count=uploadfile_list.size();
    for (int i=0;i<count; ++i)
    {
        uploadfileinfo  *info=uploadfile_list.takeFirst();

        delete  info;
    }

}


uploadtask::uploadtask()
{

}
