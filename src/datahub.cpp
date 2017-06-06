#include <chrono>
#include <iostream>

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
        if (symMktTable.find(ev.mkt.InstrumentID) != symMktTable.end()) {
            symPrevMktTable[ev.mkt.InstrumentID] = symMktTable.at(ev.mkt.InstrumentID);
        };
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
//    auto end = std::chrono::steady_clock::now();
//    std::cout << "dat " << std::chrono::duration_cast<std::chrono::microseconds>(end - ev.ts).count() << std::endl;

}
