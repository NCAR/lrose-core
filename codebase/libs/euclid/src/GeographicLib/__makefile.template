# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:39 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for GeographicLib
#
# Mike Dixon, EOL, NCAR, Boulder, Co, USA, 80307
#
# jan 2021
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

LOC_INCLUDES = -I../include
LOC_CFLAGS =

TARGET_FILE = ../libeuclid.a

HDRS =

CPP_SRCS = \
	Accumulator.cpp \
	AlbersEqualArea.cpp \
	AzimuthalEquidistant.cpp \
	CassiniSoldner.cpp \
	CircularEngine.cpp \
	DMS.cpp \
	Ellipsoid.cpp \
	EllipticFunction.cpp \
	GARS.cpp \
	GeoCoords.cpp \
	Geocentric.cpp \
	Geodesic.cpp \
	GeodesicExact.cpp \
	GeodesicExactC4.cpp \
	GeodesicLine.cpp \
	GeodesicLineExact.cpp \
	Geohash.cpp \
	Geoid.cpp \
	Georef.cpp \
	Gnomonic.cpp \
	GravityCircle.cpp \
	GravityModel.cpp \
	LambertConformalConic.cpp \
	LocalCartesian.cpp \
	MGRS.cpp \
	MagneticCircle.cpp \
	MagneticModel.cpp \
	Math.cpp \
	NormalGravity.cpp \
	OSGB.cpp \
	PolarStereographic.cpp \
	PolygonArea.cpp \
	Rhumb.cpp \
	SphericalEngine.cpp \
	TransverseMercator.cpp \
	TransverseMercatorExact.cpp \
	UTMUPS.cpp \
	Utility.cpp

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
