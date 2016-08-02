#
# Recursive make - makes the subdirectory code
#
#

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGETS =  $(GENERAL_TARGETS) $(INSTALL_TARGETS)

SUB_DIRS = \
         AsciiSelect \
	 CIDD \
         Cidd_snap \
	 DrawFmq2Bdry \
	 DrawFmq2Polygon \
         EdrSelect \
         EventMan \
         fix_map \
         GridPointSelect \
	 MapCircles \
	 MapGrid \
         MetarSelect \
         notify_cidd \
         pos_report \
	 OverlayCreate \
	 RemoteUI2Fmq \
         skewt_util \
         SoundingChart \
         SoundingChart_modified \
         SoundingText \
         StationStripChart \
         toneDownColorScale \
	 cscale_edit \
         get_url \
	 mdv_server \
	 view_colormap

include $(RAP_MAKE_INC_DIR)/rap_make_recursive_dir_targets

# ---------------------------------------------------------
# Special cases

install_include:
install_lib:
install_profile_lib:
install_debug_lib:

install_doc:
	$(CP) ../doc/*.doc $(RAP_DOC_DIR)

install: install_bin

clean_lib:
clean_all: clean clean_lib clean_bin clean_insure








