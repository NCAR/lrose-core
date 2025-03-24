###############################################################
#
# CMakeLists for cmake recursion
#
# dir: lrose-core/codebase/apps/procmap/src
#
# written by script createCMakeLists.py
#
###############################################################

project (lrose-core)

# subdirectories

add_subdirectory (print_procmap)
add_subdirectory (procmap)
add_subdirectory (procmap_register)
add_subdirectory (procmap_unregister)
add_subdirectory (test_procmap)

