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
add_subdirectory (CiddParams2JazzXml)
add_subdirectory (MapCircles)
add_subdirectory (MapGrid)
add_subdirectory (Shape2Map)
add_subdirectory (ShapeAscii2Map)
add_subdirectory (ShapeGridPt2Mdv)
add_subdirectory (OverlayCreate)
add_subdirectory (shapefile2Mapfile)
add_subdirectory (toneDownColorscale)

