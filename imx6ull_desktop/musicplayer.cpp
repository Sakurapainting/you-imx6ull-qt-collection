#include "musicplayer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QFile>
#include <QDateTime>
#include <QPixmap>

MusicPlayer::MusicPlayer(QWidget *parent)
    : QWidget(parent)
    , m_currentIndex(-1)
    , m_playMode(LoopMode)
    , m_playerState(StoppedState)
    , m_currentPosition(0)
    , m_volumeLevel(70)
    , m_playerProcess(nullptr)
    , m_progressTimer(nullptr)
    , m_isSliderPressed(false)
    , m_isPaused(false)
    , m_pausePosition(0)
{
    setupUI();
    loadStyleSheet();
    scanMusicFiles();
    
    // 初始化播放进程
    m_playerProcess = new QProcess(this);
    connect(m_playerProcess, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(onPlayerFinished(int, QProcess::ExitStatus)));
    connect(m_playerProcess, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(onPlayerError(QProcess::ProcessError)));
    
    // 初始化进度定时器
    m_progressTimer = new QTimer(this);
    connect(m_progressTimer, &QTimer::timeout, this, &MusicPlayer::updateProgress);
    
    // 设置初始音量
    setVolume(m_volumeLevel);
}

MusicPlayer::~MusicPlayer()
{
    if (m_playerProcess && m_playerProcess->state() == QProcess::Running) {
        m_playerProcess->kill();
        m_playerProcess->waitForFinished();
    }
}

