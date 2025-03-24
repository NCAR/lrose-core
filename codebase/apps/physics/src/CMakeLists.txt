###############################################################
#
# CMakeLists for cmake recursion
#
# dir: lrose-core/codebase/apps/physics/src
#
# written by script createCMakeLists.py
#
###############################################################

project (lrose-core)

# subdirectories

add_subdirectory (AverageFields)
add_subdirectory (CalcHumidity)
add_subdirectory (ModelDerive)
add_subdirectory (NegBuoyancy)
add_subdirectory (Pressure2Height)
add_subdirectory (ShearForLeadingEdge)
add_subdirectory (ThetaEAdvect)
add_subdirectory (TrendField)
add_subdirectory (VerticalAverage)
add_subdirectory (VerticalShear)

