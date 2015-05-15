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
    UI/CodeTextEdit/Document.cpp \
    UI/CodeTextEdit/Lexers/Lexer.cpp \
    UI/CodeTextEdit/Lexers/CPPLexer.cpp \
    UI/ScrollBar/ScrollBar.cpp \
    UI/TabsBar/TabsBar.cpp

HEADERS  += vmainwindow.h \
    UI/CodeTextEdit/CodeTextEdit.h \
    UI/CodeTextEdit/Document.h \
    UI/CodeTextEdit/Lexers/CPPLexer.h \
    UI/CodeTextEdit/Lexers/Lexer.h \
    UI/ScrollBar/ScrollBar.h \
    UI/TabsBar/TabsBar.h

FORMS    += vmainwindow.ui

!win32 {
    QMAKE_CXXFLAGS += -std=c++14
}

RESOURCES +=
