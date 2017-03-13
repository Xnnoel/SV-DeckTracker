#-------------------------------------------------
#
# Project created by QtCreator 2016-06-27T23:10:01
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DeckTracker
TEMPLATE = app


SOURCES += main.cpp\
        frmwindow.cpp \
    cardlist.cpp \
    svdatabase.cpp \
    carddelegate.cpp \
    svlistmodel.cpp \
    sveditmodel.cpp \
    editdelegate.cpp \
    processreader.cpp


HEADERS  += frmwindow.h \
    cardlist.h \
    svdatabase.h \
    card.h \
    carddelegate.h \
    svlistmodel.h \
    menu.h \
    sveditmodel.h \
    editdelegate.h \
    processreader.h


FORMS    += frmwindow.ui

RC_ICONS += icon.ico
