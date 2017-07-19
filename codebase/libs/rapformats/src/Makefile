# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:49 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#
# Recursive make - makes the subdirectory code
#

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGETS = $(GENERAL_TARGETS)

MODULE_NAME = rapformats

LIBNAME = lib$(MODULE_NAME).a

SUB_DIRS = \
	ac_data \
	ac_posn \
	ac_route \
	acars \
	ascii_shapeio \
	bdry \
	ccm \
	cedric \
        checktimes \
	DsRadar \
	Edr \
	flt_path \
	fos \
	GenPoly \
	GenPt \
	gint \
	hist_fore \
	iidabin \
	kavouras \
	km \
	ltg \
	Maps \
	MultBuf \
	mcidas \
	mdvt \
	metar \
	mitre \
	nids \
	nws_wwa \
	Pirep \
	radar \
	r_data \
	SigAirMet \
	Sndg \
	sounding \
	station_data \
	swap \
	taf \
	ThreshMap \
	titan \
	trec_gauge \
	var_elev \
	VerGrid \
	Usgs  \
	Windshear \
	WinsRadar \
	zr

include $(RAP_MAKE_INC_DIR)/rap_make_recursive_dir_targets

include $(RAP_MAKE_INC_DIR)/rap_make_inc_targets

include $(RAP_MAKE_INC_DIR)/rap_make_lib_targets
