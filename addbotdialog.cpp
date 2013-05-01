#include    "addbotdialog.h"
#include "ui_addbotdialog.h"

#include <QInputDialog>

#include <QSettings>
#include <QDir>


// static public methods

QString AddBotDialog::settingsPath()
{
    return qApp->applicationDirPath() + QDir::separator() + "settings.ini";
}


// ctor/dtor

AddBotDialog::AddBotDialog(QWidget *parent) : QDialog(parent), ui(new Ui::AddBotDialog)
{
    ui->setupUi(this);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);

    connect(ui->emailLineEdit,        SIGNAL(textChanged(QString)), SLOT(textFieldTextChanged()));
    connect(ui->passwordLineEdit,     SIGNAL(textChanged(QString)), SLOT(textFieldTextChanged()));
    connect(ui->dbKeyLineEdit,        SIGNAL(textChanged(QString)), SLOT(textFieldTextChanged()));
    connect(ui->profileLineEdit,      SIGNAL(textChanged(QString)), SLOT(textFieldTextChanged()));
    connect(ui->customDBPathLineEdit, SIGNAL(textChanged(QString)), SLOT(textFieldTextChanged()));

    connect(ui->defaultDBPathRadioButton, SIGNAL(clicked()), SLOT(textFieldTextChanged()));
    connect(ui->customDBPathRadioButton,  SIGNAL(clicked()), SLOT(textFieldTextChanged()));
}

AddBotDialog::~AddBotDialog()
{
    delete ui;
}


// public slots

void AddBotDialog::accept()
{
    QString botName = QInputDialog::getText(this, qApp->applicationName(), tr("Bot name:"));
    if (!botName.isEmpty())
    {
        QSettings settings(settingsPath(), QSettings::IniFormat);
        settings.beginGroup(botName);

        settings.setValue("email",    ui->emailLineEdit->text());
        settings.setValue("password", ui->passwordLineEdit->text());
        settings.setValue("dbKey",   ui->dbKeyLineEdit->text());
        settings.setValue("profile",  ui->profileLineEdit->text());

        bool isCustomDBPath = ui->customDBPathRadioButton->isChecked();
        settings.setValue("isCustomDBPath", isCustomDBPath);
        if (isCustomDBPath)
            settings.setValue("dbPath", ui->customDBPathLineEdit->text());

        settings.endGroup();

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
