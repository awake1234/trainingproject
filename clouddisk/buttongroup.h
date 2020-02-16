#ifndef BUTTONGROUP_H
#define BUTTONGROUP_H

#include <QWidget>
#include<QPaintEvent>

namespace Ui {
class Buttongroup;
}

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

public slots:
    //设置父对象
    void setParent(QWidget *parent);

private:
    Ui::Buttongroup *ui;

    QWidget* m_parent;
protected slots:
    //重写绘图事件
   void paintEvent(QPaintEvent *event);
};

#endif // BUTTONGROUP_H
