# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2012 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2012/9/7 19:48:40 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#
# Recursive make - makes the subdirectory code
#

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGETS = $(GENERAL_TARGETS)

MODULE_NAME = contour

LIBNAME = lib$(MODULE_NAME).a

SUB_DIRS = \
	contour \
	contour_alg \
	contour_smooth

include $(RAP_MAKE_INC_DIR)/rap_make_recursive_dir_targets

include $(RAP_MAKE_INC_DIR)/rap_make_inc_targets

include $(RAP_MAKE_INC_DIR)/rap_make_lib_targets
