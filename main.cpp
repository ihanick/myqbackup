#include <QCoreApplication>
#include "myqbackupmain.h"


/* incremental mode
 * --incremental-copies=N
 *  for first run it makes a full backup
 *  for each run in 2-N incremental based on previous incremental backup created
 *  for N+1 run we applying first incremental to full backup and making backup based on N
 * */
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    MyQBackupMain* backupcontroller = new MyQBackupMain(&a, &a);

    backupcontroller->start();


    return a.exec();
}
