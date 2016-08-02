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
# Makefile for geom utilities
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
	coord_system.c \
	convex_hull.c \
	create_box.c \
	create_trapezoid.c \
	equidistant_pts.c \
	fill_polygon.c \
	get_perpendicular.c \
	half_plane.c \
	hypot.c \
	inside_poly.c \
	is_rectangle.c \
	lat_long.c \
	line_intersect.c \
	line_point.c \
	line_through_ellipse.c \
	linear_comb.c \
	perp_projection.c \
	point_in_ellipse.c \
	point_in_polygon.c \
	point_on_segment.c \
	polygon_area.c \
	polygon_centroid.c \
	polygon_intersect.c \
	random_bdry.c \
	segment_intersect.c \
	sincos.c \
	unit_vector.c \
	vect_add.c

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
