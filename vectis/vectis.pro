#-------------------------------------------------
#
# Project created by QtCreator 2014-09-12T15:05:52
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = vectis
TEMPLATE = app

INCLUDEPATH += $$PWD

SOURCES += main.cpp\
        vmainwindow.cpp \
    UI/CodeTextEdit/CodeTextEdit.cpp \
    UI/ScrollBar/ScrollBar.cpp

HEADERS  += vmainwindow.h \
    UI/CodeTextEdit/CodeTextEdit.h \
    UI/ScrollBar/ScrollBar.h

FORMS    += vmainwindow.ui

#QMAKE_CXXFLAGS += -std=c++11
