#
# Recursive make - makes the subdirectory code
#

include $(LROSE_MAKE_INC_DIR)/lrose_make_macros

TARGETS = $(GENERAL_TARGETS) $(INSTALL_TARGETS)

SUB_DIRS = \
         AcTrack2Polygon \
         AcarsNc2GenPt \
         ArgentineAwsIngest \
         Awos2Spdb \
         Aws2Spdb \
         BomAws2Spdb \
         BomSounding2Spdb \
         CreateStationList \
         CsuSounding2Spdb \
         GemVolXml2Dsr \
         Gini2Mdv \
         Grib2Mdv \
         Grib2toMdv \
         GtsSockIngest \
         LdmDynamic2Static \
         LdmMetars2File \
         Ltg2Spdb \
         madis2Spdb \
         Metar2Spdb \
         Mrms2Mdv \
         MrmsGribIngest \
         NWSsoundingIngest \
         NcRadar2Dsr \
         NcRadarSplit \
         NcarAcPosn2Spdb \
         NewNidsRadial2Mdv \
         NexradBzipDecompress \
         Nids2Mdv \
         NidsRadial2Mdv \
         NidsRaster2Mdv \
         NidsVad2Spdb \
         PrintNids \
         Rdas2Dsr \
         SatOrbit2AcPosn \
         SawsUm2Mdv \
         SerialIngest \
         ShipObsIngest \
         SigAirMet2Spdb \
         SynopObsIngest \
         Uf2Dsr \
         Usgs2Spdb \
         WafsTiled2Mdv \
         Wrf2Mdv \
         WrfVIL2Mdv \
         WsiNidsIngest \
         file_repeat_day \
         nexrad2dsr \
         noaaportLtg2Spdb \
         sndgIngest \
         superResNexradII2Dsr \
         superResNexradLdmUnzip

#         Image2Mdv - needs ImLib2

include $(LROSE_MAKE_INC_DIR)/lrose_make_recursive_dir_targets