void MusicPlayer::setupUI()
{
    // 主布局
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // 左侧：播放列表区域
    QWidget *leftWidget = new QWidget();
    leftWidget->setObjectName("leftPanel");
    leftWidget->setFixedWidth(310);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(10);
    
    // 播放列表标题
    QLabel *playlistTitle = new QLabel("播放列表");
    playlistTitle->setObjectName("playlistTitle");
    playlistTitle->setFixedHeight(60);
    playlistTitle->setAlignment(Qt::AlignCenter);
    
    // 播放列表
    m_playlistWidget = new QListWidget();
    m_playlistWidget->setObjectName("playlistWidget");
    m_playlistWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_playlistWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(m_playlistWidget, &QListWidget::itemClicked, 
            this, &MusicPlayer::onSongSelected);
    
    // 列表底部控制按钮
    QWidget *listControlWidget = new QWidget();
    listControlWidget->setFixedHeight(80);
    QHBoxLayout *listControlLayout = new QHBoxLayout(listControlWidget);
    listControlLayout->setSpacing(30);
    listControlLayout->setContentsMargins(30, 0, 30, 0);
    
    m_previousButton = new QPushButton();
    m_previousButton->setObjectName("btnPrevious");
    m_previousButton->setFixedSize(60, 60);
    m_previousButton->setCursor(Qt::PointingHandCursor);
    connect(m_previousButton, &QPushButton::clicked, 
            this, &MusicPlayer::onPreviousClicked);
    
    m_playPauseButton = new QPushButton();
    m_playPauseButton->setObjectName("btnPlay");
    m_playPauseButton->setFixedSize(70, 70);
    m_playPauseButton->setCursor(Qt::PointingHandCursor);
    connect(m_playPauseButton, &QPushButton::clicked, 
            this, &MusicPlayer::onPlayPauseClicked);
    
    m_nextButton = new QPushButton();
    m_nextButton->setObjectName("btnNext");
    m_nextButton->setFixedSize(60, 60);
    m_nextButton->setCursor(Qt::PointingHandCursor);
    connect(m_nextButton, &QPushButton::clicked, 
            this, &MusicPlayer::onNextClicked);
    
    listControlLayout->addWidget(m_previousButton);
    listControlLayout->addWidget(m_playPauseButton);
    listControlLayout->addWidget(m_nextButton);
    
    leftLayout->addWidget(playlistTitle);
    leftLayout->addWidget(m_playlistWidget);
    leftLayout->addSpacing(10);
    leftLayout->addWidget(listControlWidget);
    leftLayout->addSpacing(30);
    
    // 右侧：播放控制区域
    QWidget *rightWidget = new QWidget();
    rightWidget->setObjectName("rightPanel");
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(40, 40, 40, 40);
    rightLayout->setSpacing(0);
    
    // CD封面
    m_cdWidget = new CDWidget();
    m_cdWidget->setObjectName("cdLabel");
    
    // 歌曲信息
    m_songTitleLabel = new QLabel("未播放");
    m_songTitleLabel->setObjectName("songTitle");
    m_songTitleLabel->setAlignment(Qt::AlignCenter);
    m_songTitleLabel->setWordWrap(true);
    
    m_artistLabel = new QLabel("选择歌曲开始播放");
    m_artistLabel->setObjectName("artistLabel");
    m_artistLabel->setAlignment(Qt::AlignCenter);
    
    // 播放进度条
    QWidget *progressWidget = new QWidget();
    QVBoxLayout *progressLayout = new QVBoxLayout(progressWidget);
    progressLayout->setContentsMargins(0, 0, 0, 0);
    progressLayout->setSpacing(5);
    
    m_progressSlider = new QSlider(Qt::Horizontal);
    m_progressSlider->setObjectName("progressSlider");
    m_progressSlider->setMinimum(0);
    m_progressSlider->setMaximum(100);
    connect(m_progressSlider, &QSlider::sliderPressed, 
            this, &MusicPlayer::onSliderPressed);
    connect(m_progressSlider, &QSlider::sliderReleased, 
            this, &MusicPlayer::onSliderReleased);
    
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
    
    // 底部功能按钮
    QWidget *functionWidget = new QWidget();
    QHBoxLayout *functionLayout = new QHBoxLayout(functionWidget);
    functionLayout->setContentsMargins(0, 0, 0, 0);
    functionLayout->setSpacing(40);
    
    m_favoriteButton = new QPushButton();
    m_favoriteButton->setObjectName("btnFavorite");
    m_favoriteButton->setFixedSize(32, 32);
    m_favoriteButton->setCursor(Qt::PointingHandCursor);
    connect(m_favoriteButton, &QPushButton::clicked, 
            this, &MusicPlayer::onFavoriteClicked);
    
    m_modeButton = new QPushButton();
    m_modeButton->setObjectName("btnMode");
    m_modeButton->setFixedSize(50, 32);
    m_modeButton->setCursor(Qt::PointingHandCursor);
    m_modeButton->setToolTip("列表循环");
    m_modeButton->setText("循环");
    connect(m_modeButton, &QPushButton::clicked, 
            this, &MusicPlayer::onModeClicked);
    
    m_menuButton = new QPushButton();
    m_menuButton->setObjectName("btnMenu");
    m_menuButton->setFixedSize(32, 32);
    m_menuButton->setCursor(Qt::PointingHandCursor);
    connect(m_menuButton, &QPushButton::clicked, 
            this, &MusicPlayer::onMenuClicked);
    
    m_volumeButton = new QPushButton();
    m_volumeButton->setObjectName("btnVolume");
    m_volumeButton->setFixedSize(32, 32);
    m_volumeButton->setCursor(Qt::PointingHandCursor);
    connect(m_volumeButton, &QPushButton::clicked, 
            this, &MusicPlayer::onVolumeClicked);
    
    functionLayout->addStretch();
    functionLayout->addWidget(m_favoriteButton);
    functionLayout->addWidget(m_modeButton);
    functionLayout->addWidget(m_menuButton);
    functionLayout->addWidget(m_volumeButton);
    functionLayout->addStretch();
    
    // 音量控制面板（初始隐藏）
    m_volumePanel = new QWidget(this);
    m_volumePanel->setObjectName("volumePanel");
    m_volumePanel->setFixedSize(60, 200);
    m_volumePanel->hide();
    QVBoxLayout *volumeLayout = new QVBoxLayout(m_volumePanel);
    volumeLayout->setContentsMargins(10, 20, 10, 20);
    
    m_volumeSlider = new QSlider(Qt::Vertical);
    m_volumeSlider->setObjectName("volumeSlider");
    m_volumeSlider->setMinimum(0);
    m_volumeSlider->setMaximum(100);
    m_volumeSlider->setValue(m_volumeLevel);
    connect(m_volumeSlider, &QSlider::valueChanged, 
            this, &MusicPlayer::setVolume);
    
    volumeLayout->addWidget(m_volumeSlider);
    
    // 组装右侧布局
    rightLayout->addSpacing(20);
    rightLayout->addWidget(m_cdWidget, 0, Qt::AlignHCenter);
    rightLayout->addSpacing(20);
    rightLayout->addWidget(m_songTitleLabel);
    rightLayout->addSpacing(5);
    rightLayout->addWidget(m_artistLabel);
    rightLayout->addSpacing(20);
    rightLayout->addWidget(progressWidget);
    rightLayout->addSpacing(20);
    rightLayout->addWidget(functionWidget);
    rightLayout->addSpacing(20);
    
    // 组装主布局
    mainLayout->addWidget(leftWidget);
    mainLayout->addSpacing(60);
    mainLayout->addWidget(rightWidget);
    mainLayout->addSpacing(60);
}

