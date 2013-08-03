#include "xbbackupcontroller.h"

XBBackupController::XBBackupController(QString target, QString inc_base_path, QString inc_path, QObject *parent) :
    QObject(parent), target_dir(target), backup_inc_base(inc_base_path), backup_inc_path(inc_path)
{
    connect(&process, SIGNAL(finished(int,QProcess::ExitStatus)),
                     this, SLOT(xtrabackup_backup_finished(int,QProcess::ExitStatus)));

}

void XBBackupController::start(QString xtrabackup_binary) {
    qDebug() << "Starting backup";
    if(backup_inc_path.length() == 0) { // full backup
        process.start(
                    xtrabackup_binary,
                    QStringList() << "--backup"
                    << "--parallel=4"
                    << (QString("--target-dir=")+target_dir)
                    << "--suspend-at-start" << "--suspend-at-end"
                    );
    } else {
        qDebug() << xtrabackup_binary
               << "--backup"
                                   << (QString("--target-dir=")+backup_inc_path)
                                   << (QString("--incremental-basedir=")+backup_inc_base)
                                   << "--suspend-at-start" << "--suspend-at-end";

        process.start(
                    xtrabackup_binary,
                    QStringList() << "--backup"
                    << "--parallel=4"
                    << (QString("--target-dir=")+backup_inc_path)
                    << (QString("--incremental-basedir=")+backup_inc_base)
                    << "--suspend-at-start" << "--suspend-at-end"
                    );
    }

    if(!process.waitForStarted()) {
        emit terminate();
    }
}
