#ifndef D3WINDOWDBMANAGER_H
#define D3WINDOWDBMANAGER_H

#include <QMainWindow>

#include <Windows.h>


namespace Ui { class D3WindowDBManagerClass; }

class D3WindowDBManager : public QMainWindow
{
    Q_OBJECT

public:
    D3WindowDBManager(QWidget *parent = 0);
    virtual ~D3WindowDBManager();

    void addWindow(HWND hwnd) { _windows << hwnd; }

private slots:
    void startGame();
    void startLauncher();

    void buildWindowList();
    void tileWindows();

    void flashSelectedWindow();
    void shrinkWindow();
    void restoreWindowSize();

    void selectD3Path();

private:
    Ui::D3WindowDBManagerClass *ui;

    QList<HWND> _windows;

    HWND currentWindow() const;
    int screenWidth()  const { return ::GetSystemMetrics(SM_CXSCREEN); }
    int screenHeight() const { return ::GetSystemMetrics(SM_CYSCREEN); }

    void shrinkWindowWithIndex(int windowIndex) const;
};

#endif // D3WINDOWDBMANAGER_H
