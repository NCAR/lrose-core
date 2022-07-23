# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2012 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2012/9/18 21:3:20 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 

#
# Recursive make - makes the subdirectory code
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGETS = $(GENERAL_TARGETS)

MODULE_NAME = rapplot

LIBNAME = lib$(MODULE_NAME).a

SUB_DIRS = \
	gplot \
	xutils

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_recursive_dir_targets

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_targets

# -------------------------------------------------------
# This is a special case due to multiple include subdirectories
#

install_include: mk_inc_dir FORCE
	$(CP) include/rapplot/*.h  $(LROSE_INSTALL_DIR)/include/rapplot
	$(CP) include/rapplot/*.hh $(LROSE_INSTALL_DIR)/include/rapplot
	$(CP) include/prologues/*.ps $(LROSE_INSTALL_DIR)/include/prologues

mk_inc_dir:
	if $(TEST) -d $(LROSE_INSTALL_DIR)/include/rapplot; then :; else \
		$(MKDIR) $(LROSE_INSTALL_DIR)/include/rapplot; fi
	if $(TEST) -d $(LROSE_INSTALL_DIR)/include/prologues; then :; else \
		$(MKDIR) $(LROSE_INSTALL_DIR)/include/prologues; fi






