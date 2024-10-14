###############################################################
#
# CMakeLists.txt - auto generated from Makefile
#
# library name: euclid
#
# written by script createCMakeLists.lib.py
#
# dir: lrose-core/codebase/libs/euclid/src
###############################################################

project (libeuclid)

# include directories

include_directories (./include)
include_directories (../../FiltAlg/src/include)
include_directories (../../FiltAlgVirtVol/src/include)
include_directories (../../Fmq/src/include)
include_directories (../../Mdv/src/include)
include_directories (../../Ncxx/src/include)
include_directories (../../Radx/src/include)
include_directories (../../Refract/src/include)
include_directories (../../Solo/src/include)
include_directories (../../Spdb/src/include)
include_directories (../../advect/src/include)
include_directories (../../cidd/src/include)
include_directories (../../contour/src/include)
include_directories (../../dataport/src/include)
include_directories (../../didss/src/include)
include_directories (../../dsdata/src/include)
include_directories (../../dsserver/src/include)
include_directories (../../euclid/src/include)
include_directories (../../grib/src/include)
include_directories (../../grib2/src/include)
include_directories (../../hydro/src/include)
include_directories (../../kd/src/include)
include_directories (../../physics/src/include)
include_directories (../../qtplot/src/include)
include_directories (../../radar/src/include)
include_directories (../../rapformats/src/include)
include_directories (../../rapmath/src/include)
include_directories (../../rapplot/src/include)
include_directories (../../shapelib/src/include)
include_directories (../../tdrp/src/include)
include_directories (../../titan/src/include)
include_directories (../../toolsa/src/include)
include_directories (${CMAKE_INSTALL_PREFIX}/include)
if (DEFINED netCDF_INSTALL_PREFIX)
  include_directories (${netCDF_INSTALL_PREFIX}/include)
endif()
if (DEFINED HDF5_C_INCLUDE_DIR)
  include_directories (${HDF5_C_INCLUDE_DIR})
endif()
if(IS_DIRECTORY /usr/include/hdf5/serial)
  include_directories (/usr/include/hdf5/serial)
endif()
if(IS_DIRECTORY /usr/local/include)
  include_directories (/usr/local/include)
endif()
# NOTE: cannot add /usr/include using include_directories()
add_compile_options(-I/usr/include)

# source files

