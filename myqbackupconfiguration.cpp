#include "myqbackupconfiguration.h"
#include "cliparser.h"
#include "clioption.h"
#include <QSettings>
#include <QDebug>

MyQBackupConfiguration::MyQBackupConfiguration(QObject *parent) :
    QObject(parent), is_restore_mode(false)
{
    CliParser        cliParser;
    QSettings settings("myqbackup", "myqbackup");

    settings.beginGroup("BackupParameters");
    max_incrementals = settings.value("incrementals", 6).toInt();
    backup_dest = settings.value("backup_path",".").toString();
    xtrabackup_prefix = settings.value("xtrabackup_prefix",
                                               "/home/ihanick/src/percona-xtrabackup-2.1.3").toString();
    compression = settings.value("full_backup_compression", false).toBool();

    settings.endGroup();


    settings.beginGroup("DatabaseAccess");
    server = settings.value("server","localhost").toString();
    user = settings.value("user","root").toString();
    password = settings.value("password","").toString();
    database = settings.value("database","mysql").toString();
    settings.endGroup();



    // Help option.
    cliParser.addOption( CliOption(
        QStringList()
            << QLatin1String( "?" )
            << QLatin1String( "help" ) ) );

    // --compress
    cliParser.addOption( CliOption(
        QStringList()
            << QLatin1String( "c" )
            << QLatin1String( "compress" )
            << QLatin1String( "compression" ) ) );

    // --inc
    cliParser.addOption( CliOption(
        QStringList()
            << QLatin1String( "i" )
            << QLatin1String( "inc" )
            << QLatin1String( "incremental" ), true ) );

    // --restore
    cliParser.addOption( CliOption(
        QStringList()
            << QLatin1String( "r" )
            << QLatin1String( "restore" )
            << QLatin1String( "restore-to" ), true ) );

    // --xbprefix
    cliParser.addOption( CliOption(
        QStringList()
            << QLatin1String( "x" )
            << QLatin1String( "xbprefix" )
            << QLatin1String( "xtrabackup-prefix" ), true ) );

    // --host
    cliParser.addOption( CliOption(
        QStringList()
            << QLatin1String( "h" )
            << QLatin1String( "host" )
            << QLatin1String( "server" ), true ) );

    // --user
    cliParser.addOption( CliOption(
        QStringList()
            << QLatin1String( "u" )
            << QLatin1String( "user" )
            << QLatin1String( "mysql-user" ), true ) );

    // --password
    cliParser.addOption( CliOption(
        QStringList()
            << QLatin1String( "p" )
            << QLatin1String( "pass" )
            << QLatin1String( "password" ), true ) );

    // --database
    cliParser.addOption( CliOption(
        QStringList()
            << QLatin1String( "db" )
            << QLatin1String( "database" ), true ) );

    cliParser.parse();

    const QStringList unknownOptionList
        = cliParser.getUnknownOptionNames();

    if ( ! unknownOptionList.isEmpty() )
    {
        // Complain about first option that wasn't recognised
        qDebug() << QLatin1String( "Unknown option: " ) << unknownOptionList.first();
    }

    // Arguments left which are not parameters to options
    const QStringList leftoverArgs = cliParser.getLeftoverArgs();
    if(leftoverArgs.size()) {
        backup_dest = leftoverArgs.last();
    }
    qDebug() << "backup destination" << backup_dest;


    const QString restore_arg = cliParser.getArgument( QLatin1String( "r" ) );
    if(!restore_arg.isNull()) {
        restore_dir = restore_arg;
        qDebug() << "Restore directory"<< restore_dir;
        is_restore_mode = true;
    }

    const QString xbprefix_arg = cliParser.getArgument( QLatin1String( "x" ) );
    if(!xbprefix_arg.isNull()) {
        xtrabackup_prefix = xbprefix_arg;
        qDebug() << "Xtrabackup installation prefix"<< xtrabackup_prefix;
    }

    const QString inc_arg = cliParser.getArgument( QLatin1String( "i" ) );
    if(!inc_arg.isNull()) {
        max_incrementals =inc_arg.toInt();
    }
    qDebug() << (restore_arg.isNull()?"Max incrementals":"Restore index") << max_incrementals;

    if ( ! cliParser.getArgumentList(QLatin1String("compression")).isEmpty() ) {
        compression = true;
        qDebug() << "Compression enabled";
    }

    const QString server_arg = cliParser.getArgument( QLatin1String( "host" ) );
    if(!server_arg.isNull()) {
        server = server_arg;
        qDebug() << "connection parameters: host"<< server;
    }

    const QString user_arg = cliParser.getArgument( QLatin1String( "user" ) );
    if(!user_arg.isNull()) {
        user = user_arg;
        qDebug() << "connection parameters: user"<< user;
    }

    const QString password_arg = cliParser.getArgument( QLatin1String( "password" ) );
    if(!password_arg.isNull()) {
        password = password_arg;
        qDebug() << "connection parameters: password"<< password;
    }

    const QString database_arg = cliParser.getArgument( QLatin1String( "db" ) );
    if(!database_arg.isNull()) {
        database = database_arg;
        qDebug() << "connection parameters: database"<< database;
    }

    // List of all the recognised option names (doesn't include option values)
    const QStringList optionNames = cliParser.getOptionNames();
    qDebug() << "option names"<< optionNames;

    settings.beginGroup("BackupParameters");
    if(restore_dir.length() == 0) {
        settings.setValue("incrementals", max_incrementals);
    }
    settings.setValue("backup_path", backup_dest);
    settings.setValue("xtrabackup_prefix", xtrabackup_prefix);
    settings.setValue("full_backup_compression", compression);
    settings.endGroup();

    settings.beginGroup("DatabaseAccess");
    settings.setValue("server", server);
    settings.setValue("user", user);
    settings.setValue("password", password);
    settings.setValue("database", database);
    settings.endGroup();

    xtrabackup_path = (xtrabackup_prefix + "/bin/");


}
