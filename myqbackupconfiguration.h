#ifndef MYQBACKUPCONFIGURATION_H
#define MYQBACKUPCONFIGURATION_H

#include <QObject>

class MyQBackupConfiguration : public QObject
{
    Q_OBJECT
public:
    explicit MyQBackupConfiguration(QObject *parent = 0);

signals:

public slots:


public:
    int max_incrementals;
    QString backup_dest;
    QString xtrabackup_prefix;
    bool compression;
    QString server;
    QString user;
    QString password;
    QString database;
    QString xtrabackup_path;
    QString restore_dir;

    bool is_restore_mode;
};

#endif // MYQBACKUPCONFIGURATION_H
