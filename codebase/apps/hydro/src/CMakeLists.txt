###############################################################
#
# CMakeLists for cmake recursion
#
# dir: lrose-core/codebase/apps/hydro/src
#
# written by script createCMakeLists.py
#
###############################################################

project (LROSE-CORE)

# subdirectories

add_subdirectory (BasinGenPt2Symprod)
add_subdirectory (BasinPrecip)
add_subdirectory (BasinPrecipSpdb2Xml)
add_subdirectory (PrecipAccum)
add_subdirectory (RainGage)
add_subdirectory (RateAccum)
add_subdirectory (Shape2Map)

