###############################################
#
# CMakeLists for cmake recursion
#
# dir: /home/dixon/git/lrose-core/codebase/apps/filters/src
#
# written by script createCMakeLists.py
#
# created 2021-02-12 14:00:33.261703
#
###############################################

project (LROSE-CORE)

# subdirectories

add_subdirectory (ApRemoval)
add_subdirectory (BrightBand)
add_subdirectory (ClutterRemove)
add_subdirectory (LogFilter)
add_subdirectory (NoiseFilter)
add_subdirectory (stratiform_filter)
add_subdirectory (tops_filter)

