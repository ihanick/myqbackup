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
                        QString xtrabackup_binary, QString xbstream_bin,
                        QString qcompress_bin,
                        bool compression, QString remote, QObject *parent = 0);
    
signals:
    void backup_ready();
    void preRotate();
    
public slots:
    void prepare();
    void restoreBackup();

    
private:
    QString xtrabackup_binary;
    QString xbstream_binary;
    QString qpress_path;
    QString target_dir;
    int type;
    int incremental_idx;
    QString restore_dir;
    bool use_compression;
    QString ssh_remote;

protected:
    void extract_xtrabackup_stream(QString source, QString destination);
};

#endif // XBPREPARER_H
