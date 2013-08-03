#ifndef MYSQLCONNECTION_H
#define MYSQLCONNECTION_H

#include <QObject>
#include <mysql.h>


class MySQLConnection : public QObject
{
    Q_OBJECT
public:
    explicit MySQLConnection(QString server_, QString user_, QString password_, QString database_, QObject *parent = 0);
    ~MySQLConnection();


    QString datadir;
    QString version;

signals:
    void all_tables_locked();
    void all_tables_unlocked();

public slots:
    void lock_all_tables();
    void unlock_all_tables();

private:
    MYSQL *conn;
    QString server;
    QString user;
    QString password;
    QString database;
};

#endif // MYSQLCONNECTION_H
