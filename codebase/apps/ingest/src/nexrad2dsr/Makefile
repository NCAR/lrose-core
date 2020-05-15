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
###########################################################################
#
# Terri Betancourt RAP, NCAR, Boulder, CO, 80307, USA
#
# November 2001
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = nexrad2dsr

LOC_INCLUDES =
LOC_CFLAGS = 
LOC_LDFLAGS = -L/usr/local/lib
LOC_LIBS = \
	-ldsdata -lFmq -ldsserver -ldidss \
	-lrapformats -leuclid -ltoolsa -ldataport \
	-ltdrp -lbz2 -lz -lpthread \
	-lm

HDRS = \
	$(PARAMS_HH) \
	DataMgr.hh \
	Driver.hh \
	DsrOutput.hh \
	FieldTable.hh \
	FieldType.hh \
	KmData.hh \
	KmTilt.hh \
	NexradInput.hh \
	NexradLdm.hh \
	NexradTape.hh \
	NexradTCP.hh \
	Reformat.hh \
	ScanTable.hh \
        ScanType.hh \
	Status.hh

CPPC_SRCS = \
	$(PARAMS_CC) \
	DataMgr.cc \
	Driver.cc \
	DsrOutput.cc \
	FieldTable.cc \
	FieldType.cc \
	KmData.cc \
	KmTilt.cc \
	NexradLdm.cc \
	NexradTape.cc \
	NexradTCP.cc \
	Reformat.cc \
	ScanTable.cc \
        ScanType.cc \
	dieGracefully.cc \
	main.cc

#
# tdrp macros
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_macros

#
# standard C++ targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_c++_targets

#
# tdrp targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_c++_targets

#
# local targets
#

# DO NOT DELETE THIS LINE -- make depend depends on it.


