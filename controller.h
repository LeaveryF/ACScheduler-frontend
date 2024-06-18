#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QLabel>
#include <QTimer>
#include <QWidget>

namespace Ui {
class controller;
}

class controller : public QWidget
{
    Q_OBJECT

public:
    explicit controller(QWidget *parent = nullptr);
    ~controller();

    void setCurrentTemp(qreal rounded_temp);
signals:
    void windSpeedChanged(QString target_speed, qint64 target_temp, QString target_mode);
    void powerOn();
    void powerOff();

protected:
    void paintEvent(QPaintEvent *evt) override;

private:
    Ui::controller *ui;
    QImage Texture;
    QTimer timer;
    int shownow;
    int numsign[2];
    int state;

    // 用于处理一秒钟内连续点击事件。
    QTimer *sendTimer;
    bool pendingRequest = false;
    QString pendingSpeedLevel;
    int pendingTemperature;
    QString pendingMode;
};

#endif // CONTROLLER_H
