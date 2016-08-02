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
# Makefile for ridds2mom program
#
# Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
#
# Nov 1994
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = ridds2mom

LOC_INCLUDES = $(SYS_CPPC_INCLUDES)
LOC_CFLAGS =
LOC_LDFLAGS =
LOC_LIBS = \
	-ltdrp -lFmq -ldsserver -lrapformats \
	-ldidss -ltoolsa -ldataport -lbz2 \
	-lz -lm

HDRS = ridds2mom.h

CPPC_SRCS = \
	DsReformQueue.cc \
	close_streams.cc \
	fmq_archive.cc \
	fmq_message.cc \
	fmq_output.cc \
	get_beam_time.cc \
	get_vcp.cc \
	get_target_elev.cc \
	init_streams.cc \
	list_tape_files.cc \
	parse_args.cc \
	print.cc \
	process_data_stream.cc \
	read_stream.cc \
	reformat2ds.cc \
	reformat2ll.cc \
	ridds2mom.cc \
	set_derived_params.cc \
	set_time_to_wallclock.cc \
	store_data.cc \
	shmem_output.cc \
	tape_input.cc \
	tidy_and_exit.cc \
	udp_input.cc \
	udp_output.cc \
	write_stream.cc

C_SRCS = \
	$(TDRP_C)

#
# tdrp support
#

include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_macros

#
# standard targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_c++_targets

#
# local targets
#

depend: $(TDRP_C) depend_generic

include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_targets

# DO NOT DELETE THIS LINE -- make depend depends on it.

