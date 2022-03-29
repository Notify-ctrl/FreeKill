#include "room.h"
#include "serverplayer.h"
#include "server.h"

Room::Room(Server* server)
{
    id = server->nextRoomId;
    server->nextRoomId++;
    this->server = server;
    setParent(server);
    gameStarted = false;
    timeout = 15;
    if (!isLobby()) {
        connect(this, &Room::playerAdded, server->lobby(), &Room::removePlayer);
        connect(this, &Room::playerRemoved, server->lobby(), &Room::addPlayer);
    }
}

Room::~Room()
{
    // TODO
    disconnect();
}

Server *Room::getServer() const
{
    return server;
}

int Room::getId() const
{
    return id;
}

bool Room::isLobby() const
{
    return id == 0;
}

QString Room::getName() const
{
    return name;
}

void Room::setName(const QString &name)
{
    this->name = name;
}

int Room::getCapacity() const
{
    return capacity;
}

void Room::setCapacity(int capacity)
{
    this->capacity = capacity;
}

bool Room::isFull() const
{
    return players.count() == capacity;
}

bool Room::isAbandoned() const
{
    return players.isEmpty();
}

ServerPlayer *Room::getOwner() const
{
    return owner;
}

void Room::setOwner(ServerPlayer *owner)
{
    this->owner = owner;
    QJsonArray jsonData;
    jsonData << owner->getId();
    owner->doNotify("RoomOwner", QJsonDocument(jsonData).toJson());
}

void Room::addPlayer(ServerPlayer *player)
{
    if (isFull() || !player) return;

    QJsonArray jsonData;

    // First, notify other players the new player is entering
    if (!isLobby()) {
        jsonData << player->getId();
        jsonData << player->getScreenName();
        jsonData << player->getAvatar();
        doBroadcastNotify(getPlayers(), "AddPlayer", QJsonDocument(jsonData).toJson());
    }

    players.append(player);
    player->setRoom(this);
    if (isLobby()) {
        player->doNotify("EnterLobby", "[]");
    } else {
        // Second, let the player enter room and add other players
        jsonData = QJsonArray();
        jsonData << this->capacity;
        player->doNotify("EnterRoom", QJsonDocument(jsonData).toJson());

        foreach (ServerPlayer *p, getOtherPlayers(player)) {
            jsonData = QJsonArray();
            jsonData << p->getId();
            jsonData << p->getScreenName();
            jsonData << p->getAvatar();
            player->doNotify("AddPlayer", QJsonDocument(jsonData).toJson());
        }

        if (isFull())
            start();
    }
    emit playerAdded(player);
}

void Room::removePlayer(ServerPlayer *player)
{
    players.removeOne(player);
    emit playerRemoved(player);

    if (isLobby()) return;

    // player->doNotify("QuitRoom", "[]");
    QJsonArray jsonData;
    jsonData << player->getId();
    doBroadcastNotify(getPlayers(), "RemovePlayer", QJsonDocument(jsonData).toJson());

    if (isAbandoned()) {
        emit abandoned();
    } else if (player == owner) {
        setOwner(players.first());
    }
}

QList<ServerPlayer *> Room::getPlayers() const
{
    return players;
}

QList<ServerPlayer *> Room::getOtherPlayers(ServerPlayer* expect) const
{
    QList<ServerPlayer *> others = getPlayers();
    others.removeOne(expect);
    return others;
}

ServerPlayer *Room::findPlayer(int id) const
{
    return server->findPlayer(id);
}

int Room::getTimeout() const
{
    return timeout;
}

void Room::setTimeout(int timeout)
{
    this->timeout = timeout;
}

bool Room::isStarted() const
{
    return gameStarted;
}

void Room::doRequest(const QList<ServerPlayer *> targets, int timeout)
{
    // TODO
}

void Room::doNotify(const QList<ServerPlayer *> targets, int timeout)
{
    // TODO
}

void Room::doBroadcastNotify(const QList<ServerPlayer *> targets,
                             const QString& command, const QString& jsonData)
{
    foreach (ServerPlayer *p, targets) {
        p->doNotify(command, jsonData);
    }
}

void Room::gameOver()
{
    gameStarted = false;
    // clean offline players
    foreach (ServerPlayer *p, players) {
        if (p->getState() == Player::Offline) {
            p->deleteLater();
        }
    }
}

void Room::run()
{
    gameStarted = true;
    getServer()->roomStart(this);
}

