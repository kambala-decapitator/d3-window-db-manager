#ifndef BOTINFO_H
#define BOTINFO_H

#include <QString>

struct BotInfo
{
    QString name, email, password, dbKey, profilePath, dbPath;
    bool noflash, autostart, noupdate;

    BotInfo() : noflash(true), autostart(true), noupdate(true) {}
};

#endif // BOTINFO_H
