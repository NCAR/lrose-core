# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2012 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2012/9/6 21:37:26 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#
# Recursive make - makes the subdirectory code
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGETS = $(GENERAL_TARGETS)

MODULE_NAME = Refract

LIBNAME = lib$(MODULE_NAME).a


SUB_DIRS = \
	Data \
	RefFile \
	RefractInput \
	RefParms

xxx = 	refract

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_recursive_dir_targets

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_inc_targets

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_targets





