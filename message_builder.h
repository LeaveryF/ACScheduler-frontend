#pragma once

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>

enum class MessageType {
    SwitchPower,
    Adjust,
    CurrentTemperature,
    TemperatureDeviated,
    TemperatureReached,
    UpdateCost,
    CurrentSpeed
};

class MessageBuilder {
private:
    static QJsonObject *obj_;

public:
    static void bind(QJsonObject &obj) { obj_ = &obj; }

    static auto addItem(QString key, MessageType type) {
        switch (type) {
        case MessageType::SwitchPower:
            obj_->insert(key, "SwitchPower");
        case MessageType::Adjust:
            obj_->insert(key, "Adjust");
        case MessageType::CurrentTemperature:
            obj_->insert(key, "CurrentTemperature");
        case MessageType::TemperatureDeviated:
            obj_->insert(key, "TemperatureDeviated");
        case MessageType::TemperatureReached:
            obj_->insert(key, "TemperatureReached");
        case MessageType::UpdateCost:
            obj_->insert(key, "UpdateCost");
        case MessageType::CurrentSpeed:
            obj_->insert(key, "CurrentSpeed");
        }
    }

    static auto addItem(QString key, qint64 value) { obj_->insert(key, value); }

    static auto addItem(QString key, QString value) { obj_->insert(key, value); }

    static auto addItem(QString key, qreal value) { obj_->insert(key, value); }

    static auto toString()
    {
        QJsonDocument doc(*obj_);
        return doc.toJson(QJsonDocument::Compact);
    }
};
