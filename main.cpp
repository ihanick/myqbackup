#include <QCoreApplication>
#include <QProcess>
#include <QDebug>
//#include <QRegularExpression>
#include <QFile>

#include "filecreationwatcherthread.h"
#include "xbbackupcontroller.h"
#include "noninnodbsyncer.h"
#include "xbpreparer.h"
#include "mysqlconnection.h"

/* incremental mode
 * --incremental-copies=N
 *  for first run it makes a full backup
 *  for each run in 2-N incremental based on previous incremental backup created
 *  for N+1 run we applying first incremental to full backup and making backup based on N
 * */

void printXtrabackupError(QProcess& process) {
    qDebug() << process.readAllStandardOutput();
    QByteArray xtrabackup_stderr(process.readAllStandardError());
    QString xtrabackup_output(xtrabackup_stderr);
/*
    QRegularExpression re("xtrabackup: error: (.*)$");
    QRegularExpressionMatch match = re.match(xtrabackup_output);
    if (match.hasMatch()) {
        qDebug() << "Got error:" << match.captured(1);
    }
*/
}

int main(int argc, char *argv[])
{
    QString datadir;
    QString version;
    QString xtrabackup_binary;
    bool compression = false;

    QCoreApplication a(argc, argv);

    qDebug() << a.libraryPaths();

    QStringList cmdline_args = QCoreApplication::arguments();

    QString backup_dest(".");
    QString backup_inc_base;
    QString backup_inc_path;
    QString ni_inc_base;
    QString ni_inc_last;
    QString ni_inc_path;
    QString restore_dir;
    QString xtrabackup_prefix = "/home/ihanick/src/percona-xtrabackup-2.1.3";
    int max_incrementals = 0;
    qDebug() << cmdline_args;
    foreach(QString arg, cmdline_args) {
        if(arg.startsWith("--")) {
            qDebug() << "Found option:" << arg;
            if(arg.startsWith("--inc=")) {
                max_incrementals = QStringRef(&arg, 6, arg.length()-6).toString().toInt();
            } else if(arg.startsWith("--restore-to=")) {
                restore_dir = QStringRef(&arg, QString("--restore-to=").length(),
                                         arg.length()-QString("--restore-to=").length()).toString();
            } else if(arg.startsWith("--compression")) {
                compression = true;
            } else if(arg.startsWith("--xbprefix=")) {
                xtrabackup_prefix =  restore_dir = QStringRef(&arg, QString("--xbprefix=").length(),
                        arg.length()-QString("--xbprefix=").length()).toString();
            }
        } else {
            backup_dest = arg;
        }
    }

    QString xtrabackup_path(xtrabackup_prefix + "/bin/");

    qDebug() << "Backup destination: " << backup_dest;
    if(max_incrementals) {
        if(restore_dir.length()) {
            qDebug() << "restore incremental backup";
        } else {
            qDebug() << "Making incremental backup";
        }
    }

    int incremental_idx=-1;
    QDir backup_dest_dir(backup_dest);

    if(restore_dir.length() == 0) {
        if(max_incrementals) {
            ni_inc_base = backup_dest_dir.absolutePath() + "/mysql-base-full";

            if(backup_dest_dir.exists("full") || backup_dest_dir.exists("fake-full")
                    || QFile::exists(backup_dest_dir.absolutePath() + "/full.tar.gz")) {
                qDebug() << "Full backup already exists";

                incremental_idx=1;
                for(incremental_idx=1;incremental_idx <= max_incrementals; ++ incremental_idx) {
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
        incremental_idx = max_incrementals;
        ni_inc_base = backup_dest_dir.absolutePath() + "/mysql-base-full";
        ni_inc_last = backup_dest_dir.absolutePath() + "/mysql-last-full";
        backup_inc_path = backup_dest_dir.absolutePath() + QString("/inc-%1").arg(max_incrementals);
    }

    MySQLConnection myconnection;

    version = myconnection.version;

    if(version.startsWith("5.6.")) {
        xtrabackup_binary = "xtrabackup_56";
    } else if(version.startsWith("5.5.")) {
        xtrabackup_binary = "xtrabackup_55";
    } else if(version.startsWith("5.1.")) {
        xtrabackup_binary = "xtrabackup";
    } else if(version.startsWith("5.0.")) {
        xtrabackup_binary = "xtrabackup";
    }

    QString xbbinary = xtrabackup_path.append(xtrabackup_binary);


    qDebug() << "binary:"<< xbbinary;
    qDebug() << "Dir for files:"<< backup_dest;
    qDebug() << "Base path for incremental:"<< backup_inc_base;
    qDebug() << "Incremental backup path:"<< backup_inc_path;


    XBBackupController *backupctl = new XBBackupController(backup_dest, backup_inc_base, backup_inc_path, &a);
    NonInnoDBSyncer *rsync_syncer = new NonInnoDBSyncer(backup_dest,
                                                        ni_inc_base, ni_inc_last,
                                                        restore_dir, max_incrementals, &myconnection, &a);
    XBPreparer *directory_preparer = 0;


    if(max_incrementals == 0) { // prepare standalone full backup
        directory_preparer = new XBPreparer(backup_dest, 0, incremental_idx, restore_dir, xbbinary, compression, &a);
    } else if(incremental_idx == 0) {
        directory_preparer = new XBPreparer(backup_dest, 1, incremental_idx, restore_dir, xbbinary, compression, &a);
    } else if(incremental_idx > max_incrementals) {
        directory_preparer = new XBPreparer(backup_dest, 2, incremental_idx, restore_dir, xbbinary, compression, &a);

    } else {
        directory_preparer = new XBPreparer(backup_dest, 3, incremental_idx, restore_dir, xbbinary, compression, &a);
    }

    // backup
    if(restore_dir.length() == 0) {
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
