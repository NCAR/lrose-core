###############################################################
#
# CMakeLists.txt - auto generated from Makefile
#
# scripts dir: lrose-core/codebase/apps/scripts/src
#
# written by script createCMakeLists.lib.py
#
###############################################################

project (scripts)

# install

install(PROGRAMS RUNtests.sh DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS TimeCmd.py DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS copyright_prepend DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS copyright_remove DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS ipcrm_linux DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS ldd_smart DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS loop_cmd DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS make_cdrom DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS make_cdrom2 DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS make_dvd+rw DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS make_jar DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS make_unix_cdrom DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS not_running DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS nuke_ipcs.linux DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS process_maps.csh DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS psinfo2 DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS pull_files_ftp.py DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS push_files_ftp.py DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS push_spdb_files_ftp.py DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS rm_baks DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS rm_CMakeLists DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS rm_core DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS rm_junk DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS rm_ldata_fmq_files DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS rm_makefile DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS rm_makefile.am DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS rm_objs DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS rm_wspace DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS rm_wspace_all DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS running DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS scrub DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS set_font_path DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS snuff DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS hup DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS snuff_slow DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS snuff_inst DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS snuff_usr1 DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS sync2files DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS utime_decode DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS auto_restart DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS auto_restart_stats DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS DataMapper_list_check DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS procmap_list_kill DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS procmap_list_check DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS procmap_list_start DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(PROGRAMS start_inst DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

