#ifndef MYTITLEWIDGET_H
#define MYTITLEWIDGET_H

#include <QWidget>

namespace Ui {
class mytitlewidget;
}

class mytitlewidget : public QWidget
{
    Q_OBJECT

public:
    explicit mytitlewidget(QWidget *parent = nullptr);
    ~mytitlewidget();

protected:

signals:
    void minisizesignal();
    void closesignal();
    void setsignal();

private:
    Ui::mytitlewidget *ui;


};

#endif // MYTITLEWIDGET_H
