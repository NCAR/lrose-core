# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:42 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#---------------------------------------------------------------------
#
# Makefile for script files
#
#---------------------------------------------------------------------

#	Default INSTALL_DIR, should be overridden on command line

INSTALL_DIR=$(LROSE_INSTALL_DIR)/bin

#	Default LIB install dir for Perl library

INSTALL_LIB_DIR=$(LROSE_INSTALL_DIR)/lib/perl5

#	Script files to be installed

SCRIPTS = \
        addLocationToMetarCsv.pl \
        parseMetar.pl \
        parsePirep.pl

LIBS = \
        StationUtils.text.pm \
        Monitor.pm \
        NavAids.pm \
        MetarUtils.pm

#	Targets

#all: install

install: create_subdir create_sub_libdir
	if test $(INSTALL_LIB_DIR) != . ;\
	then \
		cp $(LIBS) $(INSTALL_LIB_DIR) ;\
	fi	

	if test $(INSTALL_DIR) != . ;\
	then \
	  cp $(SCRIPTS) $(INSTALL_DIR) ;\
	fi

create_subdir:
	if test -d $(INSTALL_DIR) ;\
	then \
		cd . ;\
	else \
		mkdir $(INSTALL_DIR) ;\
	fi 

create_sub_libdir:
	if test -d $(INSTALL_LIB_DIR) ;\
	then \
		cd . ;\
	else \
		mkdir -p $(INSTALL_LIB_DIR) ;\
	fi

install_bin: install

# unused targets - needed to prevent make failures

clean:
debug:
dbgopt:
profile:
opt:
opt4:
lint:
depend:
install_include:
clean_bin:
clean_lib:
clean_tdrp:
clean_depend:
clean_all:
staticopt:
dbgopt:


