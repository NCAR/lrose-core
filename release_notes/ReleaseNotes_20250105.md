# Release notes - 20250105 - Colette

This is the final version of the Colette release.

These release notes are a summary of git commit notes since the 20240525 release.

NOTE - this is still under development.

## HawkEdit

* HawkEdit changes for save file bug; compiles and working; issue #69 from lrose-HawkEdit repo

## LUCID
* apps/radar/Lucid - working on MetRecord
* apps/radar/Lucid - in MetRecord, adding readH in QThread
* apps/radar/Lucid - making Lucid and MetRecord objects QObjects
* apps/radar/Lucid/MetRecord - working on data retrievals
* apps/radar/Lucid - Params::data_gather_mode to enum
* apps/radar/Lucid/MetRecord - adding _getBoundingBox()
* apps/radar/Lucid/MetRecord - adding mutex for checking status
* apps/radar/Lucid - adding valid() and new() to MetRecord class
* apps/radar/Lucid - adding WayPts class
* apps/radar/Lucid - adding ZoomBox class
* apps/radar/Lucid - working on MetRecord class, for data retrievals
* apps/radar/Lucid - met_record_t -> MetRecord class
* apps/radar/Lucid - testing data retrievals
* apps/radar/Lucid - working on intelligent data retrieval and rendering
* apps/radar/Lucid - working on DataHandling.cc
* Lucid - working on forcing render when selection changes
* apps/radar/Lucid - make win_param_t a class instead of a struct
* apps/radar/Lucid - testing CIDD.precip.latlon
* apps/radar/Lucid CartManager.cc - initializing _archiveMode
* apps/radar/Lucid - making met_record_t a class
* apps/radar/Lucid - collating data routines into DataHandling.cc
* apps/radar/Lucid - collating data routines into DataHandling.cc
* apps/radar/Lucid - collating data routines into DataHandling.cc
* renaming apps/radar/Qucid -> apps/radar/Lucid
* apps/radar/LUCID - moving into deprecated
* apps/radar/Qucid - working on streamlining data retrieval
* apps/radar/Qucid - working on regular grid display
* apps/radar/Qucid - working on data grid rendering
* apps/radar/Qucid - rendering horiz rect grid
* iapps/radar/Qucid - making HorizWidget and VertWidget classes stand-alone, instead of inheriting from CartWidget
* apps/radar/Qucid - testing data retrieval
* apps/radar/Qucid - adding symprodInit()
* apps/radar/Qucid - working on X11 to Qt conversion
* apps/radar/Qucid - working on X11 to Qt conversion
* apps/radar/Qucid - working on rendering
* apps/radar/Qucid - working on rendering
* apps/radar/Qucid - working on Drawable xid -> QPaintDevice qpd
* apps/radar/Qucid - working on data retrievals
* apps/radar/Qucid - working on data rendering
* apps/radar/Qucid - Zoom back action moved into zoom menu
* apps/radar/Qucid - working on vertical selector
* apps/radar/Qucid - testing window size and placement
* apps/radar/Qucid - working on vert level selector
* apps/radar/Qucid - adding VlevelManager class
* apps/radar/Qucid - adding VlevelManager class
* apps/radar/Qucid - debugging drawing maps
* apps/radar/Qucid - testing map plotting
* apps/radar/Qucid - working on window placement on desktop
* apps/radar/Qucid - working on window placement on desktop
* apps/radar/Qucid - testing map rendering
* apps/radar/Qucid - testing map rendering
* apps/radar/Qucid - working on map rendering
* apps/radar/Qucid - working on map rendering
* apps/radar/Qucid - working on map overlay code
* apps/radar/Qucid - moving calc_local_over_coords() into cidd_init.cc
* apps/radar/Qucid Overlay_t -> MapOverlay_t, cidd_overlays.h -> cidd_maps.h
* apps/radar/Qucid - initializing projection
* apps/radar/Qucid - fixing init zoom state from zoom labels
* apps/radar/Qucid - setting aspect for latlon
* apps/radar/Qucid - setting aspect for latlon
* apps/radar/Qucid - working on saved zooms
* Adding testing/Qucid.precip.test
* apps/radar/Qucid - getting zoom level from menu
* apps/radar/Qucid - working on resizing
* apps/radar/Qucid - testing color scale rendering
* apps/radar/Qucid - adjusting x,y pixel scales
* apps/radar/Qucid - testing resize window
* apps/radar/Qucid - initializing WorldPlot
* apps/radar/Qucid - working on full canvas rendering
* apps/radar/Qucid - working on rendering
* apps/radar/Qucid - working on setting zoom states
* apps/radar/Qucid - testing with data set from PRECIP
* apps/radar/Qucid - removing simulation parameters
* apps/radar/Qucid - working on WorldPlot upgrade
* apps/radar/Qucid - testing retrieval of station_location file in cidd_init
* apps/radar/Qucid - testing map file retrieval
* apps/radar/Qucid - working on reading in color scales for met records
* apps/radar/Qucid - working on color scale retieval
* apps/radar/Qucid - working on snprintf warnings
* Qucid - working on reading color scales



