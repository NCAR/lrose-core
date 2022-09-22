###############################################################
#
# CMakeLists.txt - auto generated from Makefile
#
# library name: toolsa
#
# written by script createCMakeLists.lib.py
#
# dir: lrose-core/codebase/libs/toolsa/src
###############################################################

project (libtoolsa)

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
      ./Data/DataScaler.cc
      ./Log/Benchmark.cc
      ./Log/Log.cc
      ./Log/LogFile.cc
      ./Log/MsgLog.cc
      ./Log/LogMsg.cc
      ./Log/LogStream.cc
      ./Log/LogMsgInit.cc
      ./Log/LogMsgStreamInit.cc
      ./Log/LogStreamInit.cc
      ./Log/TaThreadLog.cc
      ./Log/TaTriggerLog.cc
      ./Path/cpath.c
      ./Path/Path.cc
      ./URL/URL.cc
      ./URL/HttpURL.cc
      ./ascii_files/Data_Row.cc
      ./ansi/ansi.c
      ./attributes/Attributes.cc
      ./compress/bzip_compress.c
      ./compress/gzip_compress.c
      ./compress/lzo_compress.c
      ./compress/minilzo.c
      ./compress/rle_compress.c
      ./compress/ta_compress.c
      ./compress/ta_crc32.c
      ./compress/zlib_compress.c
      ./db_access/db_access.c
      ./dlm/dlm.c
      ./err/eprintf.c
      ./err/err.c
      ./exception/Except.cc
      ./file_io/file_io.c
      ./file_io/filecopy.c
      ./file_io/makedir.c
      ./file_io/ldmFileStrobe.cc
      ./file_io/InputDir.cc
      ./file_io/InputDirRecurse.cc
      ./file_io/PlaybackTable.cc
      ./file_io/ReadDir.cc
      ./file_io/TaFile.cc
      ./fmq/fmq_alloc.c
      ./fmq/fmq_check.c
      ./fmq/fmq_error.c
      ./fmq/fmq_file.c
      ./fmq/fmq_handle.c
      ./fmq/fmq_print.c
      ./fmq/fmq_read.c
      ./fmq/fmq_search.c
      ./fmq/fmq_stat.c
      ./fmq/fmq_swap.c
      ./fmq/fmq_write.c
      ./fmq/fmq.c
      ./font/font.c
      ./http/HTTPgetURL.c
      ./http/HTTPgetURL_via_proxy.c
      ./http/HttpConnection.cc
      ./ldata/ldata_info.c
      ./mb/mb.c
      ./mb/mbq.c
      ./mem/blockbuf.c
      ./mem/mem.c
      ./mem/mem_buf.c
      ./mem/umalloc.c
      ./mem/MemBuf.cc
      ./mem/TEST_TaArray.cc
      ./misc/Semaphore.cc
      ./misc/Shmem.cc
      ./misc/ShmemSem.cc
      ./pjg/pjg.c
      ./pjg/pjg_art.c
      ./pjg/pjg_cylind.c
      ./pjg/pjg_flat.c
      ./pjg/pjg_grid.c
      ./pjg/pjg_lc2.c
      ./pjg/pjg_ll.c
      ./pjg/pjg_mdv.c
      ./pjg/pjg_ps.c
      ./pjg/pjg_pse.c
      ./pjg/pjg_tm.c
      ./pjg/pjg_util.c
      ./pjg/sincos.c
      ./pjg/TaPjg.cc
      ./pmu/pmu.c
      ./pmu/pmu_f.c
      ./pmu/PmuInfo.cc
      ./port/port_alphasort.c
      ./port/port_scandir.c
      ./port/port_hostname.c
      ./port/port_signal.c
      ./reutil/reutil.c
      ./smu/smu.c
      ./smu/smu_auto.c
      ./smu/smu_last_data.c
      ./sockutil/sock_util.c
      ./sockutil/sku_named.c
      ./sockutil/GetHost.cc
      ./sockutil/IpCache.cc
      ./sockutil/SockUtil.cc
      ./sockutil/ServerSocket.cc
      ./sockutil/Socket.cc
      ./sockutil/HttpSocket.cc
      ./sockutil/ThreadSocket.cc
      ./sockutil/incremental.cc
      ./sok2/sok2.c
      ./str/str_binary.c
      ./str/str_blnk.c
      ./str/str_bpad.c
      ./str/str_concat.c
      ./str/str_copy.c
      ./str/str_delete.c
      ./str/str_dup.c
      ./str/str_equal.c
      ./str/str_free.c
      ./str/str_good.c
      ./str/str_insert.c
      ./str/str_mcopy.c
      ./str/str_ncopy.c
      ./str/str_parse.c
      ./str/str_parse_delim.c
      ./str/str_parse_double.c
      ./str/str_pastoc.c
      ./str/str_pos.c
      ./str/str_remove.c
      ./str/str_replace.c
      ./str/str_swap.c
      ./str/str_tokn.c
      ./str/TaStr.cc
      ./tcp/Tcp_Exchange.cc
      ./threads/TaThread.cc
      ./threads/TaThreadDoubleQue.cc
      ./threads/TaThreadPollingQue.cc
      ./threads/TaThreadPool.cc
      ./threads/TaThreadQue.cc
      ./threads/TaThreadSimple.cc
      ./threads/TaThreadSimplePolling.cc
      ./ttape/remote_tape.c
      ./ttape/tape_utils.c
      ./tty/Tty.cc
      ./umisc/fsleep.c
      ./umisc/safe_system.c
      ./umisc/safe_system_f.c
      ./umisc/ucopyright.c
      ./umisc/udaemonize.c
      ./umisc/ufread.c
      ./umisc/ufree_parsed_path.c
      ./umisc/ufwrite.c
      ./umisc/uparams_etc.c
      ./umisc/uparse_path.c
      ./umisc/ushmem.c
      ./umisc/ustring.c
      ./umisc/usystem_call.c
      ./umisc/uusleep.c
      ./umisc/gridLineConnect.cc
      ./umisc/ugetenv.cc
      ./umisc/Server.cc
      ./umisc/ArchiveDates.cc
      ./utim/tu_sleep.c
      ./utim/utim.c
      ./utim/udatetime.c
      ./utim/uvalid_datetime.c
      ./utim/cal.cc
      ./utim/DeltaTime.cc
      ./utim/DateTime.cc
      ./xdru/xdru.c
      ./xml/TaXml.cc
      ./templates/TemplateTest.cc
   )

if(APPLE)
# for apple, force static build
  add_library (toolsa STATIC ${SRCS})
else(APPLE)
# build shared library
  add_library (toolsa SHARED ${SRCS})
endif(APPLE)

# install

INSTALL(TARGETS toolsa
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
       )
INSTALL(DIRECTORY include/toolsa
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include
       )

