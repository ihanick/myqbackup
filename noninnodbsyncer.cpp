#include "noninnodbsyncer.h"
#include <QDir>

NonInnoDBSyncer::NonInnoDBSyncer(QString target, QString inc_base, QString inc_last, QString restore_dest, int incremental_idx,
                                 MySQLConnection *ndb, QObject *parent) :
    QObject(parent),
    db(ndb),
    target_dir(target),
    base_dir(inc_base),
    last_dir(inc_last),
    restore_dir(restore_dest),
    restore_inc_idx(incremental_idx)
{
}


/*
  Incremental rsync backups
touch source/a1
# full backup
rsync -av source backup/
touch source/a2
# prepare full backup
rsync -av --only-write-batch=1.rsync.batch source backup/
rm 1.rsync.batch.sh
rsync -a backup/ backup.last
# inc-1
rsync -av --read-batch=1.rsync.batch backup.last/
touch source/a3
# inc-2
rsync -av --write-batch=2.rsync.batch source backup.last/
rm 2.rsync.batch.sh
touch source/a4
# inc-3
rsync -av --write-batch=3.rsync.batch source backup.last/
rm 3.rsync.batch.sh
touch source/a5
# backup and rotate
rsync -av --write-batch=4.rsync.batch source backup.last/
rm 4.rsync.batch.sh
rsync -av --read-batch=1.rsync.batch backup/
rm 1.rsync.batch
mv 2.rsync.batch 1.rsync.batch ;mv 3.rsync.batch 2.rsync.batch ; mv 4.rsync.batch 3.rsync.batch


# restore full
rsync -a backup/ restore-full

# restore inc-1
rsync -a backup/ restore-inc-1
rsync -av --read-batch=1.rsync.batch restore-inc-1/

# restore inc-2
rsync -a backup/ restore-inc-2
rsync -av --read-batch=1.rsync.batch restore-inc-2/
rsync -av --read-batch=2.rsync.batch restore-inc-2/

# restore last
rsync -a backup.last/ restore-inc-3
*/

void NonInnoDBSyncer::startBackup() {
    {
            QString datadir = db->datadir;
            QProcess rsync_process;
        // on linux we can:
        // find /var/lib/mysql -type d -print0 | xargs -P 3 -0 rsync [rsync-options]
        // rsync --archive --no-times --ignore-times --inplace --delete --quiet --no-recursive --dirs

            QString backupto;
            QString additional_options;
            if(base_dir.length()) {
                if(last_dir.length()) {
                    backupto = last_dir;
                    additional_options = QString("--write-batch=").append(target_dir).append("/rsync.batch");
                    QDir dir;
                    if(!dir.exists(last_dir)) {
                        //  rsync -a backup/ backup.last
                        rsync_process.start("rsync",
                                            QStringList() << "-av"
                                            << (base_dir + "/")
                                            << last_dir
                                            );
                        rsync_process.waitForFinished();
                        qDebug() << rsync_process.readAll();
                    }
                } else {
                    backupto = base_dir;
                }
            } else {
                backupto = target_dir;
            }

            QDir dir;
            if(!dir.exists(backupto)) {
                dir.mkdir(backupto);
            }

            db->lock_all_tables();
            if(additional_options.length()) {
                rsync_process.start("rsync",
                                    QStringList() << "-av"
                                    << "--no-times"
                                    << "--ignore-times"
                                    << "--inplace"
                                    << "--delete"
                                    << "--exclude=*.ibd"
                                    << "--exclude=ibdata*"
                                    << "--exclude=ib_log*"
                                    << additional_options
                                    << (datadir + ".")
                                    << (backupto + "/."));
            } else {
            rsync_process.start("rsync",
                                QStringList() << "-av"
                                << "--no-times"
                                << "--ignore-times"
                                << "--inplace"
                                << "--delete"
                                << "--exclude=*.ibd"
                                << "--exclude=ibdata*"
                                << "--exclude=ib_log*"
                                << (datadir + ".")
                                << (backupto + "/."));
            }

//            qDebug() << rsync_process.program() << rsync_process.arguments();

            rsync_process.waitForFinished();
            qDebug() << rsync_process.readAll();
            emit readyToUnlock();
        }
}


void NonInnoDBSyncer::rotateBackup() {
    qDebug() << "Rotating rsync backup";
    //rsync -av --no-times --ignore-times --inplace --delete --read-batch inc-1/rsync.batch restore-inc1/
    QDir dir(target_dir);
    dir.cdUp();
    QProcess rsync_process;
    rsync_process.start("rsync",
                        QStringList() << "-av"
                        << "--no-times"
                        << "--ignore-times"
                        << "--inplace"
                        << "--delete"
                        << "--read-batch"
                        << (dir.absolutePath()+"/inc-1/rsync.batch")
                        << (base_dir + "/")
                        );
//    qDebug() << rsync_process.program() << rsync_process.arguments();
    rsync_process.waitForFinished();
    qDebug() << rsync_process.readAll();
}

void NonInnoDBSyncer::restoreBackup() {
    qDebug() << "Try to restore rsync backup from" << target_dir << "based on" << base_dir
                << "to" << restore_dir;

    QDir dir;
    if(!dir.exists(restore_dir)) {
        dir.mkdir(restore_dir);
    }

    QProcess rsync_process;
    rsync_process.start("rsync",
                        QStringList() << "-av"
                        << (base_dir + "/")
                        << (restore_dir + "/.")
                        );
//    qDebug() << rsync_process.program() << rsync_process.arguments();
    rsync_process.waitForFinished();
    qDebug() << rsync_process.readAll();

    qDebug() << "Applying incrementals starting up to "<< restore_inc_idx;
    for(int i=1; i<= restore_inc_idx; ++i) {
        applyIncremental(i);
    }
    qDebug() << "Restore complete";
    emit restoreComplete();
}

void NonInnoDBSyncer::applyIncremental(int idx) {
    qDebug() << "Applying"<< idx;
    QDir dir(target_dir);
    dir.cdUp();
    QProcess rsync_process;
    rsync_process.start("rsync",
                        QStringList() << "-av"
                        << "--no-times"
                        << "--ignore-times"
                        << "--inplace"
                        << "--delete"
                        << "--exclude=*.ibd"
                        << "--exclude=ibdata*"
                        << "--exclude=ib_log*"
                        << "--exclude=xtrabackup_*"
                        << "--read-batch"
                        << (dir.absolutePath()+(QString("/inc-%1/rsync.batch").arg(idx)) )
                        << (restore_dir + "/."));
    rsync_process.waitForFinished();
//    qDebug() << rsync_process.program() << rsync_process.arguments();
    qDebug() << rsync_process.readAll();
}