## Airborne radar calculations and corrections


## Radx Applications

* Leosphere format update (#134) Update for recent leosphere file format changes: sweep group attribute res_id is now an int attribute, not a string attribute. Catch exception of trying to convert an int to a string, and instead get its value as an int and then convert to string.  Fix degree symbols.   Update wording on combineRHI errors: change error to warning when trying to combine RHI on non RHI files, since file processing still works apart from combining RHI, make wording on multiple sweeps error a little clearer.
* apps/Radx/RadxDwellCombine - for realtime FMQ mode, upgrading to use IwrfMomReader
* (a) libs/Radx - in RadxVol, adding option to set the sweep mode labels. (b) In apps/Radx/RadxConvert - adding option to override sweep mode labels in file name
* libs/Radx/NcfRadxFile: (a) r_calib_time variable - setting units to UTC. (b) time variable - replacing spaces with underscores in long name. (c) azimuth variable: making the long name the standard name, and creating new long name. (d) elevation variable: making the long name the standard name and creating a new long name
* apps/Radx/RadxConvert - adding param to override scan name
* (a) libs/Radx - adding NOP4 to NexradLoc. (b) In IwrfTsInfo, setting NEXRAD radar location if possible
* libs/Radx/src/include/Radx/RadxField.hh, libs/Radx/src/include/Radx/RadxVol.hh - changing val of maxFractionMissing from 0.25 to 0.66
* Radx/src/RadxDwellCombine/RadxDwellCombine.cc, radar/src/HcrShortLongCombine/HcrShortLongCombine.cc - adding use of parameter dwell_stats_max_fraction_missing
* apps/Radx/RadxStats - for print_fixed_angle_table, only include sweep with nrays > 1
* libs/Radx - updating reading of Gematronik files
* libs/Radx/GemRadxFile - adding SNR2 noise-subtracted fields



## Bug fixes

* apps/radar/HcrShortLongCombine - testing archive mode
* apps/radar/HcrShortLongCombine - fixing problem with applying vel corrections twice
* libs/Mdvx - MdvxWrite.cc - when determining whether to use NetCDF vs old MDV format, extend date from 2025 to 2038
* Changing so that input fields are already qualified with short and long
* libs/Mdv/src/Mdvx/Mdvx_xml.cc - fixing compression, using requested_compression
* libs/Mdv - for Mdvx_write as XML, fixing problem with not writing XML, defaulting to NC
* modified the paxi command in _runSurveilance to use the loop command to run  the specified scan twice. This was necessary to meet the test to trigger an  end of a volume.
* libs/tdrp/load.c - fixing bug in substituting env vars
* libs/tdrp: (a) in print.c, adjusting the print format for double types to %.9g. (b) in load.c, adjusting buffer sizes to avoid warnings about sprintf buffer overruns
* libs/tdrp/load.c - fixing snprintf which was overwriting
* libs/radar/IwrfMomReader - fixing bug in file version of getNextRay()
* apps/Radx/Dsr2Radx - updating platform before writing vol
* apps/radar/HcrShortLongCombine.cc - adding check for time gate, restart processing if ray time gaps exceeds the dwell time significantly
* apps/radar/src/HcrShortLongCombine/HcrShortLongCombine.cc - fixing debug prints
* apps - fixing errors wrt const char*
* libs/qtplot, apps/radar Qt apps - fixing bug in ColorMap class causing infinite recursion when setMap() method is called
* libs/qtplot/ColorMap.cc (a) fixing bug in setMap() causing recursion and (b) modifying isColorNameValid() to use QColor::isValidColorName()

## Updates to applications and associated libraries

* apps/radar/Ts2Moments/Cmd.cc - removing C code versions, see Iq2Dsr
* apps/radar/Ts2Moments - adding PMU() call to BeamReader
* apps/radar/Ts2Moments - adding checkFixedPulseWidth()
* apps/radar/Ts2Moments/BeamReader.cc - checkIsStaggeredPrt() - checking for NULL for pulse0 or pulse1 before dereferencing
* apps/radar/Ts2Moments - params docs
* apps/radar/Ts2Moments - cleaning up paramdef file
* apps/radar/Ts2Moments - removing FMQ mode, making it REALTIME
* apps/radar/Ts2Moments - removing SpectraPrint
* adding apps/radar/Ts2Moments, a copy of Iq2Dsr. This will be cleaned up to remove unused legacy complexity
* apps/radar/HcrShortLongCombine - setting output fmq to allow multiple writers
* apps/radar/HcrShortLongCombine - adding fixed_location_mode instead of override_radar_location
* apps/radar/HcrShortLongCombine - setting output times to rounded mid time
* apps/radar/HcrShortLongCombine - adding option to override radar location for ground-based ops
* apps/radar/HcrShortLongCombine - adding mode to compute mean posn from georef data
* apps/radar/Iq2Dsr - adding param for georef_fixed_location_mode, for HCR in ground-based ops
* apps/radar/Iq2Dsr - working on dual-prt dwell processing for HCR
* apps/radar/Iq2Dsr - working on dwells with dual pulse widths
* apps/radar/src/deprecated/LUCID_shiva - adding ColorMap class to directory, in case this version is needed
* apps/radar/HawkEye class FieldColorController adding _isValidColorName() method to support qt5 and qt6
* .gitignore - adding XpolScanControl app binary
* Changing field info h_date and v_date from UTIMstruct to DateTime class
* cidd_structs - changing structs to classes where applicable
* apps/radar/HawkEye/FieldColorController.cc - isValidColor() -> isValidColorName()
* titan apps - changing legacy apps to compile under C++
* apps/radar/titan - modifying legacy apps test_tserver, track_print, verify_day, verify_grid and verify_tracks to compile as C++ instead of C
* apps/radar/titan - modifying legacy apps test_tserver, track_print, verify_day, verify_grid and verify_tracks to compile as C++ instead of C
ap.ps/titan/storms_to_ascii, storms_to_tifs - converting to C++ build
* apps/titan/grid_forecast - converting to compile as C++ app
* apps/procmap/src - procmap_register, procmap_unregister, test_procmap - converting C to C++
* apps/ingest/Mesonet2Spdb MetarCsv2Spdb UAEMesonet2Spdb: fixing link lines for cmake build
* apps/didss/wsi_ingest - upgrading to C++
* apps/didss/src/wsi_ingest - converting C to C++ for build
* apps/didss/src/fmq_print - converting C to C++ for build
* C app -> C++ app
* Adding apps ingest Mesonet2Spdb, MetarCsv2Spdb, Taf2Spdb, UAEMesonet2Spdb
* apps/ingest - adding Mesonet2Spdb, MetarCsv2SPdb, Taf2Spdb and UAEMesonet2Spdb to the build
* apps/radar/EccoStats - testing with various coverage fractions
* apps/radar/EccoStats - adding check on radar coverage
* apps/radar/EccoStats - adding option to censor based on the coverage height fraction
* apps/radar/EccoStats - adding coverage computations
* apps/radar/EccoStats - working on adding computation of coverage fields for MRMS grid
* apps/radar/EccoStats - fixing file output times
* apps/radar/EccoStats - adding frac fields
* apps/radar/EccoStats - debugging terrain ht and water flag fields
* apps/radar/src/EccoStats - development in progress - adding month range option
* apps/radar/HcrShortLongCombine - in Args.cc changing sprintf to snprintf
* apps/radar/HcrShortLongCombine - adding correction for platform vertical velocity
* Adding ls_poly_template.hh to libs/rapmath and libs/radar/regrFiltHubbert - template for forsythe polynomials
* libs/toolsa - cleaning up code for sprintf and strncpy, to ensure no array overruns
* libs/toolsa/DateTime class - adding setToZero()
* libs/euclid/PjgMath - fixing bug in operator(==) for latlon projection
* libs/qtplot/ColorMap - fixing bug in copy constructor
* libs/qtplot/ColorMap - adding isColorNameValid() method
* libs/toolsa - umalloc - in free2 and free3, extra checking for nulls
* libs/rapmath/umath - adding ForsytheC.cc, a C++ implementation of the Forsythe algorithm, as documentation
* libs/didss/DsInputPath - adding option to set month range for archive mode

## Builds - general

* commenting out autoconf option
* Moving autoconf build scripts to deprecated
* codebase/apps/ingest/src/_makefiles/makefile.lrose-core, codebase/apps/radar/src/_makefiles/makefile.lrose-core - adding new apps
* Update cmake build script to work on alma9 (#135)
* OSX updating qt6 homebrew location
* lrose_make.LINUX_LROSE_WITH_JASPER - updating QT6 includes
* Adding cmake files for apps/radar/src/HcrShortLongCombine
* build/make_include/lrose_make.LINUX_QT5 - setting c++ std to 11
* build/make_include/lrose_make.LINUX_NRIT
* Adding lrose_make.LINUX_NRIT
* createCMakeFiles.py - adding use of NO_SYSTEM_ENVIRONMENT_PATH for qt6 search
* build/cmake/createCMakeLists.py - in addFindQt, ignoring OpenG
* build/cmake/createCMakeLists.py - adding top-level CMakeLists.txt file, build should be done from this level
* build/cmake/createCMakeLists.py - adding add_custom_command section to build tdrp_gen if needed
* build/deprecated/singularity/custom.centos.8.def - changing name to be valid on windows platform
* clean up handling of tdrp_gen during build
    Creation of Params.hh and Params.cc files during build no longer
    requires an installed version of tdrp_gen. Instead, the tdrp_gen
    created in the build tree is now explicitly used if Params files must 
    be generated while building.
    This means that tdrp_gen is now never installed unless the user 
    runs "make install".


## Builds - updating to support conda forge packaging
* cmakefiles - adding code to not check for opengl in mac build for conda forge
* docs on miniforge build
* conda-build - bringing CMakeLists back to previous
* Mac OSX test for conda build. Commenting out find_path() for qt6
* codebase/CMakeLists.txt - adding messages to print out CONDA build variables
* codebase/CMakeLists.txt - testing options for qt6 build under conda-forge on OSX
* build/cmake/createCMakeLists.py - for MAMBA_BUILD do not search for packages
* createCMakeLists.py - moving from mambaBuild command line arg to MAMBA_BUILD variable
* docs/build/LROSE_build_main_page.md - adding link to miniforge build in bootstrap
* createCMakeLists.py -adding /usr/local/lib to ignore dir for mamba build
* build/cmake/createCMakeLists.py - for mambe build adding QT_HOST_PATH
* mamba build docs
* cmake - fixing createCMakeLists.py so that for the mamba build the system libraries are excluded
* build/cmake/createCMakeLists.py - updating for QT in mamba build
* cmake build for mamba - converting apps from C to C++: procmap_register, procmap_unregister, test_procmap, bearing2latlon
* cmake build - testing changes to use with mamba for conda-forge
* build/cmake/createCMakeLists.py - adding commented out option for MAMBA forge



## Documentation
* Adding docs/runtime/env_vars.md
* updating docs/runtime/env_vars.html
* Updating LROSE_DOCS_OVERVIEW.md
* Fixing broken link -- LROSE_DOCS_OVERVIEW.m
* LROSE_DOCS_OVERVIEW.md - fixing typo
* docs/build/mambaforge/chatgpt_condaforge_build.md - adding test details for meta.yaml
* Adding docs/build/mambaforge/chatgpt_condaforge_build.md
* Adding docs/build/mambaforge/perform_mamba_build.md



