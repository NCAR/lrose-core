// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/////////////////////////////////////////////////////////////
// Tstorms2NetCDF.cc
//
// Tstorms2NetCDF object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2025
//
///////////////////////////////////////////////////////////////
//
// Tstorms2NetCDF reads native TITAN binary data files,
// converts the data into NetCDF format,
// and writes the data out in NetCDF files.
//
////////////////////////////////////////////////////////////////

#include <iostream>

#include <toolsa/ucopyright.h>
#include <toolsa/pmu.h>
#include <toolsa/pjg.h>
#include <toolsa/str.h>
#include <toolsa/file_io.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/TaXml.hh>
#include <didss/RapDataDir.hh>
#include <didss/DataFileNames.hh>
#include <dsserver/DsLdataInfo.hh>
#include <titan/TitanSpdb.hh>
#include <Spdb/DsSpdb.hh>
#include <rapformats/tstorm_hull_smooth.h>
#include "Tstorms2NetCDF.hh"

using namespace std;


// Constructor

Tstorms2NetCDF::Tstorms2NetCDF(int argc, char **argv)

{

  isOK = true;
  _input = NULL;

  // set programe name

  _progName = "Tstorms2NetCDF";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;

    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
  }

  // check args in ARCHIVE mode
  
  if (_params.input_mode == Params::ARCHIVE) {
    if (_args.inputFileList.size() == 0) {
      if ((_args.startTime == 0 || _args.endTime == 0)) {
	cerr << "ERROR: " << _progName << endl;
	cerr << "In ARCHIVE mode, you must specify a file list" << endl
	     << "  or start and end times." << endl;
	isOK = FALSE;
	return;
      }
    }
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
  return;

}

// destructor

Tstorms2NetCDF::~Tstorms2NetCDF()

{

  if (_input) {
    delete _input;
  }

  _outFile.closeFile();

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Tstorms2NetCDF::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // set up input object
  
  if (_params.input_mode == Params::FILELIST) {
    
    if (_args.inputFileList.size() > 0) {
      _input = new DsInputPath(_progName,
			       _params.debug >= Params::DEBUG_VERBOSE,
			       _args.inputFileList);
      // if (_params.convert_to_legacy) {
      //   _input->setSearchExt("nc");
      // } else {
      //   _input->setSearchExt("th5");
      // }
    } else {
      cerr << "ERROR - Tstorms2NetCDF::Run()" << endl;
      cerr <<
	"  In FILELIST mode, you must specify "
	"files on the command line" << endl;
      return -1;
    }
    
  } else if (_params.input_mode == Params::ARCHIVE) {

    if (_args.startTime != 0 && _args.endTime != 0) {
      string inDir;
      RapDataDir.fillPath(_params.input_dir, inDir);
      if (_params.debug) {
	cerr << "Input dir: " << inDir << endl;
      }
      _input = new DsInputPath(_progName,
			       _params.debug >= Params::DEBUG_VERBOSE,
			       inDir,
			       _args.startTime,
			       _args.endTime);
      if (_params.convert_to_legacy) {
        _input->setSearchExt("nc");
      } else {
        _input->setSearchExt("th5");
      }
    } else {
      cerr << "ERROR - Tstorms2NetCDF::Run()" << endl;
      cerr <<
	"  In ARCHIVE mode, you must specify "
	"start and end times on the command line" << endl;
      return -1;
    }

  } else {

    cerr << "ERROR - unknown input_mode: " << _params.input_mode << endl;
    return -1;
    
  }
  
  if (_params.input_mode == Params::ARCHIVE) {
    _input->reset();
  }

  char *inputFilePath;
  while ((inputFilePath = _input->next()) != NULL) {

    _inputPath = inputFilePath;
    
    if (_params.debug) {
      cerr << "Processing input path: " << _inputPath << endl;
    }

    _processInputFile();
    
  }
  
  return 0;

}

//////////////////////////////////////////////////
// process input data

int Tstorms2NetCDF::_processInputFile()

