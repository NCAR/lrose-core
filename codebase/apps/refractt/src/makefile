#
# Recursive make - makes the subdirectory code
#

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGETS = $(GENERAL_TARGETS) $(INSTALL_TARGETS)

SUB_DIRS = \
	CalcMoisture \
	CalcNexrad \
	Chill2netCDFSweep \
	CleanupRvp8Data \
	DsrAddSnr \
	MdvAddSnr \
	PhaseDiff \
	Refract \
	RefractCalib \
	RefractParams2Tdrp \
	RefrMerge

include $(RAP_MAKE_INC_DIR)/rap_make_recursive_dir_targets
