#-------------------------------------------------
#
# Project created by QtCreator 2015-03-24T14:38:19
#
#-------------------------------------------------

QT       += core network xml gui

TARGET = httpTesting
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    parser.cpp \
    UPnPHandler.cpp

HEADERS += \
    parser.h \
    UPnPHandler.h

OTHER_FILES += \
    soapData.xml

RESOURCES += \
    soap.qrc
