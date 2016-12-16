#include <QDebug>

#include "kalman.h"
#include "struct.h"
#include "portfolio.h"
#include "oms.h"

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

    pair.yname = "au1612";
    pair.xname = "ag1612";
    pair.ymulti = 1000;
    pair.xmulti = 15;
}


Kalman::~Kalman()
{
}

void Kalman::setLogger()
{
    console = spdlog::stdout_color_mt("kalman ");
    console->set_pattern("[%H:%M:%S.%f] [%n] [%L] %v");
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

void Kalman::setPortfolio(Portfolio *pf)
{
    this->pf = pf;
}


void Kalman::onFeed(MyEvent *myev)
{
    string sym = myev->feed->InstrumentID;
    if (((sym == pair.yname) || (sym == pair.xname))
        && (pf->symList[pair.yname].mkt != nullptr) && (pf->symList[pair.xname].mkt != nullptr)) {
        // time freq filter:
        if ((string(pf->symList[pair.yname].mkt->UpdateTime).substr(3, 2) != lastTime.substr(3, 2)) &&
            string(pf->symList[pair.xname].mkt->UpdateTime).substr(3, 2) != lastTime.substr(3, 2)) {

            updateLastTime(pf->symList[sym].mkt->UpdateTime);
            updateXY(pf->symList[pair.yname].mkt->LastPrice, pf->symList[pair.xname].mkt->LastPrice);
            progress();

            //oms->setPosTarget(pair.yname.c_str(), pair.targetYpos, pair.targetYprice);
            //oms->setPosTarget(pair.xname.c_str(), pair.targetXpos, pair.targetXprice);

            oms->setPosTarget(pair.yname.c_str(), 0, y_t);
            oms->setPosTarget(pair.xname.c_str(), 0, x_t);
        }
    }
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

void Kalman::progress()
{
    F << x_t, 1.0;
    if (t > 0) R = C + Wt;
    yhat = F * theta;
    e = y_t - yhat;
    Q = F * R * F.transpose() + vt;
    A = R * F.transpose() / Q;
    C = R - A * F * R;
    theta += A * e;
    ez_thresh = (ez_thresh*std::min(t, 600) + abs(e)) / (std::min(t, 600) + 1);

    QString msg = QString("<%1> ").arg(lastTime.c_str());
    msg += QString("kalman progress t=%1, y=%2, yhat=%3, x=%4, beta=%5, thresh=%6, e=%7").
        arg(t).arg(y_t).arg(yhat).arg(x_t).arg(theta(0, 0)).arg(ez_thresh).arg(e);
    //qDebug() << msg.toStdString().c_str();
    logger(spdlog::level::info, msg.toStdString().c_str());

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
