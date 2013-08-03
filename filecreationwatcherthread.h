#ifndef FILECREATIONWATCHERTHREAD_H
#define FILECREATIONWATCHERTHREAD_H

#include <QThread>
#include <QString>
#include <QMutex>

class FileCreationWatcherThread : public QThread
{
    Q_OBJECT
public:
    explicit FileCreationWatcherThread(QObject *parent = 0);
    ~FileCreationWatcherThread();
    void watch_for_file(QString newfilename);
    
signals:
    void file_created();
    
public slots:
    
protected:
    void run();

private:
    QString filename;
    QMutex mutex;
    volatile bool abort;
};

#endif // FILECREATIONWATCHERTHREAD_H
