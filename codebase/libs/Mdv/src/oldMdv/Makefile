# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2003 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Program(RAP) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2003/5/6 11:52:42 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for old mdv code
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# Sept 2007
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INCLUDES = -I../include
LOC_CFLAGS =

TARGET_FILE = ../libMdv.a

#
# file lists
#

#DEBUG_CFLAGS = -DDEBUG

HDRS =

SRCS = \
	mdv_chunk.c \
	mdv_client.c \
	mdv_composite.c \
	mdv_convert.c \
	mdv_dataset.c \
	mdv_field_handle.c \
	mdv_grid.c \
	mdv_handle.c \
	mdv_print.c \
	mdv_print_f.c \
	mdv_private_f.c \
	mdv_proj.c \
	mdv_radar.c \
	mdv_read.c \
	mdv_read_f.c \
	mdv_user.c \
	mdv_utils.c \
	mdv_write.c \
	mdv_write_f.c \
	print_grid.c

CPPC_SRCS = \
	MdvChunk.cc \
	MdvField.cc \
	MdvFieldData.cc \
	MdvFile.cc \
	MdvGrid.cc \
	MdvInputPath.cc \
	MdvMasterHdr.cc \
	MdvPublic.cc \
	MdvRead.cc \
	MdvReadChunk.cc \
	MdvReadField.cc \
	MdvReadRadar.cc \
	MdvVlevel.cc \
	MdvVsection.cc \
	MdvVsectionField.cc \
	TimestampChunk.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
