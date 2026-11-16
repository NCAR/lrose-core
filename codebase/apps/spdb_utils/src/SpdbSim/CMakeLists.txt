project (SpdbSim)

set (SRCS
     Params.cc
     SpdbSim.cc
     PayloadRetimer.cc
     Args.cc
     Main.cc
    )

include_directories (../../../../libs/Spdb/src/include)
include_directories (../../../../libs/rapformats/src/include)
include_directories (../../../../libs/dataport/src/include)
include_directories (../../../../libs/didss/src/include)
include_directories (../../../../libs/dsserver/src/include)
include_directories (../../../../libs/tdrp/src/include)
include_directories (../../../../libs/toolsa/src/include)
include_directories (${CMAKE_INSTALL_PREFIX}/include)

link_directories (${CMAKE_INSTALL_PREFIX}/lib)
link_libraries (Spdb rapformats dsserver didss euclid rapmath toolsa dataport tdrp physics pthread)
link_libraries (netcdf hdf5_hl hdf5 bz2 z)

makeTdrpParams()

add_executable (SpdbSim ${SRCS})
add_dependencies (${PROJECT_NAME} tdrp_gen)

install (TARGETS SpdbSim RUNTIME DESTINATION bin)

