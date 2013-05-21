#ifndef BOTINFO_H
#define BOTINFO_H

#include <QString>

struct BotInfo
{
    QString name, email, password, dbKey, profilePath, dbPath;
    bool noflash, autostart, noupdate;
    bool enabled;

    BotInfo() : noflash(true), autostart(true), noupdate(true), enabled(true) {}
};

#endif // BOTINFO_H
