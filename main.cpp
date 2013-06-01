#include "d3windowdbmanager.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName("kambala");
    a.setApplicationName("D3WindowDBManager");
    a.setApplicationVersion(NVER_STRING);

    D3WindowDBManager w;
    w.show();

    return a.exec();
}
