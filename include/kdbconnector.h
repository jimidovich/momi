#ifndef KDBCONNECTOR_H
#define KDBCONNECTOR_H

#include <string>

#include "spdlog/spdlog.h"
#include "ThostFtdcUserApiDataType.h"
#include "ThostFtdcUserApiStruct.h"

#include "myevent.h"

#define KXVER 3
#include "k.h"

class PortfolioValue;

class KdbConnector : public EventSubscriber
{
public:
    KdbConnector();
    KdbConnector(std::string consoleName);
    ~KdbConnector();
    
    void setTradingDay(const char *tday);
    void setLogger(std::string consoleName);

    void onCtpEvent(CtpEvent ev);

    public slots:
    //void onFeedEvent(CThostFtdcDepthMarketDataField * feed);
//    void onEvent(QEvent *ev);

protected:
    void checkTableExist();
    void createMktTable();
    void createInfoTable();
    void insertContractInfo(CThostFtdcInstrumentField& info);
    void insertFeed(CThostFtdcDepthMarketDataField& feed);
    void insertAccount(const PortfolioValue &acc);

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

    int port                = 5010;
    int handle              = 0;
    const char *hostname    = "localhost";
    const char *username    = "";
    const char *password    = "";
    const char *tableName   = "market";
    const char *cinfoName   = "info";
    bool tableExist         = false;
    const char *tradingDay  = nullptr;
    int countTick           = 0;

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
