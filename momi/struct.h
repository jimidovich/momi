#ifndef STRUCT_H
#define STRUCT_H

#include <string>
#include <map>
#include "ThostFtdcUserApiStruct.h"
#include <qmap.h>

typedef QMap<std::string, CThostFtdcInstrumentField> SymInfoMap;
typedef QMap<std::string, CThostFtdcDepthMarketDataField*> SymTickMap;


struct Symbol {
    Symbol() {}
    Symbol(CThostFtdcDepthMarketDataField *mktf, CThostFtdcInstrumentField *info)
        :mkt(mktf), info(info) {}
    CThostFtdcDepthMarketDataField *mkt{ nullptr };
    CThostFtdcInstrumentField *info{ nullptr };
};
typedef QMap<std::string, Symbol> SymbolList;

struct AccountMtM {
    std::string accountID;
    double balance{ 0 };
    double pnl{ 0 };
    double margin{ 0 };
};

struct MyContractInfo {
    std::string Date;
    std::string Contract;
    std::string Exchange;
    std::string Name;
    std::string Product;
    std::string ProductClass;
    int DelivYear{ 0 };
    int DelivMonth{ 0 };
    int MaxMktOrderVolume{ 0 };
    int MinMktOrderVolume{ 0 };
    int MaxLmtOrderVolume{ 0 };
    int MinLmtOrderVolume{ 0 };
    int VolumeMultiples{ 0 };
    double PriceTick{ 0 };
    std::string CreateDate;
    std::string OpenDate;
    std::string ExpireDate;
    std::string StartDelivDate;
    std::string EndDelivDate;
    std::string LifePhase;
    int IsTrading;
    std::string PositionType;
    char PositionDateType{ 0 };
    double LongMarginRatio{ 0 };
    double ShortMarginRatio{ 0 };
    char MMSA{ 0 };
};

enum EnumOpenClose { OpenTrade, CloseTrade };

enum EnumOffsetFlagType
{
    Open = THOST_FTDC_OF_Open,
    Close = THOST_FTDC_OF_Close,
    ForceClose = THOST_FTDC_OF_ForceClose,
    CloseToday = THOST_FTDC_OF_CloseToday,
    CloseYesterday = THOST_FTDC_OF_CloseYesterday,
    ForceOff = THOST_FTDC_OF_ForceOff,
    LocalForceClose = THOST_FTDC_OF_LocalForceClose
};

enum EnumDirectionType
{
    Buy = THOST_FTDC_D_Buy,
    Sell = THOST_FTDC_D_Sell
};

enum EnumContingentConditionType
{
    ///立即
    Immediately = THOST_FTDC_CC_Immediately,
    ///止损
    Touch = THOST_FTDC_CC_Touch,
    ///止赢
    TouchProfit = THOST_FTDC_CC_TouchProfit,
    ///预埋单
    ParkedOrder = THOST_FTDC_CC_ParkedOrder,
    ///最新价大于条件价
    LastPriceGreaterThanStopPrice = THOST_FTDC_CC_LastPriceGreaterThanStopPrice,
    ///最新价大于等于条件价
    LastPriceGreaterEqualStopPrice = THOST_FTDC_CC_LastPriceGreaterEqualStopPrice,
    ///最新价小于条件价
    LastPriceLesserThanStopPrice = THOST_FTDC_CC_LastPriceLesserThanStopPrice,
    ///最新价小于等于条件价
    LastPriceLesserEqualStopPrice = THOST_FTDC_CC_LastPriceLesserEqualStopPrice,
    ///卖一价大于条件价
    AskPriceGreaterThanStopPrice = THOST_FTDC_CC_AskPriceGreaterThanStopPrice,
    ///卖一价大于等于条件价
    AskPriceGreaterEqualStopPrice = THOST_FTDC_CC_AskPriceGreaterEqualStopPrice,
    ///卖一价小于条件价
    AskPriceLesserThanStopPrice = THOST_FTDC_CC_AskPriceLesserThanStopPrice,
    ///卖一价小于等于条件价
    AskPriceLesserEqualStopPrice = THOST_FTDC_CC_AskPriceLesserEqualStopPrice,
    ///买一价大于条件价
    BidPriceGreaterThanStopPrice = THOST_FTDC_CC_BidPriceGreaterThanStopPrice,
    ///买一价大于等于条件价
    BidPriceGreaterEqualStopPrice = THOST_FTDC_CC_BidPriceGreaterEqualStopPrice,
    ///买一价小于条件价
    BidPriceLesserThanStopPrice = THOST_FTDC_CC_BidPriceLesserThanStopPrice,
    ///买一价小于等于条件价
    BidPriceLesserEqualStopPrice = THOST_FTDC_CC_BidPriceLesserEqualStopPrice
};

