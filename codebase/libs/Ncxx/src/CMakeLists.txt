###############################################################
#
# CMakeLists.txt - auto generated from Makefile
#
# library name: Ncxx
#
# written by script createCMakeLists.lib.py
#
# dir: lrose-core/codebase/libs/Ncxx/src
###############################################################

project (libNcxx)

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
      ./DataPort/ByteOrder.cc
      ./DataPort/NcxxPort.cc
      ./Hdf5/Hdf5xx.cc
      ./Hdf5/H5xAbstractDs.cc
      ./Hdf5/H5xArrayType.cc
      ./Hdf5/H5xAtomType.cc
      ./Hdf5/H5xAttribute.cc
      ./Hdf5/H5xCommonFG.cc
      ./Hdf5/H5xCompType.cc
      ./Hdf5/H5xDaccProp.cc
      ./Hdf5/H5xDataSet.cc
      ./Hdf5/H5xDataSpace.cc
      ./Hdf5/H5xDataType.cc
      ./Hdf5/H5xDcreatProp.cc
      ./Hdf5/H5xDxferProp.cc
      ./Hdf5/H5xEnumType.cc
      ./Hdf5/H5xException.cc
      ./Hdf5/H5xFaccProp.cc
      ./Hdf5/H5xFcreatProp.cc
      ./Hdf5/H5xFile.cc
      ./Hdf5/H5xFloatType.cc
      ./Hdf5/H5xGroup.cc
      ./Hdf5/H5xIdComponent.cc
      ./Hdf5/H5xIntType.cc
      ./Hdf5/H5xLaccProp.cc
      ./Hdf5/H5xLcreatProp.cc
      ./Hdf5/H5xLibrary.cc
      ./Hdf5/H5xLocation.cc
      ./Hdf5/H5xObject.cc
      ./Hdf5/H5xOcreatProp.cc
      ./Hdf5/H5xPredType.cc
      ./Hdf5/H5xPropList.cc
      ./Hdf5/H5xStrType.cc
      ./Hdf5/H5xVarLenType.cc
      ./Nc3x/Nc3xFile.cc
      ./Nc3x/Nc3File.cc
      ./Nc3x/Nc3Values.cc
      ./Ncxx/Ncxx.cc
      ./Ncxx/NcxxAtt.cc
      ./Ncxx/NcxxByte.cc
      ./Ncxx/NcxxChar.cc
      ./Ncxx/NcxxCheck.cc
      ./Ncxx/NcxxCompoundType.cc
      ./Ncxx/NcxxDim.cc
      ./Ncxx/NcxxDouble.cc
      ./Ncxx/NcxxEnumType.cc
      ./Ncxx/NcxxErrStr.cc
      ./Ncxx/NcxxException.cc
      ./Ncxx/NcxxFile.cc
      ./Ncxx/NcxxFloat.cc
      ./Ncxx/NcxxGroup.cc
      ./Ncxx/NcxxGroupAtt.cc
      ./Ncxx/NcxxInt.cc
      ./Ncxx/NcxxInt64.cc
      ./Ncxx/NcxxOpaqueType.cc
      ./Ncxx/NcxxShort.cc
      ./Ncxx/NcxxString.cc
      ./Ncxx/NcxxType.cc
      ./Ncxx/NcxxUbyte.cc
      ./Ncxx/NcxxUint.cc
      ./Ncxx/NcxxUint64.cc
      ./Ncxx/NcxxUshort.cc
      ./Ncxx/NcxxVar.cc
      ./Ncxx/NcxxVarAtt.cc
      ./Ncxx/NcxxVlenType.cc
   )

if(APPLE)
# for apple, force static build
  add_library (Ncxx STATIC ${SRCS})
else(APPLE)
# build shared library
  add_library (Ncxx SHARED ${SRCS})
endif(APPLE)

# install

INSTALL(TARGETS Ncxx
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
       )
INSTALL(DIRECTORY include/Ncxx
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include
       )

