#include "musicplayer_simple.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QFile>

MusicPlayerSimple::MusicPlayerSimple(QWidget *parent)
    : QWidget(parent)
    , m_currentIndex(-1)
    , m_playMode(LoopMode)
    , m_playerState(StoppedState)
    , m_currentPosition(0)
    , m_volumeLevel(70)
    , m_playerProcess(nullptr)
    , m_progressTimer(nullptr)
    , m_isSliderPressed(false)
{
    setupUI();
    loadStyleSheet();
    scanMusicFiles();
    
    m_playerProcess = new QProcess(this);
    connect(m_playerProcess, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(onPlayerFinished(int, QProcess::ExitStatus)));
    
    m_progressTimer = new QTimer(this);
    connect(m_progressTimer, &QTimer::timeout, this, &MusicPlayerSimple::updateProgress);
    
    setVolume(m_volumeLevel);
}

MusicPlayerSimple::~MusicPlayerSimple()
{
    if (m_playerProcess && m_playerProcess->state() == QProcess::Running) {
        m_playerProcess->kill();
        m_playerProcess->waitForFinished();
    }
}

void MusicPlayerSimple::setupUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // 左侧面板
    QWidget *leftWidget = new QWidget();
    leftWidget->setObjectName("leftPanel");
    leftWidget->setFixedWidth(310);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(15, 15, 15, 15);
    leftLayout->setSpacing(10);
    
    QLabel *playlistTitle = new QLabel("♫ 播放列表");
    playlistTitle->setObjectName("playlistTitle");
    playlistTitle->setFixedHeight(50);
    playlistTitle->setAlignment(Qt::AlignCenter);
    
    m_playlistWidget = new QListWidget();
    m_playlistWidget->setObjectName("playlistWidget");
    m_playlistWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_playlistWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(m_playlistWidget, &QListWidget::itemClicked, 
            this, &MusicPlayerSimple::onSongSelected);
    
    QWidget *listControlWidget = new QWidget();
    listControlWidget->setFixedHeight(70);
    QHBoxLayout *listControlLayout = new QHBoxLayout(listControlWidget);
    listControlLayout->setSpacing(20);
    listControlLayout->setContentsMargins(20, 0, 20, 0);
    
    m_previousButton = new QPushButton("◀");
    m_previousButton->setObjectName("btnControl");
    m_previousButton->setFixedSize(55, 55);
    m_previousButton->setCursor(Qt::PointingHandCursor);
    connect(m_previousButton, &QPushButton::clicked, 
            this, &MusicPlayerSimple::onPreviousClicked);
    
    m_playPauseButton = new QPushButton("▶");
    m_playPauseButton->setObjectName("btnPlayMain");
    m_playPauseButton->setFixedSize(65, 65);
    m_playPauseButton->setCursor(Qt::PointingHandCursor);
    connect(m_playPauseButton, &QPushButton::clicked, 
            this, &MusicPlayerSimple::onPlayPauseClicked);
    
    m_nextButton = new QPushButton("▶▶");
    m_nextButton->setObjectName("btnControl");
    m_nextButton->setFixedSize(55, 55);
    m_nextButton->setCursor(Qt::PointingHandCursor);
    connect(m_nextButton, &QPushButton::clicked, 
            this, &MusicPlayerSimple::onNextClicked);
    
    listControlLayout->addWidget(m_previousButton);
    listControlLayout->addWidget(m_playPauseButton);
    listControlLayout->addWidget(m_nextButton);
    
    leftLayout->addWidget(playlistTitle);
    leftLayout->addWidget(m_playlistWidget);
    leftLayout->addSpacing(10);
    leftLayout->addWidget(listControlWidget);
    leftLayout->addSpacing(20);
    
    // 右侧面板
    QWidget *rightWidget = new QWidget();
    rightWidget->setObjectName("rightPanel");
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(40, 40, 40, 40);
    rightLayout->setSpacing(15);
    
    // CD封面区域
    m_cdLabel = new QLabel("♪");
    m_cdLabel->setObjectName("cdLabel");
    m_cdLabel->setFixedSize(250, 250);
    m_cdLabel->setAlignment(Qt::AlignCenter);
    
    m_songTitleLabel = new QLabel("未播放");
    m_songTitleLabel->setObjectName("songTitle");
    m_songTitleLabel->setAlignment(Qt::AlignCenter);
    m_songTitleLabel->setWordWrap(true);
    
    m_artistLabel = new QLabel("选择歌曲开始播放");
    m_artistLabel->setObjectName("artistLabel");
    m_artistLabel->setAlignment(Qt::AlignCenter);
    
    // 进度控制
    QWidget *progressWidget = new QWidget();
    QVBoxLayout *progressLayout = new QVBoxLayout(progressWidget);
    progressLayout->setContentsMargins(0, 0, 0, 0);
    progressLayout->setSpacing(8);
    
    m_progressSlider = new QSlider(Qt::Horizontal);
    m_progressSlider->setObjectName("progressSlider");
    m_progressSlider->setMinimum(0);
    m_progressSlider->setMaximum(100);
    connect(m_progressSlider, &QSlider::sliderReleased, 
            this, &MusicPlayerSimple::onSliderReleased);
    
    QWidget *timeWidget = new QWidget();
    QHBoxLayout *timeLayout = new QHBoxLayout(timeWidget);
    timeLayout->setContentsMargins(0, 0, 0, 0);
    
    m_currentTimeLabel = new QLabel("00:00");
    m_currentTimeLabel->setObjectName("timeLabel");
    m_totalTimeLabel = new QLabel("00:00");
    m_totalTimeLabel->setObjectName("timeLabel");
    m_totalTimeLabel->setAlignment(Qt::AlignRight);
    
    timeLayout->addWidget(m_currentTimeLabel);
    timeLayout->addStretch();
    timeLayout->addWidget(m_totalTimeLabel);
    
    progressLayout->addWidget(m_progressSlider);
    progressLayout->addWidget(timeWidget);
    
    // 控制按钮
    QWidget *controlWidget = new QWidget();
    QHBoxLayout *controlLayout = new QHBoxLayout(controlWidget);
    controlLayout->setContentsMargins(0, 0, 0, 0);
    controlLayout->setSpacing(15);
    
    m_modeButton = new QPushButton("循环");
    m_modeButton->setObjectName("btnFunction");
    m_modeButton->setFixedSize(70, 35);
    m_modeButton->setCursor(Qt::PointingHandCursor);
    connect(m_modeButton, &QPushButton::clicked, 
            this, &MusicPlayerSimple::onModeClicked);
    
    // 音量控制
    QLabel *volumeLabel = new QLabel("音量");
    volumeLabel->setObjectName("volumeLabel");
    
    m_volumeSlider = new QSlider(Qt::Horizontal);
    m_volumeSlider->setObjectName("volumeSlider");
    m_volumeSlider->setMinimum(0);
    m_volumeSlider->setMaximum(100);
    m_volumeSlider->setValue(m_volumeLevel);
    m_volumeSlider->setFixedWidth(150);
    connect(m_volumeSlider, &QSlider::valueChanged, 
            this, &MusicPlayerSimple::onVolumeChanged);
    
    controlLayout->addWidget(m_modeButton);
    controlLayout->addStretch();
    controlLayout->addWidget(volumeLabel);
    controlLayout->addWidget(m_volumeSlider);
    
    // 组装右侧布局
    rightLayout->addSpacing(20);
    rightLayout->addWidget(m_cdLabel, 0, Qt::AlignHCenter);
    rightLayout->addSpacing(15);
    rightLayout->addWidget(m_songTitleLabel);
    rightLayout->addWidget(m_artistLabel);
    rightLayout->addSpacing(15);
    rightLayout->addWidget(progressWidget);
    rightLayout->addSpacing(15);
    rightLayout->addWidget(controlWidget);
    rightLayout->addStretch();
    
    mainLayout->addWidget(leftWidget);
    mainLayout->addWidget(rightWidget);
}

