#-------------------------------------------------
#
# Project created by QtCreator 2016-12-06T15:43:48
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += "./eigen3"

LIBS += $$PWD/c.o

TARGET = momi
TEMPLATE = app

SOURCES += main.cpp\
        ctpmonitor.cpp \
    dispatcher.cpp \
    kalman.cpp \
    kdbconnector.cpp \
    mdspi.cpp \
    myevent.cpp \
    oms.cpp \
    portfolio.cpp \
    position.cpp \
    rm.cpp \
    strategy.cpp \
    trader.cpp

HEADERS  += ctpmonitor.h \
    dispatcher.h \
    k.h \
    kalman.h \
    kdbconnector.h \
    mdspi.h \
    myevent.h \
    oms.h \
    portfolio.h \
    position.h \
    rm.h \
    strategy.h \
    struct.h \
    trader.h \
    ThostFtdcMdApi.h \
    ThostFtdcTraderApi.h \
    ThostFtdcUserApiDataType.h \
    ThostFtdcUserApiStruct.h

FORMS    += ctpmonitor.ui

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/v6.3.6_20160606_api_tradeapi_linux64/release/ -lthostmduserapi
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/v6.3.6_20160606_api_tradeapi_linux64/debug/ -lthostmduserapi
else:unix: LIBS += -L$$PWD/v6.3.6_20160606_api_tradeapi_linux64/ -lthostmduserapi

INCLUDEPATH += $$PWD/v6.3.6_20160606_api_tradeapi_linux64
DEPENDPATH += $$PWD/v6.3.6_20160606_api_tradeapi_linux64

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/v6.3.6_20160606_api_tradeapi_linux64/release/ -lthosttraderapi
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/v6.3.6_20160606_api_tradeapi_linux64/debug/ -lthosttraderapi
else:unix: LIBS += -L$$PWD/v6.3.6_20160606_api_tradeapi_linux64/ -lthosttraderapi

INCLUDEPATH += $$PWD/v6.3.6_20160606_api_tradeapi_linux64
DEPENDPATH += $$PWD/v6.3.6_20160606_api_tradeapi_linux64

QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter
QMAKE_CXXFLAGS += -Wno-deprecated
