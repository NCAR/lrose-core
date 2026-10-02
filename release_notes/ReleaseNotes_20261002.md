# New applications

* Adding apps/Radx/RadxCartDP - Cartesian dual-polarization radar processing, including PID, precipitation rate, QPE, convective/stratiform classification, model temperature data, beam blockage, and derived DP fields
* Adding apps/Radx/CartBeamBlock - compute 3D radar beam blockage using terrain elevation data and radar beam power patterns
* Adding apps/radar/HcrTs2Moments - compute radar moments from HCR time-series data, including support for multiple operating blocks and calibration switching
* Adding apps/radar/HcrMomentsCombine - combine HCR moments from multiple operating modes
* Adding apps/radar/TsUdp2Fmq - receive radar time-series data over UDP and write to FMQ
* Adding apps/Radx/Radx2Awips
* Adding apps/radar/RadarCal/RFStrengthPlot.py - plotting support for radar calibration and RF strength analysis

# Application updates

* apps/Radx/RadxCartDP - extensive development of Cartesian dual-pol processing
* apps/Radx/RadxCartDP - adding PID computation and PID mode filtering
* apps/Radx/RadxCartDP - adding precipitation-rate and QPE fields
* apps/Radx/RadxCartDP - adding convective/stratiform partitioning
* apps/Radx/RadxCartDP - adding interpolation of model data and computation of temperature profiles
* apps/Radx/RadxCartDP - adding beam blockage fields
* apps/Radx/RadxCartDP - adding template 3D grid support and improved control of output fields
* apps/Radx/RadxCartDP - adding range, height and radar coverage fields

* apps/Radx/CartBeamBlock - adding direct reading of SRTM DEM tiles
* apps/Radx/CartBeamBlock - adding beam power pattern and blockage calculations
* apps/Radx/CartBeamBlock - adding extinction calculations
* apps/Radx/CartBeamBlock - adding multithreading
* apps/Radx/CartBeamBlock - adding elevation field to output
* apps/Radx/CartBeamBlock - adding support for template 3D grids and copying DEM files used in processing

* apps/Radx/RadxKdp - extensive testing and updates for new KDP processing methods
* apps/Radx/RadxKdp - adding fixed-angle limits and additional testing configurations
* apps/Radx/RadxKdp - adding support for testing self-consistency and phase-shift-on-backscatter methods
* apps/Radx/RadxRate, RadxQc, RadxPid, RadxPartRain, RadxHca and RadxCov2Mom - updating to latest KdpFilt API

* apps/Radx/RadxConvert - working on support for WMO FM301 radar data
* apps/Radx/RadxMergeVols - fixing volume output accidentally disabled during previous testing
* apps/Radx/Radx2Grid - adding option to suppress selected fields from output
* apps/Radx/Radx2Grid and RadxCartDP - changing default minimum valid points for interpolation from 3 to 5
* apps/Radx/RadxRate - fixing null-field handling when adding PID debug fields

* apps/radar/HcrTs2Moments - adding handling of HCR operating blocks
* apps/radar/HcrTs2Moments - adding calibration switching based on operating mode
* apps/radar/HcrTs2Moments - adding cross-polarization moments
* apps/radar/HcrTs2Moments - adding block ID and block name to output rays
* apps/radar/HcrTs2Moments - improving error handling and archive-mode processing
* apps/radar/HcrMomentsCombine - development and testing of combined HCR moments processing

* apps/radar/Sprite - adding/testing time-series reflection (TSR) clutter filtering
* apps/radar/TsPrint - adding H/V identification for fast-alternating data when nSamples == 1
* apps/radar/Ts2Moments and Iq2Dsr - fixing scan-type checks in BeamReader

* apps/radar/HawkEye - adding optional filtering of rays by scan name

* apps/titan/Titan - adding option to obtain temperature profile directly from model data
* apps/titan/Titan - fixing memory leaks and removing unused parameters
* apps/titan/HailKE - fixing output field names and lowest-level processing
* apps/titan/HailKEswath - removing redundant parameters

* apps/refract - substantial refactoring of RefractCalib, RefractCompute and CalcMoisture
* apps/refract/RefractCalib - computing SNR from DBZ rather than power
* apps/refract/RefractCalib - adding output-path command-line option
* apps/refract/CalcNexrad renamed to NexradA1ToRefract
* apps/refract - adding RefractColorScales

