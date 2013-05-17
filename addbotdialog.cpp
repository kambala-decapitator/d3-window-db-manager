#include    "addbotdialog.h"
#include "ui_addbotdialog.h"

#include <QInputDialog>
#include <QFileDialog>

#include <QSettings>
#include <QDir>


// static public methods

QString AddBotDialog::settingsPath()
{
    return qApp->applicationDirPath() + QDir::separator() + "settings.ini";
}

void AddBotDialog::selectDBPath(QLineEdit *lineEdit)
{
    QString dbPath = QFileDialog::getOpenFileName(lineEdit->parentWidget(), tr("Select Demonbuddy.exe"), lineEdit->text(), tr("Demonbuddy executable (*.exe)"));
    if (!dbPath.isEmpty())
        lineEdit->setText(QDir::toNativeSeparators(dbPath));
}


// ctor/dtor

AddBotDialog::AddBotDialog(const QString &defaultDBPath, QWidget *parent) : QDialog(parent), ui(new Ui::AddBotDialog), _defaultDBPath(defaultDBPath)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);

    if (defaultDBPath.isEmpty())
    {
        ui->defaultDBPathRadioButton->setDisabled(true);
        ui->customDBPathRadioButton->setChecked(true);
    }

    connect(ui->emailLineEdit,        SIGNAL(textChanged(QString)), SLOT(textFieldTextChanged()));
    connect(ui->passwordLineEdit,     SIGNAL(textChanged(QString)), SLOT(textFieldTextChanged()));
    connect(ui->dbKeyLineEdit,        SIGNAL(textChanged(QString)), SLOT(textFieldTextChanged()));
    connect(ui->profileLineEdit,      SIGNAL(textChanged(QString)), SLOT(textFieldTextChanged()));
    connect(ui->customDBPathLineEdit, SIGNAL(textChanged(QString)), SLOT(textFieldTextChanged()));

    connect(ui->defaultDBPathRadioButton, SIGNAL(clicked()), SLOT(textFieldTextChanged()));
    connect(ui->customDBPathRadioButton,  SIGNAL(clicked()), SLOT(textFieldTextChanged()));

    connect(ui->selectProfileButton, SIGNAL(clicked()), SLOT(selectProfile()));
    connect(ui->selectDBPathButton,  SIGNAL(clicked()), SLOT(selectDBPath()));
}

AddBotDialog::~AddBotDialog()
{
    delete ui;
}


// public methods

BotInfo AddBotDialog::botInfo() const
{
    BotInfo bot;
    bot.name = _botName;
    bot.email = ui->emailLineEdit->text();
    bot.password = ui->passwordLineEdit->text();
    bot.dbKey = ui->dbKeyLineEdit->text();
    bot.profilePath = ui->profileLineEdit->text();
    bot.dbPath = ui->customDBPathRadioButton->isChecked() ? ui->customDBPathLineEdit->text() : _defaultDBPath;

    bot.noflash = ui->noflashCheckBox->isChecked();
    bot.autostart = ui->autostartCheckBox->isChecked();
    bot.noupdate = ui->noupdateCheckBox->isChecked();

    return bot;
}


// public slots

void AddBotDialog::accept()
{
    QString botName = QInputDialog::getText(this, qApp->applicationName(), tr("Bot name:"));
    if (!botName.isEmpty())
    {
        _botName = botName;
        QDialog::accept();
    }
}


// private slots

void AddBotDialog::textFieldTextChanged()
{
    bool isOkDisabled = ui->emailLineEdit->text().isEmpty() || ui->passwordLineEdit->text().isEmpty() || ui->dbKeyLineEdit->text().isEmpty() || ui->profileLineEdit->text().isEmpty()
                        || (ui->customDBPathRadioButton->isChecked() && ui->customDBPathLineEdit->text().isEmpty());
    ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(isOkDisabled);
}

void AddBotDialog::selectProfile()
{
    QString profilePath = QFileDialog::getOpenFileName(this, tr("Select profile"), ui->profileLineEdit->text(), tr("Demonbuddy profiles (*.xml)"));
    if (!profilePath.isEmpty())
        ui->profileLineEdit->setText(QDir::toNativeSeparators(profilePath));
}

void AddBotDialog::selectDBPath()
{
    selectDBPath(ui->customDBPathLineEdit);
    if (!ui->customDBPathLineEdit->text().isEmpty())
        ui->customDBPathRadioButton->setChecked(true);
}
