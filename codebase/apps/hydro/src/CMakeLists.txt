###############################################
#
# CMakeLists for cmake recursion
#
# dir: /home/dixon/git/lrose-core/codebase/apps/hydro/src
#
# written by script createCMakeLists.py
#
# created 2021-02-12 14:00:33.269582
#
###############################################

project (LROSE-CORE)

# subdirectories

add_subdirectory (BasinGenPt2Symprod)
add_subdirectory (BasinPrecip)
add_subdirectory (BasinPrecipSpdb2Xml)
add_subdirectory (PrecipAccum)
add_subdirectory (RainGage)
add_subdirectory (RateAccum)
add_subdirectory (Shape2Map)

