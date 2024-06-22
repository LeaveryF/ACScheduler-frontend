// Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#include "acclient.h"
#include <QtCore/QDebug>
#include "datadef.h"
#include "message_builder.h"

QT_USE_NAMESPACE

//! [constructor]
ACClient::ACClient(const QUrl &url, bool debug, int temperature, QObject *parent)
    : QObject(parent)
    , m_debug(debug)
    , _venv(this)
    , _env_temp(temperature)
{
    if (m_debug)
        qDebug() << "WebSocket server:" << url;

    // 绑定power on，power off，调节speed和调节temp。
    //connect(&m_webSocket, &QWebSocket::connected, this, &ACClient::powerOn);
    connect(&_controller, &controller::powerOn, this, &ACClient::powerOn);
    connect(&_controller, &controller::powerOff, this, &ACClient::powerOff);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &ACClient::powerOff);
    // 绑定风速变化signal和槽。
    connect(&_controller, &controller::windSpeedChanged, this, &ACClient::adjustSpeedTempChanged);
    // 绑定虚拟环境的温度到达。
    connect(&_venv, &Venv::targetReached, this, &ACClient::venvReachTarget);
    // 绑定虚拟环境重新要求送温。
    connect(&_venv, &Venv::requestRewind, this, &ACClient::deviation);
    connect(&_venv, &Venv::temperatureUpdated, this, &ACClient::updateTemperature);
    connect(&_venv, &Venv::echoCurrentTemp, this, &ACClient::echoTemp);

    _controller.setEnvTemp(temperature);
    _controller.setCurrentTemp(temperature);
    _controller.show();
    m_webSocket.open(url);
    _venv.setEnvTemp(temperature);
    _venv.setCurTemp(temperature);
}
//! [constructor]

//! [onConnected]
// 开机
void ACClient::powerOn()
{
    if (m_debug)
        qDebug() << "WebSocket connected";
    connect(&m_webSocket, &QWebSocket::textMessageReceived, this, &ACClient::onTextMessageReceived);
    QJsonObject obj;
    MessageBuilder::bind(obj);
    MessageBuilder::addItem("type", "SwitchPower");
    MessageBuilder::addItem("is_ac_power_on", qint64(1));
    setVenvPowerOn();

    m_webSocket.sendTextMessage(MessageBuilder::toString());
}
//! [onConnected]

//! [onTextMessageReceived]
void ACClient::onTextMessageReceived(QString message)
{
    qDebug() << message;
    // 将字符串转换为QJsonDocument
    QJsonDocument doc = QJsonDocument::fromJson(message.toLocal8Bit().data());
    if (doc.isNull()) {
        qDebug() << "String Null " << message;
        return;
    }

    // 获取JSON对象
    QJsonObject obj = doc.object();
    QString type = obj.value("type").toString();

    qDebug() << type;

    if (type == "CurrentSpeed") {
        QString currentSpeed = obj.value("current_speed").toString();
        qDebug() << "Current Speed:" << currentSpeed;
        Speed target_speed = getSpeed(currentSpeed);
        if (target_speed == Speed::None) {
            _venv.setRunning(false);
        } else {
            _venv.setCurSpeed(target_speed);
            _venv.setRunning(true);
        }
    } else if (type == "CurrentCost") {
        QString currentCost = obj.value("current_cost").toString();
        qDebug() << "Current Cost:" << currentCost;

        bool ok;
        qreal currentCostValue = currentCost.toDouble(&ok);
        if (ok) {
            // Format the qreal to one decimal place
            qreal formattedCurrentCostValue = QString::number(currentCostValue, 'f', 1).toDouble(&ok);
            if (ok) {
                qDebug() << "Formatted Current Cost:" << formattedCurrentCostValue;
                _controller.setCurrentCost(formattedCurrentCostValue);
            } else {
                qDebug() << "Formatting to qreal failed!";
            }
        } else {
            qDebug() << "Conversion to qreal failed!";
        }
    } else {
        qDebug() << "Unknown type:" << type;
    }
}

// 关机
void ACClient::powerOff()
{
    QJsonObject obj;
    MessageBuilder::bind(obj);
    MessageBuilder::addItem("type", "SwitchPower");
    MessageBuilder::addItem("is_ac_power_on", qint64(0));
    setVenvPowerOff();
    _venv.setRunning(false);

    m_webSocket.sendTextMessage(MessageBuilder::toString());
}

