# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1990 - 2016                                         
# ** University Corporation for Atmospheric Research (UCAR)                 
# ** National Center for Atmospheric Research (NCAR)                        
# ** Boulder, Colorado, USA                                                 
# ** BSD licence applies - redistribution and use in source and binary      
# ** forms, with or without modification, are permitted provided that       
# ** the following conditions are met:                                      
# ** 1) If the software is modified to produce derivative works,            
# ** such modified software should be clearly marked, so as not             
# ** to confuse it with the version available from UCAR.                    
# ** 2) Redistributions of source code must retain the above copyright      
# ** notice, this list of conditions and the following disclaimer.          
# ** 3) Redistributions in binary form must reproduce the above copyright   
# ** notice, this list of conditions and the following disclaimer in the    
# ** documentation and/or other materials provided with the distribution.   
# ** 4) Neither the name of UCAR nor the names of its contributors,         
# ** if any, may be used to endorse or promote products derived from        
# ** this software without specific prior written permission.               
# ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
# ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
# ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#---------------------------------------------------------------------
#
# Makefile for script files
#
#---------------------------------------------------------------------

#	Default INSTALL_DIR, should be overridden on command line

INSTALL_DIR=$(LROSE_INSTALL_DIR)/bin

#	Script files to be installed

SCRIPTS = \
	fdist

#	Targets

all: install

install: create_subdir
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

install_bin: install

# unused targets - needed to prevent make failures

clean:
debug:
profile:
opt:
opt4:
lint:
depend:
pure:
clean_bin:
clean_tdrp:
clean_depend:
clean_all:
static:
staticopt:


