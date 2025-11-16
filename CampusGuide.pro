QT += core gui widgets

CONFIG += c++17

TARGET = CampusGuide
TEMPLATE = app

SOURCES += \
    src/main_gui.cpp \
    src/graph.cpp \
    src/mainwindow.cpp \
    src/graphwidget.cpp

HEADERS += \
    src/graph.h \
    src/mainwindow.h \
    src/graphwidget.h

