#ifndef VIRTUALKEYBOARD_H
#define VIRTUALKEYBOARD_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

class VirtualKeyboard : public QWidget
{
    Q_OBJECT

public:
    explicit VirtualKeyboard(QWidget *parent = nullptr);
    ~VirtualKeyboard();

    void setLineEdit(QLineEdit *lineEdit);
    void showKeyboard();
    void hideKeyboard();

signals:
    void keyboardClosed();

private slots:
    void onKeyClicked();
    void onBackspaceClicked();
    void onClearClicked();
    void onDoneClicked();
    void onSwitchModeClicked();

private:
    void clearKeyLayout();
    void createNumericKeys();
    void createAlphaKeys(bool uppercase);
    void createKeys();

    QLineEdit *targetLineEdit;
    QVBoxLayout *mainLayout;
    QGridLayout *keyLayout;
    QHBoxLayout *controlLayout;
    QLabel *titleLabel;
    QPushButton *switchModeButton;
    QPushButton *backspaceButton;
    QPushButton *clearButton;
    QPushButton *doneButton;
    
    int currentMode;  // 0: 数字, 1: 小写字母, 2: 大写字母
};

#endif // VIRTUALKEYBOARD_H
