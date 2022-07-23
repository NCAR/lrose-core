# General configuration for LROSE package

set ( LROSE_PREFIX $ENV{LROSE_INSTALL_DIR} )

# If not, use the default location
if (NOT LROSE_PREFIX)
  set ( LROSE_PREFIX "/usr/local/lrose" )
endif ( NOT LROSE_PREFIX )

set ( LROSE_INCLUDE_DIRS ${LROSE_PREFIX}/include )
set ( LROSE_LIB_DIR ${LROSE_PREFIX}/lib )
set ( LROSE_BIN_DIR ${LROSE_PREFIX}/bin )
set ( LROSE_LINK_FLAGS -L${LROSE_LIB_DIR} )

set ( LROSE_LIBRARIES
        Refract
        FiltAlg
        dsdata
        radar
        hydro
        titan
        Fmq
        Spdb
        Mdv
        advect
        rapplot
        qtplot
        Radx
        Ncxx
        rapformats
        dsserver
        didss
        grib
        grib2
        contour
        euclid
        rapmath
        kd
        physics
        toolsa
        dataport
        tdrp
        netcdf
        hdf5
        z
        bz2
        pthread
        fftw3
        )

set ( TDRP_EXECUTABLE ${LROSE_BIN_DIR}/tdrp_gen )

# Function for creating TDRP Params.cc and Params.hh files

function(makeTdrpParams)

  # Add a custom generator for TDRP Params.cc and Params.hh files
  # from their associated paramdef.<app> file

  find_program(TDRP_EXECUTABLE tdrp_gen PATHS ${LROSE_PREFIX}/bin /usr/local/lrose/bin)
  
  add_custom_command (
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/Params.hh ${CMAKE_CURRENT_SOURCE_DIR}/Params.cc
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/paramdef.${PROJECT_NAME}
    COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && ${TDRP_EXECUTABLE}
    -c++
    -f paramdef.${PROJECT_NAME}
    -prog ${PROJECT_NAME}
    -add_ncar_copyright
    COMMENT "Generating/updating Params.hh and Params.cc for ${PROJECT_NAME}"
    )

endFunction()

message ( STATUS "lrose-config ======== start ========" )
message ( STATUS "lrose-config: PROJECT_NAME: ${PROJECT_NAME}" )
message ( STATUS "lrose-config: CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}" )
message ( STATUS "lrose-config: LROSE_PREFIX: ${LROSE_PREFIX}" )
message ( STATUS "lrose-config: LROSE_INCLUDE_DIRS: ${LROSE_INCLUDE_DIRS}" )
message ( STATUS "lrose-config: LROSE_LIB_DIR: ${LROSE_LIB_DIR}" )
message ( STATUS "lrose-config: LROSE_BIN_DIR:${LROSE_BIN_DIR}" )
message ( STATUS "lrose-config: LROSE_LINK_FLAGS: ${LROSE_LINK_FLAGS}" )
message ( STATUS "lrose-config: LROSE_LIBRARIES: ${LROSE_LIBRARIES}" )
message ( STATUS "lrose-config: TDRP_EXECUTABLE: ${TDRP_EXECUTABLE}" )

