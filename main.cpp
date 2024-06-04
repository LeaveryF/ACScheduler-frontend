#include "controller.h"
#include "appinit.h"

#include <QApplication>
#include <QFile>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AppInit::Instance()->start();

    QFile file("style.css");
    file.open(QFile::ReadOnly);
    if(!file.isOpen()){
        qDebug() << "css没有打开";
        return 0;
    }
    a.setStyleSheet(QLatin1String(file.readAll()));
    file.close();
    controller con;
    con.show();
    return a.exec();
}
