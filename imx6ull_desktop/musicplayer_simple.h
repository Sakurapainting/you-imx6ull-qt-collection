#ifndef MUSICPLAYER_SIMPLE_H
#define MUSICPLAYER_SIMPLE_H

// 这是一个不需要图片资源的简化版音乐播放器
// 使用纯QSS样式和文字按钮

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
    int duration;
};

class MusicPlayerSimple : public QWidget
{
    Q_OBJECT

public:
    explicit MusicPlayerSimple(QWidget *parent = nullptr);
    ~MusicPlayerSimple();

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
    void onVolumeChanged(int value);
    void onSongSelected(QListWidgetItem *item);
    void onSliderReleased();
    void updateProgress();
    void onPlayerFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void setupUI();
    void loadStyleSheet();
    void scanMusicFiles();
    void playSong(int index);
    void stopPlayback();
    void updatePlayButton();
    void updateModeButton();
    void updateTimeLabels();
    void setVolume(int volume);
    QString formatTime(int seconds);
    
private:
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
    
    QSlider *m_volumeSlider;
    
    QVector<SongInfo> m_songs;
    int m_currentIndex;
    PlayMode m_playMode;
    PlayerState m_playerState;
    int m_currentPosition;
    int m_volumeLevel;
    
    QProcess *m_playerProcess;
    QTimer *m_progressTimer;
    
    bool m_isSliderPressed;
};

#endif // MUSICPLAYER_SIMPLE_H
