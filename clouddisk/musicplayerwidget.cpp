#include "musicplayerwidget.h"
#include "ui_musicplayerwidget.h"
#include<QListWidgetItem>
MusicPlayerWidget::MusicPlayerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MusicPlayerWidget)
{
    ui->setupUi(this);


    player = new QMediaPlayer();
    playlist = new QMediaPlaylist;

    //初始音量设为30
    player->setVolume(30);
    ui->verticalSlider_noise->setValue(player->volume());
    //添加封面动态图片
    cover = new QMovie(":/ico/images/muscicover.gif");
    ui->label_cover->setMovie(cover);
    cover->start();
    //让声音滑动条隐藏
    ui->verticalSlider_noise->hide();
    //隐藏播放列表
    ui->scrollArea_playlist->hide();
    //修改音量
    connect(ui->verticalSlider_noise,SIGNAL(valueChanged(int)),this,SLOT(on_noise_changed(int)));
    //获取总时长
    connect(player,SIGNAL(durationChanged(qint64)),this,SLOT(getduration(qint64)));
    //获取当前的位置
    connect(player,SIGNAL(positionChanged(qint64)),this,SLOT(getcurduration(qint64)));
    //给布局申请空间
    musicitemwidget = new QListWidget;
    musicitemwidget->setViewMode(QListView::ListMode); //设置列表的形式
    musicitemwidget->setMovement(QListView::Static);  //不可移动
    QVBoxLayout * vlayout = new QVBoxLayout;
    musicitemwidget->setLayout(vlayout);
}

MusicPlayerWidget::~MusicPlayerWidget()
{
    delete ui;
}


//添加音乐到playerlist
void MusicPlayerWidget::addmusic(QString filepath)
{
    //如果playlist不为空
    if(!playlist->isEmpty())
    {
       playlist->insertMedia(player->playlist()->currentIndex()+1,QUrl(filepath));
    }else{
    playlist->addMedia(QUrl(filepath));
    player->setPlaylist(playlist);
    playlist->setCurrentIndex(0);
    //播放
    player->play();
    }
}


//将音乐名称插入到vector中
void MusicPlayerWidget::insertmusicname(QString filename)
{
    musicname.push_back(filename);
}

//初始化名字
void MusicPlayerWidget::initmusicname()
{
    //设置歌曲名称
    ui->label_songname->setText(musicname[0]);
}

//将item添加到播放列表中
void MusicPlayerWidget::addmusicitem(QString filename)
{
    QListWidgetItem * newitem = new QListWidgetItem(filename);
    musicitemwidget->addItem(newitem);
    QGridLayout * Glayout = new QGridLayout;
    Glayout->addWidget(musicitemwidget);
    ui->scrollAreaWidgetContents->setLayout(Glayout);
}

//关闭时停止音乐播放
void MusicPlayerWidget::closeEvent(QCloseEvent *)
{
    //如果不是停止状态，就停止音乐播放
    if(!(player->state()==QMediaPlayer::PausedState))
    {
        player->stop();
        cover->stop();
        playlist->clear();   //清除音乐列表
    }
    emit sigwindowclose();
}


//按钮槽函数
void MusicPlayerWidget::on_toolButton_play_clicked()
{
    //如果当前的状态是暂停的
    if(player->state()==QMediaPlayer::PausedState)
    {
        player->play();
        //改变上面的图片
        ui->toolButton_play->setIcon(QIcon(":/ico/images/stop.png"));
        //重新绘制画面
        cover->start();
        update();
    }else{
        player->pause();   //暂停
        ui->toolButton_play->setIcon(QIcon(":/ico/images/play.png"));
        cover->stop();
        update();
     }
}

//点击喇叭
void MusicPlayerWidget::on_toolButton_nosie_clicked()
{
    if(noise==false)
    {
        ui->verticalSlider_noise->show();
        noise = true;
    }else{
        ui->verticalSlider_noise->hide();
        noise = false;
    }

}

//音量改变槽函数
void MusicPlayerWidget::on_noise_changed(int volume)
{
    player->setVolume(volume);
}

void MusicPlayerWidget::getduration(qint64 totaltime)
{
    qint64 m_durationtime = totaltime/1000;
    QTime totalTime((m_durationtime/3600)%24,(m_durationtime/60)%60,m_durationtime%60,(m_durationtime * 1000)%1000);
    totaltimestr = totalTime.toString("mm:ss");
    ui->label_songtime->setText(totaltimestr);
    ui->horizontalSlider_progress->setMaximum(m_durationtime);  //设置进度条的最大值
}

//获取已经播放的时长
void MusicPlayerWidget::getcurduration(qint64 curtime)
{
    qint64 m_curdurationtime = curtime/1000;
    QTime totalTime((m_curdurationtime/3600)%24,(m_curdurationtime/60)%60,m_curdurationtime%60,(m_curdurationtime * 1000)%1000);
    QString curtimestr = totalTime.toString("mm:ss");
    ui->label_curtime->setText(curtimestr);
    ui->horizontalSlider_progress->setValue(m_curdurationtime);
}

//下一首音乐
void MusicPlayerWidget::on_toolButton_nexr_clicked()
{

    int nextindex = player->playlist()->nextIndex();
    if(nextindex==-1)
    {
        return;
    }
    //设定为下一首音乐
    player->playlist()->setCurrentIndex(nextindex);
    ui->label_songname->setText(musicname[nextindex]);
    player->play();
}

//上一首歌曲
void MusicPlayerWidget::on_toolButton_last_clicked()
{
    int previousindex = player->playlist()->previousIndex();
    if(previousindex==-1)
    {
        return;
    }
    //设定为下一首音乐
    player->playlist()->setCurrentIndex(previousindex);
    ui->label_songname->setText(musicname[previousindex]);
    player->play();
}

//点击显示播放列表
void MusicPlayerWidget::on_toolButton_playlist_clicked()
{
    //如果播放列表被隐藏
    if(ui->scrollArea_playlist->isHidden())
    {
        ui->scrollArea_playlist->show();
    }else{
        ui->scrollArea_playlist->hide();
    }
}
