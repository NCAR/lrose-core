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
# Makefile for Hiq2Dsr program
#
# Nancy Rehak, RAP, NCAR, Boulder, CO, 80307, USA
#
# November 2006
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

MODULE_TYPE = progcpp

TARGET_FILE = Hiq2Dsr

LOC_INCLUDES =
LOC_CFLAGS =
LOC_LDFLAGS =
LOC_LIBS = \
	-lFmq -lrapformats -ldsserver -ldidss \
	-leuclid -lrapmath -ltoolsa -ldataport \
	-ltdrp -lphysics -lpthread -lbz2 \
	-lz -lm

HDRS = \
	Params.hh \
	ArcBeamMsg.hh \
	ArcBeamWriter.hh \
	ArcHiqReader.hh \
	ArcMsgFactory.hh \
	Archiver.hh \
	Args.hh \
	BeamWriter.hh \
	DropEOVStrategy.hh \
	DualPol1Products.hh \
	DualPol3Products.hh \
	DualPolFull1Products.hh \
	DualPrtProducts.hh \
	EndEOVStrategy.hh \
	EolBeamMsg.hh \
	EolBeamWriter.hh \
	EolHiqReader.hh \
	EolMsgFactory.hh \
	EolRadarMsg.hh \
	EOVStrategy.hh \
	FileArchiver.hh \
	FmqArchiver.hh \
	Hiq2Dsr.hh \
	HiqMsg.hh \
	HiqReader.hh \
	MedianFilter.hh \
	MsgFactory.hh \
	MultFileArchiver.hh \
	NewSimpleProducts.hh\
	PolyProducts.hh \
	Products.hh \
	RapFileReader.hh \
	Reader.hh \
	ScanStrategy.hh \
	SimpleBeamWriter.hh \
	SimpleProducts.hh \
	SingleFileArchiver.hh \
	StartEOVStrategy.hh \
	TiltNumBeamWriter.hh \
	UdpReader.hh

CPPC_SRCS = \
	Params.cc \
	ArcBeamMsg.cc \
	ArcBeamWriter.cc \
	ArcHiqReader.cc \
	ArcMsgFactory.cc \
	Args.cc \
	BeamWriter.cc \
	DropEOVStrategy.cc \
	DualPol1Products.cc \
	DualPol3Products.cc \
	DualPolFull1Products.cc \
	DualPrtProducts.cc \
	EndEOVStrategy.cc \
	EolBeamMsg.cc \
	EolBeamWriter.cc \
	EolHiqReader.cc \
	EolMsgFactory.cc \
	EolRadarMsg.cc \
	EOVStrategy.cc \
	FileArchiver.cc \
	FmqArchiver.cc \
	Hiq2Dsr.cc \
	HiqMsg.cc \
	HiqReader.cc \
	Main.cc \
	MedianFilter.cc \
	MsgFactory.cc \
	MultFileArchiver.cc \
	NewSimpleProducts.cc \
	PolyProducts.cc \
	Products.cc \
	RapFileReader.cc \
	Reader.cc \
	ScanStrategy.cc \
	SimpleBeamWriter.cc \
	SimpleProducts.cc \
	SingleFileArchiver.cc \
	StartEOVStrategy.cc \
	TiltNumBeamWriter.cc \
	UdpReader.cc

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


