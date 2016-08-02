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
# Makefile
#
# Niles Oien, RAP, NCAR, Boulder, CO, 80307, USA
#
#
#
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = hiResNidsRadialImg2Mdv

LOC_INCLUDES = $(NETCDF_INCS) $(NETCDF4_INCS)

LOC_LIBS = -lSpdb -lMdv -lRadx -leuclid \
	-ldsserver -lrapformats -ldidss -ltoolsa \
	-ldataport -ltdrp $(NETCDF4_LIBS) -lbz2 \
	-lz -lpthread

LOC_LDFLAGS = $(NETCDF_LDFLAGS) $(NETCDF4_LDFLAGS)

LOC_CPPC_CFLAGS =


HDRS = \
	$(PARAMS_HH) \
	GraphicProductMsg.hh \
	hiResNidsRadialImg2Mdv.hh \
	hiResRadialFile.hh \
	lightweightDirSearch.hh \
	Product.hh \
	VilPrduct.hh \
	EchoTopsProduct.hh \
	StormPrecipProduct.hh \
	SpecDiffPhaseProduct.hh \
        DiffReflProduct.hh \
	HydroClassProduct.hh \
	OneHourPrecipTotal.hh \
	DigitalPrecipRate.hh \
	DigitalAccumArray.hh \
	DigitalTotalAccum.hh \
        CorrCoef.hh \
	BaseReflProduct.hh \
	BaseVelProduct.hh \
	StormTotalPrecip.hh \
	MeltingLayer.hh \
	Swap.hh \
	orpg_product.h
    
 


CPPC_SRCS = \
	$(PARAMS_CC) \
	Main.cc \
	GraphicProductMsg.cc \
	hiResNidsRadialImg2Mdv.cc \
	hiResRadialFile.cc \
	lightweightDirSearch.cc \
	Product.cc \
	VilProduct.cc \
	EchoTopsProduct.cc \
	StormPrecipProduct.cc \
	SpecDiffPhaseProduct.cc \
        DiffReflProduct.cc \
	HydroClassProduct.cc \
	OneHourPrecipTotal.cc \
	DigitalPrecipRate.cc \
	DigitalAccumArray.cc \
        CorrCoef.cc \
	BaseReflProduct.cc \
	BaseVelProduct.cc \
	StormTotalPrecip.cc \
	MeltingLayer.cc \
	Swap.cc \
	orpg_xdr.c \
	DigitalTotalAccum.cc

#
# tdrp macros
#

include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_macros

#
# C++ targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_c++_targets

#
# tdrp targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_c++_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.

