###############################################################
#
# CMakeLists.txt - auto generated from Makefile
#
# library name: radar
#
# written by script createCMakeLists.lib.py
#
# dir: lrose-core/codebase/libs/radar/src
###############################################################

project (libradar)

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
include_directories (../../radar/src/include)
include_directories (../../rapformats/src/include)
include_directories (../../rapmath/src/include)
include_directories (../../rapplot/src/include)
include_directories (../../shapelib/src/include)
include_directories (../../tdrp/src/include)
include_directories (../../titan/src/include)
include_directories (../../toolsa/src/include)
include_directories (${CMAKE_INSTALL_PREFIX}/include)
if (DEFINED X11_X11_INCLUDE_PATH)
  include_directories (${X11_X11_INCLUDE_PATH})
endif()
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

# source files

set (SRCS
      ./calib/RadarCalib.cc
      ./complex/RadarComplex.cc
      ./convstrat/ConvStratFinder.cc
      ./dorade/spol_C_utils.c
      ./dorade/DFileInput.cc
      ./dorade/dd_mapper.cc
      ./dorade/dd_sweepfiles.cc
      ./dorade/dd_testify.cc
      ./dorade/dd_utils.cc
      ./dorade/kdp_calcs.cc
      ./filter/DpolFilter.cc
      ./filter/FilterUtils.cc
      ./filter/PhidpFilt.cc
      ./fmq/ClickPointFmq.cc
      ./geom/BeamHeight.cc
      ./geom/Egm2008.cc
      ./hsrl/HsrlRawRay.cc
      ./ips/ips_ts_functions.cc
      ./ips/IpsAltModeVel.cc
      ./ips/IpsGateData.cc
      ./ips/IpsMomFields.cc
      ./ips/IpsMoments.cc
      ./ips/IpsNoiseLocator.cc
      ./ips/IpsTsCalib.cc
      ./ips/IpsTsInfo.cc
      ./ips/IpsTsPulse.cc
      ./ips/IpsTsReader.cc
      ./iwrf/chill_to_iwrf.cc
      ./iwrf/iwrf_functions.cc
      ./iwrf/IwrfCalib.cc
      ./iwrf/IwrfTsBurst.cc
      ./iwrf/IwrfTsGet.cc
      ./iwrf/IwrfTsInfo.cc
      ./iwrf/IwrfTsPulse.cc
      ./iwrf/IwrfTsReader.cc
      ./iwrf/rsm_functions.cc
      ./kdp/KdpFiltParams.cc
      ./kdp/KdpBringi.cc
      ./kdp/KdpCompute.cc
      ./kdp/KdpFilt.cc
      ./kdp/PhidpProc.cc
      ./moments/AlternatingVelocity.cc
      ./moments/AtmosAtten.cc
      ./moments/ClutFilter.cc
      ./moments/GateData.cc
      ./moments/MomentsFields.cc
      ./moments/InterestMap.cc
      ./moments/IwrfMoments.cc
      ./moments/IwrfMomReader.cc
      ./moments/NoiseLocator.cc
      ./moments/PhaseCoding.cc
      ./moments/RadarFft.cc
      ./moments/RadarMoments.cc
      ./moments/RegressionFilter.cc
      ./moments/Sz864.cc
      ./pid/NcarPidParams.cc
      ./pid/NcarParticleId.cc
      ./pid/PidImapManager.cc
      ./pid/PidInterestMap.cc
      ./pid/TempProfile.cc
      ./precip/PrecipRateParams.cc
      ./precip/PrecipRate.cc
      ./qc/HcrVelFirFilt.cc
      ./qc/HcrSurfaceVel.cc
      ./qc/IntfLocator.cc
      ./qc/SeaClutter.cc
      ./radx/RadxAppParams.cc
      ./radx/RadxAppParms.cc
      ./radx/RadxApp.cc
      ./radx/RadxAppArgs.cc
      ./radx/RadxAppCircularLookupHandler.cc
      ./radx/RadxAppConfig.cc
      ./radx/RadxAppVolume.cc
      ./radx/RadxAppRayData.cc
      ./radx/RadxAppRayLoopData.cc
      ./radx/RadxAppSweepData.cc
      ./radx/RadxAppSweepLoopData.cc
      ./spol/spol_angles.cc
      ./ts/RadarTsInfo.cc
      ./ts/RadarTsPulse.cc
      ./ts/RadarTsReader.cc
   )

if(APPLE)
# for apple, force static build
  add_library (radar STATIC ${SRCS})
else(APPLE)
# build shared library
  add_library (radar SHARED ${SRCS})
endif(APPLE)

# install

INSTALL(TARGETS radar
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
       )
INSTALL(DIRECTORY include/radar
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include
       )

