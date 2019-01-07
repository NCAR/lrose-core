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
#
# Makefile for use with RAL's standard make system
# Particularly tuned for the macros established when HOST_OS == NNT
#
include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = mrms_to_CFncdf

LOC_INCLUDES = $(NETCDF4_INCS)

LOC_LIBS = -ldsdata -lMdv -lRadx -lNcxx -leuclid \
	-lSpdb -ldsserver -ldidss -lrapformats \
	-ltoolsa -ldataport -lRadx -ltdrp \
	$(NETCDF4_LIBS) -lbz2 -lz

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

LOC_CPPC_CFLAGS =

HDRS = \
	func_prototype.h \
        HeaderAttribute.h \
        ProductInfo.h

CPPC_SRCS = \
        mrms_binary_reader.cc \
        write_netCDF_lib.cc \
        write_CF_netCDF_2d.cc \
        write_CF_netCDF_3d.cc \
        write_CF_netCDF_2d_FAA.cc \
        write_CF_netCDF_3d_FAA.cc \
        ProductInfo.cc \
        setupMRMS_ProductRefData.cc \
        HeaderAttribute.cc \
        mrms_to_CFncdf_main.cc


#
# standard targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_c++_targets


