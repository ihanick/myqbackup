#include "xbpreparer.h"

XBPreparer::XBPreparer(QString target, int prepare_type, int incremental_idx_, QString restore_to, QString nxtrabackup_binary, bool compression, QObject *parent) :
    QObject(parent),
    xtrabackup_binary(nxtrabackup_binary),
    target_dir(target),
    type(prepare_type),
    incremental_idx(incremental_idx_),
    restore_dir(restore_to),
    use_compression(compression)
{
}

bool removeDir(const QString & dirName)
{
	bool result;
	QDir dir(dirName);

	if (dir.exists(dirName)) {
		Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
			if (info.isDir()) {
				result = removeDir(info.absoluteFilePath());
			}
			else {
				result = QFile::remove(info.absoluteFilePath());
			}

			if (!result) {
				return result;
			}
		}
		result = dir.rmdir(dirName);
	}
	return result;
}

void XBPreparer::prepare() {
    qDebug() << "Prepare runs!" << type;
    QProcess xtrabackup_prepare_process;
    switch(type) {
    case 0:
        xtrabackup_prepare_process.start(
                    xtrabackup_binary,
                    QStringList()
                    << "--prepare"
                    << (QString("--target-dir=")+target_dir) );
        break;
    case 1:
    {
        QDir last_inc_dir(target_dir);
        last_inc_dir.cdUp();
        qDebug() << "Backup dir" << last_inc_dir.absolutePath();
        QString inc_base = last_inc_dir.absolutePath() + "/full";
        xtrabackup_prepare_process.start(
                    xtrabackup_binary,
                    QStringList()
                    << "--prepare" << "--apply-log-only"
                    << (QString("--target-dir=")+inc_base) );

        break;
    }
    case 2:
    {
        QDir last_inc_dir(target_dir);
        last_inc_dir.cdUp();
        qDebug() << "Backup dir" << last_inc_dir.absolutePath();
        QString inc_base = last_inc_dir.absolutePath() + "/full";
        QString inc_path = last_inc_dir.absolutePath() + "/inc-1";

        if(!last_inc_dir.exists(inc_base) && use_compression) {
            QDir dir;
            dir.mkdir(inc_base);
            QProcess tar_process;
            tar_process.start("tar",
                              QStringList()
                              << "-C" << (inc_base + "/.")
                              << "-xzf"
                              << (last_inc_dir.absolutePath() + "/full.tar.gz")
                              );
//            qDebug() << tar_process.program() << tar_process.arguments();
            tar_process.waitForFinished();
            qDebug() << tar_process.readAll() << tar_process.readAllStandardError();
        }
        xtrabackup_prepare_process.start(
                    xtrabackup_binary,
                    QStringList()
                    << "--prepare" << "--apply-log-only"
                    << QString("--target-dir=").append(inc_base)
                    << QString("--incremental-dir=").append(inc_path));
        qDebug() << "running: " << xtrabackup_binary << "--prepare" << "--apply-log-only"
                 << QString("--target-dir=").append(inc_base)
                 << QString("--incremental-dir=").append(inc_path);
        break;
    }
    default:
        emit backup_ready();
        return;
    }
    xtrabackup_prepare_process.waitForFinished();
    qDebug() << "Backup prepared by" << xtrabackup_binary;
    qDebug() << xtrabackup_prepare_process.readAll()
             << xtrabackup_prepare_process.readAllStandardError();

    if(type == 1) {
        if(use_compression) {
            QDir last_inc_dir(target_dir);
            last_inc_dir.cdUp();
            QProcess tar_process;
            tar_process.start("tar",
                              QStringList()
                              << "-C" << (last_inc_dir.absolutePath() + "/full/.")
                              << "-czf"
                              << (last_inc_dir.absolutePath() + "/full.tar.gz")
                              << "./"
                              );
//            qDebug() << tar_process.program() << tar_process.arguments();
            tar_process.waitForFinished();
            qDebug() << tar_process.readAll();

            QDir dir1(last_inc_dir.absolutePath() + "/full");
            //dir1.removeRecursively();
            removeDir(dir1.absolutePath());
        }
    } else if(type == 2) {
        emit preRotate();
        QDir last_inc_dir(target_dir);
        last_inc_dir.cdUp();
        QDir dir1(last_inc_dir.absolutePath() + "/inc-1");
        //dir1.removeRecursively();
        removeDir(dir1.absolutePath());
        for(int i=2; i<=incremental_idx; ++i) {
            qDebug() << "Move" << last_inc_dir.absolutePath() + QString("/inc-%1").arg(i) <<
                        last_inc_dir.absolutePath() + QString("/inc-%1").arg(i-1);
            last_inc_dir.rename(QString("inc-%1").arg(i),
                                QString("inc-%1").arg(i-1));
        }

        if(use_compression) { // ready to overwrite old full
            QDir last_inc_dir(target_dir);
            last_inc_dir.cdUp();
            QProcess tar_process;
            tar_process.start("tar",
                              QStringList()
                              << "-C" << (last_inc_dir.absolutePath() + "/full/.")
                              << "-czf"
                              << (last_inc_dir.absolutePath() + "/full.tar.gz")
                              << "./"
                              );
//            qDebug() << tar_process.program() << tar_process.arguments();
            tar_process.waitForFinished();
            qDebug() << tar_process.readAll() << tar_process.readAllStandardError();

            QDir dir1(last_inc_dir.absolutePath() + "/full");
            //dir1.removeRecursively();
            removeDir(dir1.absolutePath());
        }
    }
    emit backup_ready();
}

void XBPreparer::restoreBackup() {
    QDir last_inc_dir(target_dir);
    //    last_inc_dir.cdUp();

    QDir dir;
    if(!dir.exists(restore_dir)) {
        dir.mkdir(restore_dir);
    }
    qDebug() << "Backup dir" << last_inc_dir.absolutePath() << "Restore dir"<< restore_dir;


    if(use_compression) {
        QProcess tar_process;
        tar_process.start("tar",
                          QStringList()
                          << "-C" << (restore_dir + "/.")
                          << "-xzf"
                          << (last_inc_dir.absolutePath() + "/full.tar.gz")
                          );
//        qDebug() << tar_process.program() << tar_process.arguments();
        tar_process.waitForFinished();
        qDebug() << tar_process.readAll();
    } else {
        QString base_dir = last_inc_dir.absolutePath() + "/full";
        QProcess rsync_process;
        rsync_process.start("rsync",
                            QStringList() << "-av"
                            << (base_dir + "/")
                            << (restore_dir + "/.")
                            );
        //    qDebug() << rsync_process.program() << rsync_process.arguments();
        rsync_process.waitForFinished();
        qDebug() << rsync_process.readAll();
    }

    QProcess xtrabackup_prepare_process;
    for(int i=1; i<=incremental_idx; ++i) {
        QString inc_path = last_inc_dir.absolutePath() + QString("/inc-%1").arg(i);
        xtrabackup_prepare_process.start(
                    xtrabackup_binary,
                    QStringList()
                    << "--prepare" << "--apply-log-only"
                    << QString("--target-dir=").append(restore_dir)
                    << QString("--incremental-dir=").append(inc_path));
        //        qDebug() << xtrabackup_prepare_process.program() << xtrabackup_prepare_process.arguments();
        xtrabackup_prepare_process.waitForFinished();
        qDebug() << xtrabackup_prepare_process.readAll()
                 << xtrabackup_prepare_process.readAllStandardError();
    }
}
