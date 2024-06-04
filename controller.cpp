#include "controller.h"
#include "ui_controller.h"

#include <QPainter>
#include <QPainterPath>

controller::controller(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::controller)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    this->setProperty("canMove",true);
    Texture = QImage(height(),width(),QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&Texture);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(),QColor(200,200,200));
    numsign[0] = 2;
    state = 1;
    shownow = 1;
    timer.setInterval(400);
    connect(&timer,&QTimer::timeout,[this](){
        shownow ^= 1;
        switch(numsign[0]){
        case 0:{
            if(!shownow)ui->lab_Wtag1->hide();
            else ui->lab_Wtag1->show();
            ui->lab_Wtag2->hide();
            ui->lab_Wtag3->hide();
            break;
        }
        case 1:{
            if(!shownow)ui->lab_Wtag1->hide();
            else ui->lab_Wtag1->show();
            if(!shownow)ui->lab_Wtag2->hide();
            else ui->lab_Wtag2->show();
            ui->lab_Wtag3->hide();
            break;
        }
        case 2:{
            if(!shownow)ui->lab_Wtag1->hide();
            else ui->lab_Wtag1->show();
            if(!shownow)ui->lab_Wtag2->hide();
            else ui->lab_Wtag2->show();
            if(!shownow)ui->lab_Wtag3->hide();
            else ui->lab_Wtag3->show();
            break;
        }
        }
    });

    connect(ui->btn_wind,&QPushButton::clicked,[this](){
        if(timer.isActive()){
            timer.stop();
            state = 1;
            switch(numsign[0]){
            case 2:ui->lab_Wtag3->show();
            case 1:ui->lab_Wtag2->show();
            case 0:ui->lab_Wtag1->show();
            }
        }
        else {
            state = 0;
            timer.start();
        }
    });

    connect(ui->btn_up,&QPushButton::clicked,[this](){
        if(state == 0 && numsign[0] <= 1)
            numsign[0]++;
        else if(state == 1 && ui->ledNumber->value() <= 29){
            ui->ledNumber->display(ui->ledNumber->value() + 1);
        }
    });
    connect(ui->btn_down,&QPushButton::clicked,[this](){
        if(state == 0 && numsign[0] >= 1)
            numsign[0]--;
        else if(state == 1 && ui->ledNumber->value() >= 19){
            ui->ledNumber->display(ui->ledNumber->value() - 1);
        }
    });

}

controller::~controller()
{
    delete ui;
}

void controller::paintEvent(QPaintEvent *evt)
{
    QPainter painter(this);
    painter.setPen(Qt::transparent);
    painter.setBrush(Texture);
    painter.drawRoundedRect(rect(),20,20);
    QWidget::paintEvent(evt);
}
