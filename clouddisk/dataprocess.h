#ifndef DATAPROCESS_H
#define DATAPROCESS_H

#include <QWidget>
#include<QString>

//上传下载的进度条显示
namespace Ui {
class dataprocess;
}

class dataprocess : public QWidget
{
    Q_OBJECT

public:
    explicit dataprocess(QWidget *parent = nullptr);
    ~dataprocess();

    //设置文件的名字
    void setfilename(QString name="测试");
    void setprogress(qint64 value,qint64 max=100);   //value，当前值，最大值设为100

private:
    Ui::dataprocess *ui;
};

#endif // DATAPROCESS_H