enum EnumForceCloseReasonType
{
    ///非强平
    NotForceClose = THOST_FTDC_FCC_NotForceClose,
    ///资金不足
    LackDeposit = THOST_FTDC_FCC_LackDeposit,
    ///客户超仓
    ClientOverPositionLimit = THOST_FTDC_FCC_ClientOverPositionLimit,
    ///会员超仓
    MemberOverPositionLimit = THOST_FTDC_FCC_MemberOverPositionLimit,
    ///持仓非整数倍
    NotMultiple = THOST_FTDC_FCC_NotMultiple,
    ///违规
    Violation = THOST_FTDC_FCC_Violation,
    ///其它
    Other = THOST_FTDC_FCC_Other,
    ///自然人临近交割
    PersonDeliv = THOST_FTDC_FCC_PersonDeliv
};

enum EnumOrderPriceTypeType
{
    ///任意价
    AnyPrice = THOST_FTDC_OPT_AnyPrice,
    ///限价
    LimitPrice = THOST_FTDC_OPT_LimitPrice,
    ///最优价
    BestPrice = THOST_FTDC_OPT_BestPrice,
    ///最新价
    LastPrice = THOST_FTDC_OPT_LastPrice,
    ///最新价浮动上浮1个ticks
    LastPricePlusOneTicks = THOST_FTDC_OPT_LastPricePlusOneTicks,
    ///最新价浮动上浮2个ticks
    LastPricePlusTwoTicks = THOST_FTDC_OPT_LastPricePlusTwoTicks,
    ///最新价浮动上浮3个ticks
    LastPricePlusThreeTicks = THOST_FTDC_OPT_LastPricePlusThreeTicks,
    ///卖一价
    AskPrice1 = THOST_FTDC_OPT_AskPrice1,
    ///卖一价浮动上浮1个ticks
    AskPrice1PlusOneTicks = THOST_FTDC_OPT_AskPrice1PlusOneTicks,
    ///卖一价浮动上浮2个ticks
    AskPrice1PlusTwoTicks = THOST_FTDC_OPT_AskPrice1PlusTwoTicks,
    ///卖一价浮动上浮3个ticks
    AskPrice1PlusThreeTicks = THOST_FTDC_OPT_AskPrice1PlusThreeTicks,
    ///买一价
    BidPrice1 = THOST_FTDC_OPT_BidPrice1,
    ///买一价浮动上浮1个ticks
    BidPrice1PlusOneTicks = THOST_FTDC_OPT_BidPrice1PlusOneTicks,
    ///买一价浮动上浮2个ticks
    BidPrice1PlusTwoTicks = THOST_FTDC_OPT_BidPrice1PlusTwoTicks,
    ///买一价浮动上浮3个ticks
    BidPrice1PlusThreeTicks = THOST_FTDC_OPT_BidPrice1PlusThreeTicks,
    ///五档价
    FiveLevelPrice = THOST_FTDC_OPT_FiveLevelPrice
};

