#include "database.h"
#include <QDebug>


DataBase::DataBase()
{
    int rc;
    rc = sqlite3_open("test.db", &db);

    if (rc != SQLITE_OK) {
        qDebug() << sqlite3_errmsg(db);
    }

    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, "CREATE TABLE IF NOT EXISTS Messages (id INTEGER PRIMARY KEY, name TEXT NOT NULL, text TEXT NOT NULL, date TEXT NOT NULL)", -1, &stmt, NULL);
    if (rc != SQLITE_OK) qDebug() << sqlite3_errmsg(db);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_OK) qDebug() << sqlite3_errmsg(db);

    sqlite3_finalize(stmt);

    sqlite3_close(db);
}

/*

1. put sent msg
2. put received msg
3. get all msgs on start


*/


const char *strToChar(QString s) {
    QByteArray ba = s.toLocal8Bit();
    const char *c = ba.data();
    return c;
}


int DataBase::insert(QString name, QString text, QDateTime date) {
    sqlite3_open("test.db", &db);

    const char *query = "INSERT INTO Messages (name, text, date) VALUES (?, ?, ?)";

    QByteArray nb = name.toLocal8Bit();
    const char *nc = nb.data();

    QByteArray tb = text.toLocal8Bit();
    const char *tc = tb.data();

    QByteArray dd = date.toString().toLocal8Bit();
    const char *dc = dd.data();

    sqlite3_stmt *stmt;

    sqlite3_prepare_v2(db, query, -1, &stmt, NULL);

    sqlite3_bind_text(stmt, 1, nc, -1, NULL);
    sqlite3_bind_text(stmt, 2, tc, -1, NULL);
    sqlite3_bind_text(stmt, 3, dc, -1, NULL);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    sqlite3_close(db);

    return 0;
}

QStringList DataBase::select(int id) {
    QStringList sl;

    sqlite3_open("test.db", &db);

    const char *query = "SELECT * FROM Messages WHERE id = ?";

    sqlite3_stmt *stmt;

    sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, id);

    sqlite3_step(stmt);

    sl.append(QString(QChar(sqlite3_column_int(stmt, 0))));
    sl.append(QString((const char *)sqlite3_column_text(stmt, 1)));
    sl.append(QString((const char *)sqlite3_column_text(stmt, 2)));
    sl.append(QString((const char *)sqlite3_column_text(stmt, 3)));

    sqlite3_finalize(stmt);

    sqlite3_close(db);
    return sl;
}


int DataBase::check() {
    int rc;
    sqlite3_stmt *stmt;
    const char *query = "SELECT id FROM Messages ORDER BY id DESC LIMIT 1";

    sqlite3_open("test.db", &db);

    sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    rc = sqlite3_step(stmt);

    if (rc == SQLITE_DONE) {
        rc = 0;
    } else if (rc == SQLITE_ROW) {
        rc = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return rc;
}
