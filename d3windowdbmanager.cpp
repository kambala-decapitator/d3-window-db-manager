#include    "d3windowdbmanager.h"
#include "ui_d3windowdbmanager.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>

#include <QUrl>
#include <QProcess>

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


// D3WindowDBManager ctor/dtor

D3WindowDBManager::D3WindowDBManager(QWidget *parent) : QMainWindow(parent), ui(new Ui::D3WindowDBManagerClass)
{
    ui->setupUi(this);

    connect(ui->startD3Button,       SIGNAL(clicked()), SLOT(startGame()));
    connect(ui->startLauncherButton, SIGNAL(clicked()), SLOT(startLauncher()));

    connect(ui->buildWndListButton, SIGNAL(clicked()), SLOT(buildWindowList()));
    connect(ui->tileButton,         SIGNAL(clicked()), SLOT(tileWindows()));

    connect(ui->flashWindowButton,  SIGNAL(clicked()), SLOT(flashSelectedWindow()));
    connect(ui->shrinkButton,       SIGNAL(clicked()), SLOT(shrinkWindow()));
    connect(ui->restoreButton,      SIGNAL(clicked()), SLOT(restoreWindowSize()));

    connect(ui->selectD3PathButton, SIGNAL(clicked()), SLOT(selectD3Path()));

    EnumWindowsHelper::d3WindowDBManager = this;
    buildWindowList();

    ui->d3PathLineEdit->setText("C:\\Diablo III");
}

D3WindowDBManager::~D3WindowDBManager()
{
    delete ui;
}


// private slots

void D3WindowDBManager::startGame()
{
    QProcess *p = new QProcess(this);
    p->start("D3Starter.exe", QStringList() << (ui->d3PathLineEdit->text() + QDir::separator() + "Diablo III.exe") << QString::number(ui->d3InstancesSpinBox->value()));
}

void D3WindowDBManager::startLauncher()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(ui->d3PathLineEdit->text() + QDir::separator() + "Diablo III Launcher.exe"));
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
    for (int i = 0; i < _windows.size(); ++i)
        shrinkWindowWithIndex(i);
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
    shrinkWindowWithIndex(ui->windowsComboBox->currentIndex());
}

void D3WindowDBManager::restoreWindowSize()
{
    ::SetWindowPos(currentWindow(), HWND_TOP, 0, 0, screenWidth(), screenHeight(), SWP_SHOWWINDOW);
}

void D3WindowDBManager::selectD3Path()
{
    QString d3Path = QFileDialog::getExistingDirectory(this, tr("Select D3 folder"), ui->d3PathLineEdit->text());
    if (!d3Path.isEmpty())
    {
        if (QFile::exists(QString(d3Path + QDir::separator() + "Diablo III.exe")))
            ui->d3PathLineEdit->setText(d3Path);
        else
            QMessageBox::critical(this, qApp->applicationName(), tr("This is not a D3 folder: \'Diablo III.exe\' is missing."));
    }
}


// private methods

HWND D3WindowDBManager::currentWindow() const
{
    return _windows.at(ui->windowsComboBox->currentIndex());
}

void D3WindowDBManager::shrinkWindowWithIndex(int windowIndex) const
{
    int windowsPerRow = ui->windowsPerRowSpinBox->value();
    int row = windowIndex / windowsPerRow, col = windowIndex % windowsPerRow;
    int w = screenWidth() / windowsPerRow, h = screenHeight() / windowsPerRow;
    ::MoveWindow(_windows.at(windowIndex), col * w, row * h, w, h, FALSE);
}
