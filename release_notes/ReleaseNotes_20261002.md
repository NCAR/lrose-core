# New applications

* **apps/Radx/RadxCartDP** - new application for creating Cartesian dual-polarization radar products. Computes PID, precipitation rate and QPE, convective/stratiform classification, and other derived DP fields on a 3D Cartesian grid. Supports model temperature data, terrain and radar beam blockage information, radar coverage fields, and template grids for coordinating output products.

* **apps/Radx/CartBeamBlock** - new application for computing 3D radar beam blockage from terrain elevation data. Reads SRTM DEM tiles, accounts for the radar beam power pattern, and computes blockage, extinction, terrain elevation and related fields on a Cartesian grid. Includes multithreaded processing and support for template grids.

* **apps/radar/HcrTs2Moments** - new application for computing radar moments from HCR time-series data. Supports multiple HCR operating blocks, calibration switching between operating modes, cross-polarization moments, and block metadata in the output rays.

* **apps/radar/HcrMomentsCombine** - new application for combining HCR moment data from multiple operating modes.

* **apps/radar/TsUdp2Fmq** - new application for receiving radar time-series data over UDP and writing the data to an FMQ.

* **apps/Radx/Radx2Awips** - new application for producing AWIPS-compatible radar products.

* **apps/radar/RadarCal/RFStrengthPlot.py** - new plotting support for radar calibration and RF signal-strength analysis.

# Major application updates

* **apps/Radx/RadxKdp** - updated to support the substantially revised KDP processing library, including regression, FIR and FFT-based PHIDP filtering, self-consistency processing, phase-shift-on-backscatter correction, attenuation correction and PHIDP unfolding. Added additional testing configurations and fixed-angle processing limits.

* **apps/Radx/RadxConvert** - adding support for the WMO FM301 radar data exchange format.

* **apps/radar/Sprite** - adding time-series reflection (TSR) clutter filtering and associated diagnostic displays.

* **apps/radar/HawkEye** - adding optional filtering of rays by scan name.

* **apps/titan/Titan** - adding the option to obtain the temperature profile directly from model data. Also includes memory-management improvements and removal of unused parameters.

* **apps/titan/HailKE and HailKEswath** - fixes and cleanup to hail kinetic-energy processing, including output field names, lowest-level handling and removal of redundant parameters.

* **apps/refract** - substantial updates and refactoring of the refractivity processing applications, including RefractCalib, RefractCompute and CalcMoisture. RefractCalib now supports SNR computed from DBZ and configurable output paths. CalcNexrad has been renamed NexradA1ToRefract, and RefractColorScales has been added.

* **apps/titan/EsdAcIngest and apps/ingest/AcData2Spdb** - improved TCP input handling to detect server disconnects and reconnect automatically. EsdAcIngest also includes a websocket-to-TCP bridge.

* **apps/ingest/NWSsoundingIngest** - adding `get_sounding.py` for retrieving sounding data.

* **apps/ingest/file_repeat_day** - adding `simulate_files_realtime.py` for replaying files in simulated real time.

* **apps/Radx/RadxMergeVols** - fixing volume output that had inadvertently been disabled during testing.

* **apps/Radx/Radx2Grid and RadxCartDP** - changing the default minimum number of valid points used for interpolation from 3 to 5.

* **apps/Radx/RadxRate** - fixing null-field handling when adding PID diagnostic fields.

* **apps/radar/TsPrint** - improved identification of H/V pulses for fast-alternating data.

* **apps/radar/Ts2Moments and Iq2Dsr** - fixing scan-type handling in BeamReader.

# KDP processing

* **libs/radar/KdpFilt** - major revision of the KDP processing algorithms. Added weighted polynomial-regression, FIR and FFT-based PHIDP filtering; improved PHIDP unfolding; self-consistency methods; phase-shift-on-backscatter estimation and correction; and iterative attenuation correction. Attenuation parameters have been updated to directly support S-, C- and X-band radar data.

* **libs/radar/KdpFirFilt** - new FIR low-pass filtering implementation used by KdpFilt.

* **libs/radar/KdpQuadFit** - new polynomial-fitting support for KDP processing.

* **libs/radar/RadarFftDouble** - adding double-precision FFT support.

* **libs/radar/RadarFft** - adding mutex protection around FFTW initialization and cleanup for thread-safe use.

# Radar moments and time-series processing

* **libs/radar/RadarMoments and ClutFilter** - adding time-series reflection (TSR) clutter filtering. The implementation supports reflected time series to reduce edge effects associated with FFT filtering.

* **libs/radar/RadarTimeSeriesSim** - new radar time-series simulation library for generating synthetic weather and clutter signals for algorithm development and testing.

* **libs/radar/IwrfTsPulse and IWRF support** - adding metadata for radar operating blocks, including block identifiers and start/end information, to support processing of radar systems that switch between operating modes.

# Dual-polarization and Cartesian processing

* **libs/radar/NcarParticleId** - updates supporting Cartesian particle identification, including SNR checking and preparation of PID input fields.

* **libs/radar/PrecipRate** - updates and refactoring supporting Cartesian precipitation-rate and QPE calculations.

* **libs/radar/ConvStratFinder** - updates supporting Cartesian convective/stratiform partitioning, including temperature-based height information and modernization of internal data structures.

# Radx and MDV libraries

* **libs/Radx** - adding support for the WMO FM301 radar data exchange format, including FM301 field-name handling.

* **libs/Radx** - adding `fill` sweep type, scan-name support in RadxRay diagnostics, and fixes for Gematronik volume numbering and CfRadial2 latitude/longitude output.

* **libs/Mdv** - adding MdvxRemapInterp for interpolation of model fields and new functions for safe initialization of MDV structures.

* **libs/Mdv/MdvxField** - adding writable access to volume data.

# General library updates

* **libs/euclid** - adding EuclidAngle class.

* **libs/rapmath** - adding NasaPolyFit polynomial-fitting support.

* **libs/toolsa** - adding file-copy utilities and string conversion methods, along with substantial modernization of TaArray and TaArray2D.

* **libs/Fmq** - fixing shared-memory permission handling in FmqDeviceShmem.

* **libs/dsserver** - improving handling of trailing newline characters in DestUrlArray.

# Code modernization and robustness

* Broad conversion of `sprintf()` and `vsprintf()` calls to safer `snprintf()`, `vsnprintf()` and `safe_snprintf()` implementations. Added `toolsa/safe_snprintf.hh` and applied the safer formatting methods across a large number of LROSE libraries and applications.

* Extensive cleanup of compiler warnings, potentially uninitialized variables and other issues identified by newer compilers and sanitizers.

* Fixed a number of application Main.cc implementations in which the static `Prog` pointer was inadvertently shadowed by a local variable, along with duplicate SIGHUP signal handling.

# NetCDF, HDF5 and build system

* Updated CMake configuration and generated CMakeLists.txt files to use current `find_package()` support for HDF5 and netCDF, including improved handling of include directories.

* **libs/Ncxx** - compatibility updates for newer HDF5 APIs, including HDF5 2.x.

* Added `build/cmake/run_cmake` scripts and updated CMakeLists for new and modified applications.

* Continued improvements to macOS builds and Homebrew support.

# Documentation

* Updated top-level README installation and build instructions.

* Updated package download and installation documentation, including macOS Homebrew installation.

* Updated build documentation to use AlmaLinux in place of CentOS.

* Updated links to the LROSE Wiki and Quickstart Guide.

* Added `docs/apps/lrose_app_table.md` and updated release installation documentation.

