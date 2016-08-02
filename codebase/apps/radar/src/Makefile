#
# Recursive make - makes the subdirectory code
#

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGETS = $(GENERAL_TARGETS) $(INSTALL_TARGETS)

SUB_DIRS = \
	AltCpCompute \
	CalCompute \
	ChillSdbServer \
	DualDopplerLobes \
	DowSendFreq \
	RadarCal \
	RadarConst \
	RapMap2Chill \
	SimCpCompute \
	SunCal \
	SunCpCompute \
	PowerInfo2Spdb \
	SweepInfo2Spdb \
	TsFmq2Tcp \
	TsFile2Fmq \
	TsSmartSave \
	TsTcp2File \
	TsTcp2Fmq \
	VertCompute

include $(RAP_MAKE_INC_DIR)/rap_make_recursive_dir_targets

