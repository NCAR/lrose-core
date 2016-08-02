#
# Recursive make - makes the subdirectory code
#

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGETS = $(GENERAL_TARGETS) $(INSTALL_TARGETS)

SUB_DIRS = \
	ApRemoval \
	BrightBand \
	ClutterRemove \
	Googler \
	LeiseFilter \
	LogFilter \
	LookAtMap \
	LookAtLatLon \
	MissingBeamFilter \
	NoiseFilter \
	apply_partition \
	mit_storm_filter \
	png2gif \
	replicateEnv \
	stratiform_filter \
	setLdmUpstreamHost \
	tops_filter

include $(RAP_MAKE_INC_DIR)/rap_make_recursive_dir_targets
