#ifndef STRUCT_H
#define STRUCT_H

#include <string>
#include <map>

#include <QMap>
#include "ThostFtdcUserApiStruct.h"

typedef std::map<std::string, CThostFtdcInstrumentField> SymInfoTable;
typedef std::map<std::string, CThostFtdcDepthMarketDataField> SymMktTable;

struct Symbol {
    Symbol() {}
    Symbol(CThostFtdcDepthMarketDataField *mktf, CThostFtdcInstrumentField *info)
        :mkt(mktf), info(info) {}
    CThostFtdcDepthMarketDataField *mkt{ nullptr };
    CThostFtdcInstrumentField *info{ nullptr };
};
typedef QMap<std::string, Symbol> SymbolList;


struct Symbol1 {
    Symbol1() {}
    Symbol1(CThostFtdcDepthMarketDataField mkt, CThostFtdcInstrumentField info)
        :mkt(mkt), info(info) {}
    CThostFtdcDepthMarketDataField mkt;
    CThostFtdcInstrumentField info;
};
typedef QMap<std::string, Symbol1> SymbolList1;



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
    ///����
    Immediately = THOST_FTDC_CC_Immediately,
    ///ֹ��
    Touch = THOST_FTDC_CC_Touch,
    ///ֹӮ
    TouchProfit = THOST_FTDC_CC_TouchProfit,
    ///Ԥ����
    ParkedOrder = THOST_FTDC_CC_ParkedOrder,
    ///���¼۴���������
    LastPriceGreaterThanStopPrice = THOST_FTDC_CC_LastPriceGreaterThanStopPrice,
    ///���¼۴��ڵ���������
    LastPriceGreaterEqualStopPrice = THOST_FTDC_CC_LastPriceGreaterEqualStopPrice,
    ///���¼�С��������
    LastPriceLesserThanStopPrice = THOST_FTDC_CC_LastPriceLesserThanStopPrice,
    ///���¼�С�ڵ���������
    LastPriceLesserEqualStopPrice = THOST_FTDC_CC_LastPriceLesserEqualStopPrice,
    ///��һ�۴���������
    AskPriceGreaterThanStopPrice = THOST_FTDC_CC_AskPriceGreaterThanStopPrice,
    ///��һ�۴��ڵ���������
    AskPriceGreaterEqualStopPrice = THOST_FTDC_CC_AskPriceGreaterEqualStopPrice,
    ///��һ��С��������
    AskPriceLesserThanStopPrice = THOST_FTDC_CC_AskPriceLesserThanStopPrice,
    ///��һ��С�ڵ���������
    AskPriceLesserEqualStopPrice = THOST_FTDC_CC_AskPriceLesserEqualStopPrice,
    ///��һ�۴���������
    BidPriceGreaterThanStopPrice = THOST_FTDC_CC_BidPriceGreaterThanStopPrice,
    ///��һ�۴��ڵ���������
    BidPriceGreaterEqualStopPrice = THOST_FTDC_CC_BidPriceGreaterEqualStopPrice,
    ///��һ��С��������
    BidPriceLesserThanStopPrice = THOST_FTDC_CC_BidPriceLesserThanStopPrice,
    ///��һ��С�ڵ���������
    BidPriceLesserEqualStopPrice = THOST_FTDC_CC_BidPriceLesserEqualStopPrice
};

enum EnumForceCloseReasonType
{
    ///��ǿƽ
    NotForceClose = THOST_FTDC_FCC_NotForceClose,
    ///�ʽ�����
    LackDeposit = THOST_FTDC_FCC_LackDeposit,
    ///�ͻ�����
    ClientOverPositionLimit = THOST_FTDC_FCC_ClientOverPositionLimit,
    ///��Ա����
    MemberOverPositionLimit = THOST_FTDC_FCC_MemberOverPositionLimit,
    ///�ֲַ�������
    NotMultiple = THOST_FTDC_FCC_NotMultiple,
    ///Υ��
    Violation = THOST_FTDC_FCC_Violation,
    ///����
    Other = THOST_FTDC_FCC_Other,
    ///��Ȼ���ٽ�����
    PersonDeliv = THOST_FTDC_FCC_PersonDeliv
};

