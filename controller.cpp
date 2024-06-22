#include "controller.h"
#include <QPainter>
#include <QPainterPath>
#include "acclient.h"
#include "ui_controller.h"

controller::controller(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::controller)
    , sendTimer(new QTimer(this))
    , pendingRequest(false)
{
    ui->setupUi(this);
    ui->btn_down->setEnabled(false);
    ui->btn_up->setEnabled(false);
    ui->btn_wind->setEnabled(false);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    this->setProperty("canMove", true);
    Texture = QImage(height(), width(), QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&Texture);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(200, 200, 200));
    numsign[0] = 1;
    state = 1;
    shownow = 1;
    timer.setInterval(400);

    sendTimer->setInterval(1000); // 1秒的间隔
    sendTimer->setSingleShot(true);

    ui->lab_Wtag3->hide();
    ui->lab_Wtag2->show();
    ui->lab_Wtag1->show();

    connect(&timer, &QTimer::timeout, [this]() {
        shownow ^= 1;
        switch (numsign[0]) {
        case 0:
            ui->lab_Wtag1->setVisible(shownow);
            ui->lab_Wtag2->hide();
            ui->lab_Wtag3->hide();
            break;
        case 1:
            ui->lab_Wtag1->setVisible(shownow);
            ui->lab_Wtag2->setVisible(shownow);
            ui->lab_Wtag3->hide();
            break;
        case 2:
            ui->lab_Wtag1->setVisible(shownow);
            ui->lab_Wtag2->setVisible(shownow);
            ui->lab_Wtag3->setVisible(shownow);
            break;
        }
    });

    auto handleButtonClicked = [this]() {
        QString speed_level = returnSpeedLevel(numsign[0]);
        qDebug() << numsign[0];
        pendingSpeedLevel = speed_level;
        pendingTemperature = ui->ledNumber->value();
        if (pendingTemperature > 25.0) {
            pendingMode = "Hot";
        } else {
            pendingMode = "Cold";
        }

        if (!sendTimer->isActive()) {
            pendingRequest = true;
            sendTimer->start();
        }
    };

    connect(ui->btn_wind, &QPushButton::clicked, [this, handleButtonClicked]() {
        if (timer.isActive()) {
            timer.stop();
            state = 1;
            switch (numsign[0]) {
            case 2:
                ui->lab_Wtag3->show();
            case 1:
                ui->lab_Wtag2->show();
            case 0:
                ui->lab_Wtag1->show();
            }

            qreal current_temp = ui->ledNumber->value();

            emit windSpeedChanged(returnSpeedLevel(numsign[0]), current_temp, pendingMode);
        } else {
            state = 0;
            timer.start();
        }
    });

    connect(ui->btn_up, &QPushButton::clicked, [this, handleButtonClicked]() {
        if (state == 0 && numsign[0] <= 1) {
            numsign[0]++;
        } else if (state == 1 && ui->ledNumber->value() <= 29) {
            ui->ledNumber->display(ui->ledNumber->value() + 1);
            handleButtonClicked();
        }
    });

    connect(ui->btn_down, &QPushButton::clicked, [this, handleButtonClicked]() {
        if (state == 0 && numsign[0] >= 1) {
            numsign[0]--;
        } else if (state == 1 && ui->ledNumber->value() >= 19) {
            ui->ledNumber->display(ui->ledNumber->value() - 1);
            handleButtonClicked();
        }
    });

    connect(sendTimer, &QTimer::timeout, [this]() {
        if (pendingRequest) {
            emit windSpeedChanged(pendingSpeedLevel, pendingTemperature, pendingMode);
            pendingRequest = false;
        }
    });

    connect(ui->btn_power, &QPushButton::clicked, [this]() {
        ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex() ^ 1);
        if (ui->stackedWidget->currentIndex() == 0) {
            ui->btn_down->setEnabled(true);
            ui->btn_up->setEnabled(true);
            ui->btn_wind->setEnabled(true);
            emit powerOn();
        } else {
            ui->btn_down->setEnabled(false);
            ui->btn_up->setEnabled(false);
            ui->btn_wind->setEnabled(false);
            emit powerOff();
        }
    });
}

controller::~controller()
{
    delete ui;
}

void controller::setCurrentTemp(qreal rounded_temp)
{
    // 将 rounded_temp 转换为 QString，并保留一位小数
    QString tempText = QString::number(rounded_temp, 'f', 1) + " C";
    ui->lab_nowtemp->setText(tempText);
}

void controller::setCurrentCost(qreal rounded_cost)
{
    QString tempCost = QString::number(rounded_cost, 'f', 1) + " rmb";
    ui->lab_nowtemp_2->setText(tempCost);
}

void controller::setEnvTemp(int temperature)
{
    ui->ledNumber->display(temperature);
}

void controller::paintEvent(QPaintEvent *evt)
{
    QPainter painter(this);
    painter.setPen(Qt::transparent);
    painter.setBrush(Texture);
    painter.drawRoundedRect(rect(), 20, 20);
    QWidget::paintEvent(evt);
}
