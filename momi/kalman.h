#ifndef KALMAN_H
#define KALMAN_H

#include "spdlog/spdlog.h"
#include <Eigen/Dense>

#include "myevent.h"
#include "struct.h"

class OMS;
class Portfolio;

struct Pair {
    std::string yname, xname;
    int ymulti = 1, xmulti = 1;
    int currYpos = 0, currXpos = 0;
    int targetYpos = 0, targetXpos = 0;
    double targetYprice = 0, targetXprice = 0;
};

class Kalman {
public:
    Kalman();
    ~Kalman();
    void setLogger();
    void onFeed(MyEvent *myev);
    void updateXY(double y, double x);
    void updateLastTime(char *newTime);
    void progress();
    void setOMS(OMS *oms);
    void setPortfolio(Portfolio *pf);

    std::string lastTime{ "nn:nn:nn" };
    Pair pair;

private:
    void updatePos();

    template<typename ...Args>
    inline void logger(spdlog::level::level_enum lvl, const char * fmt, const Args & ...args)
    {
        console->log(lvl, fmt, args...);
        g_logger->log(lvl, fmt, args...);
    }

    int t{ 0 };
    double x_t, y_t, delta, vt, yhat, e, Q;
    double ez_thresh;
    Eigen::Matrix2d Wt, C, R;
    Eigen::RowVector2d F;
    Eigen::Vector2d theta, A;
    Eigen::VectorXd ezArray;

    OMS *oms;
    Portfolio *pf;

    std::shared_ptr<spdlog::logger> console;
    std::shared_ptr<spdlog::logger> g_logger;
    std::shared_ptr<spdlog::logger> kalman_logger;
};

#endif // !KALMAN_H
