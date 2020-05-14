
#
# Recursive make - makes the subdirectory code
#

include $(LROSE_MAKE_INC_DIR)/lrose_make_macros

TARGETS = $(GENERAL_TARGETS)

MODULE_NAME = radar

LIBNAME = lib$(MODULE_NAME).a

SUB_DIRS = \
	apar \
	calib \
	complex \
	convstrat \
	dorade \
	filter \
	geom \
	hsrl \
	iwrf \
	kdp \
	moments \
	pid \
	precip \
	qc \
	radx \
	spol \
	ts

include $(LROSE_MAKE_INC_DIR)/lrose_make_recursive_dir_targets

include $(LROSE_MAKE_INC_DIR)/lrose_make_inc_targets

include $(LROSE_MAKE_INC_DIR)/lrose_make_lib_targets
