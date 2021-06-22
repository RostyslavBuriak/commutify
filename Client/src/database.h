#pragma once

#ifndef DATABASE_H
#define DATABASE_H

#include <QString>
#include <QDateTime>
#include <sqlite3.h>

class DataBase
{
public:
    DataBase();
    int insert(QString name, QString text, QDateTime date);
    QStringList select(int id);
    int check();
private:
    sqlite3 *db;
};


#endif // DATABASE_H
