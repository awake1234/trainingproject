#ifndef TRANSFERWG_H
#define TRANSFERWG_H

#include <QWidget>
#include "uploadlayout.h"
#include "common.h"

namespace Ui {
class transferwg;
}

class transferwg : public QWidget
{
    Q_OBJECT

public:
    explicit transferwg(QWidget *parent = nullptr);
    ~transferwg();


    //显示上传任务的函数
    void showuploadtask();

private:
    Ui::transferwg *ui;
};

#endif // TRANSFERWG_H
