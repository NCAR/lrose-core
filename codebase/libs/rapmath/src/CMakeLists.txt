###############################################################
#
# CMakeLists.txt - auto generated from Makefile
#
# library name: rapmath
#
# written by script createCMakeLists.lib.py
#
# dir: lrose-core/codebase/libs/rapmath/src
###############################################################

project (librapmath)

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
if (DEFINED MAMBA_BUILD)
# MAMBA builds ignore system libs, use mamba libs
  include_directories (${MAMBA_INCLUDE_PATH})
else()
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
  
endif(DEFINED MAMBA_BUILD)
# source files

set (SRCS
      ./angles/AngleCombiner.cc
      ./complex/RapComplex.cc
      ./functions/PtFunction.cc
      ./functions/LagrangePtFunc.cc
      ./functions/LinearPtFunc.cc
      ./functions/NewtonPtFunc.cc
      ./functions/FuzzyF.cc
      ./functions/Fuzzy2d.cc
      ./functions/FuzzyFunction.cc
      ./functions/ParamsFuzzyFunction.cc
      ./functions/PtFuzzyFunc.cc
      ./functions/ScaleFuzzyFunc.cc
      ./functions/StepPtFunc.cc
      ./functions/SFuzzyF.cc
      ./functions/TrapFuzzyF.cc
      ./lists/OrderedList.cc
      ./malloc/RMmalloc.c
      ./mathparse/AssignmentNode.cc
      ./mathparse/BinaryArgs.cc
      ./mathparse/BinaryNode.cc
      ./mathparse/Find.cc
      ./mathparse/FloatUserData.cc
      ./mathparse/LeafContent.cc
      ./mathparse/LeafNode.cc
      ./mathparse/LogicalArgs.cc
      ./mathparse/LogicalNode.cc
      ./mathparse/MathData.cc
      ./mathparse/MathDataSimple.cc
      ./mathparse/MathFindSimple.cc
      ./mathparse/MathParser.cc
      ./mathparse/ProcessingNode.cc
      ./mathparse/SpecialUserData.cc
      ./mathparse/StatusUserData.cc
      ./mathparse/UnaryNode.cc
      ./misc/Math.cc
      ./round/nint.c
      ./stats/beta.c
      ./stats/exponential.c
      ./stats/gamma.c
      ./stats/normal.c
      ./stats/primes.c
      ./stats/uniform.c
      ./stats/weibull.c
      ./stats/DistLognorm.cc
      ./stats/DistNormal.cc
      ./stats/DistPolynomial.cc
      ./stats/Distribution.cc
      ./stats/ModeDiscrete.cc
      ./tree/bd_tree.c
      ./trig/trig.c
      ./umath/dir_speed_2_uv.c
      ./umath/uLatLon.c
      ./umath/uintersect.c
      ./umath/uline_through_ellipse.c
      ./umath/ulong_scale.c
      ./umath/umax_wt_bip.c
      ./umath/upoint_in_ellipse.c
      ./umath/upoint_in_polygon.c
      ./umath/usign.c
      ./umath/usort.c
      ./umath/ustat.c
      ./umath/usvd.c
      ./umath/uv_2_dir_speed.c
      ./umath/ForsytheFit.cc
      ./umath/PlaneFit.cc
      ./umath/PolyFit.cc
      ./umath/usort_stl.cc
      ./umath/uCurveFit.cc
      ./umath/uNewtRaph.cc
      ./umath/uPct.cc
   )

if(APPLE)
# for apple, force static build
  add_library (rapmath STATIC ${SRCS})
else(APPLE)
# build shared library
  add_library (rapmath SHARED ${SRCS})
endif(APPLE)

# install

INSTALL(TARGETS rapmath
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
       )
INSTALL(DIRECTORY include/rapmath
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include
       )

