# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:39 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for clump utilities
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# April 1995
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INCLUDES = -I../include
LOC_CFLAGS =

TARGET_FILE = ../libeuclid.a

HDRS =

SRCS = \
	adjust_intervals.c \
	alloc_clumps.c \
	alloc_rowh.c \
	clump_grid.c \
	clump_intervals.c \
	clump_volume.c \
	erode_clump.c \
	euclid_dist.c \
	extend_clump.c \
	extend_intervals.c \
	find_intervals.c \
	find_intervals_float.c \
	find_overlap.c \
	free_interval.c \
	get_intervals.c \
	get_intervals_float.c \
	intervals_to_array.c \
	link_intervals.c \
	oclump_intervals.c \
	ofree_clump_info.c \
	oprint_intervals.c \
	overlap.c \
	overlap_plane.c \
	overlap_rows.c \
	overlap_volume.c \
	print_intervals.c \
	reset_arrays.c \
	reset_clump_id.c \
	row_hdr.c \
	scan_interval.c \
	set_intervals.c \
	set_rp.c \
	small_clumps.c \
	sort_intervals.c \
	split_clump.c \
	stack_interval.c \
	transform_intervals.c \
	translate_array.c \
	translate_clump.c \
	translate_intervals.c \
	union_intervals.c \
	zero_clump.c

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
