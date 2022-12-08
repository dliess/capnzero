#include "CalculatorQtWebchannel_QObjectClient.h"
#include <iostream>
#include <chrono>
#include <QCoreApplication>
#include <QWebChannel>
#include <QWebSocketServer>
#include <WebSocketClientWrapper.h>
#include <WebSocketTransport.h>

using namespace capnzero::CalculatorQtWebchannel;

int main(int argc, char* argv[])
{
    zmq::context_t context;
    QCoreApplication app(argc, argv);
    
    QWebSocketServer server(QStringLiteral("Calculator"),
                            QWebSocketServer::NonSecureMode);
    if (!server.listen(QHostAddress::LocalHost, 12345))
    {
        std::cout << "Failed to open web socket server.\n"; 
        return -1;
    }
    WebSocketClientWrapper clientWrapper(&server);
    QWebChannel channel;
    QObject::connect(&clientWrapper, &WebSocketClientWrapper::clientConnected,
                        &channel, &QWebChannel::connectTo);

    QClient wcServiceObj(context,
                                                     "tcp://*:5555",
                                                     "tcp://*:5556");
    channel.registerObject(QStringLiteral("Calculator"), &wcServiceObj);

    return app.exec();
}