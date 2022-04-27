#include "TemperatureQtSignalsOnly_QObjectClient.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <QGuiApplication>
#include <QQuickView>
#include <QSocketNotifier>
#include <QQmlContext>

#include "TemperatureQtSignalsOnly_Client.h"

int main(int argc, char* argv[])
{
    zmq::context_t context;


    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    const QUrl url(QStringLiteral("qrc:/main.qml"));

    capnzero::TemperatureQtSignalsOnly::QClient backend(context, "tcp://localhost:5556");
    QQuickView *view = new QQuickView;
    view->rootContext()->setContextProperty("backend", &backend);
    view->setSource(url);
    view->show();

    return app.exec();
}