#ifndef ADDBOTDIALOG_H
#define ADDBOTDIALOG_H

#include <QDialog>

#include "botinfo.h"


namespace Ui { class AddBotDialog; }
class QLineEdit;

class AddBotDialog : public QDialog
{
    Q_OBJECT
    
public:
    static QString settingsPath();
    static void selectDBPath(QLineEdit *lineEdit);

    explicit AddBotDialog(const QString &defaultDBPath, QWidget *parent = 0);
    virtual ~AddBotDialog();

    BotInfo botInfo() const;

public slots:
    virtual void accept();

private slots:
    void textFieldTextChanged();

    void selectProfile();
    void selectDBPath();

protected:
    Ui::AddBotDialog *ui;

    QString _botName, _defaultDBPath;
};

#endif // ADDBOTDIALOG_H
