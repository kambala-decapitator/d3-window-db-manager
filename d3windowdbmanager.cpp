#include    "d3windowdbmanager.h"
#include "ui_d3windowdbmanager.h"
#include "addbotdialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QCloseEvent>

#include <QUrl>
#include <QSettings>
#include <QTimer>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#  define LPWSTR_TO_QSTRING(wstr) QString::fromUtf16(reinterpret_cast<const ushort *>(wstr))
#  define QSTRING_TO_LPCWSTR(s) reinterpret_cast<LPCWSTR>(s.utf16())
#else
#  define LPWSTR_TO_QSTRING(wstr) QString::fromUtf16(wstr)
#  define QSTRING_TO_LPCWSTR(s) s.utf16()
#endif

static const int n = 50;


class EnumWindowsHelper
{
public:
    static D3WindowDBManager *d3WindowDBManager;

    static void startEnumWindows()
    {
        ::EnumWindows(EnumWindowsProc, 0);
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


const QString D3WindowDBManager::kD3ExeName("Diablo III.exe");

// D3WindowDBManager ctor/dtor

D3WindowDBManager::D3WindowDBManager(QWidget *parent) : QWidget(parent), ui(new Ui::D3WindowDBManagerClass)
{
    ui->setupUi(this);

    loadSettings();

    connect(ui->startD3Button,       SIGNAL(clicked()), SLOT(startGame()));
    connect(ui->startLauncherButton, SIGNAL(clicked()), SLOT(startLauncher()));

    connect(ui->buildWndListButton, SIGNAL(clicked()), SLOT(buildWindowList()));
    connect(ui->tileButton,         SIGNAL(clicked()), SLOT(tileWindows()));

    connect(ui->highlightButton, SIGNAL(clicked()), SLOT(highlightWindow()));
    connect(ui->shrinkButton,    SIGNAL(clicked()), SLOT(shrinkWindow()));
    connect(ui->restoreButton,   SIGNAL(clicked()), SLOT(restoreWindowSize()));

    connect(ui->selectD3PathButton, SIGNAL(clicked()), SLOT(selectD3Path()));
    connect(ui->selectDBPathButton, SIGNAL(clicked()), SLOT(selectDBPath()));

    connect(ui->addBotButton, SIGNAL(clicked()), SLOT(addBot()));

    connect(&_d3StarterProc, SIGNAL(readyReadStandardOutput()), SLOT(readD3StarterOutput()));

    if (ui->d3PathLineEdit->text().isEmpty())
    {
        QSettings s("HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Diablo III\\", QSettings::NativeFormat);
        ui->d3PathLineEdit->setText(s.value("InstallLocation").toString());
    }

    EnumWindowsHelper::d3WindowDBManager = this;
    buildWindowList();
}

D3WindowDBManager::~D3WindowDBManager()
{
    delete ui;
}


// protected methods

void D3WindowDBManager::closeEvent(QCloseEvent *e)
{
    saveSettings();
    e->accept();
}


// private slots

void D3WindowDBManager::startGame()
{
    _d3StarterProc.start("D3Starter.exe", QStringList() << (d3Path() + kD3ExeName) << QString::number(ui->d3InstancesSpinBox->value()), QIODevice::ReadOnly);
}

void D3WindowDBManager::readD3StarterOutput()
{
    if (_d3StarterProc.readAllStandardOutput().contains("All done!"))
    {
        _d3StarterProc.close();

        QTimer::singleShot(25000, this, SLOT(buildWindowList()));
    }
}

void D3WindowDBManager::startLauncher()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(d3Path() + "Diablo III Launcher.exe"));
}

void D3WindowDBManager::buildWindowList()
{
    _windows.clear();
    EnumWindowsHelper::startEnumWindows();

    ui->windowsComboBox->clear();
    int i = 1;
    foreach (HWND hwnd, _windows)
    {
        WCHAR wndCaptionWstr[n];
        ::GetWindowText(hwnd, wndCaptionWstr, n);

        QString caption = LPWSTR_TO_QSTRING(wndCaptionWstr), newCaption;
        int underscoreIndex = caption.indexOf("_");
        if (underscoreIndex != -1)
            newCaption = caption.left(underscoreIndex + 1);
        else
            newCaption = caption + "_";
        newCaption += QString::number(i++);
        ::SetWindowText(hwnd, QSTRING_TO_LPCWSTR(newCaption));

        DWORD pid;
        ::GetWindowThreadProcessId(hwnd, &pid);

        ui->windowsComboBox->addItem(QString("%1 (%2)").arg(newCaption).arg(pid));
    }
}

