#ifndef _GLOBAL_H
#define _GLOBAL_H

// utilities

lua_State *CreateLuaState();
bool DoLuaScript(lua_State *L, const char *script);

sqlite3 *OpenDatabase(const QString &filename = "./server/users.db");
QJsonObject SelectFromDatabase(sqlite3 *db, const QString &sql);
// For Lua
QString SelectFromDb(sqlite3 *db, const QString &sql);
void ExecSQL(sqlite3 *db, const QString &sql);
void CloseDatabase(sqlite3 *db);

#endif // _GLOBAL_H
