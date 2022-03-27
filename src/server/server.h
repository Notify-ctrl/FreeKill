#ifndef _SERVER_H
#define _SERVER_H

class ServerSocket;
class ClientSocket;
class Room;
class ServerPlayer;

class Server : public QObject {
    Q_OBJECT

public:
    explicit Server(QObject *parent = nullptr);
    ~Server();

    bool listen(const QHostAddress &address = QHostAddress::Any, ushort port = 9527u);

    void createRoom(ServerPlayer *owner, const QString &name, int capacity);
    Room *findRoom(int id) const;
    Room *lobby() const;

    ServerPlayer *findPlayer(int id) const;
    void removePlayer(int id);

    void updateRoomList();

    sqlite3 *getDatabase();

    void callLua(const QString &command, const QString &jsonData);
    LuaFunction callback;

    void roomStart(Room *room);
    LuaFunction startRoom;

signals:
    void roomCreated(Room *room);
    void playerAdded(ServerPlayer *player);
    void playerRemoved(ServerPlayer *player);

public slots:
    void processNewConnection(ClientSocket *client);
    void processRequest(const QByteArray &msg);

    void onRoomAbandoned();
    void onUserDisconnected();
    void onUserStateChanged();

private:
    ServerSocket *server;
    Room *m_lobby;
    QMap<int, Room *> rooms;
    QHash<int, ServerPlayer *> players;

    sqlite3 *db;
    lua_State *L;

    void handleNameAndPassword(ClientSocket *client, const QString &name, const QString &password);
};

extern Server *ServerInstance;

#endif // _SERVER_H