set (SRCS
      ./Attributes/AttributesEuclid.cc
      ./Attributes/DataAtt.cc
      ./Attributes/EndPts.cc
      ./Attributes/Handedness.cc
      ./Attributes/MotionVector.cc
      ./GeographicLib/Accumulator.cpp
      ./GeographicLib/AlbersEqualArea.cpp
      ./GeographicLib/AzimuthalEquidistant.cpp
      ./GeographicLib/CassiniSoldner.cpp
      ./GeographicLib/CircularEngine.cpp
      ./GeographicLib/DMS.cpp
      ./GeographicLib/Ellipsoid.cpp
      ./GeographicLib/EllipticFunction.cpp
      ./GeographicLib/GARS.cpp
      ./GeographicLib/GeoCoords.cpp
      ./GeographicLib/Geocentric.cpp
      ./GeographicLib/Geodesic.cpp
      ./GeographicLib/GeodesicExact.cpp
      ./GeographicLib/GeodesicExactC4.cpp
      ./GeographicLib/GeodesicLine.cpp
      ./GeographicLib/GeodesicLineExact.cpp
      ./GeographicLib/Geohash.cpp
      ./GeographicLib/Geoid.cpp
      ./GeographicLib/Georef.cpp
      ./GeographicLib/Gnomonic.cpp
      ./GeographicLib/GravityCircle.cpp
      ./GeographicLib/GravityModel.cpp
      ./GeographicLib/LambertConformalConic.cpp
      ./GeographicLib/LocalCartesian.cpp
      ./GeographicLib/MGRS.cpp
      ./GeographicLib/MagneticCircle.cpp
      ./GeographicLib/MagneticModel.cpp
      ./GeographicLib/Math.cpp
      ./GeographicLib/NormalGravity.cpp
      ./GeographicLib/OSGB.cpp
      ./GeographicLib/PolarStereographic.cpp
      ./GeographicLib/PolygonArea.cpp
      ./GeographicLib/Rhumb.cpp
      ./GeographicLib/SphericalEngine.cpp
      ./GeographicLib/TransverseMercator.cpp
      ./GeographicLib/TransverseMercatorExact.cpp
      ./GeographicLib/UTMUPS.cpp
      ./GeographicLib/Utility.cpp
      ./Grid/Grid.cc
      ./Grid/GridGeom.cc
      ./Grid2d/Box.cc
      ./Grid2d/GridAlgs.cc
      ./Grid2d/GridExpand.cc
      ./Grid2d/GridExpandX.cc
      ./Grid2d/Grid2d.cc
      ./Grid2d/Grid2dClump.cc
      ./Grid2d/Grid2dDistToNonMissing.cc
      ./Grid2d/Grid2dEdgeBuilder.cc
      ./Grid2d/Grid2dInside.cc
      ./Grid2d/Grid2dLoop.cc
      ./Grid2d/Grid2dLoopA.cc
      ./Grid2d/Grid2dLoopAlg.cc
      ./Grid2d/Grid2dMedian.cc
      ./Grid2d/Grid2dOffset.cc
      ./Grid2d/Grid2dPolyFinder.cc
      ./GridTemplate/CircularTemplate.cc
      ./GridTemplate/CircularTemplateList.cc
      ./GridTemplate/EllipticalTemplate.cc
      ./GridTemplate/GridOffset.cc
      ./GridTemplate/GridPoint.cc
      ./GridTemplate/GridTemplate.cc
      ./GridTemplate/GridTemplateList.cc
      ./GridTemplate/GlobalCircularTemplate.cc
      ./GridTemplate/GlobalGridTemplate.cc
      ./GridTemplate/RectangularTemplate.cc
      ./GridTemplate/CenteredRectangularTemplate.cc
      ./Pjg/Pjg.cc
      ./Pjg/PjgAlbersMath.cc
      ./Pjg/PjgAzimEquidistMath.cc
      ./Pjg/PjgCalc.cc
      ./Pjg/PjgFlatCalc.cc
      ./Pjg/PjgGrib.cc
      ./Pjg/PjgGridGeom.cc
      ./Pjg/PjgLambertAzimMath.cc
      ./Pjg/PjgLambertConfMath.cc
      ./Pjg/PjgLatlonCalc.cc
      ./Pjg/PjgLatlonMath.cc
      ./Pjg/PjgLc1Calc.cc
      ./Pjg/PjgLc2Calc.cc
      ./Pjg/PjgMath.cc
      ./Pjg/PjgMercatorCalc.cc
      ./Pjg/PjgMercatorMath.cc
      ./Pjg/PjgObliqueStereoCalc.cc
      ./Pjg/PjgObliqueStereoMath.cc
      ./Pjg/PjgPolarRadarCalc.cc
      ./Pjg/PjgPolarRadarMath.cc
      ./Pjg/PjgPolarStereoCalc.cc
      ./Pjg/PjgPolarStereoMath.cc
      ./Pjg/PjgTransMercatorMath.cc
      ./Pjg/PjgVertPerspMath.cc
      ./Point/DistPoint.cc
      ./Point/IndexPoint.cc
      ./Point/Point.cc
      ./Point/PointList.cc
      ./Polygon_simplify/DPbasic.cc
      ./Polyline/ConvexHull.cc
      ./Polyline/Line.cc
      ./Polyline/LineList.cc
      ./Polyline/Polyline.cc
      ./Projection/ProjFlat.cc
      ./Projection/ProjRUC2Lambert.cc
      ./Projection/Projection.cc
      ./SunPosn/novas.c
      ./SunPosn/novascon.c
      ./SunPosn/readeph0.c
      ./SunPosn/solsys3.c
      ./SunPosn/rsts_sun_pos.c
      ./SunPosn/SunAngle.cc
      ./SunPosn/SunPosn.cc
      ./WorldPoint/WorldPoint2D.cc
      ./WorldPoint/WorldPolygon2D.cc
      ./affine/Rotate3d.cc
      ./alloc/alloc.c
      ./boundary/adj_star.c
      ./boundary/alloc_nodes.c
      ./boundary/bbox.c
      ./boundary/bdry_graph.c
      ./boundary/boundary_intervals.c
      ./boundary/fill_boundary.c
      ./boundary/find_extreme_pts.c
      ./boundary/free_clump_info.c
      ./boundary/gen_bdry.c
      ./boundary/init_ray.c
      ./boundary/make_star.c
      ./boundary/obdry_graph.c
      ./boundary/oboundary_intervals.c
      ./boundary/ogen_bdry.c
      ./boundary/otraverse_bdry_graph.c
      ./boundary/translate_bdry.c
      ./boundary/traverse_bdry_graph.c
      ./clump/adjust_intervals.c
      ./clump/alloc_clumps.c
      ./clump/alloc_rowh.c
      ./clump/clump_grid.c
      ./clump/clump_intervals.c
      ./clump/clump_volume.c
      ./clump/erode_clump.c
      ./clump/euclid_dist.c
      ./clump/extend_clump.c
      ./clump/extend_intervals.c
      ./clump/find_intervals.c
      ./clump/find_intervals_float.c
      ./clump/find_overlap.c
      ./clump/free_interval.c
      ./clump/get_intervals.c
      ./clump/get_intervals_float.c
      ./clump/intervals_to_array.c
      ./clump/link_intervals.c
      ./clump/oclump_intervals.c
      ./clump/ofree_clump_info.c
      ./clump/oprint_intervals.c
      ./clump/overlap.c
      ./clump/overlap_plane.c
      ./clump/overlap_rows.c
      ./clump/overlap_volume.c
      ./clump/print_intervals.c
      ./clump/reset_arrays.c
      ./clump/reset_clump_id.c
      ./clump/row_hdr.c
      ./clump/scan_interval.c
      ./clump/set_intervals.c
      ./clump/set_rp.c
      ./clump/small_clumps.c
      ./clump/sort_intervals.c
      ./clump/split_clump.c
      ./clump/stack_interval.c
      ./clump/transform_intervals.c
      ./clump/translate_array.c
      ./clump/translate_clump.c
      ./clump/translate_intervals.c
      ./clump/union_intervals.c
      ./clump/zero_clump.c
      ./clump/ClumpingDualThresh.cc
      ./clump/ClumpingMgr.cc
      ./clump/ClumpProps.cc
      ./geometry/coord_system.c
      ./geometry/convex_hull.c
      ./geometry/create_box.c
      ./geometry/create_trapezoid.c
      ./geometry/equidistant_pts.c
      ./geometry/fill_polygon.c
      ./geometry/get_perpendicular.c
      ./geometry/half_plane.c
      ./geometry/hypot.c
      ./geometry/inside_poly.c
      ./geometry/is_rectangle.c
      ./geometry/lat_long.c
      ./geometry/line_intersect.c
      ./geometry/line_point.c
      ./geometry/line_through_ellipse.c
      ./geometry/linear_comb.c
      ./geometry/perp_projection.c
      ./geometry/point_in_ellipse.c
      ./geometry/point_in_polygon.c
      ./geometry/point_on_segment.c
      ./geometry/polygon_area.c
      ./geometry/polygon_centroid.c
      ./geometry/polygon_intersect.c
      ./geometry/random_bdry.c
      ./geometry/segment_intersect.c
      ./geometry/sincos.c
      ./geometry/unit_vector.c
      ./geometry/vect_add.c
      ./link/link.c
      ./search/search_line_through_ellipse.c
      ./search/search_point_in_ellipse.c
      ./search/search_point_in_polygon.c
   )

if(APPLE)
# for apple, force static build
  add_library (euclid STATIC ${SRCS})
else(APPLE)
# build shared library
  add_library (euclid SHARED ${SRCS})
endif(APPLE)

# install

INSTALL(TARGETS euclid
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
       )
INSTALL(DIRECTORY include/euclid
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include
       )

