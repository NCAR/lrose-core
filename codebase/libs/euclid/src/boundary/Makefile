# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:40 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for boundary utilities
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
	adj_star.c \
	alloc_nodes.c \
	bbox.c \
	bdry_graph.c \
	boundary_intervals.c \
	fill_boundary.c \
	find_extreme_pts.c \
	free_clump_info.c \
	gen_bdry.c \
	init_ray.c \
	make_star.c \
	obdry_graph.c \
	oboundary_intervals.c \
	ogen_bdry.c \
	otraverse_bdry_graph.c \
	translate_bdry.c \
	traverse_bdry_graph.c

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
