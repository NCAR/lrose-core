###############################################################
#
# CMakeLists for cmake recursion
#
# dir: lrose-core/codebase/apps/spol/src
#
# written by script createCMakeLists.py
#
###############################################################

project (LROSE-CORE)

# subdirectories

add_subdirectory (SpolAngles2Fmq)
add_subdirectory (SpolCal2Xml)
add_subdirectory (SpolSysconRelay)
add_subdirectory (SpolTsMonitor)
add_subdirectory (SpolTs2Fmq)