enum EnumOrderPriceTypeType
{
    ///������
    AnyPrice = THOST_FTDC_OPT_AnyPrice,
    ///�޼�
    LimitPrice = THOST_FTDC_OPT_LimitPrice,
    ///���ż�
    BestPrice = THOST_FTDC_OPT_BestPrice,
    ///���¼�
    LastPrice = THOST_FTDC_OPT_LastPrice,
    ///���¼۸����ϸ�1��ticks
    LastPricePlusOneTicks = THOST_FTDC_OPT_LastPricePlusOneTicks,
    ///���¼۸����ϸ�2��ticks
    LastPricePlusTwoTicks = THOST_FTDC_OPT_LastPricePlusTwoTicks,
    ///���¼۸����ϸ�3��ticks
    LastPricePlusThreeTicks = THOST_FTDC_OPT_LastPricePlusThreeTicks,
    ///��һ��
    AskPrice1 = THOST_FTDC_OPT_AskPrice1,
    ///��һ�۸����ϸ�1��ticks
    AskPrice1PlusOneTicks = THOST_FTDC_OPT_AskPrice1PlusOneTicks,
    ///��һ�۸����ϸ�2��ticks
    AskPrice1PlusTwoTicks = THOST_FTDC_OPT_AskPrice1PlusTwoTicks,
    ///��һ�۸����ϸ�3��ticks
    AskPrice1PlusThreeTicks = THOST_FTDC_OPT_AskPrice1PlusThreeTicks,
    ///��һ��
    BidPrice1 = THOST_FTDC_OPT_BidPrice1,
    ///��һ�۸����ϸ�1��ticks
    BidPrice1PlusOneTicks = THOST_FTDC_OPT_BidPrice1PlusOneTicks,
    ///��һ�۸����ϸ�2��ticks
    BidPrice1PlusTwoTicks = THOST_FTDC_OPT_BidPrice1PlusTwoTicks,
    ///��һ�۸����ϸ�3��ticks
    BidPrice1PlusThreeTicks = THOST_FTDC_OPT_BidPrice1PlusThreeTicks,
    ///�嵵��
    FiveLevelPrice = THOST_FTDC_OPT_FiveLevelPrice
};

enum EnumTimeConditionType
{
    ///�������ɣ���������
    IOC = THOST_FTDC_TC_IOC,
    ///������Ч
    GFS = THOST_FTDC_TC_GFS,
    ///������Ч
    GFD = THOST_FTDC_TC_GFD,
    ///ָ������ǰ��Ч
    GTD = THOST_FTDC_TC_GTD,
    ///����ǰ��Ч
    GTC = THOST_FTDC_TC_GTC,
    ///���Ͼ�����Ч
    GFA = THOST_FTDC_TC_GFA
};

enum EnumVolumeConditionType
{
    ///�κ�����
    AV = THOST_FTDC_VC_AV,
    ///��С����
    MV = THOST_FTDC_VC_MV,
    ///ȫ������
    CV = THOST_FTDC_VC_CV
};

enum EnumHedgeFlagType
{
    ///Ͷ��
    Speculation = THOST_FTDC_HF_Speculation,
    ///����
    Arbitrage = THOST_FTDC_HF_Arbitrage,
    ///�ױ�
    Hedge = THOST_FTDC_HF_Hedge,
    ///������
    MarketMaker = THOST_FTDC_HF_MarketMaker
};

enum EnumOrderStatusType
{
    ///ȫ���ɽ�
    AllTraded = THOST_FTDC_OST_AllTraded,
    ///���ֳɽ����ڶ�����
    PartTradedQueueing = THOST_FTDC_OST_PartTradedQueueing,
    ///���ֳɽ����ڶ�����
    PartTradedNotQueueing = THOST_FTDC_OST_PartTradedNotQueueing,
    ///δ�ɽ����ڶ�����
    NoTradeQueueing = THOST_FTDC_OST_NoTradeQueueing,
    ///δ�ɽ����ڶ�����
    NoTradeNotQueueing = THOST_FTDC_OST_NoTradeNotQueueing,
    ///����
    Canceled = THOST_FTDC_OST_Canceled,
    ///δ֪
    Unknown = THOST_FTDC_OST_Unknown,
    ///��δ����
    NotTouched = THOST_FTDC_OST_NotTouched,
    ///�Ѵ���
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