{
  
  // open input files based on the provided path
  
  if (_inFile.openFile(_inputPath, NcxxFile::FileMode::read)) {
    cerr << "ERROR - Tstorms2NetCDF::_processInputFile" << endl;
    cerr << "  Cannot open input files, input_path: " << _inputPath << endl;
    cerr << _inFile.getErrStr();
    return -1;
  }

  if (_inFile.lockFile("r")) {
    cerr << "ERROR - Tstorms2NetCDF::_openInputFiles" << endl;
    cerr << "  " << _inFile.getErrStr() << endl;
    return -1;
  }

  // open output file for writing

  time_t dataTime;
  bool dateOnly;
  DataFileNames::getDataTime(_inputPath, dataTime, dateOnly);
  
  bool writeLegacy = false;
  if (_params.convert_to_legacy) {
    writeLegacy = true;
  }

  if (_outFile.openFile(_params.output_dir, dataTime,
                        NcxxFile::FileMode::replace, writeLegacy)) {
    cerr << "ERROR - Tstorms2NetCDF::_processInputFile" << endl;
    cerr << "  Cannot open output netcdf file: " << _outFile.getPathInUse() << endl;
    cerr << "  Error: " << _outFile.getErrStr() << endl;
    return -1;
  }

  if (_params.debug) {
    if (writeLegacy) {
      cerr << "Tstorms2NetCDF - opened output files: " << endl;
      cerr << "  " << _outFile.getStormFileHeaderPath() << endl;
      cerr << "  " << _outFile.getStormFileDataPath() << endl;
      cerr << "  " << _outFile.getTrackFileHeaderPath() << endl;
      cerr << "  " << _outFile.getTrackFileDataPath() << endl;
    } else {
      cerr << "Tstorms2NetCDF - opened output file: " << _outFile.getPathInUse() << endl;
    }
  }
  
  // read storm header

  if (_inFile.readStormHeader()) {
    cerr << "ERROR - Tstorms2NetCDF::_processInputFile" << endl;
    cerr << "  Cannot read storm header, input_path: " << _inputPath << endl;
    cerr << _inFile.getErrStr() << endl;
    return -1;
  }

  // load up scan times
  
  if (_loadScanTimes()) {
    return -1;
  }

  // loop through the scans, processing each one

  for (size_t iscan = 0; iscan < _scanTimes.size(); iscan++) {
    time_t scan_time = _scanTimes[iscan];
    if (_params.input_mode == Params::ARCHIVE) {
      if (scan_time >= _args.startTime && scan_time <= _args.endTime) {
        _processScan(iscan, scan_time);
      }
    } else {
      // FILELIST mode - process all scans
      _processScan(iscan, scan_time);
    }
  }
  
  // write the storm header

  _outFile.writeStormHeader(_inFile.stormHeader());

  // read track header
  
  if (_inFile.readTrackHeader()) {
    cerr << "ERROR - Tstorms2NetCDF::_processInputFile" << endl;
    cerr << "  Cannot read track header" << endl;
    cerr << "    input_path: " << _inputPath << endl;
    cerr << _inFile.getErrStr() << endl;
    return -1;
  }
  const TitanData::TrackHeader &theader = _inFile.trackHeader();
  
  // get the complex track numbers array
  
  const int32_t *complexTrackNums = _inFile.complexTrackNums().data();
  
  // loop through complex tracks, reading parameters for each
  
  for (int icomp = 0; icomp < theader.n_complex_tracks; icomp++) {

    int complexNum = complexTrackNums[icomp];

    // read complex parameters
    if (_inFile.readComplexTrackParams(complexNum, true)) {
      cerr << "ERROR - Tstorms2NetCDF::_processInputFile" << endl;
      cerr << "  Cannot read complex params" << endl;
      cerr << "    input_path: " << _inputPath << endl;
      cerr << "    index, complex_num: " << icomp << ", " << complexNum << endl;
      cerr << _inFile.getErrStr() << endl;
      return -1;
    }

    // write the complex params
    _outFile.writeComplexTrackParams(icomp, _inFile.complexParams());

  } // icomp
  
  // loop through the simple tracks, reading parameters for each
  
  for (int isimp = 0; isimp < theader.n_simple_tracks; isimp++) {

    int simpleTrackNum = isimp;

    // read simple parameters
    if (_inFile.readSimpleTrackParams(simpleTrackNum)) {
      cerr << "ERROR - Tstorms2NetCDF::_processInputFile" << endl;
      cerr << "  Cannot read simple params" << endl;
      cerr << "    input_path: " << _inputPath << endl;
      cerr << "    simple_num: " << simpleTrackNum << endl;
      cerr << _inFile.getErrStr() << endl;
      return -1;
    }

    // write the simple params
    _outFile.writeSimpleTrackParams(simpleTrackNum, _inFile.simpleParams());

  } // isimple

  // write the simples_per_complex arrays

  vector<int32_t> simpsPerComplex1D;
  vector<int32_t> simpsPerComplexOffsets;
  _inFile.loadVecSimplesPerComplex(simpsPerComplex1D, simpsPerComplexOffsets);
  _outFile.writeSimplesPerComplexArrays(theader.n_simple_tracks,
                                        _inFile.nSimplesPerComplex(),
                                        simpsPerComplexOffsets,
                                        simpsPerComplex1D.data());
  
  // read in track scan index
  
  if (_inFile.readTrackScanIndex()) {
    cerr << "ERROR - Tstorms2NetCDF::_processInputFile" << endl;
    cerr << "  Cannot read scan index" << endl;
    cerr << "    input_path: " << _inputPath << endl;
    cerr << _inFile.getErrStr() << endl;
    return -1;
  }

  // loop through the simple tracks
  
  for (int isimple = 0; isimple < theader.n_simple_tracks; isimple++) {

    // get simple track number - this is the same as the index
    
    int simpleTrackNum = isimple;
    
    // read simple track params
    
    if (_inFile.readSimpleTrackParams(simpleTrackNum)) {
      cerr << "ERROR - Tstorms2NetCDF::_processInputFile" << endl;
      cerr << "  Cannot read simple track params" << endl;
      cerr << "    input_path: " << _inputPath << endl;
      cerr << "    simpleTrackNum: " << simpleTrackNum << endl;
      cerr << _inFile.getErrStr() << endl;
      return -1;
    }
    TitanData::SimpleTrackParams sparams(_inFile.simpleParams());
    
    // rewind simple track - prepare for reading entries
    
    if (_inFile.rewindSimpleTrack(simpleTrackNum)) {
      cerr << "ERROR - Tstorms2NetCDF::_processInputFile" << endl;
      cerr << "  Cannot rewind simple track" << endl;
      cerr << "    input_path: " << _inputPath << endl;
      cerr << "    simpleTrackNum: " << simpleTrackNum << endl;
      cerr << _inFile.getErrStr() << endl;
      return -1;
    }

    // loop through the entries, by scan, reading entries and writing out

    for (int iscan = sparams.start_scan; iscan <= sparams.end_scan; iscan++) {
      if (_inFile.readTrackEntry()) {
        cerr << "ERROR - Tstorms2NetCDF::_processInputFile" << endl;
        cerr << "  Cannot read simple track entry" << endl;
        cerr << "    input_path: " << _inputPath << endl;
        cerr << "    simpleTrackNum: " << simpleTrackNum << endl;
        cerr << _inFile.getErrStr() << endl;
        return -1;
      }
      int writeOffset = _outFile.writeTrackEntry(_inFile.entry());
      if (iscan == sparams.start_scan) {
        sparams.first_entry_offset = writeOffset;
      }
    } // iscan

    // // set the offsets for each entry

    // for (size_t ientry = 0; ientry < entries.size(); ientry++) {
    //   track_file_entry_t &entry(entries[ientry]);
    //   entry.this_entry_offset =
    //     _outFile.getStormEntryOffset(entry.scan_num, entry.storm_num);
    //   entry.next_scan_entry_offset =
    //     _outFile.getNextScanEntryOffset(entry.scan_num, entry.storm_num);
    //   if (ientry == 0) {
    //     sparams.first_entry_offset = entry.this_entry_offset;
    //   }
    // } // entry
     
    // // set the prev and next offsets for each entry
    
    // for (size_t ientry = 0; ientry < entries.size(); ientry++) {
    //   track_file_entry_t &entry(entries[ientry]);
    //   if (ientry == 0) {
    //     entry.prev_entry_offset = 0;
    //   } else {
    //     entry.prev_entry_offset = entries[ientry - 1].this_entry_offset;
    //   }
    //   if (ientry == entries.size() - 1) {
    //     entry.next_entry_offset = -1;
    //   } else {
    //     entry.next_entry_offset = entries[ientry + 1].this_entry_offset;
    //   }
    // } // entry
    
    // // write the entries
    
    // for (size_t ientry = 0; ientry < entries.size(); ientry++) {
    //   track_file_entry_t &entry(entries[ientry]);
    //   if (_outFile.writeTrackEntry(entry) < 0) {
    //     cerr << "ERROR - Tstorms2NetCDF::_processInputFile" << endl;
    //     cerr << "  Cannot write track entry" << endl;
    //     cerr << "    simpleTrackNum: " << simpleTrackNum << endl;
    //     cerr << "    scan num: " << entry.scan_num << endl;
    //     cerr << "    storm num: " << entry.storm_num << endl;
    //     cerr << _inFile.getErrStr() << endl;
    //     return -1;
    //   }
    // } // entry
    
    // write the updated simple params
    
    _outFile.writeSimpleTrackParams(simpleTrackNum, sparams);
    
  } // isimple

  // write the track header
  
  _outFile.writeTrackHeader(_inFile.trackHeader(),
                            _inFile.complexTrackNums(),
                            _inFile.nSimplesPerComplex(),
                            _inFile.simplesPerComplex2D());

  // test truncate if requested
  
  if (_params.test_truncation) {
    _outFile.truncateData(_params.truncation_scan_number);
  }
  
  // close

  _inFile.closeFile();
  
  if (_params.debug) {
    if (writeLegacy) {
      cerr << "Tstorms2NetCDF - wrote output files: " << endl;
      cerr << "  " << _outFile.getStormFileHeaderPath() << endl;
      cerr << "  " << _outFile.getStormFileDataPath() << endl;
      cerr << "  " << _outFile.getTrackFileHeaderPath() << endl;
      cerr << "  " << _outFile.getTrackFileDataPath() << endl;
    } else {
      cerr << "Tstorms2NetCDF - wrote output file: " << _outFile.getPathInUse() << endl;
    }
  }
  
  return 0;

}

