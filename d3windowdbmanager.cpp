#include    "d3windowdbmanager.h"
#include "ui_d3windowdbmanager.h"
#include "addbotdialog.h"
#include "editbotdialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QCloseEvent>
#include <QMenu>
#include <QInputDialog>
#include <QClipboard>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QUrl>
#include <QSettings>
#include <QTimer>
#include <QRegExp>
#include <QXmlStreamReader>
#include <QTranslator>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#  define LPWSTR_TO_QSTRING(wstr) QString::fromUtf16(reinterpret_cast<const ushort *>(wstr))
#  define QSTRING_TO_LPCWSTR(s)   reinterpret_cast<LPCWSTR>(s.utf16())
#else
#  define LPWSTR_TO_QSTRING(wstr) QString::fromUtf16(wstr)
#  define QSTRING_TO_LPCWSTR(s)   s.utf16()
#endif

static const int kMaxStringLength = 50;


class EnumWindowsHelper
{
public:
    static D3WindowDBManager *d3WindowDBManager;

    static void findD3Windows()
    {
        ::EnumWindows(EnumD3WindowsProc, 0);
    }

    static void minimizeDemonbuddies()
    {
        ::EnumWindows(EnumDbWindowsProc, 0);
    }

    static void renameD3WindowsToBattleTag()
    {
        ::EnumWindows(EnumDbWindowsProc2, 0);
    }

    static void findNewDemonbuddyWindowByPid()
    {
        ::EnumWindows(FindNewDemonbuddyWindowByPidProc, 0);
    }

private:
    static BOOL CALLBACK EnumD3WindowsProc(_In_ HWND hwnd, _In_ LPARAM lParam)
    {
        Q_UNUSED(lParam);

        WCHAR wndClassWstr[kMaxStringLength];
        ::RealGetWindowClass(hwnd, wndClassWstr, kMaxStringLength);
        if (!wcscmp(wndClassWstr, L"D3 Main Window Class"))
            d3WindowDBManager->addWindow(hwnd);
        return TRUE;
    }

    static BOOL CALLBACK EnumDbWindowsProc(_In_ HWND hwnd, _In_ LPARAM lParam)
    {
        Q_UNUSED(lParam);

        WCHAR wndTitleWstr[kMaxStringLength];
        ::GetWindowText(hwnd, wndTitleWstr, kMaxStringLength);
        if (!wcscmp(wndTitleWstr, L"Demonbuddy") || isDemonbuddyLoggedInWindowTitle(wndTitleWstr))
            ::ShowWindow(hwnd, SW_SHOWMINNOACTIVE);
        return TRUE;
    }

    static BOOL CALLBACK EnumDbWindowsProc2(_In_ HWND hwnd, _In_ LPARAM lParam)
    {
        Q_UNUSED(lParam);

        WCHAR wndTitleWstr[kMaxStringLength];
        ::GetWindowText(hwnd, wndTitleWstr, kMaxStringLength);

        QRegExp re("DB - (.+\\#\\d+) - PID:(\\d+)");
        if (re.indexIn(LPWSTR_TO_QSTRING(wndTitleWstr)) != -1)
            d3WindowDBManager->renameD3WindowToBattleTag(re.cap(1), re.cap(2).toInt());
        return TRUE;
    }

    static BOOL CALLBACK FindNewDemonbuddyWindowByPidProc(_In_ HWND hwnd, _In_ LPARAM lParam)
    {
        Q_UNUSED(lParam);

        DWORD pid;
        ::GetWindowThreadProcessId(hwnd, &pid);
        if (pid == d3WindowDBManager->loginDemonbuddyPid())
        {
            WCHAR wndTitleWstr[kMaxStringLength];
            ::GetWindowText(hwnd, wndTitleWstr, kMaxStringLength);
            if (isDemonbuddyLoggedInWindowTitle(wndTitleWstr))
                d3WindowDBManager->terminateLoginDemonbuddyProc();
        }
        return TRUE;
    }

    static bool isDemonbuddyLoggedInWindowTitle(LPCWSTR wndTitle)
    {
        return LPWSTR_TO_QSTRING(wndTitle).startsWith("DB - ");
    }
};

D3WindowDBManager *EnumWindowsHelper::d3WindowDBManager;


