###########################################################################
#
# Makefile for RSL module
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# Dec 2000
#
# Version 1.30 installed June 2003 - Niles.
#
# Version 1.4 November 2009 - Niles.
#
###########################################################################

include $(LROSE_MAKE_INC_DIR)/lrose_make_macros

LOC_INCLUDES = -I../include -I.
LOC_CFLAGS =

TARGET_FILE = ../libtrmm_rsl.a

#
# file lists
#

HDRS =


SRCS = \
	africa.c \
	africa_to_radar.c \
	anyformat_to_radar.c \
	cappi.c \
	carpi.c \
	cube.c \
	dorade.c \
	dorade_print.c \
	dorade_to_radar.c \
	edge_to_radar.c \
	endian.c \
	farea.c \
	fix_headers.c \
	fraction.c \
	get_win.c \
	gts.c \
	gzip.c \
	hdf_to_radar.c \
	histogram.c \
	image_gen.c \
	interp.c \
	lassen_to_radar.c \
	mcgill.c \
	mcgill_to_radar.c \
	nsig.c \
	nsig2_to_radar.c \
	prune.c \
	radar.c \
	radar_to_hdf_1.c \
	radar_to_hdf_2.c \
	radar_to_uf.c \
	radtec.c \
	radtec_to_radar.c \
	rainbow.c \
	rainbow_to_radar.c \
	range.c \
	rapic-lex.c \
	rapic.c \
	rapic_routines.c \
	rapic_to_radar.c \
	ray_indexes.c \
	read_write.c \
	reverse.c \
	sort_rays.c \
	toga.c \
	toga_to_radar.c \
	toolkit_memory_mgt.c \
	uf_to_radar.c \
	volume.c \
	wsr88d.c \
	wsr88d_get_site.c \
	wsr88d_m31.c \
	wsr88d_to_radar.c

#
# general targets
#

include $(LROSE_MAKE_INC_DIR)/lrose_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