// 风速偏离调节
void ACClient::deviation()
{
    // QJsonObject obj;
    // MessageBuilder::bind(obj);

    // qreal cur_temp = _venv.getCurTemp();
    // cur_temp = std::round(cur_temp * 10.0) / 10.0;

    // MessageBuilder::addItem("type", "TemperatureDeviated");
    // MessageBuilder::addItem("current_temp", cur_temp);

    // qDebug() << MessageBuilder::toString();
    // m_webSocket.sendTextMessage(MessageBuilder::toString());

    QString target_mode;
    Speed target_speed = _venv.getCurrSpeed();
    qreal target_temp = _venv.getTargetTemp();

    QString speed_level;
    switch (target_speed) {
    case Speed::High:
        speed_level = "High";
        break;
    case Speed::Medium:
        speed_level = "Medium";
        break;
    case Speed::Low:
        speed_level = "Low";
        break;
    }

    if (target_temp > 25.0) {
        target_mode = "Hot";
    } else {
        target_mode = "cold";
    }
    QJsonObject obj;
    MessageBuilder::bind(obj);
    MessageBuilder::addItem("type", "Adjust");
    MessageBuilder::addItem("target_mode", target_mode);

    // 四舍五入 target_temp 并保留到一位小数
    double temp = static_cast<double>(target_temp)
                  / 10.0; // 假设 target_temp 是以 10 的倍数存储的，例如 295 表示 29.5
    temp = std::round(temp * 10.0) / 10.0;                // 四舍五入到一位小数
    qint64 rounded_temp = static_cast<qint64>(temp * 10); // 转回整数存储，例如 29.5 变回 295

    MessageBuilder::addItem("target_temp", rounded_temp);
    MessageBuilder::addItem("target_speed", speed_level);

    // 设定_venv想要的set_temp
    _venv.setSetTemp(rounded_temp);
    qDebug() << MessageBuilder::toString();
    m_webSocket.sendTextMessage(MessageBuilder::toString());
}

void ACClient::echoTemp(qreal current_temp)
{
    QJsonObject obj;
    MessageBuilder::bind(obj);
    MessageBuilder::addItem("type", "CurrentTemperature");

    // 四舍五入 target_temp 并保留到一位小数
    qreal rounded_temp = std::round(current_temp * 10.0) / 10.0; // 转回整数存储，例如 29.5 变回 295

    MessageBuilder::addItem("current_temp", rounded_temp);

    qDebug() << MessageBuilder::toString();
    m_webSocket.sendTextMessage(MessageBuilder::toString());
}

// 调节温度风速。
void ACClient::adjustSpeedTempChanged(QString target_speed, qint64 target_temp, QString target_mode)
{
    QJsonObject obj;
    MessageBuilder::bind(obj);
    MessageBuilder::addItem("type", "Adjust");
    MessageBuilder::addItem("target_mode", target_mode);

    // 四舍五入 target_temp 并保留到一位小数
    double temp = static_cast<double>(target_temp)
                  / 10.0; // 假设 target_temp 是以 10 的倍数存储的，例如 295 表示 29.5
    temp = std::round(temp * 10.0) / 10.0;                // 四舍五入到一位小数
    qint64 rounded_temp = static_cast<qint64>(temp * 10); // 转回整数存储，例如 29.5 变回 295

    MessageBuilder::addItem("target_temp", rounded_temp);
    MessageBuilder::addItem("target_speed", target_speed);

    // 设定_venv想要的set_temp
    _venv.setSetTemp(rounded_temp);
    qDebug() << MessageBuilder::toString();
    m_webSocket.sendTextMessage(MessageBuilder::toString());
}

// 虚拟环境到达指定温度
void ACClient::venvReachTarget(qreal curr_temp)
{
    // 四舍五入 curr_temp 并保留到一位小数
    qreal rounded_temp = std::round(curr_temp * 10.0) / 10.0;

    QJsonObject obj;
    MessageBuilder::bind(obj);
    MessageBuilder::addItem("type", "TemperatureReached");
    MessageBuilder::addItem("current_temp", rounded_temp);
    qDebug() << MessageBuilder::toString();
    m_webSocket.sendTextMessage(MessageBuilder::toString());
}

void ACClient::handleRequestRewind()
{
    QString speed_level = returnSpeedLevel(static_cast<int>(_venv.getCurrSpeed()));
    qreal target_temp = _venv.getTargetTemp();

    QJsonObject obj;
    MessageBuilder::bind(obj);
    MessageBuilder::addItem("type", "Adjust");
    MessageBuilder::addItem("target_mode", "cold");
    MessageBuilder::addItem("target_temp", target_temp);
    MessageBuilder::addItem("target_speed", speed_level);

    qDebug() << MessageBuilder::toString();
    m_webSocket.sendTextMessage(MessageBuilder::toString());
}

Speed ACClient::getSpeed(QString speed)
{
    if (speed == "Low")
        return Speed::Low;
    else if (speed == "Medium")
        return Speed::Medium;
    else if (speed == "High")
        return Speed::High;
    else if (speed == "None")
        return Speed::None;
}

void ACClient::updateTemperature(qreal current_temp)
{
    qreal rounded_temp = QString::number(current_temp, 'f', 1).toDouble();
    _controller.setCurrentTemp(rounded_temp);
}

void ACClient::setVenvPowerOff()
{
    _venv.setNeedResent();
    _venv.setPowerOff();
}

void ACClient::setVenvPowerOn()
{
    _venv.setPowerOn();
}
