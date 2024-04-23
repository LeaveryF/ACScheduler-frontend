#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QLabel>
#include <QWidget>
#include <QTimer>

namespace Ui {
class controller;
}

class controller : public QWidget
{
    Q_OBJECT

public:
    explicit controller(QWidget *parent = nullptr);
    ~controller();
protected:
    void paintEvent(QPaintEvent* evt) override;
private:
    Ui::controller *ui;
    QImage Texture;
    QTimer timer;
    int shownow;
    int numsign[2];
    int state;
};

#endif // CONTROLLER_H
