#ifndef RULEDESCRIBE_H
#define RULEDESCRIBE_H

#include <QWidget>
#include<QPaintEvent>
#include<QPixmap>
#include<QPainter>

class ruledescribe : public QWidget
{
    Q_OBJECT
public:
    explicit ruledescribe(QWidget *parent = nullptr);


    //重写绘图事事件
    void paintEvent(QPaintEvent *);

signals:

public slots:
};

#endif // RULEDESCRIBE_H
