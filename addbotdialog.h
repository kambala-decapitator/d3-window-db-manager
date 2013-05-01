#ifndef ADDBOTDIALOG_H
#define ADDBOTDIALOG_H

#include <QDialog>


namespace Ui { class AddBotDialog; }

class AddBotDialog : public QDialog
{
    Q_OBJECT
    
public:
    static QString settingsPath();

    explicit AddBotDialog(QWidget *parent = 0);
    virtual ~AddBotDialog();

public slots:
    virtual void accept();

private slots:
    void textFieldTextChanged();
    
private:
    Ui::AddBotDialog *ui;
};

#endif // ADDBOTDIALOG_H
