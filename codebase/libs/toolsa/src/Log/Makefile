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
# Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
# January 1998
#
# $Id: Makefile,v 1.13 2019/09/20 20:42:57 dave Exp $
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

LOC_INCLUDES = -I../include
LOC_CFLAGS =
LOC_INC_DIR = ../include 
MY_CPPCHECKFLAGS = #-AddStyleFAA -exclude-rules="4.14,4.09,4.12,4.16"

TARGET_FILE = ../libtoolsa.a
MODULE_TYPE=library

#
# file lists
#

HDRS = \
	../include/toolsa/Benchmark.hh \
	../include/toolsa/Log.hh \
	../include/toolsa/LogFile.hh \
	../include/toolsa/LogMsg.hh \
	../include/toolsa/LogMsgInit.hh \
	../include/toolsa/LogMsgStreamInit.hh \
	../include/toolsa/MsgLog.hh \
	../include/toolsa/LogStream.hh \
	../include/toolsa/LogStreamInit.hh \
	../include/toolsa/TaThreadLog.hh  \
	../include/toolsa/TaTriggerLog.hh 

CPPC_SRCS = \
	Benchmark.cc \
	Log.cc \
	LogFile.cc \
	MsgLog.cc \
	LogMsg.cc \
	LogStream.cc \
	LogMsgInit.cc \
	LogMsgStreamInit.cc \
	LogStreamInit.cc \
	TaThreadLog.cc \
	TaTriggerLog.cc 

#
# general targets
#
include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