const QString D3WindowDBManager::kD3ExeName("Diablo III.exe");

// D3WindowDBManager ctor/dtor

D3WindowDBManager::D3WindowDBManager(QWidget *parent) : QWidget(parent), ui(new Ui::D3WindowDBManagerClass), _justLogin(false), _appTranslator(0), _qtTranslator(0),
    _launchGameBox(new QGroupBox(this)), _windowsBox(new QGroupBox(this)), _pathsBox(new QGroupBox(this)), _botsBox(new QGroupBox(this))
{
    ui->setupUi(this);
    createLayout();

    QMenu *addBotMenu = new QMenu(ui->addBotButton);
    _addNewBotAction = addBotMenu->addAction(QString(), this, SLOT(addNewBot()));
    _importBotsFromYarAction = addBotMenu->addAction(QString(), this, SLOT(importBotsFromYar()));
    ui->addBotButton->setMenu(addBotMenu);

    loadSettings();
    createLanguageMenu();
    retranslateUi();

    if (ui->d3PathLineEdit->text().isEmpty())
    {
        QString hklmSoftD3Path("Microsoft/Windows/CurrentVersion/Uninstall/Diablo III/InstallLocation");
        QSettings s("HKEY_LOCAL_MACHINE\\SOFTWARE\\", QSettings::NativeFormat);
        QVariant v = s.value("Wow6432Node/" + hklmSoftD3Path);
        if (!v.isValid())
            v = s.value(hklmSoftD3Path);
        ui->d3PathLineEdit->setText(v.toString());
    }

    connect(ui->startD3Button,       SIGNAL(clicked()), SLOT(startGame()));
    connect(ui->startLauncherButton, SIGNAL(clicked()), SLOT(startLauncher()));

    connect(ui->buildWndListButton, SIGNAL(clicked()), SLOT(buildWindowList()));
    connect(ui->tileButton,         SIGNAL(clicked()), SLOT(tileWindows()));

    connect(ui->highlightButton, SIGNAL(clicked()), SLOT(highlightWindow()));
    connect(ui->shrinkButton,    SIGNAL(clicked()), SLOT(shrinkWindow()));
    connect(ui->restoreButton,   SIGNAL(clicked()), SLOT(restoreWindowSize()));

    connect(ui->selectD3PathButton, SIGNAL(clicked()), SLOT(selectD3Path()));
    connect(ui->selectDBPathButton, SIGNAL(clicked()), SLOT(selectDBPath()));

    connect(ui->startAllBotsButton,      SIGNAL(clicked()), SLOT(startAllBots()));

    connect(ui->botsTreeWidget, SIGNAL(doubleClicked(QModelIndex)), SLOT(startBotWithIndex(QModelIndex)));
    connect(ui->botsTreeWidget, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showBotContextMenu(QPoint)));

    connect(ui->aboutButton,   SIGNAL(clicked()), SLOT(about()));
    connect(ui->aboutQtButton, SIGNAL(clicked()), qApp, SLOT(aboutQt()));

    connect(&_d3StarterProc, SIGNAL(readyReadStandardOutput()), SLOT(readD3StarterOutput()));
    connect(&_loginDemonbuddyTimer, SIGNAL(timeout()), SLOT(findNewDemonbuddyWindow()));

    EnumWindowsHelper::d3WindowDBManager = this;
    buildWindowList();
}

D3WindowDBManager::~D3WindowDBManager()
{
    delete ui;
}


// public methods

DWORD D3WindowDBManager::loginDemonbuddyPid() const
{
    Q_PID procInfo = _loginDemonbuddyProc.pid();
    return procInfo ? procInfo->dwProcessId : 0;
}

void D3WindowDBManager::terminateLoginDemonbuddyProc()
{
    _loginDemonbuddyTimer.stop();
    _loginDemonbuddyProc.close();
}

void D3WindowDBManager::renameD3WindowToBattleTag(const QString &btag, int dbPid)
{
    int pidIndex = _dbPids.indexOf(dbPid);
    if (pidIndex == -1)
        return;

    int d3Pid = _d3Pids.at(pidIndex);
    HWND targetHwnd = 0;
    for each (HWND hwnd in _d3Windows)
    {
        DWORD pid;
        ::GetWindowThreadProcessId(hwnd, &pid);
        if (pid == d3Pid)
        {
            targetHwnd = hwnd;
            break;
        }
    }
    if (targetHwnd)
    {
        ::SetWindowText(targetHwnd, QSTRING_TO_LPCWSTR(btag));
        _d3Pids.removeAt(pidIndex);
        _dbPids.removeAt(pidIndex);
    }
}


