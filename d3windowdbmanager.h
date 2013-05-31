#ifndef D3WINDOWDBMANAGER_H
#define D3WINDOWDBMANAGER_H

#include <QWidget>

#include <QProcess>
#include <QModelIndex>
#include <QTimer>

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

    void addWindow(HWND hwnd) { _d3Windows << hwnd; }

    DWORD loginDemonbuddyPid() const;
    void terminateLoginDemonbuddyProc();

    void renameD3WindowToBattleTag(const QString &btag, int dbPid);

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
    void addBot();

    void startSelectedBot();
    void loginSelectedBot();
    void launchDbForSelectedBot();
    void copyEmailOfSelectedBot();
    void editSelectedBot();
    void renameSelectedBot();
    void deleteSelectedBot();

    void startBotWithIndex(const QModelIndex &index);
    void showBotContextMenu(const QPoint &p);

    void tileAndLaunchDb();
    void minimizeDemonbuddies();

    void findNewDemonbuddyWindow();

private:
    Ui::D3WindowDBManagerClass *ui;

    QList<HWND> _d3Windows;
    QProcess _d3StarterProc, _loginDemonbuddyProc;
    QList<BotInfo> _bots;
    QList<int> _d3Pids, _dbPids;
    QModelIndex _startedBotIndex;
    bool _justLogin, _shouldStartDemonbuddy;
    QTimer _loginDemonbuddyTimer;

    HWND currentWindow() const;
    int screenWidth()  const { return ::GetSystemMetrics(SM_CXSCREEN); }
    int screenHeight() const { return ::GetSystemMetrics(SM_CYSCREEN); }
    QString d3Path() const;

    void shrinkWindowWithIndex(int windowIndex) const;

    void loadSettings();
    void saveSettings() const;

    void startGames(int n);
    void startDemonbuddies();
    void startDemonbuddy(int botIndex, int d3PidIndex);

    bool isBotEnabledAt(int i) const;
    void createTreeItemFromBot(const BotInfo &bot);

    void createLayout();
};

#endif // D3WINDOWDBMANAGER_H
