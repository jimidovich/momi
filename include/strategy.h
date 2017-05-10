#ifndef STRATEGY_H
#define STRATEGY_H

class Strategy {
public:
	Strategy();
	~Strategy();

	virtual void onTick() = 0;

private:

};

#endif // !STRATEGY_H
