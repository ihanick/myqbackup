#!/bin/sh

mysql -uroot -e 'truncate table xxx' test
/home/ihanick/src/build-myqbackup-Desktop_Qt_5_1_0_GCC_32bit-Debug/myqbackup --inc=5 /home/ihanick/myqbackup

mysql -uroot -e 'insert into xxx values(6)' test
/home/ihanick/src/build-myqbackup-Desktop_Qt_5_1_0_GCC_32bit-Debug/myqbackup --inc=5 /home/ihanick/myqbackup

mysql -uroot -e 'insert into xxx values(6)' test
/home/ihanick/src/build-myqbackup-Desktop_Qt_5_1_0_GCC_32bit-Debug/myqbackup --inc=5 /home/ihanick/myqbackup

mysql -uroot -e 'insert into xxx values(6)' test
/home/ihanick/src/build-myqbackup-Desktop_Qt_5_1_0_GCC_32bit-Debug/myqbackup --inc=5 /home/ihanick/myqbackup

mysql -uroot -e 'insert into xxx values(6)' test
/home/ihanick/src/build-myqbackup-Desktop_Qt_5_1_0_GCC_32bit-Debug/myqbackup --inc=5 /home/ihanick/myqbackup

mysql -uroot -e 'insert into xxx values(6)' test
/home/ihanick/src/build-myqbackup-Desktop_Qt_5_1_0_GCC_32bit-Debug/myqbackup --inc=5 /home/ihanick/myqbackup
#first rotate
mysql -uroot -e 'insert into xxx values(6)' test
/home/ihanick/src/build-myqbackup-Desktop_Qt_5_1_0_GCC_32bit-Debug/myqbackup --inc=5 /home/ihanick/myqbackup
#second rotate
mysql -uroot -e 'insert into xxx values(6)' test
/home/ihanick/src/build-myqbackup-Desktop_Qt_5_1_0_GCC_32bit-Debug/myqbackup --inc=5 /home/ihanick/myqbackup

# many rotates
mysql -uroot -e 'insert into xxx values(6)' test
/home/ihanick/src/build-myqbackup-Desktop_Qt_5_1_0_GCC_32bit-Debug/myqbackup --inc=5 /home/ihanick/myqbackup

mysql -uroot -e 'insert into xxx values(6)' test
/home/ihanick/src/build-myqbackup-Desktop_Qt_5_1_0_GCC_32bit-Debug/myqbackup --inc=5 /home/ihanick/myqbackup

mysql -uroot -e 'insert into xxx values(6)' test
/home/ihanick/src/build-myqbackup-Desktop_Qt_5_1_0_GCC_32bit-Debug/myqbackup --inc=5 /home/ihanick/myqbackup

mysql -uroot -e 'insert into xxx values(6)' test
/home/ihanick/src/build-myqbackup-Desktop_Qt_5_1_0_GCC_32bit-Debug/myqbackup --inc=5 /home/ihanick/myqbackup

mysql -uroot -e 'insert into xxx values(6)' test
/home/ihanick/src/build-myqbackup-Desktop_Qt_5_1_0_GCC_32bit-Debug/myqbackup --inc=5 /home/ihanick/myqbackup

mysql -uroot -e 'insert into xxx values(6)' test
/home/ihanick/src/build-myqbackup-Desktop_Qt_5_1_0_GCC_32bit-Debug/myqbackup --inc=5 /home/ihanick/myqbackup
#first rotate
mysql -uroot -e 'insert into xxx values(6)' test
/home/ihanick/src/build-myqbackup-Desktop_Qt_5_1_0_GCC_32bit-Debug/myqbackup --inc=5 /home/ihanick/myqbackup
#second rotate
mysql -uroot -e 'insert into xxx values(6)' test
/home/ihanick/src/build-myqbackup-Desktop_Qt_5_1_0_GCC_32bit-Debug/myqbackup --inc=5 /home/ihanick/myqbackup
