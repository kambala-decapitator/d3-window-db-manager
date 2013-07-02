# basic config
TARGET = D3WindowDBManager
TEMPLATE = app

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG(release, debug|release): DEFINES += QT_NO_DEBUG_OUTPUT QT_NO_WARNING_OUTPUT

# app version
NVER1 = 1
NVER2 = 1
NVER3 = 0
NVER4 = 0

      greaterThan(NVER4, 0): NVER_STRING_LAST = $$sprintf("%1.%2", $$NVER3, $$NVER4)
else: greaterThan(NVER3, 0): NVER_STRING_LAST = $$sprintf("%1", $$NVER3)

isEmpty(NVER_STRING_LAST): VERSION = $$sprintf("%1.%2", $$NVER1, $$NVER2)
else                     : VERSION = $$sprintf("%1.%2.%3", $$NVER1, $$NVER2, $$NVER_STRING_LAST)

DEFINES += NVER1=$$NVER1 \
           NVER2=$$NVER2 \
           NVER3=$$NVER3 \
           NVER4=$$NVER4 \
           NVER_STRING=$$sprintf("\"\\\"%1\\\"\"", $$VERSION)

# files
SOURCES += main.cpp \
           d3windowdbmanager.cpp \
           addbotdialog.cpp \
           editbotdialog.cpp

HEADERS += d3windowdbmanager.h \
           botinfo.h \
           addbotdialog.h \
           editbotdialog.h

FORMS += d3windowdbmanager.ui \
         addbotdialog.ui

RESOURCES += resources/d3windowdbmanager.qrc

LIBS += -luser32 \
        -lgdi32

RC_FILE = resources/d3windowdbmanager.rc

TRANSLATIONS += resources/translations/d3windowdbmanager_ru.ts \
                resources/translations/d3windowdbmanager.ts
