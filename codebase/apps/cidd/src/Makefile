#
# Recursive make - makes the subdirectory code
#
#
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGETS =  $(GENERAL_TARGETS) $(INSTALL_TARGETS)

SUB_DIRS = \
	AsciiSelect \
	CheckXDisplay \
	CIDD \
	CiddClick2GenPt \
	cidd_client \
	CiddParams2JazzXml \
	create_colorscale \
	CronusPointSelect \
	cscale_edit \
	DrawFmq2Bdry \
	DrawFmq2MaskMdv \
	DrawFmq2Polygon \
	DrawFmqPolygonStats \
	EdrSelect \
	EventMan \
	get_url \
	GridPointSelect \
	initDrawQueue \
	inspectRawFmq \
	jvizMaps2ciddMaps \
	MetarSelect \
	metar_strip \
	notify_cidd \
	pos_report \
	RemoteUI2Fmq \
	Shape2Map \
	ShapeAscii2Map \
	shapefile2Mapfile \
	ShapeGridPt2Mdv \
	SoundingChart \
	SoundingText \
	StationStripChart \
	view_colormap

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_recursive_dir_targets

# ---------------------------------------------------------
# Special cases

install_include:
install_lib:
install_profile_lib:
install_debug_lib:

install_doc:
	$(CP) ../doc/*.doc $(LROSE_DOC_DIR)

install: install_bin

clean_lib:
clean_all: clean clean_lib clean_bin







