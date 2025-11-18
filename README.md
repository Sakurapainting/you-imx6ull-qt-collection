# You-IMX6ULL-Qt-Collection

基于 NXP i.MX6ULL 平台的 Qt 嵌入式开发项目集合，包含完整的桌面应用系统、内核配置文件和开发工具脚本。

## 📋 项目简介

本仓库是一个完整的 i.MX6ULL 嵌入式开发解决方案，集成了：
- **Qt 桌面应用**：功能丰富的嵌入式图形界面系统
- **内核配置**：定制的设备树和内核镜像
- **开发工具**：自动化部署和同步脚本
- **版本管理**：Git 工作流和分支管理策略

## 🗂️ 仓库结构

```
imx6ull_everything/
├── imx6ull_desktop/              # Qt 桌面应用程序（主项目）
│   ├── *.cpp, *.h                # 源代码文件
│   ├── *.ui                      # Qt Designer 界面文件
│   ├── icon/                     # 应用图标资源
│   ├── image/                    # 图片资源
│   ├── resources.qrc             # Qt 资源文件
│   ├── imx6ull_desktop.pro       # Qt 项目配置
│   └── README.md                 # 详细项目文档
│
├── build-imx6ull_desktop-*/      # Qt 构建输出目录（已忽略）
│
├── imx6ull-alientek-emmc.dts     # 设备树源文件
├── imx6ull-alientek-emmc.dtb     # 编译后的设备树二进制
├── zImage                        # Linux 内核镜像
│
├── kernel_to_tftpboot.sh         # 内核文件到 TFTP 服务器同步脚本
├── sync_kernel_files.sh          # 内核文件同步到本仓库脚本
├── private_git_sync_pr_fromdevtomain.sh  # Git 分支同步脚本
│
├── .gitignore                    # Git 忽略规则
└── README.md                     # 本文件
```

## 🚀 核心项目

### IMX6ULL Desktop

一个功能完整的嵌入式桌面应用系统，基于 Qt 5.12.9 开发。

**主要功能模块：**

#### 1️⃣ LED 控制
通过 sysfs 接口控制板载 LED，支持开关状态切换。

#### 2️⃣ 传感器监控
- ADC 模拟信号采集（原始值、电压值）
- 温度传感器实时监控
- 光敏传感器环境光检测
- 定时自动刷新（1秒间隔）

#### 3️⃣ 网络设置
- 网络接口管理（有线/无线）
- IP 地址配置（静态/DHCP）
- 网关和 DNS 设置
- 连接状态实时显示

#### 4️⃣ 系统设置
- 屏幕亮度调节（0-255级）
- 系统音量控制
- 时间和日期设置
- 开机自启动配置

#### 5️⃣ 多媒体播放器
- 音频播放（MP3, WAV, FLAC等）
- 播放控制（播放/暂停/上一曲/下一曲）
- 进度条和音量调节
- CD 旋转动画效果

#### 6️⃣ 系统信息
实时显示系统运行状态：
- 主机名、内核版本
- CPU 型号和核心数
- 内存使用情况（总量/已用/可用）
- 存储空间统计
- 系统运行时间和负载
- 网络接口和 IP 地址

#### 7️⃣ 人脸识别 ⭐ 核心功能
集成 SeetaFace2 引擎的完整人脸识别系统：
- **实时人脸检测**：摄像头画面中自动检测人脸
- **人脸录入**：采集并保存人脸特征到本地数据库
- **人脸识别**：识别已录入人脸，显示姓名和相似度
- **数据库管理**：表格展示、删除操作、统计信息
- **虚拟键盘**：触屏友好的输入方式
- **跨平台设计**：ARM 完整功能，PC 预览模式

📖 **详细文档**：[imx6ull_desktop/README.md](./imx6ull_desktop/README.md)

## 🛠️ 技术栈

### 应用层
- **Qt 5.12.9**：跨平台 C++ GUI 框架
  - Qt Widgets：UI 组件库
  - Qt Charts：数据可视化
  - Qt Network：网络通信
  - Qt Multimedia：音频播放

### 计算机视觉
- **OpenCV 3.4.1**：
  - 摄像头视频采集
  - 图像格式转换
  - 图像处理算法

- **SeetaFace2**：开源人脸识别引擎
  - 人脸检测（Face Detection）
  - 关键点定位（Landmark Detection）
  - 人脸识别（Face Recognition）

### 系统层
- **Linux Kernel 4.1.15**：NXP 官方内核
- **Device Tree**：硬件描述和配置
- **sysfs**：用户空间硬件接口

### 开发工具
- **交叉编译器**：arm-linux-gnueabihf-gcc
- **构建系统**：qmake + make
- **版本控制**：Git
- **网络工具**：TFTP、NTP

## 📦 硬件平台

### 目标开发板
- **型号**：NXP i.MX6ULL（ARM Cortex-A7）
- **内存**：512MB DDR3
- **存储**：8GB eMMC
- **显示**：7寸电容触摸屏（800x480）
- **摄像头**：USB 或板载摄像头
- **网络**：以太网 + WiFi（可选）

### 外设支持
- LED 指示灯
- ADC 模拟输入
- 温度传感器
- 光敏传感器
- 音频输出
- USB Host/OTG
- SD/TF 卡槽

## 🔨 编译和部署

### 环境准备
```bash
# 安装交叉编译工具链
sudo apt-get install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf

# 编译 Qt 5.12.9（交叉编译）
# 编译 OpenCV 3.4.1（交叉编译）
# 编译 SeetaFace2（交叉编译）
```

### Qt 项目编译
```bash
cd imx6ull_desktop

# 使用 ARM Qt qmake
/path/to/arm-qt/bin/qmake imx6ull_desktop.pro

# 编译
make

# 生成可执行文件：imx6ull_desktop

# 或使用Qt Creator可视化构建（推荐）
```

## 🌐 开发工作流

### 分支策略
```
main（主分支）
  ↑
  | PR/Merge
  |
dev（开发分支）
  ↑
  | 日常开发
  |
feature/*（功能分支）
```

## 🤝 贡献指南

欢迎提交 Issue 和 Pull Request！

## 📄 许可协议

本项目使用的第三方库遵循各自的开源协议：
- **Qt**：LGPL v3 / GPL v2/v3
- **OpenCV**：Apache 2.0 License
- **SeetaFace2**：BSD 3-Clause License
- **Linux Kernel**：GPL v2

## 📧 联系方式

如有问题或建议，请通过以下方式联系：
- 提交 GitHub Issue
- Pull Request 讨论
- 仓库 Discussions

---

**注意**：部分功能需要特定硬件平台支持。在非目标平台上运行可能需要适配修改。

**最后更新**：2025年11月18日
