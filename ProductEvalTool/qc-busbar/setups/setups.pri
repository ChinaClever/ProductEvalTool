
INCLUDEPATH += $$PWD

include(users/users.pri)

HEADERS += \
    $$PWD/setting.h \
    $$PWD/setup_mainwid.h \
    $$PWD/versiondlg.h
	
SOURCES += \
    $$PWD/setting.cpp \
    $$PWD/setup_mainwid.cpp \
    $$PWD/versiondlg.cpp

FORMS += \
    $$PWD/setting.ui \
    $$PWD/setup_mainwid.ui \
    $$PWD/versiondlg.ui




