###########################################################################
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# Jan 2008
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

LOC_INCLUDES = -I../include -I../include/radar $(NETCDF4_INCS)
LOC_CFLAGS =

all: spolTS

spolTS:
	ncargf90 fftpack_5.f90 spolTS.f -o SpolTS -I/usr/include -I/usr/local/include -L/usr/local/lib -lnetcdf -lnetcdff

clean:
	rm -f SpolTS *.o

depend: depend_generic


# DO NOT DELETE THIS LINE -- make depend depends on it.
