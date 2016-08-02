#
# Recursive make - makes the subdirectory code
#
#
# Obsolete
#	ccm_server \
#	gint_server \
#	skewt_util \
#	cedric_server \
#	cidd \
# 
#

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGETS =  $(GENERAL_TARGETS) $(INSTALL_TARGETS)

SUB_DIRS = \
	AsciiSelect \
	CheckXDisplay \
	CIDD \
	ciddArchiveDriver \
	CiddClick2GenPt \
	cidd_client \
	CiddParams2JazzXml \
	Cidd_snap \
	cindex_test \
	create_colorscale \
	cronusPointSelect \
	CronusPointSelect \
	cscale_edit \
	DrawFmq2Bdry \
	DrawFmq2MaskMdv \
	DrawFmq2Polygon \
	DrawFmqPolygonStats \
	EdrSelect \
	EventMan \
	fix_map \
	get_url \
	GridPointSelect \
	initDrawQueue \
	inspectRawFmq \
	jvizMaps2ciddMaps \
	MapCircles \
	MapGrid \
	MetarSelect \
	metar_strip \
	notify_cidd \
	OverlayCreate \
	pos_report \
	RemoteUI2Fmq \
	Shape2Map \
	ShapeAscii2Map \
	shapefile2Mapfile \
	ShapeGridPt2Mdv \
	skewt_util \
	SoundingChart \
	SoundingText \
	StationStripChart \
	toneDownColorscale \
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







