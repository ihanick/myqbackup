myqbackup
=========

Simple backup for mysql within incremental backups support for InnoDB and MyISAM.


Usage
=======

Create a backup:
====

```
myqbackup --inc=5 /home/backup/mysql
```

The command could be repeated many times.
myqbackup will maintain one full backup copy + 5 incremental copies since last command run

Restore backup:
====

```
myqbackup --restore-to=/var/lib/mysql --inc=5 /home/backup/mysql
```

The command will restore full backup to /home/ihanick/myqbackup and applies incremental backups from inc-1 to inc-5, including inc-5.

How to setup periodic backups:
====

Create a cron job file, e.g. inside: /etc/cron.d/myqbackup

With contents:
```
\# One full backup + 23 hourly incremental backups
0 * * * * root /usr/bin/myqbackup --inc=23 /home/backup/mysql
```


How to compile:
====
You need Qt4 or Qt5 core + development packages to be installed

```bash
mkdir build-myqbackup
cd build-myqbackup
qmake ../myqbackup
make
sudo cp myqbackup /usr/local/bin
```

Q: But I want to use it on server. It's not possible to install huge packages like Qt!
A: You can setup the same version of Linux in a virtual machine,
complile myqbackup binary and create standalone package in /opt/myqbackup .
All required libraries could be in /opt/myqbackup/lib .
LD_LIBRARY_PATH=/opt/myqbackup/lib /opt/myqbackup/bin/myqbackup will start backup program.
