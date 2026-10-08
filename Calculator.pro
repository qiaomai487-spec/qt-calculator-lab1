QT += core gui widgets

CONFIG += c++17

TEMPLATE = app
TARGET = Calculator

SOURCES += \
    main.cpp \
    calculator.cpp

HEADERS += \
    calculator.h

FORMS += \
    calculator.ui

RESOURCES += \
    resources.qrc
