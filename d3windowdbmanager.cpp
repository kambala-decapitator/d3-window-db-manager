#include    "d3windowdbmanager.h"
#include "ui_d3windowdbmanager.h"

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif

static const int n = 50;


class EnumWindowsHelper
{
public:
    static D3WindowDBManager *d3WindowDBManager;

    static void startEnumWindows()
    {
        BOOL b = ::EnumWindows(EnumWindowsProc, 0);
        qDebug() << "EnumWindows result" << b;
    }

private:
    static BOOL CALLBACK EnumWindowsProc(_In_ HWND hwnd, _In_ LPARAM lParam)
    {
        Q_UNUSED(lParam);

        WCHAR wndClassWstr[n];
        ::RealGetWindowClass(hwnd, wndClassWstr, n);
        if (!wcscmp(wndClassWstr, L"D3 Main Window Class"))
            d3WindowDBManager->addWindow(hwnd);
        return TRUE;
    }
};

D3WindowDBManager *EnumWindowsHelper::d3WindowDBManager;


D3WindowDBManager::D3WindowDBManager(QWidget *parent) : QMainWindow(parent), ui(new Ui::D3WindowDBManagerClass)
{
    ui->setupUi(this);

    connect(ui->buildWndListButton, SIGNAL(clicked()), SLOT(buildWindowList()));
    connect(ui->flashWindowButton,  SIGNAL(clicked()), SLOT(flashSelectedWindow()));
    connect(ui->shrinkButton,       SIGNAL(clicked()), SLOT(shrinkWindow()));
    connect(ui->restoreButton,      SIGNAL(clicked()), SLOT(restoreWindowSize()));

    EnumWindowsHelper::d3WindowDBManager = this;
}

D3WindowDBManager::~D3WindowDBManager()
{
    delete ui;
}

void D3WindowDBManager::buildWindowList()
{
    _windows.clear();
    EnumWindowsHelper::startEnumWindows();

    ui->windowsComboBox->clear();
    foreach (HWND hwnd, _windows)
    {
        WCHAR wndCaptionWstr[n];
        ::GetWindowText(hwnd, wndCaptionWstr, n);

        DWORD pid;
        ::GetWindowThreadProcessId(hwnd, &pid);

        ui->windowsComboBox->addItem(QString("%1 (%2)").arg(QString::fromUtf16(wndCaptionWstr)).arg(pid));
    }
}

void D3WindowDBManager::tileWindows()
{

}

void D3WindowDBManager::flashSelectedWindow()
{
    HWND selectedWindow = currentWindow();
    ::FlashWindow(selectedWindow, TRUE); // flash the taskbar icon

    // draw a border around the window, credits: http://comp.newsgroups.archived.at/os.ms-windows.programmer.win32/200404/04043011017.html
    HDC hDC = GetWindowDC(selectedWindow);

    // Specify device context attributes
    int iOldROP = SetROP2(hDC, R2_XORPEN); // color mixing mode
    HGDIOBJ hOldBrush = SelectObject(hDC, GetStockObject(NULL_BRUSH)); // select brush into DC
    HPEN hPen = CreatePen(PS_SOLID, 10, RGB(128, 128, 128));
    HGDIOBJ hOldPen = SelectObject(hDC, hPen);

    // Draw window border
    RECT rtWindow;
    GetWindowRect(selectedWindow, &rtWindow);
    ScreenToClient(selectedWindow, (POINT *)&rtWindow.left);
    ScreenToClient(selectedWindow, (POINT *)&rtWindow.right);
    OffsetRect(&rtWindow, -rtWindow.left, -rtWindow.top);
    Rectangle(hDC, rtWindow.left, rtWindow.top, rtWindow.right, rtWindow.bottom);

    // Restore device context attributes
    SelectObject(hDC, hOldPen);
    DeleteObject(hPen);
    SetROP2(hDC, iOldROP);
    SelectObject(hDC, hOldBrush);
    ReleaseDC(selectedWindow, hDC);
}

void D3WindowDBManager::shrinkWindow()
{
    int windowsPerRow = ui->windowsPerRowSpinBox->value(), windowIndex = ui->windowsComboBox->currentIndex();
    int row = windowIndex / windowsPerRow, col = windowIndex % windowsPerRow;
    int w = screenWidth() / windowsPerRow, h = screenHeight() / windowsPerRow;
    ::MoveWindow(currentWindow(), col * w, row * h, w, h, FALSE);
}

void D3WindowDBManager::restoreWindowSize()
{
    ::SetWindowPos(currentWindow(), HWND_TOP, 0, 0, screenWidth(), screenHeight(), SWP_SHOWWINDOW);
}

HWND D3WindowDBManager::currentWindow()
{
    return _windows.at(ui->windowsComboBox->currentIndex());
}
