#include "d3windowdbmanager.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    D3WindowDBManager w;
    w.show();

    return a.exec();
}
