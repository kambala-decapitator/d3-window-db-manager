#include "addbotdialog.h"
#include "ui_addbotdialog.h"

AddBotDialog::AddBotDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddBotDialog)
{
    ui->setupUi(this);
}

AddBotDialog::~AddBotDialog()
{
    delete ui;
}
