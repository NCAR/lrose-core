# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2012 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2012/9/18 21:43:40 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#
# Recursive make - makes the subdirectory code
#

include $(LROSE_MAKE_INC_DIR)/lrose_make_macros

TARGETS = $(GENERAL_TARGETS)

MODULE_NAME = symprod

LIBNAME = lib$(MODULE_NAME).a

SUB_DIRS = \
	prod_queue \
	spdb \
	symprod \
	ProdQueue \
	LdmMetars2File

include $(LROSE_MAKE_INC_DIR)/lrose_make_recursive_dir_targets

include $(LROSE_MAKE_INC_DIR)/lrose_make_inc_targets

include $(LROSE_MAKE_INC_DIR)/lrose_make_lib_targets
