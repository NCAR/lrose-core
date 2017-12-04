# - Find UDUnits
# Find the UDUnits includes and library
#
#  UDUNITS_INCLUDE_DIR  - user modifiable choice of where netcdf headers are
#  UDUNITS_LIBRARY      - user modifiable choice of where netcdf libraries are

#search starting from user editable cache var
if (UDUNITS_INCLUDE_DIR AND UDUNITS_LIBRARY)
  # Already in cache, be silent
  set (UDUNITS_FIND_QUIETLY TRUE)
endif ()

# find the library
# first look where the user told us
if (UDUNITS_DIR)
  find_library (UDUNITS_LIBRARY NAMES udunits2
    PATHS "${UDUNITS_DIR}/lib" "${UDUNITS_DIR}/lib64"
    NO_DEFAULT_PATH)
endif()

# next look in LD_LIBRARY_PATH for libraries
find_library (UDUNITS_LIBRARY NAMES udunits2
  PATHS ENV LD_LIBRARY_PATH NO_DEFAULT_PATH)

# finally CMake can look
find_library (UDUNITS_LIBRARY NAMES udunits2)

mark_as_advanced (UDUNITS_LIBRARY)

# find the header
# first look where the user told us
if (UDUNITS_DIR)
  find_path (UDUNITS_INCLUDE_DIR udunits2.h
    PATHS "${UDUNITS_DIR}/include" NO_DEFAULT_PATH)
endif()

# then look relative to library dir
get_filename_component(UDUNITS_LIBRARY_DIR
  ${UDUNITS_LIBRARY} DIRECTORY)

find_path (UDUNITS_INCLUDE_DIR udunits2.h
  PATHS "${UDUNITS_LIBRARY_DIR}/../include"
  NO_DEFAULT_PATH)

# finally CMake can look
find_path (UDUNITS_INCLUDE_DIR udunits2.h)

mark_as_advanced (UDUNITS_INCLUDE_DIR)

# handle the QUIETLY and REQUIRED arguments and set UDUNITS_FOUND to TRUE if
# all listed variables are TRUE
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (UDUnits
  DEFAULT_MSG UDUNITS_LIBRARY UDUNITS_INCLUDE_DIR)
