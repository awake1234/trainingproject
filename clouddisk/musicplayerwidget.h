#ifndef MUSICPLAYERWIDGET_H
#define MUSICPLAYERWIDGET_H

#include <QWidget>
#include<QMediaPlayer>
#include<QMultimedia>
#include<QtMultimediaWidgets>
#include<QCloseEvent>
#include<QMovie>
#include<QSlider>
namespace Ui {
class MusicPlayerWidget;
}

class MusicPlayerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MusicPlayerWidget(QWidget *parent = nullptr);
    ~MusicPlayerWidget();

    //将音乐加入音乐列表
    void addmusic(QString filepath);
    //设置音乐名称
    void setmusicname(QString filename);
protected:
    void closeEvent(QCloseEvent * );
private slots:
    void on_toolButton_play_clicked();
    void on_toolButton_nosie_clicked();
    void on_noise_changed(int volume);
    void getduration(qint64 totaltime); //获得总时长
    void getcurduration(qint64);        //获取已经播放的时间
private:
    QMediaPlayer * player;
    QMediaPlaylist * playlist;
    QMovie * cover;                    //封面动态图
    bool noise=false;                 //判断是否显示音量
    QString totaltimestr;             //以秒为单位的时间

    Ui::MusicPlayerWidget *ui;
};

#endif // MUSICPLAYERWIDGET_H
