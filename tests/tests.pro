QT += core gui widgets

CONFIG += c++17 console
CONFIG -= app_bundle

TEMPLATE = app
TARGET = calculator_tests

INCLUDEPATH += ..

SOURCES += \
    test_calculator.cpp \
    ../calculator.cpp

HEADERS += \
    ../calculator.h

FORMS += \
    ../calculator.ui

RESOURCES += \
    ../resources.qrc
