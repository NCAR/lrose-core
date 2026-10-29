# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2012 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2012/9/18 17:43:42 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile
#
# Rachel Ames, RAP, NCAR, Boulder, Co, USA, 80307
#
# Jan 1996
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = ../libmdv.a

LOC_INCLUDES = -I../include
LOC_CFLAGS = 

HDRS = \
	mdv_private.h \
	mdv_private_f.h \
	../include/mdv/mdv_client.h \
	../include/mdv/mdv_composite.h \
	../include/mdv/mdv_convert.h \
	../include/mdv/mdv_dataset.h \
	../include/mdv/mdv_field_handle.h \
	../include/mdv/mdv_file.h \
	../include/mdv/mdv_grid.h \
	../include/mdv/mdv_macros.h \
	../include/mdv/mdv_msgs.h \
	../include/mdv/mdv_print.h \
	../include/mdv/mdv_read.h \
	../include/mdv/mdv_user.h  \
	../include/mdv/mdv_utils.h \
	../include/mdv/mdv_write.h

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

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
