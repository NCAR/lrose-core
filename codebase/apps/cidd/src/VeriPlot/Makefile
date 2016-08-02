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
# (C)UCAR/RAL 2008  All Rights Reserved.
# F. Hage
#
##########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = RMSE_plot

LOC_INCLUDES = $(SYS_XVIEW_INCLUDES) $(SYS_X_INCLUDES) $(NETCDF4_INCS)

LOC_LIBS = -lMdv -lRadx -lSpdb -ldsserver \
	-ldidss -lrapformats -lrapplot -ltoolsa \
	-ldataport -lphysics -lrapmath -ltdrp \
	-lImlib2 -ldevguide -lxview -lolgx \
	-lX11 $(NETCDF4_LIBS) -lbz2 -lz \
	-lpthread -lm

LOC_LDFLAGS = $(SYS_XVIEW_LDFLAGS) $(SYS_X_LDFLAGS) $(NETCDF4_LDFLAGS)

LOC_CFLAGS =

HDRS = \
	$(PARAMS_HH) \
	Args.hh \
	pairAverage.hh \
	pairMgr.hh \
	RMSE_plot.hh \
	RMSE_plot_GUI_ui.h \
	TimePlot.hh

CPPC_SRCS = \
	$(PARAMS_CC) \
	Args.cc \
	Main.cc \
	pairAverage.cc \
	pairMgr.cc \
	RMSE_plot.cc \
	RMSE_plot_GUI_ui.cc \
	RMSE_plot_GUI_callbacks.cc \
	RMSE_plot_data.cc \
	RMSE_plot_image.cc \
	RMSE_plot_plot.cc \
	RMSE_plot_timer.cc \
	TimePlot.cc 

#
# tdrp macros
#

include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_macros

#
# standard C++ targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_c++_targets

#
# tdrp targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_c++_targets

#
# local targets
#

# DO NOT DELETE THIS LINE -- make depend depends on it.