* apps/titan/EsdAcIngest and apps/ingest/AcData2Spdb - detect TCP server disconnects and reconnect automatically
* apps/titan/EsdAcIngest - adding websocket-to-TCP bridge script
* apps/ingest/NWSsoundingIngest - adding get_sounding.py script
* apps/ingest/file_repeat_day - adding simulate_files_realtime.py

# Radar processing libraries

* libs/radar/KdpFilt - major update and expansion of KDP processing
* libs/radar/KdpFilt - adding FFT-based PHIDP filtering
* libs/radar/KdpFilt - adding weighted polynomial regression filtering
* libs/radar/KdpFilt - adding self-consistency methods
* libs/radar/KdpFilt - adding phase-shift-on-backscatter processing
* libs/radar/KdpFilt - adding iterative attenuation correction
* libs/radar/KdpFilt - adding PHIDP unfolding for FFT-filtered data
* libs/radar/KdpFilt - updating attenuation correction parameters for S, C and X bands
* libs/radar/KdpFilt - improved handling and censoring of invalid data

* libs/radar - adding KdpFirFilt for FIR low-pass filtering
* libs/radar - adding KdpQuadFit for polynomial fitting
* libs/radar - adding RadarFftDouble
* libs/radar/RadarFft - adding mutex protection because FFTW initialization and cleanup are not inherently thread-safe

* libs/radar/RadarMoments - adding time-series reflection (TSR) clutter filtering
* libs/radar/ClutFilter - development of TSR filtering using reflected time series
* libs/radar - adding RadarTimeSeriesSim for simulated radar time-series data

* libs/radar/NcarParticleId - updates supporting Cartesian PID computation, including SNR checking and PID preparation
* libs/radar/PrecipRate - refactoring and updates supporting Cartesian QPE
* libs/radar/ConvStratFinder - adding temperature-based height support and converting internal arrays to vectors

# Radx and MDV libraries

* libs/Radx - adding WMO FM301 support
* libs/Radx - adding FM301 string handling
* libs/Radx - adding `fill` sweep type
* libs/Radx/RadxRay - adding scan-name support to print output
* libs/Radx - fixes for Gematronik volume numbers and CfRadial2 latitude/longitude output

* libs/Mdv - adding MdvxRemapInterp for interpolation of model fields
* libs/Mdv - adding Mdvx_init functions for initialization of MDV structures
* libs/Mdv/MdvxField - adding writable access to volume data

# Radar time-series support

* libs/radar/IwrfTsPulse and IWRF headers - adding block information for radar operating modes
* libs/radar/IwrfTsPulse - adding accessors for start and end of operating blocks
* IWRF block terminology updated from block_num to block_id

# General library updates

* libs/euclid - adding EuclidAngle class
* libs/rapmath - adding NasaPolyFit
* libs/toolsa/TaFile - adding copyFileToDir()
* libs/toolsa/TaStr - adding integer, long and double to-string methods
* libs/toolsa/TaArray and TaArray2D - substantial refactoring
* libs/dsserver/DestUrlArray - improving handling of trailing newline characters
* libs/Fmq - fixing shared-memory permission flag in FmqDeviceShmem

# Code modernization and robustness

* Adding toolsa/safe_snprintf.hh
* Broad conversion of sprintf() and vsprintf() calls to safer snprintf(), safe_snprintf() and vsnprintf() implementations
* Applied safe string-formatting changes across toolsa, Radx, Mdv, Ncxx, Fmq, Spdb, didss, dsdata, dsserver, euclid and rapformats libraries
* Extensive cleanup of compiler warnings and potentially uninitialized variables across applications and libraries
* Fixed Main.cc cases where the static Prog pointer was inadvertently shadowed by a local variable
* Fixed duplicate SIGHUP signal handling in Main.cc implementations

# NetCDF, HDF5 and build system

* Updating CMake configuration to use current find_package() support for HDF5 and netCDF
* Adding HDF5 and netCDF include-directory handling to generated CMakeLists.txt files
* libs/Ncxx - compatibility updates for newer HDF5 APIs, including HDF5 2.x
* Adding build/cmake/run_cmake scripts
* Updating CMakeLists for new and modified applications
* Continued macOS build and Homebrew support improvements

# Documentation

* Updating top-level README installation and build instructions
* Updating package download and installation documentation
* Updating macOS Homebrew installation instructions
* Updating links to the LROSE Wiki and Quickstart Guide
* Updating CentOS references to AlmaLinux in build instructions
* Adding docs/apps/lrose_app_table.md
* Adding release installation documentation

