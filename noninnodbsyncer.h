#ifndef NONINNODBSYNCER_H
#define NONINNODBSYNCER_H

#include <QObject>
#include <QDebug>
#include <QProcess>
#include "mysqlconnection.h"

class NonInnoDBSyncer : public QObject
{
    Q_OBJECT
public:
    explicit NonInnoDBSyncer(QString target, QString inc_base, QString inc_last,
                             QString restore_dest,
                             int incremental_idx,
                             MySQLConnection *ndb,
                             QString mysql_data_dir = "",
                             QObject *parent = 0);
    
signals:
    void readyToUnlock();
    void backupComplete();
    void restoreComplete();
    
public slots:
    void startBackup();
    void rotateBackup();
    void restoreBackup();


    void unlockTables() {
        db->unlock_all_tables();
        qDebug() << "Unlock tables";
        emit backupComplete();
    }
    
private:
    MySQLConnection *db;
    QString target_dir;
    QString base_dir;
    QString last_dir;
    QString restore_dir;
    int restore_inc_idx;
    QString datadir;

protected:
    void applyIncremental(int idx);
};

#endif // NONINNODBSYNCER_H
