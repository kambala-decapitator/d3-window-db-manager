#ifndef EDITBOTDIALOG_H
#define EDITBOTDIALOG_H

#include "addbotdialog.h"


struct BotInfo;

class EditBotDialog : public AddBotDialog
{
public:
    explicit EditBotDialog(const QString &defaultDBPath, QWidget *parent = 0) : AddBotDialog(defaultDBPath, parent) {}
    virtual ~EditBotDialog() {}

    void setBotInfo(const BotInfo &bot);

public slots:
    virtual void accept() { QDialog::accept(); }
};

#endif // EDITBOTDIALOG_H
