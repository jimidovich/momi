#ifndef KALMAN_H
#define KALMAN_H

#include "spdlog/spdlog.h"
#include <Eigen/Dense>

#include "myevent.h"
#include "struct.h"
#include "datahub.h"

class OMS;
class Portfolio;

struct Pair {
    std::string yname, xname;
    int ymulti          = 1;
    int xmulti          = 1;
    int currYpos        = 0;
    int currXpos        = 0;
    int targetYpos      = 0;
    int targetXpos      = 0;
    double targetYprice = 0;
    double targetXprice = 0;
};

class Kalman : public EventSubscriber{
public:
    Kalman();
    ~Kalman();
    void setLogger();
    void onCtpEvent(CtpEvent ev);

    void updateXY(double y, double x);
    void updateLastTime(char *newTime);
    void progressFilter();
    void setOMS(OMS *oms);

    std::string lastTime = "nn:nn:nn";
    Pair pair;
    DataHub *dataHub;

private:
    void updatePos();

    template<typename ...Args>
    inline void logger(spdlog::level::level_enum lvl, const char * fmt, const Args & ...args)
    {
        console->log(lvl, fmt, args...);
        g_logger->log(lvl, fmt, args...);
    }

    int t            = 0;
    double x_t       = 0;
    double y_t       = 0;
    double delta     = 0;
    double vt        = 0;
    double yhat      = 0;
    double e         = 0;
    double Q         = 0;
    double ez_thresh = 0;
    Eigen::Matrix2d Wt, C, R;
    Eigen::RowVector2d F;
    Eigen::Vector2d theta, A;
    Eigen::VectorXd ezArray;

    OMS *oms;

    std::shared_ptr<spdlog::logger> console;
    std::shared_ptr<spdlog::logger> g_logger;
    std::shared_ptr<spdlog::logger> kalman_logger;
};

#endif // !KALMAN_H
