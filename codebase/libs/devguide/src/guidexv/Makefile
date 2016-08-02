###########################################################################
#
# Makefile for xutil library
#
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INCLUDES = -I../include $(SYS_X_INCLUDES) $(SYS_XVIEW_INCLUDES)
LOC_CFLAGS = $(SYS_X_INCLUDES)

TARGET_FILE = ../libdevguide.a

#
# file lists
#

HDRS =  \
	../include/devguide/gcc.h \
	../include/devguide/gcc_ui.h \
	../include/devguide/gcm.h \
	../include/devguide/gdd.h \
	../include/devguide/gfm.h \
	../include/devguide/gfm_ui.h \
	../include/devguide/group.h \
	../include/devguide/group_impl.h \
	../include/devguide/gutil.h

SRCS = \
	gcc.c \
	gcc_ui.c \
	gcm.c \
	gdd.c \
	gfm.c \
	gfm_load_dir.c \
	gfm_ui.c \
	group.c \
	group_data.c \
	gutil.c

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
