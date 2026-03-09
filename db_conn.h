#ifndef DB_CONN_H
#define DB_CONN_H

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDebug>
#include <QUuid>

struct UserInfo {
    int id;
    QString username;
    QString realName;
    double balance;
    bool isAdmin;
};

class DbManager {
public:
    static QSqlDatabase getConn(bool unique = false) {
        QString connName;
        if (unique) {
            static int connCount = 0;
            connName = QString("skylink_odbc_conn_%1").arg(connCount++);
        } else {
            connName = QSqlDatabase::defaultConnection;
        }

        QSqlDatabase db;
        if (QSqlDatabase::contains(connName)) {
            db = QSqlDatabase::database(connName);
        } else {
            db = QSqlDatabase::addDatabase("QODBC", connName);
            db.setDatabaseName("skylink_db");
            db.setUserName("root");
            db.setPassword("");
        }

        if (!db.isOpen()) {
            db.open();
        }

        return db;
    }

    static void removeConn(QSqlDatabase& db) {
        QString connName = db.connectionName();
        db.close();
        QSqlDatabase::removeDatabase(connName);
    }
};

#endif // DB_CONN_H