void MusicPlayer::loadStyleSheet()
{
    QString styleSheet = R"(
        /* 左侧面板 */
        #leftPanel {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #667eea, stop:1 #764ba2);
        }
        
        /* 右侧面板 */
        #rightPanel {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #f093fb, stop:1 #f5576c);
        }
        
        /* 播放列表标题 */
        #playlistTitle {
            color: white;
            font-size: 20px;
            font-weight: bold;
            background: rgba(255, 255, 255, 0.1);
        }
        
        /* 播放列表 */
        #playlistWidget {
            background: rgba(255, 255, 255, 0.15);
            border: none;
            color: white;
            font-size: 14px;
            padding: 5px;
        }
        
        #playlistWidget::item {
            padding: 15px 10px;
            border-radius: 8px;
            margin: 2px 5px;
        }
        
        #playlistWidget::item:hover {
            background: rgba(255, 255, 255, 0.2);
        }
        
        #playlistWidget::item:selected {
            background: rgba(255, 255, 255, 0.3);
            color: white;
        }
        
        /* 播放按钮 - 上一曲 */
        #btnPrevious {
            background: rgba(255, 255, 255, 0.2);
            border: 2px solid rgba(255, 255, 255, 0.5);
            border-radius: 30px;
            image: url(:/image/music_previous.png);
        }
        
        #btnPrevious:hover {
            background: rgba(255, 255, 255, 0.3);
            border: 2px solid rgba(255, 255, 255, 0.7);
        }
        
        #btnPrevious:pressed {
            background: rgba(255, 255, 255, 0.4);
        }
        
        /* 播放按钮 - 播放/暂停 */
        #btnPlay {
            background: rgba(255, 255, 255, 0.9);
            border: none;
            border-radius: 35px;
            image: url(:/image/music_play.png);
        }
        
        #btnPlay:hover {
            background: white;
        }
        
        #btnPlay:pressed {
            background: rgba(255, 255, 255, 0.7);
        }
        
        #btnPlay[playing="true"] {
            image: url(:/image/music_pause.png);
        }
        
        /* 播放按钮 - 下一曲 */
        #btnNext {
            background: rgba(255, 255, 255, 0.2);
            border: 2px solid rgba(255, 255, 255, 0.5);
            border-radius: 30px;
            image: url(:/image/music_next.png);
        }
        
        #btnNext:hover {
            background: rgba(255, 255, 255, 0.3);
            border: 2px solid rgba(255, 255, 255, 0.7);
        }
        
        #btnNext:pressed {
            background: rgba(255, 255, 255, 0.4);
        }
        
        /* CD封面 */
        #cdLabel {
            background: white;
            border: 5px solid rgba(255, 255, 255, 0.8);
            border-radius: 140px;
        }
        
        /* 歌曲标题 */
        #songTitle {
            color: white;
            font-size: 24px;
            font-weight: bold;
        }
        
        /* 艺术家 */
        #artistLabel {
            color: rgba(255, 255, 255, 0.9);
            font-size: 16px;
        }
        
        /* 进度条 */
        #progressSlider {
            height: 6px;
        }
        
        #progressSlider::groove:horizontal {
            background: rgba(255, 255, 255, 0.3);
            height: 6px;
            border-radius: 3px;
        }
        
        #progressSlider::sub-page:horizontal {
            background: white;
            border-radius: 3px;
        }
        
        #progressSlider::handle:horizontal {
            background: white;
            width: 16px;
            height: 16px;
            margin: -5px 0;
            border-radius: 8px;
            border: 2px solid rgba(255, 255, 255, 0.8);
        }
        
        #progressSlider::handle:horizontal:hover {
            background: #f5f5f5;
            width: 18px;
            height: 18px;
            margin: -6px 0;
            border-radius: 9px;
        }
        
        /* 时间标签 */
        #timeLabel {
            color: white;
            font-size: 12px;
        }
        
        /* 功能按钮 */
        #btnFavorite, #btnMode, #btnMenu, #btnVolume {
            background: transparent;
            border: none;
        }
        
        #btnFavorite {
            image: url(:/image/music_favorite.png);
        }
        
        #btnFavorite:hover {
            image: url(:/image/music_favorite_hover.png);
        }
        
        #btnMode {
            background: rgba(255, 255, 255, 0.25);
            border: 2px solid rgba(255, 255, 255, 0.5);
            border-radius: 16px;
            color: white;
            font-size: 12px;
            font-weight: bold;
        }
        
        #btnMode:hover {
            background: rgba(255, 255, 255, 0.35);
            border: 2px solid rgba(255, 255, 255, 0.7);
        }
        
        #btnMode:pressed {
            background: rgba(255, 255, 255, 0.45);
        }
        
        #btnMenu {
            image: url(:/image/music_menu.png);
        }
        
        #btnMenu:hover {
            opacity: 0.8;
        }
        
        #btnVolume {
            image: url(:/image/music_volume.png);
        }
        
        #btnVolume:hover {
            opacity: 0.8;
        }
        
        /* 音量面板 */
        #volumePanel {
            background: rgba(255, 255, 255, 0.95);
            border-radius: 10px;
            border: 2px solid rgba(255, 255, 255, 0.8);
        }
        
        /* 音量滑块 */
        #volumeSlider::groove:vertical {
            background: rgba(0, 0, 0, 0.1);
            width: 6px;
            border-radius: 3px;
        }
        
        #volumeSlider::sub-page:vertical {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #f5576c, stop:1 #f093fb);
            border-radius: 3px;
        }
        
        #volumeSlider::handle:vertical {
            background: white;
            height: 16px;
            width: 16px;
            margin: 0 -5px;
            border-radius: 8px;
            border: 2px solid #f5576c;
        }
    )";
    
    setStyleSheet(styleSheet);
}

