#include "editbotdialog.h"
#include "ui_addbotdialog.h"
#include "botinfo.h"


void EditBotDialog::setBotInfo(const BotInfo &bot)
{
    ui->emailLineEdit->setText(bot.email);
    ui->passwordLineEdit->setText(bot.password);
    ui->dbKeyLineEdit->setText(bot.dbKey);
    ui->profileLineEdit->setText(bot.profilePath);

    if (bot.dbPath == _defaultDBPath)
        ui->defaultDBPathRadioButton->setChecked(true);
    else
    {
        ui->customDBPathRadioButton->setChecked(true);
        ui->customDBPathLineEdit->setText(bot.dbPath);
    }

    ui->noflashCheckBox->setChecked(bot.noflash);
    ui->autostartCheckBox->setChecked(bot.autostart);
    ui->noupdateCheckBox->setChecked(bot.noupdate);
}
