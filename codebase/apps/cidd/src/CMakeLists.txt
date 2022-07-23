###############################################################
#
# CMakeLists for cmake recursion
#
# dir: lrose-core/codebase/apps/cidd/src
#
# written by script createCMakeLists.py
#
###############################################################

project (LROSE-CORE)

# subdirectories

add_subdirectory (AsciiSelect)
add_subdirectory (CheckXDisplay)
add_subdirectory (CIDD)
add_subdirectory (CiddClick2GenPt)
add_subdirectory (cidd_client)
add_subdirectory (CiddParams2JazzXml)
add_subdirectory (create_colorscale)
add_subdirectory (CronusPointSelect)
add_subdirectory (cscale_edit)
add_subdirectory (DrawFmq2Bdry)
add_subdirectory (DrawFmq2MaskMdv)
add_subdirectory (DrawFmq2Polygon)
add_subdirectory (DrawFmqPolygonStats)
add_subdirectory (EdrSelect)
add_subdirectory (EventMan)
add_subdirectory (get_url)
add_subdirectory (GridPointSelect)
add_subdirectory (initDrawQueue)
add_subdirectory (inspectRawFmq)
add_subdirectory (jvizMaps2ciddMaps)
add_subdirectory (MetarSelect)
add_subdirectory (metar_strip)
add_subdirectory (notify_cidd)
add_subdirectory (pos_report)
add_subdirectory (RemoteUI2Fmq)
add_subdirectory (Shape2Map)
add_subdirectory (ShapeAscii2Map)
add_subdirectory (shapefile2Mapfile)
add_subdirectory (ShapeGridPt2Mdv)
add_subdirectory (SoundingChart)
add_subdirectory (SoundingText)
add_subdirectory (StationStripChart)
add_subdirectory (view_colormap)

