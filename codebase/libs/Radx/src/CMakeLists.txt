###############################################################
#
# CMakeLists.txt - auto generated from Makefile
#
# library name: Radx
#
# written by script createCMakeLists.lib.py
#
# dir: lrose-core/codebase/libs/Radx/src
###############################################################

project (libRadx)

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
      ./Bufr/BufrFile.cc
      ./Bufr/BufrRadxFile.cc
      ./Bufr/BufrProduct.cc
      ./Bufr/BufrProductGeneric.cc
      ./Bufr/BufrProduct_204_31_X.cc
      ./Bufr/BufrProduct_gsi.cc
      ./Bufr/TableMapKey.cc
      ./Bufr/TableMapElement.cc
      ./Bufr/TableMap.cc
      ./Bufr/BufrTables.cc
      ./Cf2/Cf2RadxFile.cc
      ./Cf2/Cf2RadxFile_read.cc
      ./Cf2/Cf2RadxFile_write.cc
      ./Cfarr/CfarrNcRadxFile.cc
      ./DOE/DoeNcRadxFile.cc
      ./D3R/D3rNcRadxFile.cc
      ./Dorade/DoradeData.cc
      ./Dorade/DoradeRadxFile.cc
      ./EEC/EdgeNcRadxFile.cc
      ./ForayNc/ForayNcRadxFile.cc
      ./ForayNc/ForayNcRadxFile_read.cc
      ./ForayNc/ForayNcRadxFile_write.cc
      ./Gamic/GamicHdf5RadxFile.cc
      ./Gematronik/GemBlob.cc
      ./Gematronik/GemRadxFile.cc
      ./Gematronik/GemInputField.cc
      ./Gematronik/GemSweep.cc
      ./Hrd/HrdRadxFile.cc
      ./Leosphere/LeoRadxFile.cc
      ./Leosphere/LeoCf2RadxFile.cc
      ./Ncf/NcfRadxFile.cc
      ./Ncf/NcfRadxFile_read.cc
      ./Ncf/NcfRadxFile_write.cc
      ./Ncf/RadxNcfStr.cc
      ./Nexrad/NexradCmdRadxFile.cc
      ./Nexrad/NexradData.cc
      ./Nexrad/NexradLoc.cc
      ./Nexrad/NexradRadxFile.cc
      ./Nexrad/NexradVcp.cc
      ./Nexrad/NidsData.cc
      ./Nexrad/NidsRadxFile.cc
      ./NoaaFsl/NoaaFslRadxFile.cc
      ./Noxp/NoxpNcRadxFile.cc
      ./NsslMrd/NsslMrdRadxFile.cc
      ./Odim/OdimHdf5RadxFile.cc
      ./Radx/ByteOrder.cc
      ./Radx/PseudoRhi.cc
      ./Radx/Radx.cc
      ./Radx/RadxAngleHist.cc
      ./Radx/RadxAzElev.cc
      ./Radx/RadxBuf.cc
      ./Radx/RadxComplex.cc
      ./Radx/RadxCfactors.cc
      ./Radx/RadxEvent.cc
      ./Radx/RadxField.cc
      ./Radx/RadxFile.cc
      ./Radx/RadxFuzzyF.cc
      ./Radx/RadxFuzzy2d.cc
      ./Radx/RadxGeoref.cc
      ./Radx/RadxMsg.cc
      ./Radx/RadxPacking.cc
      ./Radx/RadxPath.cc
      ./Radx/RadxPlatform.cc
      ./Radx/RadxRangeGeom.cc
      ./Radx/RadxRay.cc
      ./Radx/RadxRcalib.cc
      ./Radx/RadxReadDir.cc
      ./Radx/RadxRemap.cc
      ./Radx/RadxSweep.cc
      ./Radx/RadxStr.cc
      ./Radx/RadxTime.cc
      ./Radx/RadxTimeList.cc
      ./Radx/RadxVol.cc
      ./Radx/RadxXml.cc
      ./Radx/RayxData.cc
      ./Radx/RayxMapping.cc
      ./Rapic/Linebuff.cc
      ./Rapic/PPIField.cc
      ./Rapic/RapicRadxFile.cc
      ./Rapic/RapicRay.cc
      ./Rapic/ScanParams.cc
      ./Rapic/sRadl.cc
      ./Raxpol/RaxpolNcRadxFile.cc
      ./Sigmet/SigmetRadxFile.cc
      ./Tdwr/TdwrLoc.cc
      ./Tdwr/TdwrRadxFile.cc
      ./Twolf/TwolfRadxFile.cc
      ./Uf/UfData.cc
      ./Uf/UfRadxFile.cc
   )

if(APPLE)
# for apple, force static build
  add_library (Radx STATIC ${SRCS})
else(APPLE)
# build shared library
  add_library (Radx SHARED ${SRCS})
endif(APPLE)

# install

INSTALL(TARGETS Radx
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
       )
INSTALL(DIRECTORY include/Radx
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include
       )