void MusicPlayer::scanMusicFiles()
{
    // 扫描音乐目录
    QString musicDir = "/music";
    QDir dir(musicDir);
    
    if (!dir.exists()) {
        qDebug() << "Music directory does not exist:" << musicDir;
        return;
    }
    
    QStringList filters;
    filters << "*.mp3" << "*.wav" << "*.flac" << "*.ogg";
    QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files);
    
    m_songs.clear();
    m_playlistWidget->clear();
    
    foreach (const QFileInfo &fileInfo, fileList) {
        SongInfo song;
        song.filePath = fileInfo.absoluteFilePath();
        song.fileName = fileInfo.fileName();
        
        // 尝试从文件名解析艺术家和标题 (格式: 艺术家 - 歌名.mp3)
        QString baseName = fileInfo.completeBaseName();
        QStringList parts = baseName.split(" - ");
        if (parts.size() >= 2) {
            song.artist = parts[0].trimmed();
            song.title = parts[1].trimmed();
        } else {
            song.artist = "未知艺术家";
            song.title = baseName;
        }
        
        song.duration = 0; // 需要播放时才能获取实际时长
        
        m_songs.append(song);
        
        // 添加到列表
        QString displayText = song.title + "\n" + song.artist;
        m_playlistWidget->addItem(displayText);
    }
    
    qDebug() << "Found" << m_songs.size() << "songs";
}

