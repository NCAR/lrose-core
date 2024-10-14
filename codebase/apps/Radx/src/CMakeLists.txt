###############################################################
#
# CMakeLists for cmake recursion
#
# dir: lrose-core/codebase/apps/Radx/src
#
# written by script createCMakeLists.py
#
###############################################################

project (lrose-core)

# subdirectories

add_subdirectory (Ascii2Radx)
add_subdirectory (Dsr2Radx)
add_subdirectory (FixCfradialPaths)
add_subdirectory (FixFieldVals)
add_subdirectory (FixRadxPointing)
add_subdirectory (FixRadxValues)
add_subdirectory (Radx2Dsr)
add_subdirectory (Radx2Grid)
add_subdirectory (RadxBeamBlock)
add_subdirectory (RadxBufr)
add_subdirectory (RadxCalUpdate)
add_subdirectory (RadxCheck)
add_subdirectory (RadxClutMon)
add_subdirectory (RadxClutter)
add_subdirectory (RadxConvert)
add_subdirectory (RadxCov2Mom)
add_subdirectory (RadxCreateVol)
add_subdirectory (RadxDiffFields)
add_subdirectory (RadxDiffVol)
add_subdirectory (RadxDwellCombine)
add_subdirectory (RadxEvad)
add_subdirectory (RadxFilter)
add_subdirectory (RadxHca)
add_subdirectory (RadxKdp)
add_subdirectory (RadxMergeFields)
add_subdirectory (RadxMergeVols)
add_subdirectory (RadxMesoCyclone)
add_subdirectory (RadxModelQc)
add_subdirectory (RadxMon)
add_subdirectory (RadxPartRain)
add_subdirectory (RadxPid)
add_subdirectory (RadxPrint)
add_subdirectory (RadxQc)
add_subdirectory (RadxQpe)
add_subdirectory (RadxRate)
add_subdirectory (RadxSunMon)
add_subdirectory (RadxStats)
add_subdirectory (RadxTimeStats)
add_subdirectory (RadxVolTimingStats)