//////////////////////////////////////////////////
// load up scan times from storm file

int Tstorms2NetCDF::_loadScanTimes()

{

  _scanTimes.clear();
  int nScans = _inFile.stormHeader().n_scans;
  for (int i = 0; i < nScans; i++) {
    // read in scan
    if (_inFile.readStormScan(i)) {
      cerr << "ERROR - Tstorms2NetCDF::_loadScanTimes" << endl;
      cerr << "  " << _inFile.getErrStr() << endl;
      return -1;
    }
    _scanTimes.push_back(_inFile.scan().time);
  }

  return 0;

}

/////////////////////
// process this scan

int Tstorms2NetCDF::_processScan(int scan_num,
                                 time_t scan_time)
  
{

  PMU_auto_register("Reading data....");
  
  time_t end_time = scan_time;
  time_t start_time = scan_time;
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Processing scan num: " << scan_num << endl;
    cerr << "  Start time: " << DateTime::str(start_time) << endl;
    cerr << "  End time: " << DateTime::str(end_time) << endl;
  }
  
  // read in scan, and global properties for the storms
  
  if (_inFile.readStormScan(scan_num)) {
    cerr << "ERROR - Tstorms2NetCDF::_processScan" << endl;
    cerr << "  Cannot read scan and gprops, input_path: " << _inputPath << endl;
    cerr << "    scan_num: " << scan_num << endl;
    cerr << _inFile.getErrStr() << endl;
    return -1;
  }
  
  int nStorms = _inFile.scan().nstorms;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  n_storms: " << nStorms << endl;
  }
  
  // for each storm in the scan read in the secondary properties, i.e.:
  //   * layer properties
  //   * dbz histograms
  //   * runs
  //   * proj_runs

  for (int istorm = 0; istorm < nStorms; istorm++) {
    
    if (_inFile.readStormAux(istorm)) {
      cerr << "ERROR - Tstorms2NetCDF::_processScan" << endl;
      cerr << "  Cannot read properties, storm num: "
           << istorm << ", " << _inputPath << endl;
      cerr << _inFile.getErrStr() << endl;
      return -1;
    }

    // write out secondary properties
    // side-effect - sets offsets vectors
    
    _outFile.writeStormAux(istorm,
                           _inFile.stormHeader(),
                           _inFile.scan(),
                           _inFile.gprops(),
                           _inFile.lprops(),
                           _inFile.hist(),
                           _inFile.runs(),
                           _inFile.projRuns());
    
  }

  // write the scan and global properties to NetCDF
  // the appropriate offsets have been set by writeSecProps()
  
  _outFile.writeStormScan(_inFile.stormHeader(),
                          _inFile.scan(),
                          _inFile.gprops());
  
  return 0;

}
  