// protected methods

void D3WindowDBManager::closeEvent(QCloseEvent *e)
{
    saveSettings();
    e->accept();
}

void D3WindowDBManager::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        retranslateUi();
    }
    else
        QWidget::changeEvent(e);
}


// private slots

void D3WindowDBManager::startGame()
{
    _shouldStartDemonbuddy = false;
    startGames(ui->d3InstancesSpinBox->value());
}

void D3WindowDBManager::readD3StarterOutput()
{
    QByteArray output = _d3StarterProc.readAllStandardOutput();
    QRegExp re("Process ID (\\d+) started");
    if (re.indexIn(output) != -1)
        _d3Pids << re.cap(1).toInt();

    if (output.contains("All done!"))
    {
        _d3StarterProc.close();

        QTimer::singleShot(ui->gameDbLaunchDelaySpinBox->value() * 1000, this, SLOT(tileAndLaunchDb()));
    }
}

void D3WindowDBManager::startLauncher()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(d3Path() + "Diablo III Launcher.exe"));
}

void D3WindowDBManager::buildWindowList()
{
    _d3Windows.clear();
    EnumWindowsHelper::findD3Windows();

    ui->windowsComboBox->clear();
    for each (HWND hwnd in _d3Windows)
    {
        WCHAR wndCaptionWstr[kMaxStringLength];
        ::GetWindowText(hwnd, wndCaptionWstr, kMaxStringLength);

        DWORD pid;
        ::GetWindowThreadProcessId(hwnd, &pid);

        ui->windowsComboBox->addItem(QString("%1 (PID: %2)").arg(LPWSTR_TO_QSTRING(wndCaptionWstr)).arg(pid));
    }

    bool enableButton = ui->windowsComboBox->count() > 0;
    ui->tileButton->setEnabled(enableButton);
    ui->shrinkButton->setEnabled(enableButton);
    ui->restoreButton->setEnabled(enableButton);
    ui->highlightButton->setEnabled(enableButton);
}

void D3WindowDBManager::tileWindows()
{
    for (int i = 0; i < _d3Windows.size(); ++i)
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
        if (QFile::exists(QString(d3Path + QDir::separator() + kD3ExeName)))
            ui->d3PathLineEdit->setText(d3Path);
        else
            QMessageBox::critical(this, qApp->applicationName(), tr("This is not a D3 folder: \'%1\' is missing.", "param is D3 executable name").arg(kD3ExeName));
    }
}

void D3WindowDBManager::selectDBPath()
{
    AddBotDialog::selectDBPath(ui->dbPathLineEdit);
}

void D3WindowDBManager::startAllBots()
{
    _shouldStartDemonbuddy = true;
    _justLogin = false;
    _d3Pids.clear();

    int enabledBots = 0;
    for (int i = 0; i < ui->botsTreeWidget->topLevelItemCount(); ++i)
        if (isBotEnabledAt(i))
            ++enabledBots;
    startGames(enabledBots);
}

void D3WindowDBManager::addNewBot()
{
    AddBotDialog dlg(ui->dbPathLineEdit->text(), this);
    if (dlg.exec())
    {
        BotInfo bot = dlg.botInfo();
        _bots << bot;
        createTreeItemFromBot(bot);
    }
}

