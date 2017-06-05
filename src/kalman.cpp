#include <chrono>
#include <iostream>
#include <QDebug>

#include "fmt/format.h"

#include "include/kalman.h"
#include "include/struct.h"
#include "include/portfolio.h"
#include "include/oms.h"

using namespace Eigen;
using namespace std;

Kalman::Kalman()
{
    setLogger();

    delta = 1E-5;
    Wt = delta*(1 - delta) * Matrix2d::Identity();
    vt = 1E-3;
    theta = Vector2d::Zero();
    C = Matrix2d::Zero();
    R = Matrix2d::Zero();

    pair.yname = "au1712";
    pair.xname = "ag1712";
    pair.ymulti = 1000;
    pair.xmulti = 15;
}


Kalman::~Kalman()
{
}

void Kalman::setLogger()
{
    console = spdlog::stdout_color_mt("kalman");
    console->set_pattern("[%H:%M:%S.%f] [%L] [%n] %v");
    g_logger = spdlog::get("file_logger");
    kalman_logger = spdlog::rotating_logger_mt("kalman_logger", "logs/kalman_log", 1024 * 1024 * 5, 3);
    //kalman_logger = spdlog::daily_logger_mt("kalman_logger", "logs/kalman_log", 5, 0);
    kalman_logger->flush_on(spdlog::level::info);
}

void Kalman::setOMS(OMS *oms)
{
    this->oms = oms;

    oms->addPosTarget(pair.yname.c_str());
    oms->addPosTarget(pair.xname.c_str());
}

void Kalman::onCtpEvent(CtpEvent ev)
{

    if (ev.type == MarketEvent) {
        string sym = ev.mkt.InstrumentID;
        if (((sym == pair.yname) || (sym == pair.xname))
            && (dataHub->symMktTable.find(pair.yname) != dataHub->symMktTable.end())
            && (dataHub->symMktTable.find(pair.xname) != dataHub->symMktTable.end())) {
                // time freq filter:
                if (string(dataHub->symMktTable.at(sym).UpdateTime).substr(3, 2) != lastTime.substr(3, 2)) {
                    updateLastTime(ev.mkt.UpdateTime);
                    updateXY(dataHub->symMktTable.at(pair.yname).LastPrice, dataHub->symMktTable.at(pair.xname).LastPrice);
                    progressFilter();
                }

                updateXY(dataHub->symMktTable.at(pair.yname).LastPrice, dataHub->symMktTable.at(pair.xname).LastPrice);
                e = y_t - yhat;
                updatePos();
                string msg = fmt::format("y_t={}, y_hat={}, e={}, ez_thresh={}", y_t, yhat, e, ez_thresh);
                logger(spdlog::level::info, msg.c_str());
                oms->setPosTarget(pair.yname.c_str(), pair.targetYpos, pair.targetYprice);
                oms->setPosTarget(pair.xname.c_str(), pair.targetXpos, pair.targetXprice);
                oms->handleTargets();

//                oms->setPosTarget(pair.yname.c_str(), 0, y_t);
//                oms->setPosTarget(pair.xname.c_str(), 0, x_t);

        }
    }
//    auto end = std::chrono::steady_clock::now();
//    std::cout << "kal " << std::chrono::duration_cast<std::chrono::microseconds>(end - ev.ts).count() << std::endl;
}

void Kalman::updateXY(double y, double x)
{
    y_t = y;
    x_t = x;
}

void Kalman::updateLastTime(char *newTime)
{
    lastTime = std::string(newTime);
}

void Kalman::progressFilter()
{
    F << x_t, 1.0;
    if (t > 0) R = C + Wt;
    yhat = F * theta;
    e = y_t - yhat;
    Q = F * R * F.transpose() + vt;
    A = R * F.transpose() / Q;
    C = R - A * F * R;
    theta += A * e;
    if (t > 3)
        ez_thresh = (ez_thresh*std::min(t, 600) + abs(e)) / (std::min(t, 600) + 1);

    string msg = fmt::format("<{}> kalman progress t={}, y={}, yhat={}, x={}, beta={}, thresh={}, e={}",
                             lastTime, t, y_t, yhat, x_t, theta(0, 0), ez_thresh, e);
    logger(spdlog::level::info, msg.c_str());

    updatePos();
    ++t;
}


int sign(double x) {
    if (x > 0) return 1;
    else if (x < 0) return -1;
    else return 0;
}

void Kalman::updatePos()
{
    if ((t > 3) && (abs(e) > ez_thresh)) {
        pair.targetYpos = -sign(e) * std::round(2000 / (double)pair.ymulti);
        pair.targetXpos = sign(e) * std::round(2000 * theta(0, 0) / (double)pair.xmulti);
        pair.targetYprice = y_t;
        pair.targetXprice = x_t;
    }
}
