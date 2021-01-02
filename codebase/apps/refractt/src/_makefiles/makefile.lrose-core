#
# Recursive make - makes the subdirectory code
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGETS = $(GENERAL_TARGETS) $(INSTALL_TARGETS)

SUB_DIRS = \
	CalcMoisture \
	CalcNexrad \
	Chill2netCDFSweep \
	CleanupRvp8Data \
	DsrAddSnr \
	MdvAddSnr \
	PhaseDiff \
	RefractCalib \
	RefractCompute \
	RefractParams2Tdrp \
	RefrMerge

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_recursive_dir_targets
