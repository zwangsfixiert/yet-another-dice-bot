#-------------------------------------------------
#
# Project created by QtCreator 2015-04-02T12:04:22
#
#-------------------------------------------------

QT       += core gui network webkitwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = YADB
TEMPLATE = app

CONFIG += c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    primedice.cpp \
    socketio.cpp \
    profilewidget.cpp \
    profilemanagerform.cpp \
    faucetdialog.cpp \
    webview.cpp \
    consolewidget.cpp \
    usermenuframe.cpp \
    userinfoframe.cpp \
    betinfoframe.cpp

HEADERS  += mainwindow.hpp \
    primedice.hpp \
    profile.hpp \
    socketio.hpp \
    profilewidget.hpp \
    profilemanagerform.hpp \
    faucetdialog.hpp \
    webview.hpp \
    consolewidget.hpp \
    usermenuframe.hpp \
    userinfoframe.hpp \
    betinfoframe.hpp

FORMS    += mainwindow.ui \
    profilewidget.ui \
    profilemanagerform.ui \
    faucetdialog.ui \
    consolewidget.ui \
    usermenuframe.ui \
    userinfoframe.ui \
    betinfoframe.ui

LIBS += /home/ebola/Projects/YADB/libsioclient_tls.a
LIBS += -lboost_system
LIBS += -lssl
LIBS += -lcrypto
