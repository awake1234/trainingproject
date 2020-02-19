#include "mydiskwg.h"
#include "ui_mydiskwg.h"


mydiskwg::mydiskwg(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::mydiskwg)
{
    ui->setupUi(this);
    //初始化文件列表
    this->initlistwidget();
    //添加右键菜单的内容
    this->AddMenuAction();
    //添加上传文件的按钮
    this->Adduploaditem();

    uploadtask = uploadtask::get_uploadtask_instance();

    uploadtask->moveToThread(&thread_adduploadfiles);


    //线程启动后就执行相应的操作
    connect(&thread_adduploadfiles,&QThread::started,uploadtask,&uploadtask::adduploadfiles,Qt::QueuedConnection);
    //线程结束要删除
    connect(&thread_adduploadfiles,&QThread::finished,uploadtask,&QObject::deleteLater);

    //监听子线程发送的信号
    connect(uploadtask,SIGNAL(emtfilename(QString)),this,SLOT(addprogress(QString)));

}

mydiskwg::~mydiskwg()
{
    delete ui;
}

//初始化filelistwidget的相关属性
void mydiskwg::initlistwidget()
{
    ui->filelistWidget->setViewMode(QListView::IconMode);  //从左往右排图标,文字在下面图标在上面
    ui->filelistWidget->setIconSize(QSize(80,80));   //设置图标的大小
    ui->filelistWidget->setGridSize(QSize(100,120));  //设置栅格的大小
    ui->filelistWidget->setResizeMode(QListView::Adjust); //大小变化后重新布局

    ui->filelistWidget->setMovement(QListView::Static);  //用户不能随便移动图标
    ui->filelistWidget->setSpacing(30);  //表示各个控件之间的上下间距

    ui->filelistWidget->setContextMenuPolicy(Qt::CustomContextMenu); //设置成右键菜单策略

    //发送右键菜单请求策略
    connect(ui->filelistWidget,&QListWidget::customContextMenuRequested,this,&mydiskwg::rightMenu);


    //监听所有的item点击事件
    connect(ui->filelistWidget,&QListWidget::itemClicked,[=](QListWidgetItem * selected)
    {
       if(selected->text()=="上传文件")
       {
           QStringList path = QFileDialog::getOpenFileNames(this,tr("select one or more files to upload"),"./","files(*.*)");
          // QStringList path = fileopendialog();
           uploadtask->filepath = path;    //将值传给filepath
           //启动线程
           thread_adduploadfiles.start();
       }
    });

}

//添加右键菜单的条目
void mydiskwg::AddMenuAction()
{
    m_menu1 = new mymenu(this);

    m_download = new QAction("下载",this);
    m_delete = new QAction("删除",this);
    m_share = new QAction("分享",this);
    m_property = new QAction("属性",this);
    m_menu1->addAction(m_download);
    m_menu1->addAction(m_delete);
    m_menu1->addAction(m_share);
    m_menu1->addAction(m_property);


    m_empty = new mymenu(this);
    m_upload = new QAction("上传文件",this);
    m_refresh = new QAction("刷新",this);
    m_Download_ASC = new QAction("按下载量升序",this);
    m_Download_Des = new QAction("按下载量降序",this);
    m_empty->addAction(m_upload);
    m_empty->addAction(m_refresh);
    m_empty->addAction(m_Download_ASC);
    m_empty->addAction(m_Download_Des);


}


//添加上传文件的item
void mydiskwg::Adduploaditem(QString iconpath, QString text)
{
    QListWidgetItem * uploaditem = new QListWidgetItem(QIcon(iconpath),text);
    ui->filelistWidget->addItem(uploaditem);
}


//自定义文件打开的窗口
QStringList mydiskwg::fileopendialog()
{
    QStringList ltFilePath;
    QFileDialog dialog(this, tr("Open Files"), tr("./"),"files(*.*)");
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setModal(QFileDialog::ExistingFiles);
    dialog.setOption(QFileDialog::DontUseNativeDialog);
    dialog.exec();
    ltFilePath = dialog.selectedFiles();

    return  ltFilePath;
}





//右键菜单槽函数
void mydiskwg::rightMenu(const QPoint pos)
{
    QListWidgetItem * item = ui->filelistWidget->itemAt(pos);  //得到当前位置的条目

    //如果在空白处
    if(item==nullptr)
    {
        m_empty->exec(QCursor::pos());   //在鼠标所在的位置显示
    }else{
       ui->filelistWidget->setCurrentItem(item);  //设置所选择的条目
       if(item->text()=="上传文件")
       {
           return;
       }
       m_menu1->exec(QCursor::pos());
    }
}

//在UI界面中加入进度条
void mydiskwg::addprogress(QString filename)
{
    //加载一个上传的进度条
    dataprocess * fileprogress = new dataprocess();
    fileprogress->setfilename(filename);

    //获取放置进度条的布局实例
    UploadLayout * uploadlayout_instance = UploadLayout::getInstance();
    if(uploadlayout_instance==nullptr)
    {
        qDebug()<<"获取上传文件布局实例失败";
        return;
    }

    //获取布局
    QVBoxLayout *vlayout = static_cast<QVBoxLayout * >(uploadlayout_instance->getUploadLayout());
    vlayout->insertWidget(vlayout->count()-1,fileprogress);    //因为下标是从0开始计算的，加入进度条

    //加锁
    mutex.lock();
    //将上传队列中的进度条重新设置一下
    //查找当前文件的结构体
    for(int i =0;i<uploadtask->uploadfile_list.size();i++)
    {
        if(uploadtask->uploadfile_list.at(i)->filename==filename)
        {
            uploadtask->uploadfile_list.at(i)->dp = fileprogress;
        }
    }
    mutex.unlock();  //解锁

    //发送切换界面的信号
    emit switchto_transferui(upload);
    //唤醒所有的线程
    notempty.wakeAll();



}
