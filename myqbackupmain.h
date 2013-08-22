#ifndef MYQBACKUPMAIN_H
#define MYQBACKUPMAIN_H

#include <QObject>
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

#include "xbstreamer.h"
#include "sshfilecreationwatcher.h"



class MyQBackupMain : public QObject
{
    Q_OBJECT
public:
    explicit MyQBackupMain(QObject *parent = 0);
    void start();
    
signals:
    void terminate();
    
public slots:
    void quit() {
        emit terminate();
    }
    

private:
    MyQBackupConfiguration *conf;
    QString datadir;
    QString version;
    QString xtrabackup_binary;
    QString backup_inc_base;
    QString backup_inc_path;
    QString ni_inc_base;
    QString ni_inc_last;
    QString ni_inc_path;
    int incremental_idx;
};

#endif // MYQBACKUPMAIN_H
