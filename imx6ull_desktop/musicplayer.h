#ifndef MUSICPLAYER_H
#define MUSICPLAYER_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QListWidget>
#include <QProcess>
#include <QTimer>
#include <QVector>

struct SongInfo {
    QString fileName;
    QString filePath;
    QString title;
    QString artist;
    int duration; // 歌曲时长（秒）
};

class MusicPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit MusicPlayer(QWidget *parent = nullptr);
    ~MusicPlayer();

    enum PlayMode {
        SequentialMode = 0,
        LoopMode,
        RandomMode,
        SingleLoopMode
    };

    enum PlayerState {
        StoppedState = 0,
        PlayingState,
        PausedState
    };

private slots:
    void onPlayPauseClicked();
    void onPreviousClicked();
    void onNextClicked();
    void onModeClicked();
    void onVolumeClicked();
    void onFavoriteClicked();
    void onMenuClicked();
    void onSongSelected(QListWidgetItem *item);
    void onSliderPressed();
    void onSliderReleased();
    void updateProgress();
    void onPlayerFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onPlayerError(QProcess::ProcessError error);

private:
    void setupUI();
    void loadStyleSheet();
    void scanMusicFiles();
    void playSong(int index);
    void stopPlayback();
    void pausePlayback();
    void resumePlayback();
    void updatePlayButton();
    void updateModeButton();
    void updateTimeLabels();
    void setVolume(int volume);
    QString formatTime(int seconds);
    
private:
    // UI组件
    QLabel *m_cdLabel;
    QLabel *m_songTitleLabel;
    QLabel *m_artistLabel;
    QLabel *m_currentTimeLabel;
    QLabel *m_totalTimeLabel;
    QSlider *m_progressSlider;
    QListWidget *m_playlistWidget;
    
    QPushButton *m_playPauseButton;
    QPushButton *m_previousButton;
    QPushButton *m_nextButton;
    QPushButton *m_modeButton;
    QPushButton *m_volumeButton;
    QPushButton *m_favoriteButton;
    QPushButton *m_menuButton;
    
    QSlider *m_volumeSlider;
    QWidget *m_volumePanel;
    
    // 播放器数据
    QVector<SongInfo> m_songs;
    int m_currentIndex;
    PlayMode m_playMode;
    PlayerState m_playerState;
    int m_currentPosition; // 当前播放位置（秒）
    int m_volumeLevel; // 音量等级 0-100
    
    // ALSA播放进程
    QProcess *m_playerProcess;
    QTimer *m_progressTimer;
    
    // 控制标志
    bool m_isSliderPressed;
    bool m_isPaused;
    qint64 m_pausePosition;
};

#endif // MUSICPLAYER_H
