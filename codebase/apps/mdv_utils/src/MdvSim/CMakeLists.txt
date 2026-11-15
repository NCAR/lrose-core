project (MdvSim)

set (SRCS
     Params.cc
     MdvSim.cc
     Args.cc
     Main.cc
    )

include_directories (../../../../libs/Mdv/src/include)
include_directories (../../../../libs/Radx/src/include)
include_directories (../../../../libs/dataport/src/include)
include_directories (../../../../libs/didss/src/include)
include_directories (../../../../libs/dsserver/src/include)
include_directories (../../../../libs/tdrp/src/include)
include_directories (../../../../libs/toolsa/src/include)
include_directories (${CMAKE_INSTALL_PREFIX}/include)

link_directories (${CMAKE_INSTALL_PREFIX}/lib)

link_libraries (Mdv rapformats dsserver didss euclid rapmath toolsa)
link_libraries (dataport tdrp Radx Ncxx physics pthread netcdf)
link_libraries (hdf5_hl hdf5 bz2 z)

makeTdrpParams()

add_executable (MdvSim ${SRCS})
add_dependencies (${PROJECT_NAME} tdrp_gen)

install (TARGETS MdvSim RUNTIME DESTINATION bin)
