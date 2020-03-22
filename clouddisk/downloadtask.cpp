#include "downloadtask.h"

//分配空间
DownloadTask * DownloadTask::instance = new DownloadTask;


DownloadTask *DownloadTask::getInstance()
{
    return instance;
}


void DownloadTask::clearlist()
{
    int n = list.size();
    for(int i = 0;i<n;i++)
    {
        DownloadInfo *temp = list.takeFirst();
        delete  temp;
    }
}


bool DownloadTask::isEmpty()
{
    return list.isEmpty();
}


DownloadInfo *DownloadTask::takeTask()
{
    if(isEmpty())
    {
        return nullptr;
    }
    list.at(0)->isDownload = true;
    return list.at(0);

}

//删除下载完的任务
void DownloadTask::delDownloadTask()
{
    for(int i = 0;i<list.size();i++)
    {
        if(list.at(i)->isDownload==true)
        {
            DownloadInfo * tmp = list.takeAt(i);

            //获取布局
            DownloadLayout * downloadlayout = DownloadLayout::getInstance();
            QLayout * layout = downloadlayout->getDownloadLayout();

            layout->removeWidget(tmp->dp);
            delete tmp->dp;

            QFile *file = tmp->file;
            file->close();
            delete file;

            delete tmp; //释放空间
            return;

        }
    }

}

bool DownloadTask::isdownload()
{
    for(int i = 0;i!=list.size();++i)
    {
        if(list.at(i)->isDownload==true)
        {
            return true;
        }
    }
    return false;
}


//判断是不是被分享的文件
bool DownloadTask::isshared()
{
    if(isEmpty())
    {
        return NULL;
    }
    return list.at(0)->isShare;
}

//添加任务到下载队列
int DownloadTask::appendDownloadList(FileInfo *info, QString filePath, bool isShare)
{
    for(int i = 0;i<list.size();i++)
    {
        if(list.at(i)->user == info->user && list.at(i)->filename==info->filename)
        {
            qDebug()<<info->filename<<"已经在下载队列中";
            return -1;
        }
    }

    QFile * file = new QFile(filePath);  //文件指针分配空间

    if(!file->open(QIODevice::WriteOnly))
    { //如果打开文件失败，则删除 file，并使 file 指针为 NULL，然后返回
       qDebug() << "file open error";

        delete file;
        file = nullptr;
        return -2;
    }


    DownloadInfo * tmp = new DownloadInfo;
    tmp->user = info->user;   //用户
    tmp->file = file;         //文件指针
    tmp->filename = info->filename; //文件名字
    tmp->md5 = info->md5;           //文件md5
    tmp->url = info->url;           //下载网址
    tmp->isDownload = false;        //没有在下载
    tmp->isShare = isShare;         //是否为共享文件下载


    dataprocess * p = new dataprocess;
    p->setfilename(tmp->filename);

    //获取布局
    DownloadLayout * downloadlayout = DownloadLayout::getInstance();
    QVBoxLayout * layout = (QVBoxLayout *) downloadlayout->getDownloadLayout();

    tmp->dp = p;

    //添加到布局，最后一个是弹簧,插入到弹簧上面
    layout->insertWidget(layout->count()-1,p);

    //插入节点
    list.append(tmp);

    return 0;
}



DownloadTask::DownloadTask()
{

}
