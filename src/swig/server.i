%nodefaultctor Server;
%nodefaultdtor Server;
class Server : public QObject {
public:
    void createRoom(ServerPlayer *owner, const QString &name, unsigned int capacity);
    Room *findRoom(unsigned int id) const;
    Room *lobby() const;

    ServerPlayer *findPlayer(unsigned int id) const;

    void updateRoomList();

    LuaFunction callback;
};

%{
void Server::callLua(const QString& command, const QString& json_data)
{
    Q_ASSERT(callback);

    lua_rawgeti(L, LUA_REGISTRYINDEX, callback);
    SWIG_NewPointerObj(L, this, SWIGTYPE_p_Server, 0);
    lua_pushstring(L, command.toUtf8());
    lua_pushstring(L, json_data.toUtf8());

    int error = lua_pcall(L, 3, 0, 0);
    if (error) {
        const char *error_msg = lua_tostring(L, -1);
        qDebug() << error_msg;
    }
}
%}

extern Server *ServerInstance;

%nodefaultctor Room;
%nodefaultdtor Room;
class Room : public QThread {
public:
    // Property reader & setter
    // ==================================={
    Server *getServer() const;
    unsigned int getId() const;
    bool isLobby() const;
    QString getName() const;
    void setName(const QString &name);
    unsigned int getCapacity() const;
    void setCapacity(unsigned int capacity);
    bool isFull() const;
    bool isAbandoned() const;

    ServerPlayer *getOwner() const;
    void setOwner(ServerPlayer *owner);

    void addPlayer(ServerPlayer *player);
    void removePlayer(ServerPlayer *player);
    QList<ServerPlayer *> getPlayers() const;
    ServerPlayer *findPlayer(unsigned int id) const;

    void setGameLogic(GameLogic *logic);
    GameLogic *getGameLogic() const;
    // ====================================}

    void startGame();
    void doRequest(const QList<ServerPlayer *> targets, int timeout);
    void doNotify(const QList<ServerPlayer *> targets, int timeout);
};
