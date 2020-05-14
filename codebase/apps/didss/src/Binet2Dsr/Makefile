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
# Makefile for Binet2Dsr program
#
# Nancy Rehak, RAP, NCAR, Boulder, CO, 80307, USA
#
# Sept 1995
#
###########################################################################

include $(LROSE_MAKE_INC_DIR)/lrose_make_macros

MODULE_TYPE = progcpp

TARGET_FILE = Binet2Dsr

LOC_INCLUDES =
LOC_CFLAGS =
LOC_LDFLAGS =
LOC_LIBS = \
	-lFmq -ldsserver -lrapformats -ldidss \
	-ltoolsa -lpthread -ldataport -ltdrp -lrapmath \
	-lbz2 -lz -lm

HDRS = \
	Params.hh \
	Archiver.hh \
	Args.hh \
	BeamWriter.hh \
	Binet2Dsr.hh \
	BinetBeamMsg.hh \
	BinetMsg.hh \
	BinetMsgFactory.hh \
	BinetRadarMsg.hh \
	BinetReader.hh \
	DropEOVStrategy.hh \
	DualPol1Products.hh \
	DualPol3Products.hh \
	DualPolFull1Products.hh \
	DualPrtProducts.hh \
	EndEOVStrategy.hh \
	EOVStrategy.hh \
	FileArchiver.hh \
	FmqArchiver.hh \
	MultFileArchiver.hh \
	NewSimpleProducts.hh\
	PolyProducts.hh \
	Products.hh \
	RapFileReader.hh \
	Reader.hh \
	ScanStrategy.hh \
	SimpleProducts.hh \
	SingleFileArchiver.hh \
	StartEOVStrategy.hh \
	UdpReader.hh

CPPC_SRCS = \
	Params.cc \
	Args.cc \
	ScanStrategy.cc \
	BeamWriter.cc \
	Binet2Dsr.cc \
	BinetBeamMsg.cc \
	BinetMsg.cc \
	BinetMsgFactory.cc \
	BinetRadarMsg.cc \
	BinetReader.cc \
	DropEOVStrategy.cc \
	DualPol1Products.cc \
	DualPol3Products.cc \
	DualPolFull1Products.cc \
	DualPrtProducts.cc \
	EndEOVStrategy.cc \
	EOVStrategy.cc \
	FileArchiver.cc \
	FmqArchiver.cc \
	Main.cc \
	MultFileArchiver.cc \
	NewSimpleProducts.cc \
	PolyProducts.cc \
	Products.cc \
	RapFileReader.cc \
	Reader.cc \
	SimpleProducts.cc \
	SingleFileArchiver.cc \
	StartEOVStrategy.cc \
	UdpReader.cc

#
# tdrp macros
#

include $(LROSE_MAKE_INC_DIR)/lrose_make_tdrp_macros

#
# standard C++ targets
#

include $(LROSE_MAKE_INC_DIR)/lrose_make_c++_targets

#
# tdrp targets
#

include $(LROSE_MAKE_INC_DIR)/lrose_make_tdrp_c++_targets

#
# local targets
#

# DO NOT DELETE THIS LINE -- make depend depends on it.