void MusicPlayer::playSong(int index)
{
    if (index < 0 || index >= m_songs.size()) {
        return;
    }
    
    stopPlayback();
    
    m_currentIndex = index;
    const SongInfo &song = m_songs[index];
    
    // 更新UI
    m_songTitleLabel->setText(song.title);
    m_artistLabel->setText(song.artist);
    m_playlistWidget->setCurrentRow(index);
    
    // 使用aplay播放
    QStringList args;
    args << song.filePath;
    
    m_playerProcess->start("aplay", args);
    
    if (m_playerProcess->waitForStarted()) {
        m_playerState = PlayingState;
        m_currentPosition = 0;
        m_isPaused = false;
        m_progressTimer->start(500); // 每0.5秒更新一次进度
        updatePlayButton();
        
        // 启动CD旋转
        m_cdWidget->startRotation();
        
        qDebug() << "Playing:" << song.filePath;
    } else {
        qDebug() << "Failed to start aplay";
    }
}

void MusicPlayer::stopPlayback()
{
    if (m_playerProcess && m_playerProcess->state() != QProcess::NotRunning) {
        m_playerProcess->kill();
        m_playerProcess->waitForFinished(1000);
    }
    
    if (m_progressTimer) {
        m_progressTimer->stop();
    }
    
    // 停止并重置CD旋转
    m_cdWidget->resetRotation();
    
    m_playerState = StoppedState;
    m_currentPosition = 0;
    m_isPaused = false;
    updatePlayButton();
}

void MusicPlayer::pausePlayback()
{
    if (m_playerProcess && m_playerProcess->state() == QProcess::Running) {
        // ALSA的aplay不支持暂停，所以我们记录位置并停止
        m_pausePosition = m_currentPosition;
        m_playerProcess->kill();
        m_playerProcess->waitForFinished();
        m_progressTimer->stop();
        m_playerState = PausedState;
        m_isPaused = true;
        updatePlayButton();
        
        // 暂停CD旋转
        m_cdWidget->stopRotation();
    }
}

void MusicPlayer::resumePlayback()
{
    if (m_currentIndex >= 0 && m_currentIndex < m_songs.size()) {
        const SongInfo &song = m_songs[m_currentIndex];
        
        // aplay不支持从特定位置开始播放
        // 作为替代方案，我们重新开始播放
        // 如果需要支持暂停/恢复，需要使用其他播放器如ffplay或自己实现
        QStringList args;
        args << song.filePath;
        
        m_playerProcess->start("aplay", args);
        
        if (m_playerProcess->waitForStarted()) {
            m_playerState = PlayingState;
            m_isPaused = false;
            m_progressTimer->start(500);
            updatePlayButton();
            
            // 恢复CD旋转
            m_cdWidget->startRotation();
        }
    }
}

void MusicPlayer::onPlayPauseClicked()
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
        pausePlayback();
    } else if (m_playerState == PausedState) {
        resumePlayback();
    }
}

void MusicPlayer::onPreviousClicked()
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

void MusicPlayer::onNextClicked()
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

void MusicPlayer::onModeClicked()
{
    m_playMode = static_cast<PlayMode>((m_playMode + 1) % 4);
    updateModeButton();
}

