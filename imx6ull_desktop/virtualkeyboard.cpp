#include "virtualkeyboard.h"
#include <QDebug>

VirtualKeyboard::VirtualKeyboard(QWidget *parent)
    : QWidget(parent)
    , targetLineEdit(nullptr)
    , currentMode(0)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_DeleteOnClose, false);
    
    mainLayout = new QVBoxLayout(this);
    
    /* 标题 */
    titleLabel = new QLabel("虚拟键盘", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; padding: 5px;");
    mainLayout->addWidget(titleLabel);
    
    /* 键盘布局 */
    keyLayout = new QGridLayout();
    keyLayout->setSpacing(5);
    mainLayout->addLayout(keyLayout);
    
    /* 控制按钮 */
    controlLayout = new QHBoxLayout();
    
    switchModeButton = new QPushButton("ABC", this);
    switchModeButton->setMinimumHeight(40);
    connect(switchModeButton, SIGNAL(clicked()), this, SLOT(onSwitchModeClicked()));
    
    backspaceButton = new QPushButton("删除", this);
    backspaceButton->setMinimumHeight(40);
    connect(backspaceButton, SIGNAL(clicked()), this, SLOT(onBackspaceClicked()));
    
    clearButton = new QPushButton("清空", this);
    clearButton->setMinimumHeight(40);
    connect(clearButton, SIGNAL(clicked()), this, SLOT(onClearClicked()));
    
    doneButton = new QPushButton("完成", this);
    doneButton->setMinimumHeight(40);
    doneButton->setStyleSheet("background-color: #4CAF50; color: white;");
    connect(doneButton, SIGNAL(clicked()), this, SLOT(onDoneClicked()));
    
    controlLayout->addWidget(switchModeButton);
    controlLayout->addWidget(backspaceButton);
    controlLayout->addWidget(clearButton);
    controlLayout->addWidget(doneButton);
    
    mainLayout->addLayout(controlLayout);
    
    /* 设置窗口样式 */
    setStyleSheet("QWidget { background-color: #f0f0f0; }"
                  "QPushButton { background-color: white; border: 1px solid #ccc; "
                  "border-radius: 3px; font-size: 14px; min-height: 35px; }"
                  "QPushButton:pressed { background-color: #ddd; }");
    
    /* 创建初始键盘 */
    createNumericKeys();
    
    /* 设置窗口大小 */
    setMinimumSize(400, 300);
}

VirtualKeyboard::~VirtualKeyboard()
{
}

void VirtualKeyboard::setLineEdit(QLineEdit *lineEdit)
{
    targetLineEdit = lineEdit;
}

void VirtualKeyboard::showKeyboard()
{
    if (parentWidget()) {
        /* 居中显示在父窗口 */
        QPoint parentCenter = parentWidget()->rect().center();
        move(parentCenter.x() - width() / 2, parentCenter.y() - height() / 2);
    }
    show();
    raise();
    activateWindow();
}

void VirtualKeyboard::hideKeyboard()
{
    hide();
}

void VirtualKeyboard::onKeyClicked()
{
    if (!targetLineEdit) {
        return;
    }
    
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (button) {
        QString text = button->text();
        targetLineEdit->insert(text);
    }
}

void VirtualKeyboard::onBackspaceClicked()
{
    if (!targetLineEdit) {
        return;
    }
    
    targetLineEdit->backspace();
}

void VirtualKeyboard::onClearClicked()
{
    if (!targetLineEdit) {
        return;
    }
    
    targetLineEdit->clear();
}

void VirtualKeyboard::onDoneClicked()
{
    emit keyboardClosed();
    hideKeyboard();
}

void VirtualKeyboard::onSwitchModeClicked()
{
    currentMode = (currentMode + 1) % 3;
    
    switch (currentMode) {
    case 0:
        switchModeButton->setText("ABC");
        createNumericKeys();
        break;
    case 1:
        switchModeButton->setText("abc");
        createAlphaKeys(false);
        break;
    case 2:
        switchModeButton->setText("123");
        createAlphaKeys(true);
        break;
    }
}

void VirtualKeyboard::clearKeyLayout()
{
    QLayoutItem *item;
    while ((item = keyLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
}

void VirtualKeyboard::createNumericKeys()
{
    clearKeyLayout();
    
    /* 数字键盘布局 */
    QString keys[4][3] = {
        {"1", "2", "3"},
        {"4", "5", "6"},
        {"7", "8", "9"},
        {"0", ".", "-"}
    };
    
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 3; col++) {
            QPushButton *button = new QPushButton(keys[row][col], this);
            button->setMinimumSize(80, 50);
            connect(button, SIGNAL(clicked()), this, SLOT(onKeyClicked()));
            keyLayout->addWidget(button, row, col);
        }
    }
}

void VirtualKeyboard::createAlphaKeys(bool uppercase)
{
    clearKeyLayout();
    
    /* 字母键盘布局 */
    QString row1 = uppercase ? "QWERTYUIOP" : "qwertyuiop";
    QString row2 = uppercase ? "ASDFGHJKL" : "asdfghjkl";
    QString row3 = uppercase ? "ZXCVBNM" : "zxcvbnm";
    
    /* 第一行 */
    for (int i = 0; i < row1.length(); i++) {
        QPushButton *button = new QPushButton(QString(row1[i]), this);
        button->setMinimumSize(35, 45);
        connect(button, SIGNAL(clicked()), this, SLOT(onKeyClicked()));
        keyLayout->addWidget(button, 0, i);
    }
    
    /* 第二行 */
    for (int i = 0; i < row2.length(); i++) {
        QPushButton *button = new QPushButton(QString(row2[i]), this);
        button->setMinimumSize(35, 45);
        connect(button, SIGNAL(clicked()), this, SLOT(onKeyClicked()));
        keyLayout->addWidget(button, 1, i);
    }
    
    /* 第三行 */
    for (int i = 0; i < row3.length(); i++) {
        QPushButton *button = new QPushButton(QString(row3[i]), this);
        button->setMinimumSize(35, 45);
        connect(button, SIGNAL(clicked()), this, SLOT(onKeyClicked()));
        keyLayout->addWidget(button, 2, i + 1);
    }
    
    /* 空格键 */
    QPushButton *spaceButton = new QPushButton("空格", this);
    spaceButton->setMinimumSize(200, 45);
    connect(spaceButton, &QPushButton::clicked, [this]() {
        if (targetLineEdit) {
            targetLineEdit->insert(" ");
        }
    });
    keyLayout->addWidget(spaceButton, 3, 2, 1, 4);
}

void VirtualKeyboard::createKeys()
{
    createNumericKeys();
}
