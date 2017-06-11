#include "include/account.h"

Account::Account()
{
}

Account::Account(CThostFtdcTradingAccountField &af)
{
    ctpAccInfo = af;

    brokerID = af.BrokerID;
    accountID = af.AccountID;
    preDeposit = af.PreDeposit;
    preBalance = af.PreBalance;
    preMargin = af.PreMargin;
    balance = af.Balance;
    available = af.Available;
    deposit = af.Deposit;
    withdraw = af.Withdraw;
    margin = af.CurrMargin;
    commission = af.Commission;
    cashBalance = preBalance - withdraw + deposit;
    closeProfit = af.CloseProfit;
    positionProfit = af.PositionProfit;
    grossPnl = closeProfit + positionProfit;
    netPnl = grossPnl - commission;

}
