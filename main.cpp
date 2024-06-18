#include "appinit.h"
#include "controller.h"

#include <QApplication>
#include <QFile>
#include <QtCore/QCommandLineOption>
#include <QtCore/QCommandLineParser>
#include "acclient.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    using namespace Qt::Literals::StringLiterals;

    // 读取命令行参数并且初始化websocket。
    QCommandLineParser parser;
    parser.setApplicationDescription("QtWebSockets example: echoclient");
    parser.addHelpOption();

    QCommandLineOption dbgOption(QStringList{u"d"_s, u"debug"_s},
                                 QCoreApplication::translate("main",
                                                             "Debug output [default: off]."));
    parser.addOption(dbgOption);

    QCommandLineOption hostnameOption(QStringList{u"n"_s, u"hostname"_s},
                                      QCoreApplication::translate("main",
                                                                  "Hostname [default: localhost]."),
                                      "hostname",
                                      "localhost");
    parser.addOption(hostnameOption);

    QCommandLineOption portOption(QStringList{u"p"_s, u"port"_s},
                                  QCoreApplication::translate("main", "Port [default: 1234]."),
                                  "port",
                                  "1234");
    parser.addOption(portOption);

    // 新增房间号参数选项
    QCommandLineOption roomOption(QStringList{u"r"_s, u"room"_s},
                                  QCoreApplication::translate("main", "Room number [default: 0]."),
                                  "room",
                                  "0");
    parser.addOption(roomOption);

    parser.process(a);
    bool debug = parser.isSet(dbgOption);
    bool ok = true;
    int port = parser.value(portOption).toInt(&ok);
    if (!ok || port < 1 || port > 65535) {
        qWarning("Port invalid, must be a number between 1 and 65535\n%s",
                 qPrintable(parser.helpText()));
        return 1;
    }

    QString room = parser.value(roomOption);

    QUrl url;
    url.setScheme(u"ws"_s);
    url.setHost(parser.value(hostnameOption));
    url.setPort(port);
    url.setPath("/websocket/" + room); // 追加房间号到路径中

    // 前端显示。
    AppInit::Instance()->start();

    QFile file("style.css");
    file.open(QFile::ReadOnly);
    if (!file.isOpen()) {
        qDebug() << "css没有打开";
        return 0;
    }
    a.setStyleSheet(QLatin1String(file.readAll()));
    file.close();

    // 初始化ACClient websocket对象。
    // debug信息：
    //url = "ws://localhost:8765/websocket/";
    ACClient client(url, debug);
    //QObject::connect(&client, &ACClient::closed, &a, &QApplication::quit);

    //controller con;
    //con.show();
    return a.exec();
}
