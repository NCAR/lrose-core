# New applications

* Adding apps/titan/Tstorms2NetCDF - convert Titan binary data to NetCDF
* Adding apps/Radx/src/Radx2Fmq - replay Radx files into FMQ

# Application updates

* apps/radar/Lucid - adding prototype to release
* apps/radar/Lucid - major additions to Lucid prototype since previous release
* apps/radar/Lucid - now supports plan view rendering of most data types

* apps/radar/HawkEye - cleaning up unused members in PolarWidget
* apps/radar/HawkEdit/BoundaryPointEditor.cc - adding test QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
* apps/radar/HawkEdit - m_buffer.byteCount() deprecated, using m_buffer.sizeInBytes()
* apps/radar/HawkEdit - fixing compiler warnings, commenting out unused vars
* apps/radar/HawkEdit - bug fix - TimeNavController and TimeNavModel
* apps/radar/HawkEdit - fixing calls to getArchiveFileList so that it returns the vector, rather than just a reference to a local variable

* apps/titan/Rview/parse_args - adding _usage()
* apps/titan/Rview - adding circular node icons when plotting vectors

* apps/titan/Titan - debugging convectivity thresholding
* apps/titan/Titan - turning off file locking in archive mode

* apps/titan/PrintTitanFiles - updating to work with TitanFile and TitanData classes
* apps/titan/PrintTitanFiles - adding print support for netcdf
* apps/titan/PrintTitanFiles - adding option to force print of verification data
* apps/titan/PrintTitanFiles - fixing bug in use with legacy files

* apps/Radx/RadxConvert - adding option to limit censoring to within elevation limits
* apps/Radx/RadxConvert - adding option to compute SNR from DBZ

* apps/radar/EccoStats - adding into the build
* apps/radar/EccoStats - updating for titan masking and censoring
* apps/radar/EccoStats - working on reading in Titan data

* apps/ingest/Era5Nc2Mdv - adding support for multiple levels in a single file

* apps/radar/Iq2Dsr - testing new parametrer center_indexed_beams_starting_at_zero
* apps/radar/Iq2Dsr - bug fixes

* apps/radar/TsAscope - updates for Qt6
                    
# Scripts

* Adding apps/scripts/runas - run as new user with X forwarding

# Libraries

* Qt6 - adding check for version 6.4 for deprecated isValidColor() call
* Qt6 - apps/radar/IpsEye/FieldColorController - fixing deprecated use of isValidColor()

* libs - cleaning up old C K&R interfaces
* libs - adding libs/titan/TitanFile class
* libs - adding libs/titan/TitanData class

* libs/titan - adding TdataServer and TdataComplexTrack, upgrades from TitanServer and TitanComplexTrack
* libs/titan - adding define for N_POLY_SIDES into storm_v5.h and storm_v6.h. In tdata_server.h N_POLY_SIDES is also defined if it does not already exist.
* libs/titan - cleaning up compiler warnings from string copies

* libs/Mdv/MdvxProj - updating operator== to use fabs(diff) instead of absolute equality
* libs/euclid/PjgMath - updating operator== to use fabs(diff) instead of absolute equality
* libs/Mdv/MdvxProj - updating operator== to use fabs(diff) instead of absolute equality

* libs/Radx - adding SWEEP_MODE enums for APAR-specific scans
* libs/Radx - tweak new enums and fix sweepModeFromStr str.find precedence

* libs/Radx/src/NoaaFsl/NoaaFslRadxFile.cc - handling missing values, debugging on latest India data files, debugging for India NOAA netcdf format file
* libs/Radx/NoaaFslRadxFile.cc - updating for India sample data
* libs/Radx/NoaaFslRadxFile - adding vars etc

* libs/Radx/src/Gematronik - updating long names for V1 and V2 fields
* libs/Radx/Gematronik/GemRadxFile.cc - bug fix on reading dual PRF data files, working on handling dual-prt vel fields. These fields sometimes have only every second ray available. Duplicate adjacent rays in the output file
* libs/Radx/src/Gematronik - adding support for dual PRF velocitiesa
* libs/Radx/GemRadxFile - adding _checkForVolKeyword() to correctly handle situation in which xml appears in the first line of a file as a type

* libs/Mdv/oldMdv/mdv_write: sprintf -> snprintf
* libs/Ncxx/src/include/Ncxx/NcxxVar.hh - adding makeIndex()
* libs/toolsa/TaStr - adding replaceSubStr() method

* removed <rapformats/DsRadarMsg.hh> from list of includes. There is no longer a dependence on DsRadarMsg.

# Build system

* createCMakeLists.py - c++11 -> c++17
* Updating createCMakeLists.py to handle tdrp singleton
* Adding Lucid and EccoStats to the CmakeLists
* apps/mdv_utils - cleaning up Makefiles
* apps/didss - cleaning up Makefiles

# Docs

* Adding documentation links to README.md at the top level










