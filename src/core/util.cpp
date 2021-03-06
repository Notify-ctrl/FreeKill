#include "util.h"

extern "C" {
  int luaopen_fk(lua_State *);
}

lua_State *CreateLuaState()
{
  lua_State *L = luaL_newstate();
  luaL_openlibs(L);
  luaopen_fk(L);

  return L;
}

bool DoLuaScript(lua_State *L, const char *script)
{
  lua_getglobal(L, "debug");
  lua_getfield(L, -1, "traceback");
  lua_replace(L, -2);

  luaL_loadfile(L, script);
  int error = lua_pcall(L, 0, LUA_MULTRET, -2);

  if (error) {
    const char *error_msg = lua_tostring(L, -1);
    qDebug() << error_msg;
    lua_pop(L, 2);
    return false;
  }
  lua_pop(L, 1);
  return true;
}

// For Lua debugging
void Dumpstack(lua_State *L)
{
  int top = lua_gettop(L);
  for (int i = 1; i <= top; i++) {
    printf("%d\t%s\t", i, luaL_typename(L, i));
    switch (lua_type(L, i)) {
    case LUA_TNUMBER:
      printf("%g\n",lua_tonumber(L, i));
      break;
    case LUA_TSTRING:
      printf("%s\n",lua_tostring(L, i));
      break;
    case LUA_TBOOLEAN:
      printf("%s\n", (lua_toboolean(L, i) ? "true" : "false"));
      break;
    case LUA_TNIL:
      printf("%s\n", "nil");
      break;
    default:
      printf("%p\n",lua_topointer(L, i));
      break;
    }
  }
}

sqlite3 *OpenDatabase(const QString &filename)
{
  sqlite3 *ret;
  int rc;
  if (!QFile::exists(filename)) {
    QFile file("./server/init.sql");
    if (!file.open(QIODevice::ReadOnly)) {
      qDebug() << "cannot open init.sql. Quit now.";
      qApp->exit(1);
    }

    QTextStream in(&file);
    char *err_msg;
    sqlite3_open(filename.toLatin1().data(), &ret);
    rc = sqlite3_exec(ret, in.readAll().toLatin1().data(), nullptr, nullptr, &err_msg);

    if (rc != SQLITE_OK ) {
      qDebug() << "sqlite error:" << err_msg;
      sqlite3_free(err_msg);
      sqlite3_close(ret);
      qApp->exit(1);
    }
  } else {
    rc = sqlite3_open(filename.toLatin1().data(), &ret);
    if (rc != SQLITE_OK) {
      qDebug() << "Cannot open database:" << sqlite3_errmsg(ret);
      sqlite3_close(ret);
      qApp->exit(1);
    }
  }
  return ret;
}

// callback for handling SELECT expression
static int callback(void *jsonDoc, int argc, char **argv, char **cols) {
  QJsonObject obj;
  for (int i = 0; i < argc; i++) {
    QJsonArray arr = obj[QString(cols[i])].toArray();
    arr << QString(argv[i] ? argv[i] : "#null");
    obj[QString(cols[i])] = arr;
  }
  ((QJsonObject *)jsonDoc)->swap(obj);
  return 0;
}

QJsonObject SelectFromDatabase(sqlite3 *db, const QString &sql) {
  QJsonObject obj;
  sqlite3_exec(db, sql.toUtf8().data(), callback, (void *)&obj, nullptr);
  return obj;
}

QString SelectFromDb(sqlite3 *db, const QString &sql) {
  QJsonObject obj = SelectFromDatabase(db, sql);
  return QJsonDocument(obj).toJson();
}

void ExecSQL(sqlite3 *db, const QString &sql) {
  sqlite3_exec(db, sql.toUtf8().data(), nullptr, nullptr, nullptr);
}

void CloseDatabase(sqlite3 *db) {
  sqlite3_close(db);
}
