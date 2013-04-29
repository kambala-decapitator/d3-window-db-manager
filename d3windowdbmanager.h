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
    void buildWindowList();
    void tileWindows();
    void flashSelectedWindow();
    void shrinkWindow();
    void restoreWindowSize();

private:
    Ui::D3WindowDBManagerClass *ui;

    QList<HWND> _windows;

    HWND currentWindow();
    int screenWidth()  const { return ::GetSystemMetrics(SM_CXSCREEN); }
    int screenHeight() const { return ::GetSystemMetrics(SM_CYSCREEN); }
};

#endif // D3WINDOWDBMANAGER_H
