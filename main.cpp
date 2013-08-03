#include <QCoreApplication>
#include <QProcess>
#include <QDebug>
#include <QFile>

#include "filecreationwatcherthread.h"
#include "xbbackupcontroller.h"
#include "noninnodbsyncer.h"
#include "xbpreparer.h"
#include "mysqlconnection.h"
#include "myqbackupconfiguration.h"


/* incremental mode
 * --incremental-copies=N
 *  for first run it makes a full backup
 *  for each run in 2-N incremental based on previous incremental backup created
 *  for N+1 run we applying first incremental to full backup and making backup based on N
 * */
int main(int argc, char *argv[])
{
    QString datadir;
    QString version;
    QString xtrabackup_binary;

    QCoreApplication a(argc, argv);

    MyQBackupConfiguration *conf = new MyQBackupConfiguration(&a);

    QString backup_inc_base;
    QString backup_inc_path;
    QString ni_inc_base;
    QString ni_inc_last;
    QString ni_inc_path;

    qDebug() << "Backup destination: " << conf->backup_dest;
    if(conf->max_incrementals) {
        if(conf->is_restore_mode) {
            qDebug() << "restore incremental backup";
        } else {
            qDebug() << "Making incremental backup";
        }
    }

    int incremental_idx=-1;
    QDir backup_dest_dir(conf->backup_dest);

    QString backup_dest = conf->backup_dest;

    if(! conf->is_restore_mode) {
        if(conf->max_incrementals) {
            ni_inc_base = backup_dest_dir.absolutePath() + "/mysql-base-full";

            if(backup_dest_dir.exists("full") || backup_dest_dir.exists("fake-full")
                    || QFile::exists(backup_dest_dir.absolutePath() + "/full.tar.gz")) {
                qDebug() << "Full backup already exists";

                incremental_idx=1;
                for(incremental_idx=1;incremental_idx <= conf->max_incrementals; ++ incremental_idx) {
                    if(backup_dest_dir.exists(QString("inc-%1").arg(incremental_idx))) {
                        qDebug() << "Found existing incremental backup";
                    } else {
                        break;
                    }
                }

                ni_inc_last = backup_dest_dir.absolutePath() + "/mysql-last-full";
                qDebug() << "Next backup number:"<<incremental_idx;
                if(incremental_idx == 1) {
                    backup_inc_base = backup_dest_dir.absolutePath() + "/fake-full";
                } else {
                    backup_inc_base = backup_dest_dir.absolutePath() + QString("/inc-%1").arg(incremental_idx-1);
                }

                backup_inc_path = backup_dest_dir.absolutePath() + QString("/inc-%1").arg(incremental_idx);
                backup_dest = backup_inc_path;
            } else {
                incremental_idx=0;
                backup_inc_base = backup_dest_dir.absolutePath() + "/full";
                backup_inc_path.clear();
                backup_dest = backup_inc_base;
            }
        } else {
            // full non-incremental backup
        }
    } else {
        incremental_idx = conf->max_incrementals;
        ni_inc_base = backup_dest_dir.absolutePath() + "/mysql-base-full";
        ni_inc_last = backup_dest_dir.absolutePath() + "/mysql-last-full";
        backup_inc_path = backup_dest_dir.absolutePath() + QString("/inc-%1").arg(conf->max_incrementals);
    }

    MySQLConnection *myconnection = new MySQLConnection(conf->server, conf->user,
                                                        conf->password, conf->database, &a);

    version = myconnection->version;

    if(version.startsWith("5.6.")) {
        xtrabackup_binary = "xtrabackup_56";
    } else if(version.startsWith("5.5.")) {
        xtrabackup_binary = "xtrabackup_55";
    } else if(version.startsWith("5.1.")) {
        xtrabackup_binary = "xtrabackup";
    } else if(version.startsWith("5.0.")) {
        xtrabackup_binary = "xtrabackup";
    }

    QString xbbinary = conf->xtrabackup_path.append(xtrabackup_binary);


    qDebug() << "binary:"<< xbbinary;
    qDebug() << "Dir for files:"<< backup_dest;
    qDebug() << "Base path for incremental:"<< backup_inc_base;
    qDebug() << "Incremental backup path:"<< backup_inc_path;


    XBBackupController *backupctl = new XBBackupController(backup_dest, backup_inc_base, backup_inc_path, &a);
    NonInnoDBSyncer *rsync_syncer = new NonInnoDBSyncer(backup_dest,
                                                        ni_inc_base, ni_inc_last,
                                                        conf->restore_dir, conf->max_incrementals, myconnection, &a);
    XBPreparer *directory_preparer = 0;


    if(conf->max_incrementals == 0) { // prepare standalone full backup
        directory_preparer = new XBPreparer(backup_dest, 0, incremental_idx,
                                            conf->restore_dir, xbbinary, conf->compression, &a);
    } else if(incremental_idx == 0) {
        directory_preparer = new XBPreparer(backup_dest, 1, incremental_idx,
                                            conf->restore_dir, xbbinary, conf->compression, &a);
    } else if(incremental_idx > conf->max_incrementals) {
        directory_preparer = new XBPreparer(backup_dest, 2, incremental_idx,
                                            conf->restore_dir, xbbinary, conf->compression, &a);

    } else {
        directory_preparer = new XBPreparer(backup_dest, 3, incremental_idx,
                                            conf->restore_dir, xbbinary, conf->compression, &a);
    }

    // backup
    if(! conf->is_restore_mode) {
        QObject::connect(backupctl, SIGNAL(terminate()), &a, SLOT(quit()));

        FileCreationWatcherThread* xtrabackup_start_watcher = new FileCreationWatcherThread(&a);
        QObject::connect(xtrabackup_start_watcher, SIGNAL(file_created()),
                         backupctl, SLOT(xtrabackup_process_started()));
        xtrabackup_start_watcher->watch_for_file(backup_dest + "/xtrabackup_suspended_1");

        FileCreationWatcherThread* xtrabackup_finish_watcher = new FileCreationWatcherThread(&a);
        QObject::connect(xtrabackup_finish_watcher, SIGNAL(file_created()),
                         backupctl, SLOT(xtrabackup_copied_all_innodb_data()));
        xtrabackup_finish_watcher->watch_for_file(backup_dest + "/xtrabackup_suspended_2");

        QObject::connect(backupctl, SIGNAL(dataCopied()),
                         rsync_syncer, SLOT(startBackup()));
        QObject::connect(rsync_syncer, SIGNAL(readyToUnlock()),
                         backupctl, SLOT(xtrabackup_process_finished()));
        QObject::connect(backupctl, SIGNAL(finished_ok()),
                         rsync_syncer, SLOT(unlockTables()));
        QObject::connect(rsync_syncer, SIGNAL(backupComplete()),
                         directory_preparer, SLOT(prepare()));

        QObject::connect(directory_preparer, SIGNAL(preRotate()),
                         rsync_syncer, SLOT(rotateBackup()));

        QObject::connect(directory_preparer, SIGNAL(backup_ready()),
                         &a, SLOT(quit()));

        backupctl->start(xbbinary);
    } else {
        // restore
        directory_preparer->restoreBackup();
        rsync_syncer->restoreBackup();

        exit(0);
    }

    return a.exec();
}
