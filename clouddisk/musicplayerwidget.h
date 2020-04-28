#ifndef MUSICPLAYERWIDGET_H
#define MUSICPLAYERWIDGET_H

#include <QWidget>
#include<QMediaPlayer>
#include<QMultimedia>
#include<QtMultimediaWidgets>
#include<QCloseEvent>
#include<QMovie>
#include<QSlider>
#include <QVector>
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
    void insertmusicname(QString filename);
    void initmusicname();

    //将音乐条目放进播放列表中
    void addmusicitem(QString filename);

signals:
    void sigwindowclose();
protected:
    void closeEvent(QCloseEvent * );
private slots:
    void on_toolButton_play_clicked();
    void on_toolButton_nosie_clicked();
    void on_noise_changed(int volume);
    void getduration(qint64 totaltime); //获得总时长
    void getcurduration(qint64);        //获取已经播放的时间
    void on_toolButton_nexr_clicked();
    void on_toolButton_last_clicked();
    void on_toolButton_playlist_clicked();

private:
    QMediaPlayer * player;
    QMediaPlaylist * playlist;
    QMovie * cover;                    //封面动态图
    bool noise=false;                 //判断是否显示音量
    QString totaltimestr;             //以秒为单位的时间
    QVector<QString> musicname;        //维护一个数组来保存歌曲的名字
    QListWidget * musicitemwidget;     //音乐条目类
    Ui::MusicPlayerWidget *ui;
};

#endif // MUSICPLAYERWIDGET_H