void MusicPlayerSimple::loadStyleSheet()
{
    QString styleSheet = R"(
        QWidget {
            font-family: "Microsoft YaHei", "SimHei", sans-serif;
        }
        
        #leftPanel {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #667eea, stop:1 #764ba2);
        }
        
        #rightPanel {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #f093fb, stop:0.5 #f5576c, stop:1 #fda085);
        }
        
        #playlistTitle {
            color: white;
            font-size: 18px;
            font-weight: bold;
            background: rgba(255, 255, 255, 0.15);
            border-radius: 10px;
        }
        
        #playlistWidget {
            background: rgba(255, 255, 255, 0.15);
            border: none;
            border-radius: 10px;
            color: white;
            font-size: 13px;
            padding: 5px;
        }
        
        #playlistWidget::item {
            padding: 12px 10px;
            border-radius: 8px;
            margin: 2px 3px;
        }
        
        #playlistWidget::item:hover {
            background: rgba(255, 255, 255, 0.25);
        }
        
        #playlistWidget::item:selected {
            background: rgba(255, 255, 255, 0.35);
            color: white;
        }
        
        #btnControl {
            background: rgba(255, 255, 255, 0.25);
            border: 2px solid rgba(255, 255, 255, 0.5);
            border-radius: 27px;
            color: white;
            font-size: 20px;
            font-weight: bold;
        }
        
        #btnControl:hover {
            background: rgba(255, 255, 255, 0.35);
            border: 2px solid rgba(255, 255, 255, 0.7);
        }
        
        #btnControl:pressed {
            background: rgba(255, 255, 255, 0.45);
        }
        
        #btnPlayMain {
            background: rgba(255, 255, 255, 0.95);
            border: none;
            border-radius: 32px;
            color: #667eea;
            font-size: 24px;
            font-weight: bold;
        }
        
        #btnPlayMain:hover {
            background: white;
        }
        
        #btnPlayMain:pressed {
            background: rgba(255, 255, 255, 0.85);
        }
        
        #cdLabel {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 rgba(255, 255, 255, 0.9), 
                stop:0.5 rgba(255, 255, 255, 0.95),
                stop:1 rgba(255, 255, 255, 0.9));
            border: 8px solid rgba(255, 255, 255, 0.7);
            border-radius: 125px;
            color: #f5576c;
            font-size: 80px;
            font-weight: bold;
        }
        
        #songTitle {
            color: white;
            font-size: 22px;
            font-weight: bold;
        }
        
        #artistLabel {
            color: rgba(255, 255, 255, 0.95);
            font-size: 15px;
        }
        
        #progressSlider {
            height: 8px;
        }
        
        #progressSlider::groove:horizontal {
            background: rgba(255, 255, 255, 0.35);
            height: 8px;
            border-radius: 4px;
        }
        
        #progressSlider::sub-page:horizontal {
            background: white;
            border-radius: 4px;
        }
        
        #progressSlider::handle:horizontal {
            background: white;
            width: 18px;
            height: 18px;
            margin: -5px 0;
            border-radius: 9px;
            border: 3px solid rgba(255, 255, 255, 0.9);
        }
        
        #progressSlider::handle:horizontal:hover {
            background: #fff;
            width: 20px;
            height: 20px;
            margin: -6px 0;
            border-radius: 10px;
        }
        
        #timeLabel {
            color: white;
            font-size: 12px;
        }
        
        #btnFunction {
            background: rgba(255, 255, 255, 0.25);
            border: 2px solid rgba(255, 255, 255, 0.5);
            border-radius: 17px;
            color: white;
            font-size: 13px;
        }
        
        #btnFunction:hover {
            background: rgba(255, 255, 255, 0.35);
        }
        
        #btnFunction:pressed {
            background: rgba(255, 255, 255, 0.45);
        }
        
        #volumeLabel {
            color: white;
            font-size: 13px;
        }
        
        #volumeSlider {
            height: 8px;
        }
        
        #volumeSlider::groove:horizontal {
            background: rgba(255, 255, 255, 0.35);
            height: 8px;
            border-radius: 4px;
        }
        
        #volumeSlider::sub-page:horizontal {
            background: white;
            border-radius: 4px;
        }
        
        #volumeSlider::handle:horizontal {
            background: white;
            width: 16px;
            height: 16px;
            margin: -4px 0;
            border-radius: 8px;
            border: 2px solid rgba(255, 255, 255, 0.9);
        }
    )";
    
    setStyleSheet(styleSheet);
}

