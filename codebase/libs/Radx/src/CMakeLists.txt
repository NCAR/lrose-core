list(APPEND SOURCES
  Radx/ByteOrder.cc
  Radx/PseudoRhi.cc
  Radx/Radx.cc
  Radx/RadxAngleHist.cc
  Radx/RadxAzElev.cc
  Radx/RadxBuf.cc
  Radx/RadxCfactors.cc
  Radx/RadxComplex.cc
  Radx/RadxEvent.cc
  Radx/RadxField.cc
  Radx/RadxFile.cc
  Radx/RadxFuzzy2d.cc
  Radx/RadxFuzzyF.cc
  Radx/RadxGeoref.cc
  Radx/RadxMsg.cc
  Radx/RadxPacking.cc
  Radx/RadxPath.cc
  Radx/RadxPlatform.cc
  Radx/RadxRangeGeom.cc
  Radx/RadxRay.cc
  Radx/RadxRcalib.cc
  Radx/RadxReadDir.cc
  Radx/RadxRemap.cc
  Radx/RadxStr.cc
  Radx/RadxSweep.cc
  Radx/RadxTime.cc
  Radx/RadxTimeList.cc
  Radx/RadxVol.cc
  Radx/RadxXml.cc
  Radx/RayxData.cc
  Radx/RayxMapping.cc
)

if(Module_CfRadial)
  list(APPEND SOURCES
    Ncf/NcfRadxFile.cc
    Ncf/NcfRadxFile_read.cc
    Ncf/NcfRadxFile_write.cc
    Ncf/NcxxRadxFile.cc
    Ncf/NcxxRadxFile_read.cc
    Ncf/NcxxRadxFile_write.cc
  )
endif()

if(Module_CfRadial2)
  list(APPEND SOURCES
    Cf2/Cf2RadxFile.cc
	Cf2/Cf2RadxFile_read.cc
	Cf2/Cf2RadxFile_write.cc
  )
endif()

if(Module_D3R)
  list(APPEND SOURCES
    D3R/D3rNcRadxFile.cc
  )
endif()

if(Module_DOE)
  list(APPEND SOURCES
    DOE/DoeNcRadxFile.cc
  )
endif()

if(Module_DORADE)
  list(APPEND SOURCES
    Dorade/DoradeData.cc
    Dorade/DoradeRadxFile.cc
  )
endif()

if(Module_EEC)
  list(APPEND SOURCES
    EEC/EdgeNcRadxFile.cc
  )
endif()

if(Module_FORAY)
  list(APPEND SOURCES
    ForayNc/ForayNcRadxFile.cc
    ForayNc/ForayNcRadxFile_read.cc
    ForayNc/ForayNcRadxFile_write.cc
  )
endif()

if(Module_GAMIC)
  list(APPEND SOURCES
    Gamic/GamicHdf5RadxFile.cc
  )
endif()

if(Module_Gematronik)
  list(APPEND SOURCES
    Gematronik/GemBlob.cc
    Gematronik/GemRadxFile.cc
    Gematronik/GemInputField.cc
    Gematronik/GemSweep.cc
  )
endif()

if(Module_HRD)
  list(APPEND SOURCES
    Hrd/HrdData.cc
    Hrd/HrdRadxFile.cc  
  )
endif()

if(Module_Leosphere)
  list(APPEND SOURCES
    Leosphere/LeoRadxFile.cc
  )
endif()

if(Module_MRD)
  list(APPEND SOURCES
    NsslMrd/NsslMrdRadxFile.cc
  )
endif()

if(Module_NEXRAD)
  list(APPEND SOURCES
    Nexrad/NexradCmdRadxFile.cc
    Nexrad/NexradData.cc
    Nexrad/NexradLoc.cc
    Nexrad/NexradRadxFile.cc
    Nexrad/NexradVcp.cc
    Nexrad/NidsData.cc
    Nexrad/NidsRadxFile.cc
  )
endif()

if(Module_NOXP)
  list(APPEND SOURCES
    Noxp/NoxpNcRadxFile.cc
  )
endif()

if(Module_ODIM)
  list(APPEND SOURCES
    Odim/OdimHdf5RadxFile.cc
  )
endif()

if(Module_RAPIC)
  list(APPEND SOURCES
    Rapic/Linebuff.cc
    Rapic/PPIField.cc
    Rapic/RapicRadxFile.cc
    Rapic/RapicRay.cc
    Rapic/ScanParams.cc
    Rapic/sRadl.cc
  )
endif()

if(Module_SIGMET)
  list(APPEND SOURCES
    Sigmet/SigmetRadxFile.cc
  )
endif()

if(Module_TDWR)
  list(APPEND SOURCES
    Tdwr/TdwrLoc.cc
    Tdwr/TdwrRadxFile.cc
  )
endif()

if(Module_TWOLF)
  list(APPEND SOURCES
    Twolf/TwolfRadxFile.cc
  )
endif()

if(Module_UF)
  list(APPEND SOURCES
    Uf/UfData.cc
    Uf/UfRadxFile.cc
  )
endif()

include_directories(. ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/../Ncxx/src/include)

add_library(Radx ${SOURCES})
target_link_libraries(Radx ${ZLIB_LIBRARIES} ${BZIP2_LIBRARIES} ${UDUNITS_LIBRARY} ${HDF5_LIBRARIES} ${NETCDF_LIBRARIES})
if(MSVC)
  target_link_libraries(Radx ${THREADS_PTHREADS_WIN32_LIBRARY})
endif()
