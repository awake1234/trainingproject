#ifndef FILEPROPERTY_H
#define FILEPROPERTY_H

#include <QWidget>
#include "common.h"

namespace Ui {
class fileproperty;
}

class fileproperty : public QWidget
{
    Q_OBJECT

public:
    explicit fileproperty(QWidget *parent = nullptr);
    ~fileproperty();
    //设置文件信息到标签上
    void setfileinfo(FileInfo * info);

private:
    Ui::fileproperty *ui;
};

#endif // FILEPROPERTY_H
