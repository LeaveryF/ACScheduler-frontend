#pragma once

#include <QObject>


class AppInit : public QObject
{
    Q_OBJECT

private:
    static AppInit* self;

protected:
    bool eventFilter(QObject* obj,QEvent* evt) override;

public:
    static AppInit* Instance();
    explicit AppInit(QObject *parent = nullptr);
    void start();

signals:

};
