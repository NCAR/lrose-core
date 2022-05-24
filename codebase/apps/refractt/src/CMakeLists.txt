###############################################################
#
# CMakeLists for cmake recursion
#
# dir: lrose-core/codebase/apps/refractt/src
#
# written by script createCMakeLists.py
#
###############################################################

project (LROSE-CORE)

# subdirectories

add_subdirectory (CalcMoisture)
add_subdirectory (CalcNexrad)
add_subdirectory (Chill2netCDFSweep)
add_subdirectory (CleanupRvp8Data)
add_subdirectory (DsrAddSnr)
add_subdirectory (MdvAddSnr)
add_subdirectory (PhaseDiff)
add_subdirectory (RefractCalib)
add_subdirectory (RefractCompute)
add_subdirectory (RefractParams2Tdrp)
add_subdirectory (RefrMerge)