void D3WindowDBManager::importBotsFromYar()
{
    QString yarBotsPath = QFileDialog::getOpenFileName(this, tr("Select YAR config file"), QString(), tr("XML files (*.xml)"));
    if (!yarBotsPath.isEmpty())
    {
        QFile yarFile(yarBotsPath);
        if (!yarFile.open(QIODevice::ReadOnly | QFile::Text))
        {
            QMessageBox::critical(this, qApp->applicationName(), tr("Error reading YAR config file.\nReason: %1").arg(yarFile.errorString()));
            return;
        }

        QXmlStreamReader xmlReader(&yarFile);
        xmlReader.readNextStartElement(); // ArrayOfBotClass
        while (xmlReader.readNextStartElement()) // BotClass
        {
            BotInfo bot;
            xmlReader.readNextStartElement(); // Name
            bot.name = xmlReader.readElementText();
            xmlReader.readNextStartElement(); // Description
            if (bot.name.isEmpty())
                bot.name = xmlReader.readElementText();
            else
                xmlReader.skipCurrentElement();

            xmlReader.readNextStartElement(); // IsEnabled
            bot.enabled = xmlReader.readElementText() == "true";

            xmlReader.readNextStartElement(); // Demonbuddy
            while (xmlReader.readNextStartElement())
            {
                if (xmlReader.name() == "Location")
                    bot.dbPath = xmlReader.readElementText();
                else if (xmlReader.name() == "Key")
                    bot.dbKey = xmlReader.readElementText();
                else if (xmlReader.name() == "NoFlash")
                    bot.noflash = xmlReader.readElementText() == "true";
                else if (xmlReader.name() == "NoUpdate")
                    bot.noupdate = xmlReader.readElementText() == "true";
                else
                    xmlReader.skipCurrentElement();
            }

            xmlReader.readNextStartElement(); // Diablo
            while (xmlReader.readNextStartElement())
            {
                if (xmlReader.name() == "Username")
                    bot.email = xmlReader.readElementText();
                else if (xmlReader.name() == "Password")
                    bot.password = xmlReader.readElementText();
                else
                    xmlReader.skipCurrentElement();
            }

            xmlReader.readNextStartElement(); // Week
            xmlReader.skipCurrentElement();

            xmlReader.readNextStartElement(); // ProfileSchedule
            while (xmlReader.readNextStartElement())
            {
                if (xmlReader.name() == "Profiles")
                {
                    while (xmlReader.readNextStartElement()) // Profiles
                    {
                        bool isFirst = true; // only the first profile is saved
                        if (isFirst && xmlReader.name() == "Profile")
                        {
                            isFirst = false;
                            while (xmlReader.readNextStartElement()) // Profiles
                            {
                                if (xmlReader.name() == "Location")
                                    bot.profilePath = xmlReader.readElementText();
                                else
                                    xmlReader.skipCurrentElement();
                            }
                        }
                        else
                            xmlReader.skipCurrentElement();
                    }
                }
                else
                    xmlReader.skipCurrentElement();
            }

            // skip all advanced options
            while (xmlReader.readNextStartElement())
                xmlReader.skipCurrentElement();

            _bots << bot;
            createTreeItemFromBot(bot);
        }
    }
}

void D3WindowDBManager::startSelectedBot()
{
    if (ui->botsTreeWidget->currentIndex().isValid())
    {
        _shouldStartDemonbuddy = true;
        _justLogin = false;
        startBotWithIndex(ui->botsTreeWidget->currentIndex());
    }
}

void D3WindowDBManager::loginSelectedBot()
{
    if (ui->botsTreeWidget->currentIndex().isValid())
    {
        _justLogin = true;
        startBotWithIndex(ui->botsTreeWidget->currentIndex());
    }
}

void D3WindowDBManager::launchDbForSelectedBot()
{
    startDemonbuddy(ui->botsTreeWidget->currentIndex().row(), -1);
}

void D3WindowDBManager::copyEmailOfSelectedBot()
{
    qApp->clipboard()->setText(_bots.at(ui->botsTreeWidget->currentIndex().row()).email);
}

void D3WindowDBManager::editSelectedBot()
{
    int botIndex = ui->botsTreeWidget->currentIndex().row();
    if (botIndex == -1)
        return;

    const BotInfo &bot = _bots.at(botIndex);
    EditBotDialog dlg(ui->dbPathLineEdit->text(), this);
    dlg.setWindowTitle(bot.name);
    dlg.setBotInfo(bot);

    if (dlg.exec())
    {
        BotInfo newBot = dlg.botInfo();
        newBot.name = bot.name;
        _bots.replace(botIndex, newBot);

        QTreeWidgetItem *botItem = ui->botsTreeWidget->currentItem();
        botItem->setText(2, newBot.email);
        botItem->setText(3, QFileInfo(newBot.profilePath).completeBaseName());
    }
}

