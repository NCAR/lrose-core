#
# Recursive make - makes the subdirectory code
#

include $(LROSE_MAKE_INC_DIR)/lrose_make_macros

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

include $(LROSE_MAKE_INC_DIR)/lrose_make_recursive_dir_targets