void MusicPlayer::onVolumeClicked()
{
    if (m_volumePanel->isVisible()) {
        m_volumePanel->hide();
    } else {
        // 定位音量面板在按钮上方
        QPoint buttonPos = m_volumeButton->mapTo(this, QPoint(0, 0));
        m_volumePanel->move(buttonPos.x() - 15, buttonPos.y() - 210);
        m_volumePanel->show();
        m_volumePanel->raise();
    }
}

void MusicPlayer::onFavoriteClicked()
{
    // 收藏功能的实现
    qDebug() << "Favorite clicked";
}

void MusicPlayer::onMenuClicked()
{
    // 菜单功能的实现
    qDebug() << "Menu clicked";
}

void MusicPlayer::onSongSelected(QListWidgetItem *item)
{
    int index = m_playlistWidget->row(item);
    playSong(index);
}

void MusicPlayer::onSliderPressed()
{
    m_isSliderPressed = true;
}

void MusicPlayer::onSliderReleased()
{
    m_isSliderPressed = false;
    
    // aplay不支持seek功能
    // 如果需要支持，需要使用其他播放器
    qDebug() << "Seek is not supported with aplay";
}

void MusicPlayer::updateProgress()
{
    if (m_playerState == PlayingState && !m_isSliderPressed) {
        m_currentPosition++;
        
        if (!m_isSliderPressed && m_progressSlider->maximum() > 0) {
            m_progressSlider->setValue(m_currentPosition);
        }
        
        updateTimeLabels();
    }
}

void MusicPlayer::onPlayerFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
    
    if (m_isPaused) {
        return; // 如果是暂停导致的结束，不处理
    }
    
    qDebug() << "Playback finished";
    
    // 根据播放模式处理
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

void MusicPlayer::onPlayerError(QProcess::ProcessError error)
{
    qDebug() << "Player error:" << error;
    stopPlayback();
}

void MusicPlayer::updatePlayButton()
{
    if (m_playerState == PlayingState) {
        m_playPauseButton->setProperty("playing", "true");
    } else {
        m_playPauseButton->setProperty("playing", "false");
    }
    m_playPauseButton->style()->unpolish(m_playPauseButton);
    m_playPauseButton->style()->polish(m_playPauseButton);
}

void MusicPlayer::updateModeButton()
{
    QString tooltip;
    QString buttonText;
    
    switch (m_playMode) {
    case SequentialMode:
        tooltip = "顺序播放";
        buttonText = "顺序";
        break;
    case LoopMode:
        tooltip = "列表循环";
        buttonText = "循环";
        break;
    case RandomMode:
        tooltip = "随机播放";
        buttonText = "随机";
        break;
    case SingleLoopMode:
        tooltip = "单曲循环";
        buttonText = "单曲";
        break;
    }
    
    m_modeButton->setToolTip(tooltip);
    m_modeButton->setText(buttonText);
}

void MusicPlayer::updateTimeLabels()
{
    m_currentTimeLabel->setText(formatTime(m_currentPosition));
    
    if (m_currentIndex >= 0 && m_currentIndex < m_songs.size()) {
        int duration = m_songs[m_currentIndex].duration;
        if (duration > 0) {
            m_totalTimeLabel->setText(formatTime(duration));
            m_progressSlider->setMaximum(duration);
        }
    }
}

void MusicPlayer::setVolume(int volume)
{
    m_volumeLevel = volume;
    
    // 使用amixer设置ALSA音量（如果失败不影响播放）
    QProcess *volumeProcess = new QProcess();
    volumeProcess->start("amixer", QStringList() << "set" << "Master" << QString::number(volume) + "%");
    
    // 异步执行，不等待完成
    connect(volumeProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            volumeProcess, &QProcess::deleteLater);
}

QString MusicPlayer::formatTime(int seconds)
{
    int minutes = seconds / 60;
    int secs = seconds % 60;
    return QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0'));
}
