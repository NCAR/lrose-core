###############################################################
#
# CMakeLists for cmake recursion
#
# dir: lrose-core/codebase/apps/didss/src
#
# written by script createCMakeLists.py
#
###############################################################

project (LROSE-CORE)

# subdirectories

add_subdirectory (Dsr2titanAscii)
add_subdirectory (Dsr2UF)
add_subdirectory (Dsr2Vol)
add_subdirectory (File2Dsr)
add_subdirectory (Fmq2Fmq)
add_subdirectory (Fmq2MultMsgFmq)
add_subdirectory (FmqMon)
add_subdirectory (FmqTest)
add_subdirectory (GenPt2Spdb)
add_subdirectory (Hiq2Dsr)
add_subdirectory (InputWatcher)
add_subdirectory (Janitor)
add_subdirectory (LdataInformer)
add_subdirectory (LdataWatcher)
add_subdirectory (LdataMultWatcher)
add_subdirectory (LdataWriter)
add_subdirectory (Mdv2Dsr)
add_subdirectory (MultMsgFmq2Fmq)
add_subdirectory (RadMon)
add_subdirectory (Rapic2Dsr)
add_subdirectory (Scout)
add_subdirectory (Socket2File)
add_subdirectory (Test2Dsr)
add_subdirectory (TimeCalc)
add_subdirectory (VerifyGrid)
add_subdirectory (fmq_print)
add_subdirectory (ltgSpdb2GenPt)
add_subdirectory (nprtLtg2Spdb)
add_subdirectory (wsi_ingest)
add_subdirectory (wsim2mdv)

