#include <WebSocketClientWrapper.h>
#include <WebSocketTransport.h>

#include <QCoreApplication>
#include <QWebChannel>
#include <QWebSocketServer>
#include <chrono>
#include <iostream>

#include "CalculatorQtWebchannel_QObjectClient.h"

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
   qRegisterMetaType<capnzero::CalculatorQtWebchannel::EColor>("capnzero::CalculatorQtWebchannel::EColor");
   WebSocketClientWrapper clientWrapper(&server);
   QWebChannel channel;
   QObject::connect(&clientWrapper, &WebSocketClientWrapper::clientConnected,
                    &channel, &QWebChannel::connectTo);

   QClient wcServiceObj(context, "tcp://127.0.0.1:5555", "tcp://127.0.0.1:5556");
   channel.registerObject(QStringLiteral("Calculator"), &wcServiceObj);

   return app.exec();
}