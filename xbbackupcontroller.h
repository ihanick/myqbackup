#ifndef XBBACKUPCONTROLLER_H
#define XBBACKUPCONTROLLER_H

#include <QObject>
#include <QProcess>
#include <QDebug>
#include <QFile>

class XBBackupController : public QObject
{
    Q_OBJECT
public:
    explicit XBBackupController(QString target, QString inc_base_path, QString inc_path, QObject *parent = 0);
    void start(QString xtrabackup_binary);
    
signals:
    void terminate();
    void xbbackup_finished();
    void dataCopied();
    void finished_ok();

public slots:
    void xtrabackup_backup_finished(int exitcode, QProcess::ExitStatus status) {
        if(exitcode) {
            qDebug() << "Error during backup" << exitcode;
            emit terminate();
        }
        qDebug() << "Xtrabackup backup procedure finished with" << exitcode << status;
        emit xbbackup_finished();
    }

    void xtrabackup_process_started() {
        QFile::remove( target_dir + "/xtrabackup_suspended_1");
        qDebug() << "Backup started!";
    }

    void xtrabackup_copied_all_innodb_data() {
        qDebug() << "Xtrabackup copied all innodb files";
        emit dataCopied();
    }

    void xtrabackup_process_finished() {
        qDebug() << "Stopping log copy process";
        QFile::remove( target_dir + "/xtrabackup_suspended_2");
        if (!process.waitForFinished()) {
            qDebug() << "Error during xtrabackup stop";
            emit terminate();
        }
        emit finished_ok();
    }


private:
    QProcess process;
    QString target_dir;
    QString backup_inc_base;
    QString backup_inc_path;

};

#endif // XBBACKUPCONTROLLER_H
