#ifndef BUTTONGROUP_H
#define BUTTONGROUP_H

#include <QWidget>
#include<QPaintEvent>
#include<QSignalMapper>
#include<QMap>

namespace Ui {
class Buttongroup;
}

class QToolButton;   //前向声明可以减少不必要的include头文件，提高编译效率
enum Page{MYDISK,SHARE,DOWNLOADRANK,TRANSFER,SWITCHUSER};
class Buttongroup : public QWidget
{
    Q_OBJECT


public:
    explicit Buttongroup(QWidget *parent = nullptr);
    ~Buttongroup();
signals:
    void closewindow();   //窗口关闭信号
    void minsizewindow();  //窗口最小化
    void maxsizewindow();  //窗口最大化
    /*切换窗口的信号*/
    void sigmydisk();
    void sigshare();
    void sigdownloadrank();
    void sigtransfer();
    void sigswitchuser();

public slots:
    //设置父对象
    void setParent(QWidget *parent);

    //按钮处理槽函数
    void slotButtonClick_str(QString text);
    void slotButtonClick_page(Page cur);
private:
    Ui::Buttongroup *ui;
    QWidget* m_parent;

    QSignalMapper *m_mapper;   //信号映射指针
    QToolButton * m_curButton;  //当前按钮指针
    //建立对应关系
    QMap<QString,QToolButton *> m_btns;
    QMap<Page,QString> m_pages;


protected slots:
    //重写绘图事件
   void paintEvent(QPaintEvent *event);
};

#endif // BUTTONGROUP_H
