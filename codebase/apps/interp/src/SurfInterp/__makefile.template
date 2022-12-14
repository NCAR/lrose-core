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
# Makefile for SurfInterp program
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = SurfInterp

LOC_INCLUDES = $(NETCDF4_INCS)

LOC_LIBS = \
	-ldsdata -lMdv -lSpdb -lFmq \
	-lrapformats -ldsserver -ldidss -leuclid \
	-lrapmath -ltoolsa -ldataport -ltdrp \
	-lRadx -lNcxx -lphysics -lpthread \
	$(NETCDF4_LIBS) -lbz2 -lz -lm

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

LOC_CFLAGS =


HDRS = \
	$(PARAMS_HH) \
	AltInterpField.hh \
	Args.hh \
	BarnesInterpolater.hh \
	ConvDerivedField.hh \
	DataMgr.hh \
	DerivedField.hh \
	DewptDeprInterpField.hh \
	DewptInterpField.hh \
	FltCatDerivedField.hh \
	GenPtInterpField.hh \
	Interpolater.hh \
	InterpField.hh \
	LiftedIndexInterpField.hh \
	LiqAccumInterpField.hh \
	NearestInterpolater.hh \
	Output.hh \
	PotTempInterpField.hh \
	PrecipRateInterpField.hh \
	PressInterpField.hh \
	RelHumInterpField.hh \
	RunwayVisRangeInterpField.hh \
	SealevelRelCeilingInterpField.hh \
	StnInterpField.hh \
	SurfInterp.hh\
	TempInterpField.hh \
	Terrain.hh \
	TerrainDerivedField.hh \
	TerrainRelCeilDerivedField.hh \
	UwindInterpField.hh \
	VisInterpField.hh \
	VwindInterpField.hh \
	WindGustInterpField.hh

CPPC_SRCS = \
	$(PARAMS_CC) \
	AltInterpField.cc \
	Args.cc \
	BarnesInterpolater.cc \
	ConvDerivedField.cc \
	DataMgr.cc \
	DerivedField.cc \
	DewptDeprInterpField.cc \
	DewptInterpField.cc \
	FltCatDerivedField.cc \
	GenPtInterpField.cc \
	Interpolater.cc \
	InterpField.cc \
	LiftedIndexInterpField.cc \
	LiqAccumInterpField.cc \
	Main.cc \
	NearestInterpolater.cc \
	Output.cc \
	PotTempInterpField.cc \
	PrecipRateInterpField.cc \
	PressInterpField.cc \
	RelHumInterpField.cc \
	RunwayVisRangeInterpField.cc \
	SealevelRelCeilingInterpField.cc \
	StnInterpField.cc \
	SurfInterp.cc \
	TempInterpField.cc \
	Terrain.cc \
	TerrainDerivedField.cc \
	TerrainRelCeilDerivedField.cc \
	UwindInterpField.cc \
	VisInterpField.cc \
	VwindInterpField.cc \
	WindGustInterpField.cc

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


