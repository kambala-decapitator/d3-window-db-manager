#ifndef ADDBOTDIALOG_H
#define ADDBOTDIALOG_H

#include <QDialog>

namespace Ui {
class AddBotDialog;
}

class AddBotDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit AddBotDialog(QWidget *parent = 0);
    ~AddBotDialog();
    
private:
    Ui::AddBotDialog *ui;
};

#endif // ADDBOTDIALOG_H
