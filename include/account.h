#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <string>
#include "include/ThostFtdcUserApiStruct.h"

class Account
{
public:
    Account();
    Account(CThostFtdcTradingAccountField& accInfo);

    CThostFtdcTradingAccountField ctpAccInfo;
    std::string brokerID;
    std::string accountID;
    std::string tradingDay;
    double preDeposit     = 0;
    double preBalance     = 0;
    double preMargin      = 0;
    double balance        = 0;
    double available      = 0;
    double deposit        = 0;
    double withdraw       = 0;
    double margin         = 0;
    double commission     = 0;
    double cashBalance    = 0;
    double closeProfit    = 0;
    double positionProfit = 0;
    double grossPnl       = 0;
    double netPnl         = 0;
};

#endif // ACCOUNT_H
