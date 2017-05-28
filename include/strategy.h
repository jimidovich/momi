#ifndef STRATEGY_H
#define STRATEGY_H

class Portfolio;
class Trader;
//class Algo;
class OMS;
//class RM;

class Strategy {
public:
	Strategy();
	~Strategy();

//	virtual void onTick() = 0;

    Portfolio *pf;
    Trader *trader;
//    Algo *algo;
    OMS *oms;
//    RM *rm;

private:

};

#endif // !STRATEGY_H
