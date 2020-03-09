#include "mypicwg.h"
#include "ui_mypicwg.h"



mypicwg::mypicwg(QWidget *parent) :
    QWidget (parent),
    ui(new Ui::mypicwg)
{
    ui->setupUi(this);

    //初始化列表
    initpiclistwidget();

}

mypicwg::~mypicwg()
{
    delete ui;
}

void mypicwg::initpiclistwidget()
{

    ui->piclistwg->setViewMode(QListView::IconMode);  //从左往右排图标,文字在下面图标在上面
    ui->piclistwg->setIconSize(QSize(80,80));   //设置图标的大小
    ui->piclistwg->setGridSize(QSize(100,120));  //设置栅格的大小
    ui->piclistwg->setResizeMode(QListView::Adjust); //大小变化后重新布局
    ui->piclistwg->setMovement(QListView::Static);  //用户不能随便移动图标
    ui->piclistwg->setSpacing(30);  //表示各个控件之间的上下间距
    ui->piclistwg->setContextMenuPolicy(Qt::CustomContextMenu); //设置成右键菜单策略

}

//清除所有的item
void mypicwg::clearpicitems()
{
    int n = ui->piclistwg->count();

    for(int i = 0;i<n;i++)
    {
        QListWidgetItem * item = ui->piclistwg->takeItem(0);
        delete item;
    }

}


void mypicwg::refreshpicItems()
{
     clearpicitems();

    //如果文件列表不为空
    if(!m_piclist.isEmpty())
    {
        qDebug()<<"m_piclist size:"<<m_piclist.size();
        for(int i = 0;i<m_piclist.size();i++)
        {
            ui->piclistwg->addItem(m_piclist.at(i));
        }
        m_piclist.clear();
    }

}


