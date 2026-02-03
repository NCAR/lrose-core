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
// RefractCompute
//
// Nancy Rehak, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2026
//
/////////////////////////////////////////////////////////////
//
// RefractCompute computes the refractivity value at each radar
// gate, given the AIQ/NIQ data, plus the calibration results
// from previously running RefractCalib.
//
//////////////////////////////////////////////////////////////

#include "RefractCompute.hh"
#include "Params.hh"
#include "Reader.hh"
#include <dsdata/DsTrigger.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <assert.h>

// Global variables

/*********************************************************************
 * Constructor
 */

RefractCompute::RefractCompute(int argc, char **argv)
{

  okay = true;
  _input = nullptr;
  _reader = nullptr;
  _processor = nullptr;

  // set programe name

  _progName = "RefractCompute";
  ucopyright((char *) _progName.c_str());

  // get command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    okay = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    okay = false;
  }

  // initialize the data input object
  
  if (_params.mode == Params::REALTIME) {

    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _params.max_realtime_data_age_secs,
			     PMU_auto_register, true);

  } else if (_params.mode == Params::ARCHIVE) {

    time_t startTime = DateTime::parseDateTime(_params.start_time);
    time_t endTime = DateTime::parseDateTime(_params.end_time);
    if (startTime == DateTime::NEVER) {
      cerr << "ERROR: RefractCompute" << endl;
      cerr << "  bad start time: " << _params.start_time << endl;
      okay = false;
    }
    if (endTime == DateTime::NEVER) {
      cerr << "ERROR: RefractCompute" << endl;
      cerr << "  bad end time: " << _params.end_time << endl;
      okay = false;
    }

    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     startTime, endTime);

  } else if (_params.mode == Params::FILELIST) {

    if (_args.inputFileList.size() == 0) {

      cerr << "ERROR: RefractCompute" << endl;
      cerr << "  Mode is FILELIST"; 
      cerr << "  You must use -f to specify files on the command line."
           << endl;
      _args.usage(_progName, cerr);
      okay = false;
      
    } else {
      
      _input = new DsInputPath(_progName,
                               _params.debug >= Params::DEBUG_VERBOSE,
                               _args.inputFileList);

    }
    
  } // if (_params.mode == ...
  
  // create reader
  
  _reader = new Reader(_params);

  // create processor
  
  _processor = new Processor(_params);

  // init process mapper registration
  
  if (_params.mode == Params::REALTIME) {
    PMU_auto_init(_progName.c_str(),
                  _params.instance,
                  _params.procmap_register_interval);
  }

}


/*********************************************************************
 * Destructor
 */

RefractCompute::~RefractCompute()
{
  
  // Unregister process
  
  PMU_auto_unregister();
  
  // Free contained objects

  delete _input;
  delete _reader;
  delete _processor;
  
}

/*********************************************************************
 * run()
 */

int RefractCompute::run()
{

  int nSuccess = 0;
  char *inputPath = nullptr;
  while ((inputPath = _input->next()) != nullptr) {
    
    time_t inputTime;
    if (DsInputPath::getDataTime(inputPath, inputTime)) {
      cerr << "ERROR: RefractCompute::run()" << endl;
      cerr << "  Cannot compute time from input file path: " << inputPath << endl;
      cerr << "  Ignoring this file" << endl;
      continue;
    }

    DateTime fileTime(inputTime);
    if (!_processData(inputPath, fileTime)) {
      cerr << "ERROR: RefractCompute::run()" << endl;
      cerr << "  processing data for time: " << fileTime << endl;
      continue;
    }

    nSuccess++;
    
  } // while ...
  
  if (_params.debug) {
    cerr << "==>>end of data" << endl;
  }

  if (nSuccess > 0) {
    return 0;
  } else {
    return -1;
  }
  
}


/*********************************************************************
 * _processData()
 */

bool RefractCompute::_processData(string inputPath,
                                  const DateTime &dataTime)
{

  PMU_auto_register("Processing data");
  
  LOG(DEBUG) << "**** Input file: " << inputPath;
  LOG(DEBUG) << "**** Processing data for time: " << dataTime;
  
  // Get the next scan from the input handler

  DsMdvx data_file;
  if (!_readInputFile(data_file, dataTime)) {
    return false;
  }
  
  if (_processor->processScan(dataTime.utime(), data_file)) {
    // Generate output file
    LOG(DEBUG) << "---> Writing data for scan at time "
	       << DateTime::str(data_file.getMasterHeader().time_centroid);
  
    if (data_file.writeToDir(_params.output_dir) != 0)
    {
      LOG(ERROR) << "writing output file to URL: " << _params.output_dir;
      LOG(ERROR) << data_file.getErrStr();
      return false;
    }
    LOG(DEBUG) << "Wrote output file: " << data_file.getPathInUse();
    return true;
  } else {
    return false;
  }
  
}


/*********************************************************************
 * _readInputFile()
 */

bool RefractCompute::_readInputFile(DsMdvx &mdvx, const DateTime &data_time)
{
  if (!_reader->getScan(data_time, 0, _params.input_dir, mdvx))
  {
    LOG(ERROR) << "Could not read in data";
    return false;
  }
  return true;
}