void D3WindowDBManager::renameSelectedBot()
{
    QString oldName = ui->botsTreeWidget->currentItem()->text(0);
    QString newName = QInputDialog::getText(this, tr("Rename"), tr("New bot name:"), QLineEdit::Normal, oldName);
    if (!newName.isEmpty() && oldName != newName)
    {
        _bots[ui->botsTreeWidget->currentIndex().row()].name = newName;
        ui->botsTreeWidget->currentItem()->setText(1, newName);
    }
}

void D3WindowDBManager::deleteSelectedBot()
{
    if (ui->botsTreeWidget->currentIndex().isValid())
    {
        int botIndex = ui->botsTreeWidget->currentIndex().row();
        if (QMessageBox::question(this, qApp->applicationName(), tr("Are you sure you want to delete '%1'?").arg(_bots.at(botIndex).name), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
        {
            _bots.removeAt(botIndex);
            delete ui->botsTreeWidget->currentItem();
        }
    }
}

void D3WindowDBManager::startBotWithIndex(const QModelIndex &index)
{
    _shouldStartDemonbuddy = true;
    _d3Pids.clear();
    _startedBotIndex = index;
    startGames(1);
}

void D3WindowDBManager::showBotContextMenu(const QPoint &p)
{
    if (ui->botsTreeWidget->itemAt(p))
    {
        QAction *startAction = new QAction(tr("Start"), ui->botsTreeWidget);
        connect(startAction, SIGNAL(triggered()), SLOT(startSelectedBot()));

        QAction *loginAction = new QAction(tr("Just login"), ui->botsTreeWidget);
        connect(loginAction, SIGNAL(triggered()), SLOT(loginSelectedBot()));

        QAction *launchDbAction = new QAction(tr("Launch Demonbuddy"), ui->botsTreeWidget);
        connect(launchDbAction, SIGNAL(triggered()), SLOT(launchDbForSelectedBot()));

        QAction *separator = new QAction(ui->botsTreeWidget);
        separator->setSeparator(true);

        QAction *copyEmailAction = new QAction(tr("Copy E-mail"), ui->botsTreeWidget);
        connect(copyEmailAction, SIGNAL(triggered()), SLOT(copyEmailOfSelectedBot()));

        QAction *separator2 = new QAction(ui->botsTreeWidget);
        separator2->setSeparator(true);

        QAction *editAction = new QAction(tr("Edit"), ui->botsTreeWidget);
        connect(editAction, SIGNAL(triggered()), SLOT(editSelectedBot()));

        QAction *renameAction = new QAction(tr("Rename"), ui->botsTreeWidget);
        connect(renameAction, SIGNAL(triggered()), SLOT(renameSelectedBot()));

        QAction *deleteAction = new QAction(tr("Delete"), ui->botsTreeWidget);
        connect(deleteAction, SIGNAL(triggered()), SLOT(deleteSelectedBot()));

        QMenu::exec(QList<QAction *>() << startAction << loginAction << launchDbAction << separator << copyEmailAction << separator2 << editAction << renameAction << deleteAction,
                    ui->botsTreeWidget->viewport()->mapToGlobal(p));
    }
}

void D3WindowDBManager::tileAndLaunchDb()
{
    buildWindowList();

    if (_shouldStartDemonbuddy)
    {
        startDemonbuddies();
        QTimer::singleShot(2000, this, SLOT(minimizeDemonbuddies()));

        if (!_justLogin)
            tileWindows();
        else
            _justLogin = false;
    }
}

void D3WindowDBManager::minimizeDemonbuddies()
{
    EnumWindowsHelper::minimizeDemonbuddies();

    int attempts = 0;
    while (attempts++ < 20 && !_d3Pids.isEmpty())
    {
        EnumWindowsHelper::renameD3WindowsToBattleTag();
        ::Sleep(2000);
    }
    buildWindowList();
}

void D3WindowDBManager::findNewDemonbuddyWindow()
{
    EnumWindowsHelper::findNewDemonbuddyWindowByPid();
}

void D3WindowDBManager::switchLanguage(QAction *action, bool ignoreLocaleCheck)
{
    QString locale = action->data().toString();
    if (!ignoreLocaleCheck)
    {
        if (_lastLocale == locale)
            return;
        _lastLocale = locale;
    }

    if (!_appTranslator)
    {
        _appTranslator = new QTranslator(this);
        qApp->installTranslator(_appTranslator);
    }
    _appTranslator->load(qApp->applicationName() + "_" + locale, translationsPath());

    if (!_qtTranslator)
    {
        _qtTranslator = new QTranslator(this);
        qApp->installTranslator(_qtTranslator);
    }
    _qtTranslator->load("qt_" + locale, translationsPath());
}

void D3WindowDBManager::about()
{
    QString appFullName = qApp->applicationName() + " v" + qApp->applicationVersion(), email("decapitator@ukr.net");
    QMessageBox::about(this, tr("About %1").arg(qApp->applicationName()), QString("<b>%1</b><br><br>").arg(appFullName) + tr("<i>Author:</i> Filipenkov Andrey (kambala)")
                       + QString("<br><i>E-mail:</i> <a href=\"mailto:%1?subject=%2\">%1</a><br><br>").arg(email, appFullName)
                       + QString("<a href=\"http://www.thebuddyforum.com/demonbuddy-forum/plugins/122563-standalone-d3windowdbmanager-v1-0-simple-bot-window-manager.html\">%1</a><br>")
                         .arg(tr("Demonbuddy Forum thread")) + "<a href=\"https://github.com/kambala-decapitator/d3-window-db-manager\">GitHub</a><br><br>"
                       + tr("<b>Credits</b>:<ul>"
                            "<li>Demonbuddy team for the great D3 bot</li>"
                            "<li>guys that created YAR, D3Starter and MultiboxD.III</li>"
                            "<li>and Blizzard for actually creating the game :)</li>"
                            "</ul>"));
}


// private methods

HWND D3WindowDBManager::currentWindow() const
{
    return _d3Windows.at(ui->windowsComboBox->currentIndex());
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
    ::MoveWindow(_d3Windows.at(windowIndex), col * w, row * h, w, h, FALSE);
}

void D3WindowDBManager::loadSettings()
{
    QSettings settings(AddBotDialog::settingsPath(), QSettings::IniFormat);

    restoreGeometry(settings.value("geometry").toByteArray());
    _lastLocale = settings.value("language", QLocale::system().name().left(2)).toString();

    ui->d3InstancesSpinBox->setValue(settings.value("d3Instances", 3).toInt());
    ui->windowsPerRowSpinBox->setValue(settings.value("windowsPerRow", 3).toInt());

    ui->d3PathLineEdit->setText(settings.value("d3Path").toString());
    ui->dbPathLineEdit->setText(settings.value("dbPath").toString());

    ui->useDelayCheckBox->setChecked(settings.value("useDelay").toBool());
    ui->differentDbLaunchDelaySpinBox->setValue(settings.value("differentDbDelay", 10).toInt());
    ui->gameDbLaunchDelaySpinBox->setValue(settings.value("gameDbDelay", 20).toInt());

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
        bot.noflash = settings.value("noflash", true).toBool();
        bot.autostart = settings.value("autostart", true).toBool();
        bot.noupdate = settings.value("noupdate", true).toBool();
        bot.enabled = settings.value("enabled", true).toBool();
        _bots << bot;
        createTreeItemFromBot(bot);
    }
    settings.endArray();

    for (int i = 0; i < ui->botsTreeWidget->columnCount(); ++i)
        ui->botsTreeWidget->resizeColumnToContents(i);
}

void D3WindowDBManager::saveSettings() const
{
    QSettings settings(AddBotDialog::settingsPath(), QSettings::IniFormat);

    settings.setValue("geometry", saveGeometry());
    settings.setValue("language", _lastLocale);

    settings.setValue("d3Instances",   ui->d3InstancesSpinBox->value());
    settings.setValue("windowsPerRow", ui->windowsPerRowSpinBox->value());

    settings.setValue("d3Path", ui->d3PathLineEdit->text());
    settings.setValue("dbPath", ui->dbPathLineEdit->text());

    settings.setValue("useDelay", ui->useDelayCheckBox->isChecked());
    settings.setValue("differentDbDelay", ui->differentDbLaunchDelaySpinBox->value());
    settings.setValue("gameDbDelay", ui->gameDbLaunchDelaySpinBox->value());

    settings.beginWriteArray("bots");
    for (int i = 0; i < _bots.size(); ++i)
    {
        settings.setArrayIndex(i);

        const BotInfo &bot = _bots.at(i);
        settings.setValue("name",      bot.name);
        settings.setValue("email",     bot.email);
        settings.setValue("password",  bot.password);
        settings.setValue("dbKey",     bot.dbKey);
        settings.setValue("profile",   bot.profilePath);
        settings.setValue("dbPath",    bot.dbPath);
        settings.setValue("noflash",   bot.noflash);
        settings.setValue("autostart", bot.autostart);
        settings.setValue("noupdate",  bot.noupdate);
        settings.setValue("enabled",   isBotEnabledAt(i));
    }
    settings.endArray();
}

void D3WindowDBManager::startGames(int n)
{
    if (n)
        _d3StarterProc.start("D3Starter.exe", QStringList() << (d3Path() + kD3ExeName) << QString::number(n), QIODevice::ReadOnly);
    else
        QMessageBox::warning(this, qApp->applicationName(), tr("No bots to start"));
}

void D3WindowDBManager::startDemonbuddies()
{
    if (_startedBotIndex.isValid())
    {
        startDemonbuddy(_startedBotIndex.row(), 0);
        _startedBotIndex = QModelIndex();
    }
    else
        for (int i = 0, j = 0, n = ui->botsTreeWidget->topLevelItemCount(); i < n; ++i)
            if (isBotEnabledAt(i))
            {
                startDemonbuddy(i, j++);
                if (ui->useDelayCheckBox->isChecked() && i < n-1)
                    ::Sleep(ui->differentDbLaunchDelaySpinBox->value() * 1000);
            }
}

void D3WindowDBManager::startDemonbuddy(int botIndex, int d3PidIndex)
{
    const BotInfo &bot = _bots.at(botIndex);
    QStringList params = QStringList() << "-routine=Trinity" << "-key=" + bot.dbKey << "-profile=" + bot.profilePath
                                       << "-bnetaccount=" + bot.email << "-bnetpassword=" + bot.password << "-YarEnableAll";
    if (d3PidIndex > -1)
        params << "-pid=" + QString::number(_d3Pids.at(d3PidIndex));
    if (bot.autostart || _justLogin)
        params << "-autostart";
    if (bot.noflash)
        params << "-noflash";
    if (bot.noupdate)
        params << "-noupdate";

    if (_justLogin)
    {
        _loginDemonbuddyProc.start(bot.dbPath, params, QIODevice::ReadOnly);
        _loginDemonbuddyTimer.start(2000);
    }
    else
    {
        qint64 pid;
        if (QProcess::startDetached(bot.dbPath, params, QString(), &pid) && d3PidIndex > -1)
            _dbPids << pid;
    }
}

bool D3WindowDBManager::isBotEnabledAt(int i) const
{
    return ui->botsTreeWidget->topLevelItem(i)->data(0, Qt::CheckStateRole) == Qt::Checked;
}

void D3WindowDBManager::createTreeItemFromBot(const BotInfo &bot)
{
    QTreeWidgetItem *newBotItem = new QTreeWidgetItem(ui->botsTreeWidget);
    newBotItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
    newBotItem->setData(0, Qt::CheckStateRole, bot.enabled ? Qt::Checked : Qt::Unchecked);
    newBotItem->setText(1, bot.name);
    newBotItem->setText(2, bot.email);
    newBotItem->setText(3, QFileInfo(bot.profilePath).completeBaseName());
}

void D3WindowDBManager::createLayout()
{
    QHBoxLayout *hbl = new QHBoxLayout(_launchGameBox);
    hbl->addWidget(ui->startD3Button);
    hbl->addWidget(ui->d3InstancesSpinBox);
    hbl->addStretch();
    hbl->addWidget(ui->startLauncherButton);

    QVBoxLayout *vbl = new QVBoxLayout(_windowsBox);
    hbl = new QHBoxLayout;
    hbl->addWidget(ui->buildWndListButton);
    hbl->addWidget(ui->tileButton);
    hbl->addStretch();
    hbl->addWidget(ui->windowsPerRowLabel);
    hbl->addWidget(ui->windowsPerRowSpinBox);
    vbl->addLayout(hbl);
    hbl = new QHBoxLayout;
    hbl->addWidget(ui->windowsComboBox);
    hbl->addWidget(ui->shrinkButton);
    hbl->addWidget(ui->restoreButton);
    hbl->addWidget(ui->highlightButton);
    vbl->addLayout(hbl);

    vbl = new QVBoxLayout(_pathsBox);
    hbl = new QHBoxLayout;
    hbl->addWidget(ui->d3PathLabel);
    hbl->addWidget(ui->d3PathLineEdit);
    hbl->addWidget(ui->selectD3PathButton);
    vbl->addLayout(hbl);
    hbl = new QHBoxLayout;
    hbl->addWidget(ui->dbPathLabel);
    hbl->addWidget(ui->dbPathLineEdit);
    hbl->addWidget(ui->selectDBPathButton);
    vbl->addLayout(hbl);

    vbl = new QVBoxLayout(_botsBox);
    hbl = new QHBoxLayout;
    hbl->addWidget(ui->startAllBotsButton);
    hbl->addStretch();
    hbl->addWidget(ui->gameDbLaunchDelayLabel);
    hbl->addWidget(ui->gameDbLaunchDelaySpinBox);
    vbl->addLayout(hbl);
    hbl = new QHBoxLayout;
    hbl->addWidget(ui->addBotButton);
    hbl->addStretch();
    hbl->addWidget(ui->useDelayCheckBox);
    hbl->addWidget(ui->differentDbLaunchDelaySpinBox);
    vbl->addLayout(hbl);
    vbl->addWidget(ui->botsTreeWidget);

    hbl = new QHBoxLayout;
    hbl->addWidget(ui->langButton);
    hbl->addStretch();
    hbl->addWidget(ui->aboutQtButton);
    hbl->addWidget(ui->aboutButton);

    vbl = new QVBoxLayout(this);
    vbl->addWidget(_launchGameBox);
    vbl->addWidget(_windowsBox);
    vbl->addWidget(_pathsBox);
    vbl->addWidget(_botsBox);
    vbl->addLayout(hbl);
}

void D3WindowDBManager::retranslateUi()
{
    _launchGameBox->setTitle(tr("Game"));
    _windowsBox->setTitle(tr("Windows"));
    _pathsBox->setTitle(tr("Paths"));
    _botsBox->setTitle(tr("Bots"));

    _addNewBotAction->setText(tr("New..."));
    _importBotsFromYarAction->setText(tr("Import from YAR..."));
}

void D3WindowDBManager::createLanguageMenu()
{
    QStringList fileNames = QDir(translationsPath(), QString("%1_*.qm").arg(qApp->applicationName())).entryList(QDir::Files);
    if (fileNames.isEmpty())
    {
        ui->langButton->setDisabled(true);
        ui->langButton->setToolTip("No other languages found"); // no need to put it inside tr()
        return;
    }

    QActionGroup *languageActionGroup = new QActionGroup(this);
    connect(languageActionGroup, SIGNAL(triggered(QAction *)), SLOT(switchLanguage(QAction *)));

    QMenu *languageMenu = new QMenu(ui->langButton);
    ui->langButton->setMenu(languageMenu);

    QString defaultLocale("en");
    fileNames.prepend("_" + defaultLocale); // HACK: insert English language
    for each (const QString &fileName in fileNames)
    {
        QTranslator translator;
        translator.load(fileName, translationsPath());

        QString locale = fileName.mid(fileName.indexOf('_') + 1, 2), language = translator.translate("Language", "English", "Your language name");
        if (language.isEmpty())
            language = "English";

        QAction *action = new QAction(language, this);
        action->setCheckable(true);
        action->setData(locale);
        languageMenu->addAction(action);
        languageActionGroup->addAction(action);

        if (_lastLocale == locale)
        {
            action->setChecked(true);
            if (locale != defaultLocale) // no need to create English translators
                switchLanguage(action, true);
        }
    }

    // select English by default
    if (!languageActionGroup->checkedAction())
    {
        languageActionGroup->actions().at(0)->setChecked(true);
        _lastLocale = defaultLocale;
    }
}

QString D3WindowDBManager::translationsPath() const
{
    return qApp->applicationDirPath() + "/translations";
}
