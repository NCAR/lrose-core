###########################################################################
#
# Makefile for jasper code in Grib2 library
#
# Mike Dixon, EOL, NCAR, Boulder, Co, USA, 80307
#
# April 2022
#
###########################################################################
#
#
# JasPer is required for decoding jpeg2000 compression.
#
# jasper code copied from https://github.com/jasper-software/jasper
#
############################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = ../libgrib2.a

LOC_INCLUDES = -I../include $(SYS_X_INCLUDES)
LOC_CPPC_CFLAGS = -DSIZE_MAX=18446744073709551615UL

MODULE_TYPE=library

#
# file lists
#

C_SRCS = \
	bmp_cod.c \
	bmp_dec.c \
	bmp_enc.c \
	jas_cm.c \
	jas_debug.c \
	jas_getopt.c \
	jas_icc.c \
	jas_iccdata.c \
	jas_image.c \
	jas_init.c \
	jas_malloc.c \
	jas_seq.c \
	jas_stream.c \
	jas_string.c \
	jas_tmr.c \
	jas_tvp.c \
	jas_version.c \
	jp2_cod.c \
	jp2_dec.c \
	jp2_enc.c \
	jpc_bs.c \
	jpc_cod.c \
	jpc_cs.c \
	jpc_dec.c \
	jpc_enc.c \
	jpc_math.c \
	jpc_mct.c \
	jpc_mqcod.c \
	jpc_mqdec.c \
	jpc_mqenc.c \
	jpc_qmfb.c \
	jpc_t1cod.c \
	jpc_t1dec.c \
	jpc_t1enc.c \
	jpc_t2cod.c \
	jpc_t2dec.c \
	jpc_t2enc.c \
	jpc_tagtree.c \
	jpc_tsfb.c \
	jpc_util.c \
	jpg_dec.c \
	jpg_enc.c \
	jpg_val.c \
	mif_cod.c \
	pgx_cod.c \
	pgx_dec.c \
	pgx_enc.c \
	pnm_cod.c \
	pnm_dec.c \
	pnm_enc.c \
	ras_cod.c \
	ras_dec.c \
	ras_enc.c

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.

