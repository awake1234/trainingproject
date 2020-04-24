#-------------------------------------------------
#
# Project created by QtCreator 2020-01-17T21:50:05
#
#-------------------------------------------------

QT       += core gui network multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = clouddisk
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    login.cpp \
    mytitlewidget.cpp \
    common.cpp \
    des.c \
    logininfoinstance.cpp \
    base64.c \
    buttongroup.cpp \
    mydiskwg.cpp \
    mymenu.cpp \
    uploadtask.cpp \
    dataprocess.cpp \
    uploadlayout.cpp \
    transferwg.cpp \
    consumerthread_file.cpp \
    fileproperty.cpp \
    mypicwg.cpp \
    sharefilelist.cpp \
    ranklist.cpp \
    downloadtask.cpp \
    downloadlayout.cpp \
    createlink.cpp \
    linkdownload.cpp \
    musicplayerwidget.cpp

HEADERS += \
        mainwindow.h \
    login.h \
    mytitlewidget.h \
    common.h \
    des.h \
    logininfoinstance.h \
    base64.h \
    buttongroup.h \
    mydiskwg.h \
    mymenu.h \
    uploadtask.h \
    dataprocess.h \
    uploadlayout.h \
    transferwg.h \
    consumerthread_file.h \
    fileproperty.h \
    mypicwg.h \
    sharefilelist.h \
    ranklist.h \
    downloadtask.h \
    downloadlayout.h \
    createlink.h \
    linkdownload.h \
    musicplayerwidget.h

FORMS += \
        mainwindow.ui \
    login.ui \
    mytitlewidget.ui \
    buttongroup.ui \
    mydiskwg.ui \
    dataprocess.ui \
    transferwg.ui \
    fileproperty.ui \
    mypicwg.ui \
    sharefilelist.ui \
    ranklist.ui \
    createlink.ui \
    linkdownload.ui \
    musicplayerwidget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    picture.qrc

DISTFILES +=
