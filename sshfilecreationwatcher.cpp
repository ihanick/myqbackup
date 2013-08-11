#include "sshfilecreationwatcher.h"
#include <QDebug>

SshFileCreationWatcher::SshFileCreationWatcher(QString ssh_details, QObject *parent) :
    QThread(parent),
    ssh_remote_access(ssh_details),
    timeout(5000)
{
    connect(&process, SIGNAL(finished(int,QProcess::ExitStatus)),
                     this, SLOT(ssh_check_finished(int,QProcess::ExitStatus)));
}

void SshFileCreationWatcher::run() {
}

void SshFileCreationWatcher::ssh_check_finished(int exitcode, QProcess::ExitStatus status) {
    process.waitForFinished();
    qDebug() << process.readAll() << process.readAllStandardError();

    if(exitcode==0 || status != QProcess::NormalExit) {
        emit file_created();
    }
}

void SshFileCreationWatcher::watch_for_file(QString newfilename) {
    filename = newfilename;
    process.start(
                "ssh",
                QStringList()
                << ssh_remote_access
                << "bash"
                << "-c"
                << ( QString("\"while [ ! -f '%1' ] ; do sleep 1 ; done\"").arg(filename) )
                );
    qDebug() << process.program() << process.arguments();
    if(!process.waitForStarted()) {
        emit terminate();
    }
}

void SshFileCreationWatcher::remove_file() {
    disconnect(&process, SIGNAL(finished(int,QProcess::ExitStatus)),
               this, SLOT(ssh_check_finished(int,QProcess::ExitStatus)));

    process.start( "ssh", QStringList() << ssh_remote_access << "rm" << filename );
    qDebug() << process.program() << process.arguments();
    if(!process.waitForStarted()) {
        emit terminate();
    }
    process.waitForFinished();
    qDebug() << process.readAll() << process.readAllStandardError();
    emit file_removed();
}
