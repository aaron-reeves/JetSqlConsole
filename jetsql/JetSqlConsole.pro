#-------------------------------------------------
#
# Project created by QtCreator 2012-02-28T11:41:27
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = jetsql
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += \
           ../cadosql/include \
           ../

LIBS += \
    ../cadosql/lib/cadosql.lib

SOURCES += main.cpp \
    ../ar_general_purpose/qcout.cpp \
    ../ar_general_purpose/csql.cpp \
    ../ar_general_purpose/cqstringlist.cpp \
    ../ar_general_purpose/ccmdline.cpp \
    ../ar_general_purpose/strutils.cpp

HEADERS += \
    ../ar_general_purpose/qcout.h \
    ../ar_general_purpose/csql.h \
    ../ar_general_purpose/cqstringlist.h \
    ../ar_general_purpose/ccmdLine.h \
    ../ar_general_purpose/strutils.h