void D3WindowDBManager::tileWindows()
{
    for (int i = 0; i < _windows.size(); ++i)
        shrinkWindowWithIndex(i);
}

void D3WindowDBManager::highlightWindow()
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
        if (QFile::exists(QString(d3Path + kD3ExeName)))
            ui->d3PathLineEdit->setText(d3Path);
        else
            QMessageBox::critical(this, qApp->applicationName(), tr("This is not a D3 folder: \'%1\' is missing.", "param is D3 executable name").arg(kD3ExeName));
    }
}

void D3WindowDBManager::selectDBPath()
{
    AddBotDialog::selectDBPath(ui->dbPathLineEdit);
}

void D3WindowDBManager::addBot()
{
    AddBotDialog dlg(ui->dbPathLineEdit->text(), this);
    if (dlg.exec())
    {
        BotInfo bot = dlg.botInfo();
        _bots << bot;
        /*QTreeWidgetItem *newBotItem = */new QTreeWidgetItem(ui->botsTreeWidget, QStringList() << bot.name << bot.email << QFileInfo(bot.profilePath).baseName() << bot.dbPath);
    }
}

void D3WindowDBManager::startAllBots()
{
}


// private methods

HWND D3WindowDBManager::currentWindow() const
{
    return _windows.at(ui->windowsComboBox->currentIndex());
}

QString D3WindowDBManager::d3Path() const
{
    return ui->d3PathLineEdit->text() + QDir::separator();
}

void D3WindowDBManager::shrinkWindowWithIndex(int windowIndex) const
{
    int windowsPerRow = ui->windowsPerRowSpinBox->value();
    int row = windowIndex / windowsPerRow, col = windowIndex % windowsPerRow;
    int w = screenWidth() / windowsPerRow, h = screenHeight() / windowsPerRow;
    ::MoveWindow(_windows.at(windowIndex), col * w, row * h, w, h, FALSE);
}

void D3WindowDBManager::loadSettings()
{
    QSettings settings(AddBotDialog::settingsPath(), QSettings::IniFormat);
    QVariant v = settings.value("d3Instances");
    if (v.isValid())
        ui->d3InstancesSpinBox->setValue(v.toInt());

    v = settings.value("windowsPerRow");
    if (v.isValid())
        ui->windowsPerRowSpinBox->setValue(v.toInt());

    ui->d3PathLineEdit->setText(settings.value("d3Path").toString());
    ui->dbPathLineEdit->setText(settings.value("dbPath").toString());

    int n = settings.beginReadArray("bots");
    for (int i = 0; i < n; ++i)
    {
        settings.setArrayIndex(i);

        BotInfo bot;
        bot.name = settings.value("name").toString();
        bot.email = settings.value("email").toString();
        bot.password = settings.value("password").toString();
        bot.dbKey = settings.value("dbKey").toString();
        bot.profilePath = settings.value("profile").toString();
        bot.dbPath = settings.value("dbPath").toString();
        _bots << bot;
        /*QTreeWidgetItem *newBotItem = */new QTreeWidgetItem(ui->botsTreeWidget, QStringList() << bot.name << bot.email << QFileInfo(bot.profilePath).baseName() << bot.dbPath);
    }
    settings.endArray();
}

void D3WindowDBManager::saveSettings() const
{
    QSettings settings(AddBotDialog::settingsPath(), QSettings::IniFormat);
    settings.setValue("d3Instances",   ui->d3InstancesSpinBox->value());
    settings.setValue("windowsPerRow", ui->windowsPerRowSpinBox->value());

    settings.setValue("d3Path", ui->d3PathLineEdit->text());
    settings.setValue("dbPath", ui->dbPathLineEdit->text());

    settings.beginWriteArray("bots");
    for (int i = 0; i < _bots.size(); ++i)
    {
        settings.setArrayIndex(i);

        const BotInfo &bot = _bots.at(i);
        settings.setValue("name",     bot.name);
        settings.setValue("email",    bot.email);
        settings.setValue("password", bot.password);
        settings.setValue("dbKey",    bot.dbKey);
        settings.setValue("profile",  bot.profilePath);
        settings.setValue("dbPath",   bot.dbPath);
    }
    settings.endArray();
}
