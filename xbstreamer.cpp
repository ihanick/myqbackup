#include "xbstreamer.h"
#include <QDebug>
#include <QDir>

XBStreamer::XBStreamer(const QString& destination_file_,
                       const QString& base_inc_path,
                       const QString& inc_path,
                       const QString& ssh_access_param_,
                       const QString& xtrabackup_binary_, QObject *parent) :
    QObject(parent),
    ssh_access_param(ssh_access_param_),
    destination_file(destination_file_),
    xtrabackup_binary(xtrabackup_binary_),
    backup_inc_path(inc_path),
    backup_base_path(base_inc_path)
{
}

void XBStreamer::start() {
    qDebug() << "Starting backup stream";
    if(backup_inc_path.length() == 0) { // full backup
        process.setStandardOutputFile(destination_file+".xbstream");
        process.start(
                    "ssh",
                    QStringList()
                    << ssh_access_param
                    //                << xtrabackup_binary
                    << "xtrabackup_55"
                    << "--defaults-group=\"mysqld\""
                    << "--backup"
                    << "--target-dir=/tmp"
                    << "--tmpdir=/tmp"
                    << "--compress"
                    << "--compress-threads=8"
                    << "--parallel=4"
                    << "--stream=xbstream"
                    << "--suspend-at-end"
                    );
    } else {
        qDebug() << "Should make incremental remote backup" << backup_inc_path << backup_base_path;

        QFile cpinfo(backup_base_path + "/xtrabackup_checkpoints");
        QDir dir(backup_base_path);
        dir.cdUp();
        if((backup_base_path + "/xtrabackup_checkpoints") != (dir.absolutePath()+ "/fake-full/xtrabackup_checkpoints")) {
        qDebug() << "Copying checkpoint info "<< (backup_base_path + "/xtrabackup_checkpoints")
                 << "to" << (dir.absolutePath()+ "/fake-full/xtrabackup_checkpoints");
        cpinfo.remove(dir.absolutePath()+ "/fake-full/xtrabackup_checkpoints");
        cpinfo.copy(dir.absolutePath()+ "/fake-full/xtrabackup_checkpoints");
        } else {
            qDebug() << "already good checkpoint info" << (backup_base_path + "/xtrabackup_checkpoints");
        }
        QProcess scp;
        scp.start(
                    "scp",
                    QStringList()
                    << "-r"
                    << (dir.absolutePath()+ "/fake-full")
                    << (ssh_access_param + ":" + "/tmp/")
                    );

        scp.waitForFinished();
        qDebug() << scp.readAll() << scp.readAllStandardError();

        process.setStandardOutputFile(destination_file+".xbstream");
        process.start(
                    "ssh",
                    QStringList()
                    << ssh_access_param
                    //                << xtrabackup_binary
                    << "xtrabackup_55"
                    << "--defaults-group=\"mysqld\""
                    << "--backup"
                    << "--target-dir=/tmp"
                    << (QString("--incremental-basedir=")+"/tmp/fake-full")
                    << "--tmpdir=/tmp"
                    << "--compress"
                    << "--compress-threads=8"
                    << "--parallel=4"
                    << "--stream=xbstream"
                    << "--suspend-at-end"
                    );
    }
    qDebug() << process.program() << process.arguments();
    if(!process.waitForStarted()) {
        emit terminate();
    }
}

void XBStreamer::xbbackup_finished() {
   qDebug() << "Xtrabackup finished, stop log shipping";
   process.waitForFinished();
   qDebug() << process.readAll() << process.readAllStandardError();
   QProcess remover;
   remover.start("ssh",
                 QStringList()
                 << ssh_access_param
                 << "rm"
                 << "/tmp/xtrabackup_log_copied"
                 );
   remover.waitForFinished();
   qDebug() << remover.readAll() << remover.readAllStandardOutput();
   emit finished_ok();
}
