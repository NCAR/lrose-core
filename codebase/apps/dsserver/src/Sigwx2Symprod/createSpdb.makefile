# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2009 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2009/9/11 8:39:34 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 



all:

# createSpdb
	g++ -D_BSD_TYPES -DF_UNDERSCORE2 -g -c  -I/home/steves/tech/taiwan/sigwx2symprod/rapinstall/include -I/home/steves/tech/taiwan/sigwx2symprod/rapbase/include   createSpdb.cc

	g++ -g -o createSpdb createSpdb.o -L/home/steves/tech/taiwan/sigwx2symprod/rapinstall/lib -L/home/steves/tech/taiwan/sigwx2symprod/rapbase/lib   -lSpdb -ldsserver -ldidss -lrapformats -ltoolsa -ldataport -ltdrp -lpthread -lm



db: all
	rm -rf tempdir
	./createSpdb -o tempdir


queryList:	
	SpdbQuery -url tempdir -start '2008 01 01 00 00 00' -end '2009 01 01 00 00 00' -mode timelist


queryInterval:	
	SpdbQuery -url tempdir -start '2008 01 01 00 00 00' -end '2009 01 01 00 00 00' -mode interval


queryServerTimes:
	SpdbQuery -url spdbp:Sigwx2Symprod://myriad:5450:/home/steves/tech/taiwan/sigwx2symprod/cvs/apps/dsserver/src/Sigwx2Symprod/tempdir -start '2008 01 01 00 00 00' -end '2009 01 01 00 00 00' -mode times


queryServerInterval:
	SpdbQuery -url spdbp:Sigwx2Symprod://myriad:5450:/home/steves/tech/taiwan/sigwx2symprod/cvs/apps/dsserver/src/Sigwx2Symprod/tempdir -start '2008 01 01 00 00 00' -end '2009 01 01 00 00 00' -mode interval





server: all
	./Sigwx2Symprod -debug



client: all
	SpdbQuery -url spdbp:Sigwx2Symprod://localhost:5450:/home/steves/tech/taiwan/sigwx2symprod/cvs/apps/dsserver/src/Sigwx2Symprod/tempdir -mode latest


test:
