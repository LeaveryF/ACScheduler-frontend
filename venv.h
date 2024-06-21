#ifndef VENV_H
#define VENV_H

#include <QObject>
#include <QTimer>
#include "datadef.h"

class Venv : public QObject
{
    Q_OBJECT

private:
    void setup_connections();
    bool reach_target();

    qreal env_temp_;
    bool is_running_;
    qreal curr_temp_;
    qreal set_temp_;
    Speed curr_speed_;
    QTimer timer_; // 将定时器声明为成员变量
    qreal time_stamp_;
    bool need_resent_;
    bool power_off_;

private slots:
    void updateTemperature();

public:
    Venv(QObject *parent = nullptr);

    void setCurSpeed(Speed speed) { curr_speed_ = speed; }
    void setRunning(bool running) { is_running_ = running; }
    void setSetTemp(qreal set_temp) { set_temp_ = set_temp; }

    qreal getTargetTemp() { return set_temp_; }
    Speed getCurrSpeed() { return curr_speed_; }
    void setPowerOff() { power_off_ = true; }
    void setPowerOn() { power_off_ = false; }

    qreal getCurTemp() { return curr_temp_; }
    void sendPacket();
signals:
    void targetReached(qreal curr_temp);
    void requestRewind();
    void temperatureUpdated(qreal current_temp);
    void echoCurrentTemp(qreal current_temp);
};

#endif // VENV_H