void MusicPlayerSimple::scanMusicFiles()
{
    QString musicDir = "/music";
    QDir dir(musicDir);
    
    if (!dir.exists()) {
        qDebug() << "Music directory does not exist:" << musicDir;
        // 创建示例提示
        m_playlistWidget->addItem("请在 /music 目录放置音乐文件");
        m_playlistWidget->addItem("支持格式：MP3, WAV, FLAC, OGG");
        return;
    }
    
    QStringList filters;
    filters << "*.mp3" << "*.wav" << "*.flac" << "*.ogg";
    QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files);
    
    m_songs.clear();
    m_playlistWidget->clear();
    
    if (fileList.isEmpty()) {
        m_playlistWidget->addItem("目录为空，请添加音乐文件");
        return;
    }
    
    foreach (const QFileInfo &fileInfo, fileList) {
        SongInfo song;
        song.filePath = fileInfo.absoluteFilePath();
        song.fileName = fileInfo.fileName();
        
        QString baseName = fileInfo.completeBaseName();
        QStringList parts = baseName.split(" - ");
        if (parts.size() >= 2) {
            song.artist = parts[0].trimmed();
            song.title = parts[1].trimmed();
        } else {
            song.artist = "未知艺术家";
            song.title = baseName;
        }
        
        song.duration = 0;
        
        m_songs.append(song);
        
        QString displayText = song.title + "\n" + song.artist;
        m_playlistWidget->addItem(displayText);
    }
    
    qDebug() << "Found" << m_songs.size() << "songs";
}

void MusicPlayerSimple::playSong(int index)
{
    if (index < 0 || index >= m_songs.size()) {
        return;
    }
    
    stopPlayback();
    
    m_currentIndex = index;
    const SongInfo &song = m_songs[index];
    
    m_songTitleLabel->setText(song.title);
    m_artistLabel->setText(song.artist);
    m_playlistWidget->setCurrentRow(index);
    
    QStringList args;
    args << song.filePath;
    
    m_playerProcess->start("aplay", args);
    
    if (m_playerProcess->waitForStarted()) {
        m_playerState = PlayingState;
        m_currentPosition = 0;
        m_progressTimer->start(1000);
        updatePlayButton();
        qDebug() << "Playing:" << song.filePath;
    } else {
        qDebug() << "Failed to start aplay";
    }
}

