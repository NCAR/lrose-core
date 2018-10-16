
#
# Recursive make - makes the subdirectory code
#

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGETS = $(GENERAL_TARGETS)

MODULE_NAME = radar

LIBNAME = lib$(MODULE_NAME).a

SUB_DIRS = \
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

include $(RAP_MAKE_INC_DIR)/rap_make_recursive_dir_targets

include $(RAP_MAKE_INC_DIR)/rap_make_inc_targets

include $(RAP_MAKE_INC_DIR)/rap_make_lib_targets
