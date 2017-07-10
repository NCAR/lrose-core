list(APPEND SOURCES
  Ncxx/Ncxx.cc
  Ncxx/NcxxAtt.cc
  Ncxx/NcxxByte.cc
  Ncxx/NcxxChar.cc
  Ncxx/NcxxCheck.cc
  Ncxx/NcxxCompoundType.cc
  Ncxx/NcxxDim.cc
  Ncxx/NcxxDouble.cc
  Ncxx/NcxxEnumType.cc
  Ncxx/NcxxErrStr.cc
  Ncxx/NcxxException.cc
  Ncxx/NcxxFile.cc
  Ncxx/NcxxFloat.cc
  Ncxx/NcxxGroup.cc
  Ncxx/NcxxGroupAtt.cc
  Ncxx/NcxxInt.cc
  Ncxx/NcxxInt64.cc
  Ncxx/NcxxOpaqueType.cc
  Ncxx/NcxxShort.cc
  Ncxx/NcxxString.cc
  Ncxx/NcxxType.cc
  Ncxx/NcxxUbyte.cc
  Ncxx/NcxxUint.cc
  Ncxx/NcxxUint64.cc
  Ncxx/NcxxUshort.cc
  Ncxx/NcxxVar.cc
  Ncxx/NcxxVarAtt.cc
  Ncxx/NcxxVlenType.cc
)

include_directories(. ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(Ncxx ${SOURCES})
target_link_libraries(Ncxx ${ZLIB_LIBRARIES} ${BZIP2_LIBRARIES} ${UDUNITS_LIBRARY} ${HDF5_LIBRARIES} ${NETCDF_LIBRARIES})
if(MSVC)
  target_link_libraries(Ncxx ${THREADS_PTHREADS_WIN32_LIBRARY})
endif()
