QT += core gui network xml webkit sql

win32 {
LIBS += libkernel32 \
        libpsapi
}

TARGET = QuiteRSS
TEMPLATE = app

HEADERS += \
    src/VersionNo.h \
    src/updatethread.h \
    src/rsslisting.h \
    src/parsethread.h \
    src/parseobject.h \
    src/optionsdialog.h \
    src/newsview.h \
    src/newsmodel.h \
    src/newsheader.h \
    src/feedsmodel.h \
    src/delegatewithoutfocus.h \
    src/addfeeddialog.h \
    src/aboutdialog.h \
    src/qtsingleapplication/qtsingleapplication.h \
    src/qtsingleapplication/qtlockedfile.h \
    src/qtsingleapplication/qtlocalpeer.h \
    src/updateappdialog.h \
    src/feedpropertiesdialog.h \
    src/sqlite/sqlite3.h
SOURCES += \
    src/updatethread.cpp \
    src/rsslisting.cpp \
    src/parsethread.cpp \
    src/parseobject.cpp \
    src/optionsdialog.cpp \
    src/newsview.cpp \
    src/newsmodel.cpp \
    src/newsheader.cpp \
    src/main.cpp \
    src/feedsmodel.cpp \
    src/delegatewithoutfocus.cpp \
    src/addfeeddialog.cpp \
    src/aboutdialog.cpp \
    src/qtsingleapplication/qtsingleapplication.cpp \
    src/qtsingleapplication/qtlockedfile_win.cpp \
    src/qtsingleapplication/qtlockedfile_unix.cpp \
    src/qtsingleapplication/qtlockedfile.cpp \
    src/qtsingleapplication/qtlocalpeer.cpp \
    src/updateappdialog.cpp \
    src/feedpropertiesdialog.cpp \
    src/sqlite/sqlite3.c

RESOURCES += \
    QuiteRSS.qrc
RC_FILE = QuiteRSSApp.rc

BUILD_DIR = release
if(!debug_and_release|build_pass):CONFIG(debug, debug|release) {
  BUILD_DIR = debug
}

CODECFORTR  = UTF-8
CODECFORSRC = UTF-8
include(lang/lang.pri)

