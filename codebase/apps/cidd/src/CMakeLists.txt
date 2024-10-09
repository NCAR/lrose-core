###############################################################
#
# CMakeLists for cmake recursion
#
# dir: lrose-core/codebase/apps/cidd/src
#
# written by script createCMakeLists.py
#
###############################################################

project (lrose-core)

# subdirectories

add_subdirectory (fix_map)
add_subdirectory (MapCircles)
add_subdirectory (MapGrid)
add_subdirectory (OverlayCreate)
add_subdirectory (toneDownColorscale)

