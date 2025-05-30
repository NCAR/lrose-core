###############################################################
#
# CMakeLists for cmake recursion
#
# dir: lrose-core/codebase/apps/radar/src
#
# written by script createCMakeLists.py
#
###############################################################

project (lrose-core)

# subdirectories

add_subdirectory (AcGeoref2Spdb)
add_subdirectory (AcGeorefCompare)
add_subdirectory (AltCpCompute)
add_subdirectory (CalCompute)
add_subdirectory (CedricPrint)
add_subdirectory (ChillCal2Xml)
add_subdirectory (ChillSdb2Dsr)
add_subdirectory (ChillSdbServer)
add_subdirectory (ChillTsFile2Fmq)
add_subdirectory (ChillTsTcp2Fmq)
add_subdirectory (CmdVerify)
add_subdirectory (Colide)
add_subdirectory (Cov2Mom)
add_subdirectory (DowSendFreq)
add_subdirectory (DsrGrabber)
add_subdirectory (DsrMerge)
add_subdirectory (DualDopplerLobes)
add_subdirectory (Ecco)
add_subdirectory (EccoStats)
add_subdirectory (HawkEye)
add_subdirectory (HawkEdit)
add_subdirectory (HcrTempRxGain)
add_subdirectory (HcrVelCorrect)
add_subdirectory (Hsrl2Radx)
add_subdirectory (IntfRemove)
add_subdirectory (IpsEye)
add_subdirectory (IpsTsArchive)
add_subdirectory (IpsTs2Moments)
add_subdirectory (IpsTs2NetCDF)
add_subdirectory (IpsTsPrint)
add_subdirectory (IpsTsSim)
add_subdirectory (Iq2Dsr)
add_subdirectory (Mdv2Vad)
add_subdirectory (MdvFold)
add_subdirectory (MdvRadarShear)
add_subdirectory (MergeHcrAndHsrl)
add_subdirectory (NexradPowerStats)
add_subdirectory (NoiseMon)
add_subdirectory (PointingCpCompute)
add_subdirectory (PowerInfo2Spdb)
add_subdirectory (Qmap2Cidd)
add_subdirectory (QpeVerify)
add_subdirectory (QuadratureTest)
add_subdirectory (RadarCal)
add_subdirectory (RadarConst)
add_subdirectory (RadarMdvCompare)
add_subdirectory (RadxDealias)
add_subdirectory (RapMap2Chill)
add_subdirectory (RcalXml2Table)
add_subdirectory (SimCpCompute)
add_subdirectory (SpectraPrint)
add_subdirectory (Sprite)
add_subdirectory (StaggeredLimits)
add_subdirectory (SunCal)
add_subdirectory (SunCpCompute)
add_subdirectory (SweepInfo2Spdb)
add_subdirectory (Ts2Moments)
add_subdirectory (Ts2NetCDF)
add_subdirectory (TsCalAuto)
add_subdirectory (TsConvert)
add_subdirectory (TsFile2Fmq)
add_subdirectory (TsFmq2Tcp)
add_subdirectory (TsMaxPower2Fmq)
add_subdirectory (TsGamic2Iwrf)
add_subdirectory (TsMerge)
add_subdirectory (TsOverlay)
add_subdirectory (TsPrint)
add_subdirectory (TsRefract2Dsr)
add_subdirectory (TsScanInfoMerge)
add_subdirectory (TsSim)
add_subdirectory (TsSmartSave)
add_subdirectory (TsStatusMonitor)
add_subdirectory (TsTcp2File)
add_subdirectory (TsTcp2Fmq)
add_subdirectory (TsTimeFix)
add_subdirectory (UfRadarPrint)
add_subdirectory (VertCompute)
add_subdirectory (ZdrCalSimHv)

