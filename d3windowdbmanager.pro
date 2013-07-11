# basic config
TARGET = D3WindowDBManager
TEMPLATE = app

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

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
OTHER_FILES += $$RC_FILE

TRANSLATIONS += resources/translations/d3windowdbmanager_ru.ts \
                resources/translations/d3windowdbmanager.ts

# defines
DEFINES += BUILDING_FROM_PRO \ # to set app version in .rc correctly
           NOMINMAX

CONFIG(release, debug|release): {
    IS_RELEASE_BUILD = 1
    DEFINES += QT_NO_DEBUG_OUTPUT \
               QT_NO_WARNING_OUTPUT \
               _USING_V110_SDK71_ # for WinXP support in MSVS2012
}

# custom actions
defineReplace(toNativeSeparators) {
    path = $$1
    path ~= s,/,\\,g
    return($$path)
}

isEmpty(IS_RELEASE_BUILD): OUT_FOLDER = debug
                     else: OUT_FOLDER = release
D3STARTER_DEST_PATH = $$OUT_PWD/$$OUT_FOLDER/D3Starter.exe
!exists($$D3STARTER_DEST_PATH): QMAKE_POST_LINK = copy /B $$toNativeSeparators($$_PRO_FILE_PWD_/resources/D3Starter.exe) $$toNativeSeparators($$D3STARTER_DEST_PATH)
