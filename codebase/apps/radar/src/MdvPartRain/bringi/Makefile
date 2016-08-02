###########################################################################
#
# Makefile for Dsr2Mdv program
#
# Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
#
# May 1998
#
###########################################################################

all: kdp_f kdp_c kdp2_f

kdp_f: fort_driver.f Sband_kdp.f Makefile
	gfortran -fdefault-real-8 -g fort_driver.f Sband_kdp.f -o kdp_f

kdp2_f: fort2_driver.f Sband_kdp_Feb25_2008.f Makefile
	gfortran -fdefault-real-8 -g fort2_driver.f Sband_kdp_Feb25_2008.f -o kdp2_f

kdp_c: c_driver.cc Sband_kdp.cc Makefile
	g++ -Wall -g c_driver.cc Sband_kdp.cc -o kdp_c

kdp_driver: kdp_driver.cc KdpSband.cc Makefile
	g++ -Wall -g kdp_driver.cc KdpSband.cc -o kdp_driver

clean:
	/bin/rm -f kdp_c kdp_f kdp2_f *.o

clean_all: clean
	/bin/rm -f *~

# DO NOT DELETE THIS LINE -- make depend depends on it.
