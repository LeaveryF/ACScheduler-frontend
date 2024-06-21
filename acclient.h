#pragma once

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtCore/QObject>
#include <QtWebSockets/QWebSocket>

#include "controller.h"
#include "datadef.h"
#include "venv.h"

class ACClient : public QObject
{
    Q_OBJECT

public:
    explicit ACClient(const QUrl &url,
                      bool debug = false,
                      int temperature = 25,
                      QObject *parent = nullptr);

private Q_SLOTS:
    void powerOn();
    void onTextMessageReceived(QString message);
    void powerOff();
    void adjustSpeedTempChanged(QString target_speed, qint64 target_temp, QString target_mode);
    void venvReachTarget(qreal curr_temp);
    void handleRequestRewind();
    void updateTemperature(qreal current_temp);
    void setVenvPowerOff();
    void setVenvPowerOn();
    void deviation();
    void echoTemp(qreal current_temp);

private:
    QWebSocket m_webSocket;
    bool m_debug;
    controller _controller;
    Venv _venv;
    int _env_temp;

    Speed getSpeed(QString speed);
};
