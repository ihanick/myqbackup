#ifndef XBSTREAMER_H
#define XBSTREAMER_H

#include <QObject>
#include <QProcess>
#include <QDebug>

class XBStreamer : public QObject
{
    Q_OBJECT
public:
    explicit XBStreamer(const QString& destination_file,
                        const QString& base_inc_path,
                        const QString& inc_path,
                        const QString& ssh_access_param, const QString& xtrabackup_binary_, QObject *parent = 0);
    void start();
    
signals:
    void terminate();
    void dataCopied();
    void finished_ok();

public slots:
    void xbbackup_finished();


    private:
    QString ssh_access_param;
    QString destination_file;
    QString xtrabackup_binary;
    QString backup_inc_path;
    QString backup_base_path;
    QProcess process;
};

#endif // XBSTREAMER_H
