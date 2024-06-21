#include "venv.h"
#include "qdebug.h"

Venv::Venv(QObject *parent)
    : QObject(parent)
    , env_temp_(25.0)
    , is_running_(false)
    , curr_temp_(25.0)
    , set_temp_(25.0)
    , curr_speed_(Speed::Medium)
    , timer_(this)
    , time_stamp_(0.0)
    , need_resent_(false)
    , power_off_(false)
{
    setup_connections();

    timer_.setInterval(10000);
    timer_.start();

    // timer2_.setInterval(10000);
    // timer2_.start();
}

void Venv::setup_connections()
{
    connect(&timer_, &QTimer::timeout, this, &Venv::updateTemperature);
    connect(&timer_, &QTimer::timeout, this, &Venv::sendPacket);
    // connect(&timer2_, &QTimer::timeout, this, &Venv::sendPacket);
}

// 每隔一定时间间隔，发送packet报文。
void Venv::sendPacket()
{
    qreal curr_temp = curr_temp_;

    // 如果没有关机，需要告诉服务器当前的温度。
    if (!power_off_) {
        emit echoCurrentTemp(curr_temp_);
    }
}

void Venv::updateTemperature()
{
    time_stamp_ += 0.1;
    qreal adjustment = 0.0;
    int direct;
    if (is_running_ && !power_off_) {
        direct = set_temp_ > curr_temp_ ? 1 : -1;
        switch (curr_speed_) {
        case Speed::Low:
            adjustment = 0.3;
            break;
        case Speed::Medium:
            adjustment = 0.5;
            break;
        case Speed::High:
            adjustment = 0.6;
            break;
        }
    } else {
        if (env_temp_ < curr_temp_ - 0.1 || env_temp_ > curr_temp_ + 0.1) {
            direct = env_temp_ > curr_temp_ ? 1 : -1;
            adjustment = 0.5; // 低速调整，向环境温度靠拢
        } else {
            adjustment = 0;
            curr_temp_ = env_temp_;
            direct = 0;
        }
    }
    curr_temp_ += direct * adjustment;

    emit temperatureUpdated(curr_temp_); // 发出环境温度变化槽。

    // qDebug() << "Current Temp:" << curr_temp_ << " Set Temp:" << set_temp_
    //          << " Env Temp:" << env_temp_ << " Adjustment:" << adjustment
    //          << " Direction:" << direct;

    if (is_running_ && reach_target() && !power_off_) {
        is_running_ = false;
        need_resent_ = true;

        curr_temp_ = set_temp_;
        emit targetReached(curr_temp_);
    }

    if (!is_running_ && !reach_target() && !power_off_) {
        if (set_temp_ >= curr_temp_ + 1 || set_temp_ + 1 <= curr_temp_) {
            // 重新发出送风请求。
            if (need_resent_) {
                need_resent_ = false;
                emit requestRewind();
            }
        }
    }
}

bool Venv::reach_target()
{
    qreal loss = 0.1;
    // if (curr_temp_ <= set_temp_ + loss && curr_temp_ >= set_temp_ - loss) {
    //     curr_temp_ = set_temp_;
    //     return true;
    // }
    // return false;

    return curr_temp_ <= set_temp_ + loss && curr_temp_ >= set_temp_ - loss;
}
