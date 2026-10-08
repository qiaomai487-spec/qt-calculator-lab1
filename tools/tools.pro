QT += core gui widgets

CONFIG += c++17 console
CONFIG -= app_bundle

TEMPLATE = app
TARGET = make_screenshots

INCLUDEPATH += ..

SOURCES += \
    make_screenshots.cpp \
    ../calculator.cpp

HEADERS += \
    ../calculator.h

FORMS += \
    ../calculator.ui

RESOURCES += \
    ../resources.qrc
