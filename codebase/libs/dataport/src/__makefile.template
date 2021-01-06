# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:34 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#
# Recursive make - makes the subdirectory code
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGETS = $(GENERAL_TARGETS)

MODULE_NAME = dataport

LIBNAME = lib$(MODULE_NAME).a

SUB_DIRS = \
	bigend \
	smallend \
	swap

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_recursive_dir_targets

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_inc_targets

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_targets

#---------------------------------------------------------------

debugport: FORCE
	$(MAKE) clean
	$(MAKE) SWAP_DEBUG=-DFORCE_SWAP
	$(CP) $(LIBNAME) $(LROSE_INSTALL_DIR)/lib/libdebugport.a
	if $(TEST) -f $(RANLIB);\
	then\
		$(RANLIB) $(LROSE_INSTALL_DIR)/lib/libdebugport.a;\
	fi






