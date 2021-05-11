###############################################################
#
# CMakeLists for cmake recursion
#
# dir: lrose-core/codebase/apps/dsserver/src
#
# written by script createCMakeLists.py
#
###############################################################

project (LROSE-CORE)

# subdirectories

add_subdirectory (Acars2Symprod)
add_subdirectory (AcTrack2Symprod)
add_subdirectory (Bdry2Symprod)
add_subdirectory (DataMapper)
add_subdirectory (DissectURL)
add_subdirectory (DsFCopyServer)
add_subdirectory (DsFileDist)
add_subdirectory (DsFmqServer)
add_subdirectory (DsLdataServer)
add_subdirectory (DsMdvServer)
add_subdirectory (DsProxyServer)
add_subdirectory (DsServerMgr)
add_subdirectory (DsServerQuery)
add_subdirectory (DsSpdbServer)
add_subdirectory (DsTitanServer)
add_subdirectory (FltPath2Symprod)
add_subdirectory (GenPoly2Symprod)
add_subdirectory (GenPt2Symprod)
add_subdirectory (GenPtField2Symprod)
add_subdirectory (GetPortNumber)
add_subdirectory (HydroStation2Symprod)
add_subdirectory (Ltg2Symprod)
add_subdirectory (Mad2Symprod)
add_subdirectory (Metar2Symprod)
add_subdirectory (Pirep2Symprod)
add_subdirectory (PosnRpt2Symprod)
add_subdirectory (PrintDataMap)
add_subdirectory (print_server_mgr)
add_subdirectory (resolve_host)
add_subdirectory (ResolveUrl)
add_subdirectory (Rhi2Symprod)
add_subdirectory (SigAirMet2Symprod)
add_subdirectory (SpdbQuery)
add_subdirectory (SpdbXml2Table)
add_subdirectory (SunCal2Symprod)
add_subdirectory (SyncDataMap)
add_subdirectory (TestDataMap)
add_subdirectory (Tstorms2Symprod)
add_subdirectory (Vergrid2Symprod)
add_subdirectory (WxHazards2Symprod)

