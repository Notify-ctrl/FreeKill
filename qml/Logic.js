var callbacks = {}

callbacks["ErrorMsg"] = function(jsonData) {
    toast.show(jsonData);
    mainWindow.busy = false;
}

callbacks["EnterLobby"] = function(jsonData) {
    if (mainStack.depth === 1) {
        mainStack.push(lobby);
    }
    mainWindow.busy = false;
}

callbacks["EnterRoom"] = function(jsonData) {
    mainStack.push(room);
    mainWindow.busy = false;
}

callbacks["UpdateRoomList"] = function(jsonData) {
    let current = mainStack.currentItem;    // should be lobby
    current.roomModel.clear();
    JSON.parse(jsonData).forEach(function(room) {
        current.roomModel.append({
        roomId: room[0],
        roomName: room[1],
        gameMode: room[2],
        playerNum: room[3],
        capacity: room[4],
        });
    });
}