enum EnumTimeConditionType
{
    ///立即完成，否则撤销
    IOC = THOST_FTDC_TC_IOC,
    ///本节有效
    GFS = THOST_FTDC_TC_GFS,
    ///当日有效
    GFD = THOST_FTDC_TC_GFD,
    ///指定日期前有效
    GTD = THOST_FTDC_TC_GTD,
    ///撤销前有效
    GTC = THOST_FTDC_TC_GTC,
    ///集合竞价有效
    GFA = THOST_FTDC_TC_GFA
};

enum EnumVolumeConditionType
{
    ///任何数量
    AV = THOST_FTDC_VC_AV,
    ///最小数量
    MV = THOST_FTDC_VC_MV,
    ///全部数量
    CV = THOST_FTDC_VC_CV
};

enum EnumHedgeFlagType
{
    ///投机
    Speculation = THOST_FTDC_HF_Speculation,
    ///套利
    Arbitrage = THOST_FTDC_HF_Arbitrage,
    ///套保
    Hedge = THOST_FTDC_HF_Hedge,
    ///做市商
    MarketMaker = THOST_FTDC_HF_MarketMaker
};

enum EnumOrderStatusType
{
    ///全部成交
    AllTraded = THOST_FTDC_OST_AllTraded,
    ///部分成交还在队列中
    PartTradedQueueing = THOST_FTDC_OST_PartTradedQueueing,
    ///部分成交不在队列中
    PartTradedNotQueueing = THOST_FTDC_OST_PartTradedNotQueueing,
    ///未成交还在队列中
    NoTradeQueueing = THOST_FTDC_OST_NoTradeQueueing,
    ///未成交不在队列中
    NoTradeNotQueueing = THOST_FTDC_OST_NoTradeNotQueueing,
    ///撤单
    Canceled = THOST_FTDC_OST_Canceled,
    ///未知
    Unknown = THOST_FTDC_OST_Unknown,
    ///尚未触发
    NotTouched = THOST_FTDC_OST_NotTouched,
    ///已触发
    Touched = THOST_FTDC_OST_Touched

};

namespace mymap
{
    const std::map<TThostFtdcInstLifePhaseType, std::string> instLiftPhase_string{
        {THOST_FTDC_IP_NotStart, "NotStart"},
        {THOST_FTDC_IP_Started, "Started"},
        {THOST_FTDC_IP_Pause, "Pause"},
        {THOST_FTDC_IP_Expired, "Expired"}
    };

    const std::map<TThostFtdcProductClassType, std::string> productClass_string{
        {THOST_FTDC_PC_Futures, "Futures"},
        {THOST_FTDC_PC_Options, "Options"},
        {THOST_FTDC_PC_Combination, "Combination"},
        {THOST_FTDC_PC_Spot, "Spot"},
        {THOST_FTDC_PC_EFP, "EFP"},
        {THOST_FTDC_PC_SpotOption, "SpotOption"}
    };

    const std::map<TThostFtdcPositionTypeType, std::string> positionType_string{
        {THOST_FTDC_PT_Net, "Net"},
        {THOST_FTDC_PT_Gross, "Gross"}
    };

    const std::map<TThostFtdcPositionDateType, char> positionDate_char{
        {THOST_FTDC_PSD_Today, 'T'},
        {THOST_FTDC_PSD_History, 'H'}
    };

    const std::map<TThostFtdcDirectionType, char> direction_char{
        {THOST_FTDC_D_Buy, 'L'},
        {THOST_FTDC_D_Sell, 'S'}
    };

    const std::map<TThostFtdcPosiDirectionType, char> posiDirection_char{
        {THOST_FTDC_PD_Net, 'N'},
        {THOST_FTDC_PD_Long, 'L'},
        {THOST_FTDC_PD_Short, 'S'}
    };

    const std::map<TThostFtdcHedgeFlagType, char> hedgeFlag_char{
        {THOST_FTDC_HF_Speculation, 'S'},
        {THOST_FTDC_HF_Arbitrage, 'A'},
        {THOST_FTDC_HF_Hedge, 'H'},
        {THOST_FTDC_HF_MarketMaker, 'M'}
    };

