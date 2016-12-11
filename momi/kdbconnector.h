#ifndef KDBCONNECTOR_H
#define KDBCONNECTOR_H

#include <string>
#include <QObject>
#include <qmutex.h>

#include "ThostFtdcUserApiDataType.h"
#include "ThostFtdcUserApiStruct.h"
#include "myevent.h"
#include "spdlog/spdlog.h"

#define KXVER 3
#include "k.h"

class Account;

class KdbConnector : public QObject {
    Q_OBJECT

public:
    KdbConnector(QObject *parent = nullptr);
    KdbConnector(std::string consoleName, QObject *parent = nullptr);
    ~KdbConnector();
    
    void setTradingDay(const char *tday);

    void setLogger(std::string consoleName);

    public slots:
    //void onFeedEvent(CThostFtdcDepthMarketDataField * feed);
    void onEvent(QEvent *ev);

protected:
    void checkTableExist();
    void createMktTable();
    void createInfoTable();
    void insertContractInfo(CThostFtdcInstrumentField *info);
    void insertFeed(CThostFtdcDepthMarketDataField *feed);
    void insertAccount(const Account &acc);

    template<typename ...Args>
    inline void logger(spdlog::level::level_enum lvl, const char * fmt, const Args & ...args)
    {
        console->log(lvl, fmt, args...);
        g_logger->log(lvl, fmt, args...);
    }

    std::string qStatementToCreateTable(const char *tbName);
    K qMakeTime(char *time, int millisec);
    K qDataList(K tspan, K time);
    K date2qDate(char *date);

    const char *hostname{ "localhost" };
    int port{ 5010 };
    const char *username{ "" };
    const char *password{ "" };
    int handle{ 0 };
    const char *tableName{ "market" };
    const char *cinfoName{ "info" };
    bool tableExist{ false };
    const char *tradingDay{ nullptr };
    QMutex mutex;
    int countTick{ 0 };

    std::shared_ptr<spdlog::logger> console;
    std::shared_ptr<spdlog::logger> g_logger;
    std::shared_ptr<spdlog::logger> kdb_logger;
};

class TickSubscriber : public KdbConnector {
public:
    TickSubscriber();
    TickSubscriber(std::string consoleName);
    void subscribe();
};

#endif // !KDBCONNECTOR_H