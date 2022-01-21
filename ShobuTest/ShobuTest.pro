QT += testlib
QT -= gui
QT += network

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle
CONFIG += c++17

TEMPLATE = app

SOURCES +=  \
    gamestate.cpp \
    greedylogic.cpp \
    hardlogic.cpp \
    machineplayer.cpp \
    organicplayer.cpp \
    randomlogic.cpp \
    shobuclient.cpp \
    shobumodel.cpp \
    shobupersistence.cpp \
    tst_main.cpp

HEADERS += \
    gamestate.h \
    gameutils.h \
    greedylogic.h \
    hardlogic.h \
    machinelogic.h \
    machineplayer.h \
    organicplayer.h \
    randomlogic.h \
    shobuclient.h \
    shobuexception.h \
    shobumodel.h \
    shobupersistence.h \
    shobuplayer.h
