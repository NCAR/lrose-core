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
# Makefile for TerrainHtServer program
#
# Mike Dixon, RAP, NCAR
# POBox 3000, Boulder, CO, 80307, USA
#
# Oct 2014
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = TerrainHtServer

XMLRPC_LIBS = \

# NOTE: RH packages to install for xmlrpc
#   xmlrpc-c
#   xmlrpc-c-c++
#   xmlrpc-c-client
#   xmlrpc-c-client++
#   xmlrpc-c-devel
#   xmlrpc-client
#   xmlrpc-server
# yum install -y xmlrpc-c xmlrpc-c-c++ xmlrpc-c-client xmlrpc-c-client++ xmlrpc-c-devel xmlrpc-client xmlrpc-server

LOC_INCLUDES = $(NETCDF4_INCS)

LOC_LIBS = -ltoolsa -ltdrp -ldataport \
	-lxmlrpc \
	-lxmlrpc++ \
	-lxmlrpc_abyss \
	-lxmlrpc_client \
	-lxmlrpc_client++ \
	-lxmlrpc_cpp \
	-lxmlrpc_server \
	-lxmlrpc_server++ \
	-lxmlrpc_server_abyss \
	-lxmlrpc_server_abyss++ \
	-lxmlrpc_server_pstream++ \
	-lxmlrpc_util \
	-lNcxx \
	$(NETCDF4_LIBS) -lbz2 -lz -lpthread

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

LOC_CFLAGS =

HDRS = \
	$(PARAMS_HH) \
	Args.hh \
	ServerThread.hh \
	SquareDegree.hh \
	TerrainHtServer.hh

CPPC_SRCS = \
	$(PARAMS_CC) \
	Args.cc \
	Main.cc \
	ServerThread.cc \
	SquareDegree.cc \
	TerrainHtServer.cc

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

all: TestClientTerrainHt TerrainHtServer WorldTerrain

TestClientTerrainHt: TestClientTerrainHt.cc
	g++ $(DBUG_OPT_FLAGS) -o TestClientTerrainHt \
	TestClientTerrainHt.cc $(XMLRPC_LIBS)

WorldTerrain: WorldTerrain.cc
	g++ $(DBUG_OPT_FLAGS) -I$(LROSE_INSTALL_DIR)/include -o WorldTerrain \
	WorldTerrain.cc $(XMLRPC_LIBS) $(MDV_LIBS)

cleaner: clean_all
	/bin/rm TestClientTerrainHt TerrainHtServer WorldTerrain

# DO NOT DELETE THIS LINE -- make depend depends on it.

