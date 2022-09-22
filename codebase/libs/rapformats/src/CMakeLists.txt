###############################################################
#
# CMakeLists.txt - auto generated from Makefile
#
# library name: rapformats
#
# written by script createCMakeLists.lib.py
#
# dir: lrose-core/codebase/libs/rapformats/src
###############################################################

project (librapformats)

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
      ./ac_data/ac_data.c
      ./ac_posn/ac_posn.c
      ./ac_posn/ac_georef.cc
      ./ac_posn/acPosVector.cc
      ./ac_route/ac_route.c
      ./acars/acars.c
      ./acars/acarsXml.cc
      ./acars/Amdar.cc
      ./ascii_shapeio/shapeio.cc
      ./ascii_shapeio/sio_index_file.cc
      ./ascii_shapeio/sio_util.cc
      ./ascii_shapeio/type_subtype_list.cc
      ./bdry/bdry_c.c
      ./bdry/bdry_extrap.c
      ./bdry/Bdry.cc
      ./bdry/BdryPoint.cc
      ./bdry/BdryPointShearInfo.cc
      ./bdry/BdryPolyline.cc
      ./ccm/ccm_io_lib.c
      ./cedric/Cedric.cc
      ./checktimes/ChecktimeReport.cc
      ./DsRadar/ds_radar.c
      ./DsRadar/ds_radar_calib.c
      ./DsRadar/ds_radar_ts.c
      ./DsRadar/DsBeamData.cc
      ./DsRadar/DsBeamDataFieldParms.cc
      ./DsRadar/DsRadarAz.cc
      ./DsRadar/DsRadarCalib.cc
      ./DsRadar/DsRadarElev.cc
      ./DsRadar/DsRadarParams.cc
      ./DsRadar/DsFieldParams.cc
      ./DsRadar/DsRadarBeam.cc
      ./DsRadar/DsRadarFlags.cc
      ./DsRadar/DsRadarMsg.cc
      ./DsRadar/DsRadarPower.cc
      ./DsRadar/DsRadarSweep.cc
      ./DsRadar/DsPlatformGeoref.cc
      ./Edr/Edr_expanded.cc
      ./Edr/Edr.cc
      ./flt_path/flt_path.c
      ./fos/sigmet.c
      ./fos/wxadv.c
      ./GenPoly/GenPoly.cc
      ./GenPoly/GenPolyStats.cc
      ./GenPt/ComboPt.cc
      ./GenPt/GenPt.cc
      ./GenPt/GenPtArray.cc
      ./gint/gint_user.c
      ./hist_fore/hist_fore.c
      ./iidabin/iidabin.cc
      ./kavouras/dcmp6h.c
      ./kavouras/dcmp6l.c
      ./kavouras/kavouras_io.c
      ./km/decode_header.c
      ./km/fix_header_time.c
      ./km/set_header_time.c
      ./km/unpack_raw.c
      ./km/KavMosaic.cc
      ./ltg/gailtg.c
      ./ltg/kavltg.c
      ./ltg/ltg.c
      ./ltg/twnltg.c
      ./ltg/GaiLtgFile.cc
      ./ltg/GldnLtgFile.cc
      ./ltg/KavLtgFile.cc
      ./ltg/LtgGroup.cc
      ./ltg/LtgWrapper.cc
      ./ltg/MitLtg.cc
      ./Maps/Map.cc
      ./Maps/MapIcon.cc
      ./Maps/MapIconDef.cc
      ./Maps/MapIconPoint.cc
      ./Maps/MapObject.cc
      ./Maps/MapPoint.cc
      ./Maps/MapPointOffset.cc
      ./Maps/MapPolyline.cc
      ./Maps/MapSimpleLabel.cc
      ./MultBuf/MultBuf.cc
      ./MultBuf/MultBufPart.cc
      ./mcidas/area.c
      ./mdvt/mdvt_user.c
      ./metar/antoi.c
      ./metar/dcdmetar.c
      ./metar/dcdmtrmk.c
      ./metar/fracpart.c
      ./metar/stspack2.c
      ./mitre/mitre.c
      ./nids/nids_file.c
      ./nws_wwa/NWS_WWA.cc
      ./Pirep/pirep_c.c
      ./Pirep/pirepXml.cc
      ./Pirep/Pirep.cc
      ./radar/clutter_table.c
      ./radar/polar2mdv_lookup.c
      ./radar/radar_scan_table.c
      ./radar/RadarSpectra.cc
      ./radar/UfRadar.cc
      ./radar/UfRecord.cc
      ./r_data/rData.cc
      ./SigAirMet/SigAirMet.cc
      ./Sndg/CapeCinSounding.cc
      ./Sndg/Sndg.cc
      ./sounding/sounding_chunk.c
      ./station_data/station_reports.c
      ./station_data/print_metar_data.c
      ./station_data/decoded_metar_to_report.c
      ./station_data/HydroStation.cc
      ./station_data/SsiFile.cc
      ./station_data/station_file.cc
      ./station_data/StationData.cc
      ./station_data/WxObs.cc
      ./station_data/WxObsField.cc
      ./station_data/WxTypeField.cc
      ./station_data/TaiwanAwos.cc
      ./swap/swap_lincoln.c
      ./swap/swap_ridds.c
      ./swap/swap_rp7.c
      ./taf/Taf.cc
      ./ThreshMap/FieldThresh.cc
      ./ThreshMap/FieldThresh2.cc
      ./ThreshMap/FieldThresh2WithGrid.cc
      ./ThreshMap/MultiThresh.cc
      ./ThreshMap/MultiThreshFcstBiasMapping.cc
      ./ThreshMap/MultiThreshItem.cc
      ./ThreshMap/MultiThresholdsBiasMapping.cc
      ./ThreshMap/TiledMultiThresh.cc
      ./ThreshMap/TileInfo.cc
      ./ThreshMap/TileLatLon.cc
      ./titan/tstorm_hull_smooth.c
      ./titan/tstorm_spdb.c
      ./titan/titan_grid.c
      ./trec_gauge/trec_gauge.c
      ./var_elev/var_elev.c
      ./VerGrid/VerGridRegion.cc
      ./Usgs/UsgsData.cc
      ./Windshear/WindshearArena.cc
      ./Windshear/Windshear.cc
      ./Windshear/WindshearAlpha.cc
      ./Windshear/WindshearAlphas.cc
      ./WinsRadar/WinsRadar.cc
      ./zr/zr.c
      ./zr/zrpf.c
      ./zr/ZVis.cc
      ./zr/ZvisCal.cc
      ./zr/ZvisFcast.cc
   )

if(APPLE)
# for apple, force static build
  add_library (rapformats STATIC ${SRCS})
else(APPLE)
# build shared library
  add_library (rapformats SHARED ${SRCS})
endif(APPLE)

# install

INSTALL(TARGETS rapformats
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
       )
INSTALL(DIRECTORY include/rapformats
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include
       )

