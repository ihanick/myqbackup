#-------------------------------------------------
#
# Project created by QtCreator 2013-07-24T14:44:09
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = myqbackup
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    filecreationwatcherthread.cpp \
    xbbackupcontroller.cpp \
    noninnodbsyncer.cpp \
    xbpreparer.cpp \
    mysqlconnection.cpp \
    clioption.cc \
    cliparser.cc \
    myqbackupconfiguration.cpp

HEADERS += \
    filecreationwatcherthread.h \
    xbbackupcontroller.h \
    noninnodbsyncer.h \
    xbpreparer.h \
    mysqlconnection.h \
    clioption.h \
    cliparser.h \
    myqbackupconfiguration.h

INCLUDEPATH += /usr/include/mysql
LIBS += -lmysqlclient_r
