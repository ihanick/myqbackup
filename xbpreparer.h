#ifndef XBPREPARER_H
#define XBPREPARER_H

#include <QObject>
#include <QProcess>
#include <QDebug>
#include <QDir>

class XBPreparer : public QObject
{
    Q_OBJECT
public:
    explicit XBPreparer(QString target, int prepare_type,
                        int incremental_idx_, QString restore_to,
                        QString xtrabackup_binary, QObject *parent = 0);
    
signals:
    void backup_ready();
    void preRotate();
    
public slots:
    void prepare();
    void restoreBackup();

    
private:
    QString xtrabackup_binary;
    QString target_dir;
    int type;
    int incremental_idx;
    QString restore_dir;
};

#endif // XBPREPARER_H
