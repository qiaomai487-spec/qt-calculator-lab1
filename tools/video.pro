QT += core gui widgets

CONFIG += c++17 console
CONFIG -= app_bundle

TEMPLATE = app
TARGET = make_video_frames

INCLUDEPATH += ..

SOURCES += \
    make_video_frames.cpp \
    ../calculator.cpp

HEADERS += \
    ../calculator.h

FORMS += \
    ../calculator.ui

RESOURCES += \
    ../resources.qrc
