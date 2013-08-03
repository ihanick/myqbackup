#include "filecreationwatcherthread.h"
#include <QFile>
#include <QDebug>

FileCreationWatcherThread::FileCreationWatcherThread(QObject *parent) :
    QThread(parent), abort(false)
{
}

FileCreationWatcherThread::~FileCreationWatcherThread() {
    mutex.lock();
    abort = true;
    mutex.unlock();
    while(isRunning());
}

void FileCreationWatcherThread::run() {
    while(!QFile::exists(filename)) {
        msleep(20);
        QMutexLocker locker(&mutex);
        if(abort) {
            qDebug() << "Watch aborted";
            return;
        }
    }
    emit file_created();
    qDebug() << "File created!" << filename;
}

void FileCreationWatcherThread::watch_for_file(QString newfilename) {
    if(abort) {
        return; // during shutdown
    }
    if(isRunning()) {
        return;
    }

    filename = newfilename;
    qDebug() << "Will watch for:"<< filename;
    start(LowPriority);
}
