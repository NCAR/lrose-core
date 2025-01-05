# Release notes - 20250105 - Colette

This is the final version of the Colette release.

These release notes are a summary of git commit notes since the 20240525 release.

NOTE - this is still under development.

## HawkEdit

* updated the autocomplete text for script commands
* Added changes to run on Linux; display on Mac

## Airborne radar calculations and corrections

* Added Michael Bell's airborne radar QC navigation code (Cai et. al.)  This is a conversion from Fortran to C/C++.
* Cai et al. adding a Makefile_save to record the additional flags and paths in order to compile the fortran code on a Mac
* Cai et al - adding a note about how to find the -lSystem library

## Radx Applications

* Halo Photonics --  Added support for HaloPhotonics lidar file format; debug lines only print if verbose command line option is specified.
* apps/Radx/RadxConvert - adding param combine_sweeps_by_fixed_angle
* libs/Radx/RadxVol - adding method combineSweepsByFixedAngle()
* libs/Ncxx/Nc3File and Nc3Values - adding support for unsigned short
* apps/Radx/RadxVolTimingStats - adding height MSL output field

## Updates to applications and associated libraries

* apps/radar/Sprite - adding CMD_FRAC plotting for wind turbine clutter detect
* apps/radar/HawkEye - adding beamHt computation for PolarManager
* libs/radar/DwellSpectra - setting cmdThresholdDetect default from 0.9 to 0.8
* apps/radar/Iq2Dsr - testing spec cmd detection for wind turbine clutter
* radar/src/Iq2Dsr, radar/src/Sprite - adding wind turbine clutter detection
* rename apps/ingest/EraNc2Mdv -> EraGrib2Mdv
* adding apps/ingest/src/Era5Nc2Mdv
* apps/radar/Ts2NetCDF - updating for all xmit/rcv pulsing schemes
* apps/radar/Iq2DSr - adding rhohv test to CMD
* libs/radar/RadarMoments.cc - adding member _isStagPrt
* libs/radar/MomentsFields, apps/radar/Iq2Dsr/Cmd - adding rhohv test to output stream
* libs/radar/RadarMoments: remove deprecated methods related to staggered PRT filtering
* libs/radar/MomentsFields, apps/radar/Iq2Dsr - testing and improving application of rhohv test. Adding separate thresholds, for power, vel, phase and rho fields
* apps/radar/Iq2Dsr - regression filtering with rhohv test
* libs/Radx, apps/Radx/Dsr2Radx - adding option in RadxFile to add the fixed angle to the output file name on write
* apps/radar/Iq2Dsr - in _cleanUp(), adding test for number of iterations to prevent infinite loop
* apps/radar/Ecco - testing terrain ht
* apps/radar/Iq2Dsr - adding vel folding attr to output
* apps/radar/TsCalAuto - fixing compiler warnings from fgets() from stdin for operator input
* adding app radar/HcrShortLongCombine
* libs/Radx/RadxTime - adding constructor with double, plus boolean to disambiguate
* apps/radar/HcrShortLongCombine - adding vel unfolding
* libs/Radx/GemRadxFile - checking better for number of rays and gates per sweep, ensuring these are the same for all fields. Remove fields that do not conform to the max
* apps/Radx/RadxDwellCombine - cleaning up paramdef and Args
* apps/radar/src/HcrShortLongCombine - cleaning up parameters
* libs/radar/IwrfMomReader - adding archive mode
* apps/radar/src/Ts2NetCDF - working on support for single channel transmit, and single pol
* libs/didss/DataFileNames/DataFileNames.cc - in getDataTime(), checking for yyyymmddhh_yyyymmddhh
* apps/Radx/src/RadxQpe - in Geom.cc constructor, computing _output_na from angle res, instead of setting explicitly
* Adding parameters to constrain input sweeps by elevation angle
* apps/radar/src/Ts2NetCDF - adding writing of georef vars if available
* apps/radar/src/Ts2NetCDF - added ARCHIVE mode with start and end times
* libs/Radx/RadxVol, apps/Radx/RadxConvert - adding options to override convention, subconvention and height_agl
* toolsa/DateTime, Radx/RadxTime - adding setToNever() method
* apps/tdrp/TdrpTest - adding modify option
* libs/tdrp/mem.c - fixed realloc bug for arrays. val_offset was being used instead of array_offset
* apps/tdrp/TdrpTest - testing for array realloc bug, improving param mods
* libs/tdrp/find_item.c - fixing bad print
* libs/radar - changing constructor API for IwrftsReaderFile to be less ambiguous
* libs/euclid/GeographicLib - changing license attribution from MIT/X11 to MIT, to avoid X11 dependency
* Cleaning up references to X11 in places that do not need X11
* apps/radar/Ecco - adding terrain height
* apps/radar/Mpd2Radx - testing
* libs/tdrp - adding tdrpCheckArgAndWarn(). apps/radar - using tdrpCheckArgAndWarn()
* apps/radar/Iq2Dsr/Beam.cc - adding _copyVelToCorrectedVel() method
* apps/radar/Iq2Dsr/Cmd.cc - adding thresholds to rhohv_test
* apps/radar/HcrShortLongCombine - setting nyquist for combined rays
* apps/radar/TsCalAuto - adding param prompt_user_with_attenuation

## Bug fixes

* Fix RayGeom for one ray.  Use default constructor for RayGeom when creating the blank default, since this still initializes start height and gate spacing to 0, but also sets ray count to 0 (instead of 1). This prevents returning the blank default RayGeom object when processing a file with only 1 ray.
* Leosphere data files:  Fix combineRhi()  (#128) (#104)
* Fix build scripts. Since this script is in lrose-core/build/scripts, go back up by two directories to get to lrose/codebase. (#125)
* HawkEye: Fix RHI segfault (#129)
* Alma Linux 9: Merge pull request (#124) from NCAR/fix-alma9-packages
* TerrainHtServer:  Code cleanup, include in HCR build (#121) 

## Builds - updating to support QT6:

* build/make_include - support for qt5 + qt6
* build/make_include/lrose_make_qt_targets - commenting out definition of QT_INCLUDE. This is now in lrose_make.host_os files.
* for createCMakeLists.py, adding check for needX11 and needQt
* createCMakeLists.py - adding needX11 as well as needQt at global level, so as not to require X11 or Qt for samurai build
* qt6 port - cmake - upgrading compiler flags to c++17
* cmake qt5 -> qt6 for mac
* docs/build/lrose_package_dependencies.linux.md - rhel 9
* build/cmake/createCMakeLists.py - adding check to determine which Qt version is installed
* cmake build - checking for qt version
* Generalizing cmake to support both QT5 and QT6
* Upgrading cmake to handle multiple qt versions - qt5 and qt6
* Fixing QT bug in createCMakeLists.py
