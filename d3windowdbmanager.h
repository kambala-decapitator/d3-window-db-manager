#ifndef D3WINDOWDBMANAGER_H
#define D3WINDOWDBMANAGER_H

#include <QWidget>

#include <QProcess>

#include <Windows.h>

#include "botinfo.h"


namespace Ui { class D3WindowDBManagerClass; }

class D3WindowDBManager : public QWidget
{
    Q_OBJECT

public:
    static const QString kD3ExeName;

    D3WindowDBManager(QWidget *parent = 0);
    virtual ~D3WindowDBManager();

    void addWindow(HWND hwnd) { _windows << hwnd; }

protected:
    void closeEvent(QCloseEvent *e);

private slots:
    void startGame();
    void readD3StarterOutput();
    void startLauncher();

    void buildWindowList();
    void tileWindows();

    void highlightWindow();
    void shrinkWindow();
    void restoreWindowSize();

    void selectD3Path();
    void selectDBPath();

    void addBot();
    void startAllBots();

private:
    Ui::D3WindowDBManagerClass *ui;

    QList<HWND> _windows;
    QProcess _d3StarterProc;
    QList<BotInfo> _bots;

    HWND currentWindow() const;
    int screenWidth()  const { return ::GetSystemMetrics(SM_CXSCREEN); }
    int screenHeight() const { return ::GetSystemMetrics(SM_CYSCREEN); }
    QString d3Path() const;

    void shrinkWindowWithIndex(int windowIndex) const;

    void loadSettings();
    void saveSettings() const;
};

#endif // D3WINDOWDBMANAGER_H
