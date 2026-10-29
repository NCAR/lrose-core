#
# Recursive make - makes the subdirectory code
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGETS = $(GENERAL_TARGETS) $(INSTALL_TARGETS)

SUB_DIRS = \
	bdry2spdb \
	load_spdb \
	metar2spdb \
	print_spdb \
	print_spdb_index \
	spdb2bdry 

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_recursive_dir_targets
