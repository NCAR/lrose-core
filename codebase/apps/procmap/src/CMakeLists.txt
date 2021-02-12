###############################################
#
# CMakeLists for cmake recursion
#
# dir: /home/dixon/git/lrose-core/codebase/apps/procmap/src
#
# written by script createCMakeLists.py
#
# created 2021-02-12 14:00:33.044878
#
###############################################

project (LROSE-CORE)

# subdirectories

add_subdirectory (print_procmap)
add_subdirectory (procmap)
add_subdirectory (procmap_register)
add_subdirectory (procmap_unregister)
add_subdirectory (test_procmap)

