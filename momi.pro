#-------------------------------------------------
#
# Project created by QtCreator 2016-12-06T15:43:48
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += "./eigen3"

LIBS += $$PWD/kdb/c.o

TARGET = momi
TEMPLATE = app

SOURCES += src/main.cpp\
    src/ctpmonitor.cpp \
    src/dispatcher.cpp \
    src/kalman.cpp \
    src/kdbconnector.cpp \
    src/mdspi.cpp \
    src/myevent.cpp \
    src/oms.cpp \
    src/portfolio.cpp \
    src/position.cpp \
    src/rm.cpp \
    src/strategy.cpp \
    src/trader.cpp

HEADERS += include/ctpmonitor.h \
    include/dispatcher.h \
    include/k.h \
    include/kalman.h \
    include/kdbconnector.h \
    include/mdspi.h \
    include/myevent.h \
    include/oms.h \
    include/portfolio.h \
    include/position.h \
    include/rm.h \
    include/strategy.h \
    include/struct.h \
    include/trader.h \
    include/ThostFtdcMdApi.h \
    include/ThostFtdcTraderApi.h \
    include/ThostFtdcUserApiDataType.h \
    include/ThostFtdcUserApiStruct.h

FORMS += ctpmonitor.ui

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
