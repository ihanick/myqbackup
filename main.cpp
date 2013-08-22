#include <QCoreApplication>
#include "myqbackupmain.h"

#include <mysql.h>
MYSQL *conn;

/* incremental mode
 * --incremental-copies=N
 *  for first run it makes a full backup
 *  for each run in 2-N incremental based on previous incremental backup created
 *  for N+1 run we applying first incremental to full backup and making backup based on N
 * */
int main(int argc, char *argv[])
{
    // having different thread id after running mysql_init
    conn = mysql_init(NULL);

    QCoreApplication a(argc, argv);
    MyQBackupMain* backupcontroller = new MyQBackupMain(&a);

    QObject::connect(backupcontroller, SIGNAL(terminate()),
                     &a, SLOT(quit()));

    backupcontroller->start();


    return a.exec();
}
