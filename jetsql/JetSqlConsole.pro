QT       += core

QT       -= gui

TARGET = jetsql
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

# Definitions for the application name and version
VERSION = 1.4.0.20180913
DATE = "13-Sep-2018"
DEFINES += \
  APP_VERSION=\\\"$$VERSION\\\" \
  APP_DATE=\\\"$$DATE\\\" \
  APP_NAME=\\\"jetsql\\\"

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
    ../ar_general_purpose/strutils.cpp \
    ../ar_general_purpose/help.cpp

HEADERS += \
    ../ar_general_purpose/qcout.h \
    ../ar_general_purpose/csql.h \
    ../ar_general_purpose/cqstringlist.h \
    ../ar_general_purpose/ccmdLine.h \
    ../ar_general_purpose/strutils.h \
    ../ar_general_purpose/help.h
    
win32:RC_FILE = jetsqlconsole.rc    
