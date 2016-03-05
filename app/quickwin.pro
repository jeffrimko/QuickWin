#-------------------------------------------------
#
# Project created by QtCreator 2014-02-01T10:21:42
#
#-------------------------------------------------

QT       += core gui

LIBS += -lpsapi

QMAKE_CXXFLAGS += -std=c++11


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = quickwin
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp\
        cmdformatter.cpp

HEADERS  += mainwindow.h cmdformatter.h

FORMS    += mainwindow.ui

