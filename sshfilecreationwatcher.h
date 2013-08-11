#ifndef SSHFILECREATIONWATCHER_H
#define SSHFILECREATIONWATCHER_H

#include <QThread>
#include <QProcess>

class SshFileCreationWatcher : public QThread
{
    Q_OBJECT
public:
    explicit SshFileCreationWatcher(QString ssh_details, QObject *parent = 0);
    void watch_for_file(QString newfilename);

signals:
    void file_created();
    void file_removed();

public slots:
    void ssh_check_finished(int exitcode, QProcess::ExitStatus status);
    void remove_file();

protected:
    void run();


private:
    QString filename;
    QString ssh_remote_access;
    volatile bool abort;
    int timeout;
    QProcess process;
};

#endif // SSHFILECREATIONWATCHER_H
