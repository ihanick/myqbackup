#include "mysqlconnection.h"
#include "mysql.h"
#include "stdio.h"

MySQLConnection::MySQLConnection(QString server_, QString user_,
                                 QString password_, QString database_, QObject *parent) :
    QObject(parent),
    server(server_),
    user(user_),
    password(password_),
    database(database_)
{

    server = "localhost";
    user = "root";
    password = ""; /* set me first */
    database = "mysql";
    MYSQL_RES *res;
    MYSQL_ROW row;

    conn = mysql_init(NULL);
    /* Connect to database */
    if (!mysql_real_connect(conn, server.toLocal8Bit().data(),
                            user.toLocal8Bit().data(), password.toLocal8Bit().data(),
                            database.toLocal8Bit().data(), 0, NULL, 0)) {
       fprintf(stderr, "%s\n", mysql_error(conn));
       exit(1);
    }
    /* send SQL query */
    if (mysql_query(conn, "select @@datadir")) {
       fprintf(stderr, "%s\n", mysql_error(conn));
       exit(1);
    }
    res = mysql_use_result(conn);
    /* output table name */
    while ((row = mysql_fetch_row(res)) != NULL)
       datadir = row[0];
    mysql_free_result(res);

    /* send SQL query */
    if (mysql_query(conn, "select @@version")) {
       fprintf(stderr, "%s\n", mysql_error(conn));
       exit(1);
    }
    res = mysql_use_result(conn);
    /* output table name */
    while ((row = mysql_fetch_row(res)) != NULL)
       version = row[0];
    mysql_free_result(res);
}


MySQLConnection::~MySQLConnection() {
    /* close connection */
    mysql_close(conn);
}

void MySQLConnection::lock_all_tables() {
    if (mysql_query(conn, "FLUSH TABLES WITH READ LOCK")) {
       fprintf(stderr, "%s\n", mysql_error(conn));
       exit(1);
    }
}


void MySQLConnection::unlock_all_tables() {
    if (mysql_query(conn, "UNLOCK TABLES")) {
       fprintf(stderr, "%s\n", mysql_error(conn));
       exit(1);
    }
}