    const std::map<TThostFtdcOffsetFlagType, std::string> offsetFlag_string{
        {THOST_FTDC_OF_Open, "Open"},                       // Speculation
        {THOST_FTDC_OF_Close, "Close"},                     // Speculation
        {THOST_FTDC_OF_ForceClose, "ForceClose"},           // Arbitrage
        {THOST_FTDC_OF_CloseToday, "CloseToday"},           // Hedge
        {THOST_FTDC_OF_CloseYesterday, "CloseYesterday"},   // MarketMaker
        {THOST_FTDC_OF_ForceOff, "ForceOff"},               // MarketMaker
        {THOST_FTDC_OF_LocalForceClose, "LocalForceClose"}  // MarketMaker
    };

    const std::map<TThostFtdcOffsetFlagType, EnumOffsetFlagType> offsetFlag_enum{
        {THOST_FTDC_OF_Open, EnumOffsetFlagType::Open},                       // Speculation
        {THOST_FTDC_OF_Close, EnumOffsetFlagType::Close},                     // Speculation
        {THOST_FTDC_OF_ForceClose, EnumOffsetFlagType::ForceClose},           // Arbitrage
        {THOST_FTDC_OF_CloseToday, EnumOffsetFlagType::CloseToday},           // Hedge
        {THOST_FTDC_OF_CloseYesterday, EnumOffsetFlagType::CloseYesterday},   // MarketMaker
        {THOST_FTDC_OF_ForceOff, EnumOffsetFlagType::ForceOff},               // MarketMaker
        {THOST_FTDC_OF_LocalForceClose, EnumOffsetFlagType::LocalForceClose}  // MarketMaker
    };

    const std::map<TThostFtdcMaxMarginSideAlgorithmType, char> maxMarginSideAlgo_char{
        {THOST_FTDC_MMSA_NO, 'N'},
        {THOST_FTDC_MMSA_YES, 'Y'}
    };

    const std::map<TThostFtdcOrderStatusType, EnumOrderStatusType> orderStatus_enum{
        {THOST_FTDC_OST_AllTraded, AllTraded},
        {THOST_FTDC_OST_PartTradedQueueing, PartTradedQueueing},
        {THOST_FTDC_OST_PartTradedNotQueueing, PartTradedNotQueueing},
        {THOST_FTDC_OST_NoTradeQueueing, NoTradeQueueing},
        {THOST_FTDC_OST_NoTradeNotQueueing, NoTradeNotQueueing},
        {THOST_FTDC_OST_Canceled, Canceled},
        {THOST_FTDC_OST_Unknown, Unknown},
        {THOST_FTDC_OST_NotTouched, NotTouched},
        {THOST_FTDC_OST_Touched, Touched}
    };

    const std::map<std::string, EnumOffsetFlagType> string_offsetFlag{
        {"open", EnumOffsetFlagType::Open},                       // Speculation
        {"o", EnumOffsetFlagType::Open},                          // Speculation
        {"close", EnumOffsetFlagType::Close},                     // Speculation
        {"c", EnumOffsetFlagType::Close},                         // Speculation
        {"forceclose", EnumOffsetFlagType::ForceClose},           // Arbitrage
        {"fclose", EnumOffsetFlagType::ForceClose},               // Arbitrage
        {"closetoday", EnumOffsetFlagType::CloseToday},           // Hedge
        {"closeyesterday", EnumOffsetFlagType::CloseYesterday},   // MarketMaker
        {"forceoff", EnumOffsetFlagType::ForceOff},               // MarketMaker
        {"localforceclose", EnumOffsetFlagType::LocalForceClose}  // MarketMaker
    };

    const std::map<std::string, EnumDirectionType> string_directionFlag{
        {"buy", EnumDirectionType::Buy},
        {"b", EnumDirectionType::Buy},
        {"sell", EnumDirectionType::Sell},
        {"s", EnumDirectionType::Sell}
    };
}
#endif // !STRUCT_H
