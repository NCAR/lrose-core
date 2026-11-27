###########################################################################
#
# Makefile for tdwr2moments program
#
# Based on makefile by Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# August 1997
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INCLUDES = 
LOC_CFLAGS =
LOC_LDFLAGS =
LOC_LIBS = -ltoolsa -ldataport -ldidss -lm

MODULE_TYPE=program

TARGET_FILE = tdwr2moments

HDRS =

SRCS = \
	tdwr2moments.c \
	tdwr.c \
	send_shm.c \
	recv_cmd.c \
	process_tape_pkts.c \
	rtapelib.c \
	recv_shm.c \
	get_params.c \
	swap_tdwr.c \
	write_stream.c \
	fmq_output.c

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_targets

#
# local targets
#

# DO NOT DELETE THIS LINE -- make depend depends on it.
