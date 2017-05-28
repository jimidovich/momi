#include "include/datahub.h"

void DataHub::onCtpEvent(CtpEvent ev)
{
    switch (ev.type)
    {
    case ContractInfoEvent:
    {
        symInfoTable[ev.contractInfo.InstrumentID] = ev.contractInfo;
        break;
    }
    case MarketEvent:
    {
        symMktTable[ev.mkt.InstrumentID] = ev.mkt;
        break;
    }
    case PositionEvent:
    {
        break;
    }
    case PositionDetailEvent:
    {
        break;
    }
    case AccountInfoEvent:
    {
        break;
    }
    case TradeEvent:
    {
        break;
    }
    case OrderEvent:
    {
        break;
    }
    default:
        break;
    }
}
