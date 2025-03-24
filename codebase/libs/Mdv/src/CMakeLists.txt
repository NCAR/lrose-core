###############################################################
#
# CMakeLists.txt - auto generated from Makefile
#
# library name: Mdv
#
# written by script createCMakeLists.lib.py
#
# dir: lrose-core/codebase/libs/Mdv/src
###############################################################

project (libMdv)

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
  #add_compile_options(-I/usr/include)
  
endif(DEFINED MAMBA_BUILD)
# source files

set (SRCS
      ./ClimoFileFinder/DailyByYearFileFinder.cc
      ./ClimoFileFinder/DailyFileFinder.cc
      ./ClimoFileFinder/ClimoFileFinder.cc
      ./ClimoFileFinder/ExternalDiurnalFileFinder.cc
      ./ClimoFileFinder/HourlyByMonthFileFinder.cc
      ./ClimoFileFinder/HourlyByYearFileFinder.cc
      ./ClimoFileFinder/HourlyDiurnalFileFinder.cc
      ./ClimoFileFinder/HourlyFileFinder.cc
      ./ClimoFileFinder/MonthlyByYearFileFinder.cc
      ./ClimoFileFinder/MonthlyFileFinder.cc
      ./ClimoFileFinder/TwoHourlyByMonthFileFinder.cc
      ./ClimoFileFinder/TwoHourlyByYearFileFinder.cc
      ./ClimoFileFinder/TwoHourlyDiurnalFileFinder.cc
      ./ClimoFileFinder/ThreeHourlyByMonthFileFinder.cc
      ./ClimoFileFinder/ThreeHourlyDiurnalFileFinder.cc
      ./ClimoStatCalc/MaxDateStatCalc.cc
      ./ClimoStatCalc/MaxStatCalc.cc
      ./ClimoStatCalc/MeanStatCalc.cc
      ./ClimoStatCalc/MinDateStatCalc.cc
      ./ClimoStatCalc/MinStatCalc.cc
      ./ClimoStatCalc/NumObsGEStatCalc.cc
      ./ClimoStatCalc/NumObsGTStatCalc.cc
      ./ClimoStatCalc/NumObsLEStatCalc.cc
      ./ClimoStatCalc/NumObsLTStatCalc.cc
      ./ClimoStatCalc/NumObsStatCalc.cc
      ./ClimoStatCalc/NumTimesStatCalc.cc
      ./ClimoStatCalc/PercentObsGEStatCalc.cc
      ./ClimoStatCalc/PercentObsGTStatCalc.cc
      ./ClimoStatCalc/PercentObsLEStatCalc.cc
      ./ClimoStatCalc/PercentObsLTStatCalc.cc
      ./ClimoStatCalc/PercentObsStatCalc.cc
      ./ClimoStatCalc/StatCalc.cc
      ./ClimoStatCalc/StatNamer.cc
      ./ClimoStatCalc/StdDevStatCalc.cc
      ./DsMdvx/DsMdvx.cc
      ./DsMdvx/DsMdvxInput.cc
      ./DsMdvx/DsMdvxMsg.cc
      ./DsMdvx/DsMdvxThreaded.cc
      ./DsMdvx/DsMdvxTimes.cc
      ./DsMdvx/msg_add.cc
      ./DsMdvx/msg_assemble.cc
      ./DsMdvx/msg_disassemble.cc
      ./DsMdvx/msg_get.cc
      ./DsMdvx/msg_print.cc
      ./Mdvx/Mdvx.cc
      ./Mdvx/Mdvx_BE.cc
      ./Mdvx/Mdvx_ncf.cc
      ./Mdvx/Mdvx_print.cc
      ./Mdvx/Mdvx_read.cc
      ./Mdvx/Mdvx_timelist.cc
      ./Mdvx/Mdvx_vsect.cc
      ./Mdvx/Mdvx_write.cc
      ./Mdvx/Mdvx_xml.cc
      ./MdvxChunk/MdvxChunk.cc
      ./MdvxContour/MdvxContour.cc
      ./MdvxField/MdvxField.cc
      ./MdvxField/MdvxFieldCode.cc
      ./MdvxField/MdvxStdAtmos.cc
      ./MdvxGrid/MdvxGrid.cc
      ./MdvxProj/GenPolyGrid.cc
      ./MdvxProj/MdvxPjg.cc
      ./MdvxProj/MdvxProj.cc
      ./MdvxRadar/MdvxRadar.cc
      ./MdvxRemapLut/MdvxRemapLut.cc
      ./MdvxTimeList/MdvxTimeList.cc
      ./MdvxTimeStamp/MdvxTimeStamp.cc
      ./MdvxUrlWatcher/MdvxUrlWatcher.cc
      ./MdvxVsectLut/MdvxVsectLut.cc
      ./oldMdv/mdv_chunk.c
      ./oldMdv/mdv_client.c
      ./oldMdv/mdv_composite.c
      ./oldMdv/mdv_convert.c
      ./oldMdv/mdv_dataset.c
      ./oldMdv/mdv_field_handle.c
      ./oldMdv/mdv_grid.c
      ./oldMdv/mdv_handle.c
      ./oldMdv/mdv_print.c
      ./oldMdv/mdv_print_f.c
      ./oldMdv/mdv_private_f.c
      ./oldMdv/mdv_proj.c
      ./oldMdv/mdv_radar.c
      ./oldMdv/mdv_read.c
      ./oldMdv/mdv_read_f.c
      ./oldMdv/mdv_user.c
      ./oldMdv/mdv_utils.c
      ./oldMdv/mdv_write.c
      ./oldMdv/mdv_write_f.c
      ./oldMdv/print_grid.c
      ./oldMdv/MdvChunk.cc
      ./oldMdv/MdvField.cc
      ./oldMdv/MdvFieldData.cc
      ./oldMdv/MdvFile.cc
      ./oldMdv/MdvGrid.cc
      ./oldMdv/MdvInputPath.cc
      ./oldMdv/MdvMasterHdr.cc
      ./oldMdv/MdvPublic.cc
      ./oldMdv/MdvRead.cc
      ./oldMdv/MdvReadChunk.cc
      ./oldMdv/MdvReadField.cc
      ./oldMdv/MdvReadRadar.cc
      ./oldMdv/MdvVlevel.cc
      ./oldMdv/MdvVsection.cc
      ./oldMdv/MdvVsectionField.cc
      ./oldMdv/TimestampChunk.cc
      ./NcfMdv/Mdv2NcfTrans.cc
      ./NcfMdv/Ncf2MdvField.cc
      ./NcfMdv/Ncf2MdvTrans.cc
      ./NcfMdv/NcfFieldData.cc
      ./NcfMdv/NcfGridInfo.cc
      ./NcfMdv/NcfMdv.cc
      ./NcfMdv/NcfVlevelInfo.cc
      ./Radx/GenericRadxFile.cc
      ./Radx/MdvRadxFile.cc
   )

if(APPLE)
# for apple, force static build
  add_library (Mdv STATIC ${SRCS})
else(APPLE)
# build shared library
  add_library (Mdv SHARED ${SRCS})
endif(APPLE)

# install

INSTALL(TARGETS Mdv
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
       )
INSTALL(DIRECTORY include/Mdv
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include
       )

