###########################################################################
#
# Makefile for xview library
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# July 2000
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

LOC_INCLUDES = -I../include
LOC_CFLAGS = -DX11R6 $(SYS_X_INCLUDES)

TARGET_FILE = ../libxview.a

HDRS =

SRCS = \
	ndet_auto.c \
	ndet_death.c \
	ndet_die.c \
	ndet_dodis.c \
	ndet_event.c \
	ndet_fcntl.c \
	ndet_fd.c \
	ndet_g_fd.c \
	ndet_g_in.c \
	ndet_g_out.c \
	ndet_g_pri.c \
	ndet_g_sig.c \
	ndet_loop.c \
	ndet_nodis.c \
	ndet_read.c \
	ndet_s_fd.c \
	ndet_s_in.c \
	ndet_s_out.c \
	ndet_s_pri.c \
	ndet_s_sig.c \
	ndet_sig.c \
	ndet_value.c \
	ndet_veto.c \
	ndet_wait.c \
	ndetg_wait.c \
	ndetgdeath.c \
	ndetgetfnc.c \
	ndetgevent.c \
	ndetgexcep.c \
	ndetgitimr.c \
	ndetitimer.c \
	ndetpdeath.c \
	ndetpevent.c \
	ndetremove.c \
	ndets_wait.c \
	ndetsdeath.c \
	ndetselect.c \
	ndetsevent.c \
	ndetsexcep.c \
	ndetsingle.c \
	ndetsitimr.c \
	ndis_d_pri.c \
	ndisd_wait.c \
	ndisdispch.c \
	ndisdsched.c \
	ndisgsched.c \
	ndisssched.c \
	nint_copy.c \
	nint_get.c \
	nint_i_fd.c \
	nint_i_in.c \
	nint_i_out.c \
	nint_i_sig.c \
	nint_inter.c \
	nint_n_fd.c \
	nint_n_in.c \
	nint_n_out.c \
	nint_n_sig.c \
	nint_next.c \
	nint_r_fd.c \
	nint_r_in.c \
	nint_r_out.c \
	nint_r_sig.c \
	nint_set.c \
	nint_stack.c \
	ninti_wait.c \
	nintideath.c \
	nintievent.c \
	nintiexcpt.c \
	nintiitimr.c \
	nintn_wait.c \
	nintndeath.c \
	nintnevent.c \
	nintnexcpt.c \
	nintnitimr.c \
	nintr_wait.c \
	nintrdeath.c \
	nintremove.c \
	nintrevent.c \
	nintrexcpt.c \
	nintritimr.c \
	notifydata.c \
	ntfy_cond.c \
	ntfy_ctbl.c \
	ntfy_debug.c \
	ntfy_dump.c \
	ntfy_fd_op.c \
	ntfy_list.c \
	ntfy_node.c \
	ntfyclient.c \
	ntfyperror.c \
	ntfyprotec.c \
	sys_fcntl.c \
	sys_read.c \
	sys_select.c \
	linux_select.c

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
