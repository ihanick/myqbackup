#ifndef MYSQLCONNECTION_H
#define MYSQLCONNECTION_H

#define USE_LIB_MYSQL_CLIENT

#include <QObject>

#ifdef USE_LIB_MYSQL_CLIENT
#include <mysql.h>
#endif


#ifdef USE_LIB_MYSQL_CLIENT
extern MYSQL *conn;
#endif

class MySQLConnection : public QObject
{
    Q_OBJECT
public:
    explicit MySQLConnection(QString server_, int port_, QString user_,
                             QString password_, QString database_,
                             QObject *parent = 0);
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
    QString server;
    int     port;
    QString user;
    QString password;
    QString database;
};

#endif // MYSQLCONNECTION_H
