#ifndef MYPICWG_H
#define MYPICWG_H

#include <QWidget>
#include "common.h"

namespace Ui {
class mypicwg;
}

class mypicwg : public QWidget
{
    Q_OBJECT

public:
    explicit mypicwg(QWidget *parent = nullptr);
    ~mypicwg();

    void initpiclistwidget();
    void clearpicitems();
    void refreshpicItems();

    QList<QListWidgetItem *> m_piclist;
private:

    Ui::mypicwg *ui;


};

#endif // MYPICWG_H
