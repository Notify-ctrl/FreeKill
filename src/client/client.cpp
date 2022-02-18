#include "client.h"
#include "client_socket.h"

Client::Client(QObject* parent)
    : QObject(parent)
{
    self = nullptr;

    ClientSocket *socket = new ClientSocket;
    router = new Router(this, socket, Router::TYPE_CLIENT);
}

Client::~Client()
{
    router->getSocket()->deleteLater();
}

void Client::connectToHost(const QHostAddress& server, ushort port)
{
    router->getSocket()->connectToHost(server, port);
}

void Client::requestServer(const QString& command, const QString& json_data, int timeout)
{
    int type = Router::TYPE_REQUEST | Router::SRC_CLIENT | Router::DEST_SERVER;
    router->request(type, command, json_data, timeout);
}

void Client::replyToServer(const QString& command, const QString& json_data)
{
    int type = Router::TYPE_REQUEST | Router::SRC_CLIENT | Router::DEST_SERVER;
    router->reply(type, command, json_data);
}

void Client::notifyServer(const QString& command, const QString& json_data)
{
    int type = Router::TYPE_REQUEST | Router::SRC_CLIENT | Router::DEST_SERVER;
    router->notify(type, command, json_data);
}



