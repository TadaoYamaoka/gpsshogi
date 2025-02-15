#!/bin/sh -
# policy: comment out/in 
echo $0

#server='192.168.0.1' # for gpw
#server='wdoor.c.u-tokyo.ac.jp' # for test
server="gserver.computer-shogi.org"

#table_size=4800000 # for 8GB?
table_size=8000000 # for 16GB?

export TABLE_SIZE=$table_size
export TABLE_RECORD_LIMIT=200  

export SHOGIUSER=TeamGPS
export SHOGIPASS=os4QRTvls
#export SHOGIPASS=gps-1500-0

#export CHALLENGE=t
export SERVER=$server;
export LIMIT=1600
export GPSOPTS=""
export GPSNAME="/usr/local/bin/ruby -I /home/ktanaka/smp/gpsshogi-nodist/slugshogi /home/ktanaka/smp/gpsshogi-nodist/slugshogi/gpsshogi_par3.rb"
exec ./network.pl