void MusicPlayerSimple::stopPlayback()
{
    if (m_playerProcess && m_playerProcess->state() == QProcess::Running) {
        m_playerProcess->kill();
        m_playerProcess->waitForFinished();
    }
    
    m_progressTimer->stop();
    m_playerState = StoppedState;
    m_currentPosition = 0;
    updatePlayButton();
}

void MusicPlayerSimple::onPlayPauseClicked()
{
    if (m_playerState == StoppedState) {
        if (m_songs.isEmpty()) {
            return;
        }
        if (m_currentIndex < 0) {
            m_currentIndex = 0;
        }
        playSong(m_currentIndex);
    } else if (m_playerState == PlayingState) {
        stopPlayback();
        m_playerState = PausedState;
        updatePlayButton();
    } else if (m_playerState == PausedState) {
        if (m_currentIndex >= 0) {
            playSong(m_currentIndex);
        }
    }
}

void MusicPlayerSimple::onPreviousClicked()
{
    if (m_songs.isEmpty()) {
        return;
    }
    
    m_currentIndex--;
    if (m_currentIndex < 0) {
        m_currentIndex = m_songs.size() - 1;
    }
    
    playSong(m_currentIndex);
}

void MusicPlayerSimple::onNextClicked()
{
    if (m_songs.isEmpty()) {
        return;
    }
    
    m_currentIndex++;
    if (m_currentIndex >= m_songs.size()) {
        m_currentIndex = 0;
    }
    
    playSong(m_currentIndex);
}

void MusicPlayerSimple::onModeClicked()
{
    m_playMode = static_cast<PlayMode>((m_playMode + 1) % 4);
    updateModeButton();
}

void MusicPlayerSimple::onVolumeChanged(int value)
{
    setVolume(value);
}

void MusicPlayerSimple::onSongSelected(QListWidgetItem *item)
{
    int index = m_playlistWidget->row(item);
    if (index >= 0 && index < m_songs.size()) {
        playSong(index);
    }
}

void MusicPlayerSimple::onSliderReleased()
{
    // aplay 不支持 seek
    qDebug() << "Seek not supported with aplay";
}

void MusicPlayerSimple::updateProgress()
{
    if (m_playerState == PlayingState) {
        m_currentPosition++;
        m_progressSlider->setValue(m_currentPosition);
        updateTimeLabels();
    }
}

void MusicPlayerSimple::onPlayerFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
    
    qDebug() << "Playback finished";
    
    switch (m_playMode) {
    case SequentialMode:
        if (m_currentIndex < m_songs.size() - 1) {
            onNextClicked();
        } else {
            stopPlayback();
        }
        break;
        
    case LoopMode:
        onNextClicked();
        break;
        
    case RandomMode: {
        int randomIndex = qrand() % m_songs.size();
        m_currentIndex = randomIndex;
        playSong(m_currentIndex);
        break;
    }
        
    case SingleLoopMode:
        playSong(m_currentIndex);
        break;
    }
}

void MusicPlayerSimple::updatePlayButton()
{
    if (m_playerState == PlayingState) {
        m_playPauseButton->setText("❚❚");
    } else {
        m_playPauseButton->setText("▶");
    }
}

void MusicPlayerSimple::updateModeButton()
{
    switch (m_playMode) {
    case SequentialMode:
        m_modeButton->setText("顺序");
        break;
    case LoopMode:
        m_modeButton->setText("循环");
        break;
    case RandomMode:
        m_modeButton->setText("随机");
        break;
    case SingleLoopMode:
        m_modeButton->setText("单曲");
        break;
    }
}

void MusicPlayerSimple::updateTimeLabels()
{
    m_currentTimeLabel->setText(formatTime(m_currentPosition));
    
    if (m_currentIndex >= 0 && m_currentIndex < m_songs.size()) {
        int duration = m_songs[m_currentIndex].duration;
        if (duration > 0) {
            m_totalTimeLabel->setText(formatTime(duration));
            m_progressSlider->setMaximum(duration);
        } else {
            // 估算时长
            m_progressSlider->setMaximum(300); // 默认5分钟
        }
    }
}

void MusicPlayerSimple::setVolume(int volume)
{
    m_volumeLevel = volume;
    QProcess::execute("amixer", QStringList() << "set" << "Master" << QString::number(volume) + "%");
}

QString MusicPlayerSimple::formatTime(int seconds)
{
    int minutes = seconds / 60;
    int secs = seconds % 60;
    return QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0'));
}
