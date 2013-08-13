#include "myqbackupmain.h"


MyQBackupMain::MyQBackupMain(QCoreApplication* app, QObject *parent) :
    QObject(parent), myapp(app)
{
    conf = new MyQBackupConfiguration(myapp);
    incremental_idx=-1;
}

void MyQBackupMain::start() {
    qDebug() << "Backup destination: " << conf->backup_dest;
    if(conf->max_incrementals) {
        if(conf->is_restore_mode) {
            qDebug() << "restore incremental backup";
        } else {
            qDebug() << "Making incremental backup";
        }
    }

    QDir backup_dest_dir(conf->backup_dest);
    QString backup_dest = conf->backup_dest;

    if(! conf->is_restore_mode) {
        if(conf->max_incrementals) {
            ni_inc_base = backup_dest_dir.absolutePath() + "/mysql-base-full";

            if(backup_dest_dir.exists("full") || backup_dest_dir.exists("fake-full")
                    || QFile::exists(backup_dest_dir.absolutePath() + "/full.tar.gz")
                    || QFile::exists(backup_dest_dir.absolutePath() + "/full.xbstream")
                    ) {
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
                                                        conf->password, conf->database, this);

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

    QString qpress = conf->xtrabackup_path + "qpress";
    QString xbstream = conf->xtrabackup_path + "xbstream";
    QString xbbinary = conf->xtrabackup_path.append(xtrabackup_binary);


    qDebug() << "binary:"<< xbbinary;
    qDebug() << "Dir for files:"<< backup_dest;
    qDebug() << "Base path for incremental:"<< backup_inc_base;
    qDebug() << "Incremental backup path:"<< backup_inc_path;


    // need remote backup controller for:
    // time xtrabackup_55  --defaults-group="mysqld" --backup --target-dir=$PWD --tmpdir=$PWD --compress --compress-threads=8 --parallel=4 --stream=xbstream > db.xbstream
    XBBackupController *backupctl = new XBBackupController(backup_dest, backup_inc_base, backup_inc_path, this);

    XBStreamer *backup_streamer = new XBStreamer(backup_dest, backup_inc_base, backup_inc_path, conf->ssh_host, xbbinary, this);
    SshFileCreationWatcher *ssh_file_watcher = new SshFileCreationWatcher(conf->ssh_host, this);

    NonInnoDBSyncer *rsync_syncer = new NonInnoDBSyncer(backup_dest,
                                                        ni_inc_base, ni_inc_last,
                                                        conf->restore_dir, conf->max_incrementals, myconnection,
                                                        (conf->ssh_host+":"+myconnection->datadir), this);
    XBPreparer *directory_preparer = 0;


    if(conf->max_incrementals == 0) { // prepare standalone full backup
        directory_preparer = new XBPreparer(backup_dest, 0, incremental_idx,
                                            conf->restore_dir, xbbinary, xbstream, qpress, conf->compression, conf->ssh_host, this);
    } else if(incremental_idx == 0) {
        directory_preparer = new XBPreparer(backup_dest, 1, incremental_idx,
                                            conf->restore_dir, xbbinary, xbstream, qpress, conf->compression, conf->ssh_host, this);
    } else if(incremental_idx > conf->max_incrementals) {
        directory_preparer = new XBPreparer(backup_dest, 2, incremental_idx,
                                            conf->restore_dir, xbbinary, xbstream, qpress, conf->compression, conf->ssh_host, this);

    } else {
        directory_preparer = new XBPreparer(backup_dest, 3, incremental_idx,
                                            conf->restore_dir, xbbinary, xbstream, qpress, conf->compression, conf->ssh_host, this);
    }

    // backup
    if(! conf->is_restore_mode) {

        if(conf->ssh_host.length() == 0) {
            connect(backupctl, SIGNAL(terminate()), myapp, SLOT(quit()));

            FileCreationWatcherThread* xtrabackup_start_watcher = new FileCreationWatcherThread(this);
            connect(xtrabackup_start_watcher, SIGNAL(file_created()),
                             backupctl, SLOT(xtrabackup_process_started()));
            xtrabackup_start_watcher->watch_for_file(backup_dest + "/xtrabackup_suspended_1");

            FileCreationWatcherThread* xtrabackup_finish_watcher = new FileCreationWatcherThread(this);
            connect(xtrabackup_finish_watcher, SIGNAL(file_created()),
                             backupctl, SLOT(xtrabackup_copied_all_innodb_data()));
            xtrabackup_finish_watcher->watch_for_file(backup_dest + "/xtrabackup_suspended_2");

            connect(backupctl, SIGNAL(dataCopied()),
                             rsync_syncer, SLOT(startBackup()));


            connect(rsync_syncer, SIGNAL(readyToUnlock()),
                             backupctl, SLOT(xtrabackup_process_finished()));
            connect(backupctl, SIGNAL(finished_ok()),
                             rsync_syncer, SLOT(unlockTables()));
            connect(rsync_syncer, SIGNAL(backupComplete()),
                             directory_preparer, SLOT(prepare()));

            connect(directory_preparer, SIGNAL(preRotate()),
                             rsync_syncer, SLOT(rotateBackup()));

            connect(directory_preparer, SIGNAL(backup_ready()),
                             myapp, SLOT(quit()));

            backupctl->start(xbbinary);
        } else {
            // in case of errors exit
            connect(backup_streamer, SIGNAL(terminate()),
                             myapp, SLOT(quit()));
            // at the end of xtrabackup backup, lock tables
            connect(ssh_file_watcher, SIGNAL(file_created()),
                             myconnection, SLOT(lock_all_tables()));

            // when tables became locked, start rsync backup
            connect(ssh_file_watcher, SIGNAL(file_created()),
                             rsync_syncer, SLOT(startBackup()));

            // at the end of rsync backup unlock database
            connect(rsync_syncer, SIGNAL(readyToUnlock()),
                             myconnection, SLOT(unlock_all_tables()));

            // when tables became unlocked we are ready to stop innodb log shipping, by removing lock file
            connect(myconnection, SIGNAL(all_tables_unlocked()),
                             ssh_file_watcher, SLOT(remove_file()));

            // when lock file removed, stopping log shipping
            connect(ssh_file_watcher, SIGNAL(file_removed()),
                             backup_streamer, SLOT(xbbackup_finished()));

            // when xtrabackup process finished everything is done and we can exit
            connect(backup_streamer, SIGNAL(finished_ok()),
                             directory_preparer, SLOT(prepare()));

            // after preparing backup we can exit
            connect(directory_preparer, SIGNAL(backup_ready()),
                             myapp, SLOT(quit()));

            ssh_file_watcher->watch_for_file("/tmp/xtrabackup_suspended_2");
            backup_streamer->start();
        }
    } else {
        // restore
        directory_preparer->restoreBackup();
        rsync_syncer->restoreBackup();

        exit(0);
    }
}
