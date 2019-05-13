TEMPLATE = app
TARGET = crusta
QT += webenginewidgets sql webchannel

HEADERS += \
    api/eobject.h \
    api/ewebpage.h \
    api/ewindows.h \
    src/bookmarks.h \
    src/browser.h \
    src/cookies.h \
    src/crscheme.h \
    src/database.h \
    src/history.h \
    src/mainwindow.h \
    src/preferences.h \
    src/sidebar.h \
    src/tab.h \
    src/tabwidget.h \
    src/theme.h \
    src/toolbar.h \
    src/utils.h \
    src/webpage.h \
    src/webview.h \
    src/window.h

SOURCES += \
    api/eobject.cpp \
    api/ewebpage.cpp \
    api/ewindows.cpp \
    src/main.cpp \
    src/bookmarks.cpp \
    src/browser.cpp \
    src/cookies.cpp \
    src/crscheme.cpp \
    src/database.cpp \
    src/history.cpp \
    src/mainwindow.cpp \
    src/preferences.cpp \
    src/sidebar.cpp \
    src/tab.cpp \
    src/tabwidget.cpp \
    src/theme.cpp \
    src/toolbar.cpp \
    src/utils.cpp \
    src/webpage.cpp \
    src/webview.cpp \
    src/window.cpp

RESOURCES += \
    resources.qrc \

macx {
LIBS += -framework Foundation -framework Cocoa

OBJECTIVE_HEADERS += \
    mac/mac_utils.h

OBJECTIVE_SOURCES += \
    mac/mac_utils.mm

ICON = mac/logos.icns
}
