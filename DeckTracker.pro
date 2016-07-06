#-------------------------------------------------
#
# Project created by QtCreator 2016-06-27T23:10:01
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DeckTracker
TEMPLATE = app


SOURCES += main.cpp\
    perceptualhash.cpp \
        frmwindow.cpp \
    cardlist.cpp \
    svdatabase.cpp \
    carddelegate.cpp \
    svlistmodel.cpp


HEADERS  += frmwindow.h \
    perceptualhash.h \
    asmopencv.h \
    cardlist.h \
    svdatabase.h \
    card.h \
    carddelegate.h \
    svlistmodel.h \
    menu.h


FORMS    += frmwindow.ui

INCLUDEPATH += C:\\opencv\\build\\include

win32: LIBS += -LC:\\opencv\\build\\x64\\vc12\\lib \
 -lopencv_calib3d2413 \
 -lopencv_contrib2413 \
 -lopencv_core2413 \
 -lopencv_features2d2413 \
 -lopencv_flann2413 \
 -lopencv_gpu2413 \
 -lopencv_highgui2413 \
 -lopencv_imgproc2413 \
 -lopencv_legacy2413 \
 -lopencv_ml2413 \
 -lopencv_nonfree2413 \
 -lopencv_objdetect2413 \
 -lopencv_ocl2413 \
 -lopencv_photo2413 \
 -lopencv_stitching2413 \
 -lopencv_superres2413 \
 -lopencv_ts2413 \
 -lopencv_video2413 \
 -lopencv_videostab2413
