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
           //添加文件到上传队列中
            this->adduploadfiles();
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


//将文件添加到上传列表中
void mydiskwg::adduploadfiles()
{
    //这里面做上传文件到文件列表中的操作
    QStringList path = QFileDialog::getOpenFileNames(this,tr("select one or more files to upload"),"./","files(*.*)");


   uploadtask *uploadtask = uploadtask::get_uploadtask_instance();

    for(int i = 0;i<path.size();i++)
    {
        int ret = uploadtask->appendtolist(path.at(i));
        if(ret==0)
        {
           qDebug()<<"文件已经成功添加到上传列表";
        }else {
           qDebug()<<"文件添加到上传列表失败";
           return;
        }
    }

    return;
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
