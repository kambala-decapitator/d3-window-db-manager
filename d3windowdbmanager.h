#ifndef D3WINDOWDBMANAGER_H
#define D3WINDOWDBMANAGER_H

#include <QWidget>

#include <QProcess>
#include <QModelIndex>

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

    void startAllBots();
    void startSelectedBot();
    void loginSelectedBot();
    void addBot();
    void editSelectedBot();
    void deleteSelectedBot();

    void startBotWithIndex(const QModelIndex &index);
    void showBotContextMenu(const QPoint &p);

    void tileAndLaunchDb();
    void minimizeDemonbuddies();

private:
    Ui::D3WindowDBManagerClass *ui;

    QList<HWND> _windows;
    QProcess _d3StarterProc;
    QList<BotInfo> _bots;
    QList<int> _pids;
    QModelIndex _startedBotIndex;
    bool _justLogin;

    HWND currentWindow() const;
    int screenWidth()  const { return ::GetSystemMetrics(SM_CXSCREEN); }
    int screenHeight() const { return ::GetSystemMetrics(SM_CYSCREEN); }
    QString d3Path() const;

    void shrinkWindowWithIndex(int windowIndex) const;

    void loadSettings();
    void saveSettings() const;

    void startGames(int n);
    void startDemonbuddies();
    void startDemonbuddy(int botIndex, int pidIndex);
};

#endif // D3WINDOWDBMANAGER_H
