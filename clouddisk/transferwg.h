#ifndef TRANSFERWG_H
#define TRANSFERWG_H

#include <QWidget>
#include "uploadlayout.h"
#include "common.h"
#include "logininfoinstance.h"
#include <QFile>

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

    //显示传输记录
    void showdatarecord(QString path=RECORD_DIR);

private slots:
    void on_toolButton_clearrecord_clicked();

private:
    Ui::transferwg *ui;
};

#endif // TRANSFERWG_H
