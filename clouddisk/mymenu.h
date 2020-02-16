#ifndef MYMENU_H
#define MYMENU_H

#include <QMenu>

//右键显示的菜单的类

class mymenu:public QMenu
{
    Q_OBJECT

public:
    explicit mymenu(QWidget *parent=nullptr);
};

#endif // MYMENU_H
