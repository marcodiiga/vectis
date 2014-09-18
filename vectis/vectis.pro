#-------------------------------------------------
#
# Project created by QtCreator 2014-09-12T15:05:52
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = vectis
TEMPLATE = app


SOURCES += main.cpp\
        vmainwindow.cpp \
    customscrollbar.cpp \
    customcodeedit.cpp

HEADERS  += vmainwindow.h \
    customscrollbar.h \
    customcodeedit.h

FORMS    += vmainwindow.ui

QMAKE_CXXFLAGS += -std=c++11
