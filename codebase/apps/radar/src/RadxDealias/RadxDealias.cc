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
/////////////////////////////////////////////////////////
//
// RadxDealias.cc: methods of RadxDealias class
// 
/////////////////////////////////////////////////////////

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <Radx/RadxRay.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxFile.hh>
// cfradial file format
// #include <Radx/NcfRadxFile.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include "RadxDealias.hh"
using namespace std;

///////////////////////////////////////////////////////
// 
// Constructor
//
RadxDealias::RadxDealias(int argc, char **argv)
{
  isOK = true;

  //
  // set programe name
  //
  _progName = "RadxDealias";

  ucopyright((char *) _progName.c_str());
  
  jamesCopyright();

  //
  // get command line args
  //
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  //
  // get TDRP params
  //
  _paramsPath = (char *) "unknown";

  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }

  //
  // init process mapper registration
  //
  PMU_auto_init((char *) _progName.c_str(),
  		_params.instance,
  	PROCMAP_REGISTER_INTERVAL);

  //
  // Initialize Dsr2Radar pointers
  //
  //_currRadarVol = NULL;
 
  //_prevRadarVol = NULL;

  return;
}

//////////////////////////////////////////////////////////////
// 
// destructor
//
RadxDealias::~RadxDealias()

{
  //if (_currRadarVol)
  //  delete (_currRadarVol);

  //if (_prevRadarVol)
  //  delete (_prevRadarVol);

  //
  // unregister process
  //
  PMU_auto_unregister();

}

////////////////////////////////////////////////////////
//
// Run
//
int RadxDealias::Run ()
{

  // create the dealiasing object and send it the input parameters
  _fourDD = new FourDD(_params);

  if (_params.mode == Params::ARCHIVE) {
    return _runArchive();
  } else if (_params.mode == Params::FILELIST) {
    return _runFilelist();
  } else {

    // run real time mode 
    // register with procmap
    //
    PMU_auto_register("Run");
    /*
    while (true) {
      _run();
      cerr << "RadxDealias::Run:" << endl;
      cerr << "  Trying to contact input server at url: "
	   << _params.input_fmq_url << endl;
      sleep(2);
    }
    */
  }

  return 0;
}

//////////////////////////////////////////////////
// Run in filelist mode

int RadxDealias::_runFilelist()
{
  
  int iret = 0;

  if (_params.debug) {
    cerr << "Running RadxDealias" << endl;
    cerr << "  n input files: " << _args.inputFileList.size() << endl;
  }

  int nGood = 0;
  int nError = 0;
  
  //  if (!_params.aggregate_all_files_on_read) {

    Volume *prevVelVol = NULL;
    Volume *currDbzVol = NULL;
    Volume *currVelVol = NULL;  

    RadxVol vol;

    // loop through the input file list

    for (int ii = 0; ii < (int) _args.inputFileList.size(); ii++) {
      string inputPath = _args.inputFileList[ii];
      // read input file
      int jret = _readFile(inputPath, vol);
      if (jret == 0) {
        //_processVol(vol);

        vol.loadFieldsFromRays();
	//vol.convertToFl32(); // TODO: does FourDD use signed ints?
	// convert from RadxVol to Volume structures
	currDbzVol = _extractFieldData(vol, "DBZ"); 
	currVelVol = _extractFieldData(vol, "VEL");
	// 
	if ((currDbzVol != NULL) && (currVelVol != NULL)) {
	time_t volTime = vol.getStartTimeSecs(); // _currRadarVol->getVolTime();

	_processVol(prevVelVol, currVelVol, currDbzVol, volTime);

	// insert dealiased values from Volume structures to RadxVol  
	//_insertDbz(currDbzVol, vol);
	//_insertVel(currVelVol, vol);

	prevVelVol = currVelVol;

        // finalize the volume
        //_finalizeVol(vol);
        // write the volume out
        if (_writeVol(vol)) {
          cerr << "ERROR - RadxDealias::_runFileList" << endl;
          cerr << "  Cannot write volume to file" << endl;
          iret = -1;
          nError++;
          if (_params.debug) {
            cerr << "  ====>> n errors so far: " << nError << endl;
          }
        } else {
          nGood++;
          if (_params.debug) {
            cerr << "  ====>> n good files so far: " << nGood << endl;
            cerr << "  ====>> n errors     so far: " << nError << endl;
            cerr << "  ====>> sum          so far: " << nGood + nError << endl;
          }
        }
      } else if (jret < 0) {
        iret = -1;
        nError++;
        if (_params.debug) {
          cerr << "  ====>> n errors so far: " << nError << endl;
        }
      }
      // free up
      // vol.clear();
      } // end if !NULL
    } // end for each file
    /*
  } else {
    
    // aggregate the files into a single volume on read
    
    RadxVol vol;
    GenericRadxFile inFile;
    _setupRead(inFile);
    vector<string> paths = _args.inputFileList;
    if (inFile.aggregateFromPaths(paths, vol)) {
      cerr << "ERROR - RadxDealias::_runFileList" << endl;
      cerr << "  paths: " << endl;
      for (size_t ii = 0; ii < paths.size(); ii++) {
        cerr << "         " << paths[ii] << endl;
      }
      return -1;
    }
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      for (size_t ii = 0; ii < paths.size(); ii++) {
        cerr << "==>> read in file: " << paths[ii] << endl;
      }
    }
    
    // finalize the volume
    
    _finalizeVol(vol);
    
    // write the volume out
    if (_writeVol(vol)) {
      cerr << "ERROR - RadxDealias::_runFileList" << endl;
      cerr << "  Cannot write aggregated volume to file" << endl;
      iret = -1;
    }

    nGood++;
    
  } // if (!_params.aggregate_all_files_on_read) {
    */

  if (_params.debug) {
    cerr << "RadxDealias done" << endl;
    cerr << "====>> n good files processed: " << nGood << endl;
  }

  return iret;

}


//////////////////////////////////////////////////
// Run in archive mode

int RadxDealias::_runArchive()
{
  int iret = 0;

  /*
  // get the files to be processed

  RadxTimeList tlist;
  tlist.setDir(_params.input_dir);
  tlist.setModeInterval(_args.startTime, _args.endTime);
  if (_params.aggregate_sweep_files_on_read) {
    tlist.setReadAggregateSweeps(true);
  }
  if (tlist.compile()) {
    cerr << "ERROR - RadxDealias::_runFilelist()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(_args.startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(_args.endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - RadxDealias::_runFilelist()" << endl;
    cerr << "  No files found, dir: " << _params.input_dir << endl;
    return -1;
  }
  
  // loop through the input file list

  RadxVol vol;
  int iret = 0;
  for (size_t ii = 0; ii < paths.size(); ii++) {
    // read input file
    int jret = _readFile(paths[ii], vol);
    if (jret == 0) {
      // finalize the volume
      //_finalizeVol(vol);
      // write the volume out
      if (_writeVol(vol)) {
        cerr << "ERROR - RadxDealias::_runArchive" << endl;
        cerr << "  Cannot write volume to file" << endl;
        return -1;
      }
    } else if (jret < 0) {
      iret = -1;
    }
    // free up
    vol.clear();
  }
  */
  return iret;

}



/*
//////////////////////////////////////////////////////////////
//
// _run: initialize fmqs, create Dsr2Radar and FourDD objects
//       for storing beams and dealiasing volumes,
//       start reading messages, processing and writing volumes.
//
int RadxDealias::_run ()
{
  // Instantiate and initialize the DsRadar queues
  //

  DsRadarQueue radarQueue, outputQueue;

  DsRadarMsg radarMsg;

  //
  // Option to pad the beam data to a constant number of gates
  //

  if ( _params.input_num_gates > 0 )
    {
      radarMsg.padBeams( true, _params.input_num_gates );
    }

  if (_params.seek_to_end_of_input)
    {
      if (radarQueue.init(_params.input_fmq_url, _progName.c_str(),
                          _params.debug,
                          DsFmq::BLOCKING_READ_ONLY, DsFmq::END ))
        {
          fprintf(stderr, "ERROR - %s:RadxDealias::_run\n", _progName.c_str());
          fprintf(stderr, "Could not initialize radar queue '%s'\n",
                  _params.input_fmq_url);
          return -1;
        }
    }
  else
    {
      if (radarQueue.init(_params.input_fmq_url, _progName.c_str(),
                          _params.debug,
                          DsFmq::BLOCKING_READ_ONLY, DsFmq::START ))
        {
          fprintf(stderr, "ERROR - %s:RadxDealias::_run\n", _progName.c_str());
          fprintf(stderr, "Could not initialize radar queue '%s'\n",
                  _params.input_fmq_url);
          return -1;
        }
    }

  if( outputQueue.init( _params.output_fmq_url,
                        _progName.c_str(),
                        _params.debug,
                        DsFmq::READ_WRITE, DsFmq::END,
                        _params.output_fmq_compress,
                        _params.output_fmq_nslots,
                        _params.output_fmq_size, 1000) )
    {
      fprintf(stderr, "Error - %s: Could not initialize fmq %s", _progName.c_str(), _params.output_fmq_url );
      return( -1);
    }

  //
  // Create Dsr2Radar objects for reformatting and storing radar data

  //_currRadarVol = new Dsr2Radar(_params);
  //_prevRadarVol = new Dsr2Radar(_params);

  // Create FourDD object for dealiasing
  _fourDD = new FourDD(_params);

  // Read beams from the queue and process them

  while (true)
    {
      bool end_of_vol;
      int contents;
      if (_readMsg(radarQueue, radarMsg, end_of_vol, contents) == 0)
        {
          if (end_of_vol)
            {
              _processVol();
              _writeVol(outputQueue);
              _reset();
            }
        } //  if (_readMsg() 
    } // while (true)

  return 0;
}
*/

//////////////////////////////////////////////////////////////
// 
// _run: X initialize fmqs, create Dsr2Radar and FourDD objects
//       for storing beams and dealiasing volumes,
//       X start reading messages, processing and writing volumes.  
//       open data file(s) pass RadxVol to FourDD as an RSL radar structure.
/*
int RadxDealias::_run ()
{

  //
  // Option to pad the beam data to a constant number of gates
  //
  if ( _params.input_num_gates > 0 ) 
    {
      radarMsg.padBeams( true, _params.input_num_gates );
    }

  if (_params.seek_to_end_of_input) 
    {
      if (radarQueue.init(_params.input_fmq_url, _progName.c_str(),
			  _params.debug,
			  DsFmq::BLOCKING_READ_ONLY, DsFmq::END )) 
	{
	  fprintf(stderr, "ERROR - %s:RadxDealias::_run\n", _progName.c_str());
	  fprintf(stderr, "Could not initialize radar queue '%s'\n",
		  _params.input_fmq_url);
	  return -1;
	}
    } 
  else 
    {
      if (radarQueue.init(_params.input_fmq_url, _progName.c_str(),
			  _params.debug,
			  DsFmq::BLOCKING_READ_ONLY, DsFmq::START )) 
	{
	  fprintf(stderr, "ERROR - %s:RadxDealias::_run\n", _progName.c_str());
	  fprintf(stderr, "Could not initialize radar queue '%s'\n",
		  _params.input_fmq_url);
	  return -1;
	}
    }
  
  if( outputQueue.init( _params.output_fmq_url,
                        _progName.c_str(),
                        _params.debug,
                        DsFmq::READ_WRITE, DsFmq::END,
                        _params.output_fmq_compress,
                        _params.output_fmq_nslots,
                        _params.output_fmq_size, 1000) ) 
    {
      fprintf(stderr, "Error - %s: Could not initialize fmq %s", _progName.c_str(), _params.output_fmq_url );
      return( -1);
    }

  //
  // Create Dsr2Radar objects for reformatting and storing radar data
  //
  _currRadarVol = new Dsr2Radar(_params);
  
  _prevRadarVol = new Dsr2Radar(_params);

  //
  // Create FourDD object for dealiasing
  //
  _fourDD = new FourDD(_params);



  //
  // Read beams from the queue and process them
  //
  while (true) 
    {
      bool end_of_vol;
            
      int contents;
      
      string path; 
      RadxVol radxVol;

      int error = ncfRadxFile.readFromPath(path, radxVol);
      
      if (!error) {
         _processVol(radxVol);
	 _writeVol(radxVol);
     	 _reset();
      }
    } // while (true)
  
  return 0;
}
*/
//////////////////////////////////////////////////
// Read in a file
// accounting for special cases such as gematronik
// Returns 0 on success
//         1 if already read,
//         -1 on failure

int RadxDealias::_readFile(const string &readPath,
                           RadxVol &vol)
{

  PMU_auto_register("Processing file");

  // clear all data on volume object

  // vol.clear();

  // check we have not already processed this file
  // in the file aggregation step
  /*
  if (_params.aggregate_sweep_files_on_read ||
      _params.aggregate_all_files_on_read) {
    RadxPath thisPath(readPath);
    for (size_t ii = 0; ii < _readPaths.size(); ii++) {
      RadxPath listPath(_readPaths[ii]);
      if (thisPath.getFile() == listPath.getFile()) {
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "Skipping file: " << readPath << endl;
          cerr << "  Previously processed in aggregation step" << endl;
        }
        return 1;
      }
    }
  }
  */

  if (_params.debug) {
    cerr << "INFO - RadxDealias::Run" << endl;
    cerr << "  Input path: " << readPath << endl;
  }

  // At this point, the file must be a cfradial file;
  // otherwise, use RadxConvert to get it to cfradial    
  RadxFile inFile;
  //_setupRead(inFile);
  
  // read in file

  if (inFile.readFromPath(readPath, vol)) {
    cerr << "ERROR - RadxDealias::_readFile" << endl;
    cerr << "  path: " << readPath << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }
  /*
  _readPaths = inFile.getReadPaths();
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    for (size_t ii = 0; ii < _readPaths.size(); ii++) {
      cerr << "  ==>> read in file: " << _readPaths[ii] << endl;
    }
  }
  */

  return 0;

}

//////////////////////////////////////////////////
// write out the volume

int RadxDealias::_writeVol(RadxVol &vol)
{

  // output file
  /*

  Ncf/NcfRadxFile_write.cc:int NcfRadxFile::writeToDir(const RadxVol &vol,
  RadxFile outFile;
  //_setupWrite(outFile);

  string outputDir = _params.output_dir;
    
  if (_params.output_filename_mode == Params::SPECIFY_FILE_NAME) {
    
    string outPath = outputDir;
    outPath += PATH_DELIM;
    outPath += _params.output_filename;

    // write to path
  
    if (outFile.writeToPath(vol, outPath)) {
      cerr << "ERROR - RadxDealias::_writeVol" << endl;
      cerr << "  Cannot write file to path: " << outPath << endl;
      cerr << outFile.getErrStr() << endl;
      return -1;
    }
    
  } else {

    // write to dir
  
    if (outFile.writeToDir(vol, outputDir,
                           _params.append_day_dir_to_output_dir,
                           _params.append_year_dir_to_output_dir)) {
      cerr << "ERROR - RadxDealias::_writeVol" << endl;
      cerr << "  Cannot write file to dir: " << outputDir << endl;
      cerr << outFile.getErrStr() << endl;
      return -1;
    }

  }

  string outputPath = outFile.getPathInUse();
  */
  return 0;

}

/*

//////////////////////////////////////////////////                                                                                    
// handle file via specified path                                                                                                     

int RadxPrint::_handleViaPath(const string &path)
{

  // does file exist?                                                                                                                 

  struct stat fileStat;
  if (stat(path.c_str(), &fileStat)) {
    int errNum = errno;
    cerr << "ERROR - RadxPrint::_handleViaPath" << endl;
    cerr << "  Cannot stat file: " << path << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }

  // set up file                                                                                                                      

  GenericRadxFile file;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setDebug(true);
    file.setVerbose(true);
  }

  if (_params.debug) {
    cerr << "Working on file: " << path << endl;
  }

  // set up read                                                                                                                     
  // _setupRead(file);
  if (_params.aggregate_sweep_files_on_read) {
    file.setReadAggregateSweeps(true);
  } else {
    file.setReadAggregateSweeps(false);
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.printReadRequest(cerr);
  }

  // read into vol                                                                                                                    
  RadxVol vol;

  if (!_params.aggregate_all_files_on_read) {

    // read in file                                                                                                                   

    if (file.readFromPath(path, vol)) {
      cerr << "ERROR - RadxPrint::_handleViaPath" << endl;
      cerr << "  Printing file: " << path << endl;
      cerr << file.getErrStr() << endl;
      return -1;
    }
    _readPaths = file.getReadPaths();
    if (_params.debug) {
      for (size_t ii = 0; ii < _readPaths.size(); ii++) {
        cerr << "  ==>> read in file: " << _readPaths[ii] << endl;
      }
    }

  } else {

    // aggregate files on read                                                                                                        

    _readPaths = _args.inputFileList;
    if (file.aggregateFromPaths(_readPaths, vol)) {
      cerr << "ERROR - RadxPrint::_handleViaPath" << endl;
      cerr << "Aggregating files on read" << endl;
      cerr << "  paths: " << endl;
      for (size_t ii = 0; ii < _readPaths.size(); ii++) {
        cerr << "         " << _readPaths[ii] << endl;
      }
      return -1;
    }

    if (_params.debug) {
      cerr << "Aggregating files on read" << endl;
      for (size_t ii = 0; ii < _readPaths.size(); ii++) {
        cerr << "==>> read in path: " << _readPaths[ii] << endl;
      }
    }

  }

  // set number of gates constant if requested                                                                                        

  if (_params.set_ngates_constant) {
    vol.setNGatesConstant();
  }

  // trim to 360s if requested                                                                                                        

  if (_params.trim_surveillance_sweeps_to_360deg) {
    vol.trimSurveillanceSweepsTo360Deg();
  }

  // do print                                                                                                                         

  _printVol(vol);

  return 0;

}
*/

/*
////////////////////////////////////////////////////                                                                                  
// Handle search via specified time and search mode                                                                                   

int RadxPrint::_handleViaTime()
{

  GenericRadxFile file;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setDebug(true);
    file.setVerbose(true);
  }

  // set up read                                                                                                                      

  _setupRead(file);

  if (_params.aggregate_sweep_files_on_read) {
    file.setReadAggregateSweeps(true);
  } else {
    file.setReadAggregateSweeps(false);
  }

  RadxField::StatsMethod_t dwellStatsMethod;
  switch (_params.read_dwell_stats) {
  case Params::DWELL_STATS_MEAN:
    dwellStatsMethod = RadxField::STATS_METHOD_MEAN;
    break;
  case Params::DWELL_STATS_MEDIAN:
    dwellStatsMethod = RadxField::STATS_METHOD_MEDIAN;
    break;
  case Params::DWELL_STATS_MAXIMUM:
    dwellStatsMethod = RadxField::STATS_METHOD_MAXIMUM;
    break;
  case Params::DWELL_STATS_MINIMUM:
    dwellStatsMethod = RadxField::STATS_METHOD_MINIMUM;
    break;
  case Params::DWELL_STATS_MIDDLE:
  default:
    dwellStatsMethod = RadxField::STATS_METHOD_MIDDLE;
    break;
  }

  switch (_params.read_search_mode) {
  case Params::READ_RAYS_IN_INTERVAL:
    file.setReadRaysInInterval(_readStartTime,
			       _readEndTime,
			       _params.read_dwell_secs,
			       dwellStatsMethod);
    break;
  case Params::READ_CLOSEST:
    file.setReadModeClosest(_readSearchTime.utime(),
			    _params.read_search_margin);
    break;
  case Params::READ_FIRST_BEFORE:
    file.setReadModeFirstBefore(_readSearchTime.utime(),
				_params.read_search_margin);
    break;
  case Params::READ_FIRST_AFTER:
    file.setReadModeFirstAfter(_readSearchTime.utime(),
			       _params.read_search_margin);
    break;
  case Params::READ_LATEST:
  default:
    file.setReadModeLast();
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.printReadRequest(cerr);
  }

  // perform the read                                                                                                                 

  RadxVol vol;
  if (file.readFromDir(_params.dir, vol)) {
    cerr << "ERROR - RadxPrint::_handleViaTime" << endl;
    cerr << file.getErrStr() << endl;
    return -1;
  }

  _readPaths = file.getReadPaths();
  if (_params.debug) {
    for (size_t ii = 0; ii < _readPaths.size(); ii++) {
      cerr << "  ==>> read in file: " << _readPaths[ii] << endl;
    }
  }

  // set number of gates constant if requested                                                                                        

  if (_params.set_ngates_constant) {
    vol.setNGatesConstant();
  }

  // trim to 360s if requested                                                                                                        

  if (_params.trim_surveillance_sweeps_to_360deg) {
    vol.trimSurveillanceSweepsTo360Deg();
  }

  // do print                                                                                                                         

  _printVol(vol);

  return 0;

}
*/

//////////////////////////////////////////////////                                                                                    
// set up read                                                                                                                        
 /*
void RadxDealias::_setupRead(NcfRadxFile &file)
{

}
 */


/*
////////////////////////////////////////////////////////////////////
// _readMsg()
//
// Read a message from the queue. If appropriate, reformat and store 
// data, set end of vol flag.
//

int RadxDealias::_readMsg(DsRadarQueue &radarQueue, DsRadarMsg &radarMsg,
		      bool &end_of_vol, int &contents) 
  
{
  
  PMU_auto_register("Reading radar queue");

  end_of_vol = false;
  
  if (radarQueue.getDsMsg(radarMsg, &contents)) 
    {
      return -1;
    }

  if (contents != 0)
    {
      //
      // Reformat beam and store in RSL structs
      //
      _currRadarVol->reformat(radarMsg, contents);
      
      //
      // Check for end of volume.
      //
      if (contents & DsRadarMsg::RADAR_FLAGS) 
	{      
	  const DsRadarFlags &flags = radarMsg.getRadarFlags();
        
	  if (_params.end_of_vol_decision == Params::END_OF_VOL_FLAG) 
	    {
	      if (flags.endOfVolume) 
		{
		  end_of_vol = true;
		}
	    } 
	  else if (flags.endOfTilt &&
		   flags.tiltNum == _params.last_tilt_in_vol) 
	    {
	      end_of_vol = true;
	    } 
	} // end if (contents & DsRadarMsg::RADAR_FLAGS)
      
    } // end if (contents ! = 0);
  return 0;

}

////////////////////////////////////////////////////////////////
// 
// _processVol(): Dealias the volume if possible.
//              The James dealiaser requires that the previous
//              radar volume (if there is one) is the same 
//              size (ie. same number of tilts) as the current
//              volume. If previous and current are not the same
//              size we dont do the dealiasing. 
//

void RadxDealias::_volumeProcessWrapper(Dsr2Radar *prevRadarVol, Dsr2Radar *currRadarVol)
{

  if (_params.debug) {
      cerr << "_volumeProcessWrapper(): " << endl;
  }
 
  if (_currRadarVol->isEmpty()) {
    cerr << "Current volume is empty. No processing.\n";
    return;
  }

  Volume *currDbzVol = currRadarVol->getDbzVolume();
  Volume *currVelVol = currRadarVol->getVelVolume();
  Volume *prevVelVol = prevRadarVol->getVelVolume();
  time_t volTime = currRadarVol->getVolTime();
  
  _processVol(prevVelVol, currVelVol, currDbzVol, volTime);

}
*/

////////////////////////////////////////////////////////////////
// 
// _processVol(): Dealias the volume if possible.
//              The James dealiaser requires that the previous
//              radar volume (if there is one) is the same 
//              size (ie. same number of tilts) as the current
//              volume. If previous and current are not the same
//              size we dont do the dealiasing. 
//
void RadxDealias::_processVol(Volume *prevVelVol, Volume *currVelVol,
			      Volume *currDbzVol, time_t volTime) {

  if (_params.debug) {
      cerr << "_processVol(): " << endl;
  }
 
  // If there is a previous volume, make 
  // sure the previous and the current volumes are the same
  // size before dealiasing. The James dealiaser presently 
  // requires it.
  bool okToProceed = TRUE;

  if (prevVelVol != NULL) {
    if (prevVelVol->h.nsweeps != currVelVol->h.nsweeps) {
      okToProceed = FALSE;
    }
  }

  if (!okToProceed) {
    fprintf(stderr, "Cannot dealias velocity volumes of different sizes. Time associated with volume %ld\n", volTime);
  } else { 
     _fourDD->Dealias(prevVelVol, currVelVol, currDbzVol, volTime);
  }
}

/*
//////////////////////////////////////////////////////////////////////
//
//  _writeVol(): write current radar volume to output fmq.
//
void RadxDealias::_writeVol(DsRadarQueue &outputQueue)
{
  
  if (_params.debug) 
    {
      cerr << "_writeVol(): writing processed beams to fmq " << endl;
    }
 
  _currRadarVol->writeVol(outputQueue);

}
*/

 /*
/////////////////////////////////////////////////////////////////
//  
// _reset(): clean out _prevRadarVol for recycling,
//           _currRadarVol is set to _prevRadarVol,
//           _prevRadarVol gets set to _currRadarVol
//
void RadxDealias::_reset()

{
  if ( ! _currRadarVol->isEmpty() )
    {
      _prevRadarVol->clearData();
      
      Dsr2Radar *tmpRadarVol = _prevRadarVol;
      
      _prevRadarVol = _currRadarVol;
      
      _currRadarVol = tmpRadarVol;
    }
  else 
    _currRadarVol->clearData();


  if (_params.debug) {
    cerr << "=========== Start of volume ==================" << endl;
  }
}
 */

  /*
//////////////////////////////////////////////////
// write out the volume

int RadxDealias::_writeVol(RadxVol &vol)
{

  // output file

  GenericRadxFile outFile;
  _setupWrite(outFile);

  string outputDir = _params.output_dir;
    
  if (_params.output_filename_mode == Params::SPECIFY_FILE_NAME) {
    
    string outPath = outputDir;
    outPath += PATH_DELIM;
    outPath += _params.output_filename;

    // write to path
  
    if (outFile.writeToPath(vol, outPath)) {
      cerr << "ERROR - RadxDealias::_writeVol" << endl;
      cerr << "  Cannot write file to path: " << outPath << endl;
      cerr << outFile.getErrStr() << endl;
      return -1;
    }
    
  } else {

    // write to dir
  
    if (outFile.writeToDir(vol, outputDir,
                           _params.append_day_dir_to_output_dir,
                           _params.append_year_dir_to_output_dir)) {
      cerr << "ERROR - RadxDealias::_writeVol" << endl;
      cerr << "  Cannot write file to dir: " << outputDir << endl;
      cerr << outFile.getErrStr() << endl;
      return -1;
    }

  }

  string outputPath = outFile.getPathInUse();

  // write latest data info file if requested 
  
  if (_params.write_latest_data_info) {
    DsLdataInfo ldata(outputDir);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      ldata.setDebug(true);
    }
    string relPath;
    RadxPath::stripDir(outputDir, outputPath, relPath);
    ldata.setRelDataPath(relPath);
    ldata.setWriter(_progName);
    if (ldata.write(vol.getStartTimeSecs())) {
      cerr << "WARNING - RadxDealias::_writeVol" << endl;
      cerr << "  Cannot write latest data info file to dir: "
           << outputDir << endl;
    }
  }

  return 0;

}

  */

// convert from RadxVol to Volume structures
// These variables need to be filled:
// h.nsweeps        X
// h.nrays          X
// h.nbins
// range
// h.azimuth
// h.range_bin1
// h.gate_size
// h.elev
//  (ray) h.bias
//  (ray) h.scale
//  (ray) h.nyq_vel
//   

   /*
Volume *RadxDealias::_extractDbz(const RadxVol &radxVol) {

  // Volume
  Volume *volume = Rsl::new_volume(radxVol.getNSweeps());  

  // Sweeps
  for (int i=0; i<volume->h.nsweeps; i++) {
    Sweep *sweep = volume->sweep[i];
    sweep = Rsl::new_sweep(radxVol.getNRays());

    // Rays    
    vector<RadxRay *> radxRays = radxVol.getRays();      

    for (int j=0; j<sweep->h.nrays; j++) {

      RadxRay *radxRay = radxRays.at(j);
  
      // convert the rays
      Ray *ray = sweep->ray[j];
      ray = Rsl::new_ray(radxRay->getNGates());
      ray->h.azimuth = radxRay->getAzimuthDeg();
      ray->h.elev = radxRay->getElevationDeg();
      ray->h.bias = radxRay->getAzimuthDeg();  // TODO: bias
      ray->h.scale = radxRay->getAzimuthDeg(); // TODO: scale
      ray->h.nyq_vel = radxRay->getNyquistMps();
      // get the Range Geometry
      // void RadxVol::getPredomRayGeom(double &startRangeKm, double &gateSpacingKm)
      // radxRay->getGateRangeKm? radxRay->getGateSpacingKm?
      double startRangeKm;
      double gateSpacingKm;
      radxVol.getPredomRayGeom(startRangeKm, gateSpacingKm);
      ray->h.gate_size = startRangeKm; 
      ray->h.range_bin1 = gateSpacingKm;

      // keep the data as float; ignore the RANGE type
    
      // move the field data
      RadxField *velocityField = radxVol.getField("VEL");
      Radx::fl32 *data = velocityField->getDataFl32();

      // TODO; Hmmm, copy the data, or just let the data be modified?
      //memcpy(ray->range, data, sizeof(Radx::fl32)*ray->getNGates());
      ray->range = data;
 
    } // for each ray
  } // for each sweep  

  if (_params.debug) {
    Rsl::print_volume(volume);
  }
  return volume;
}
   */


Volume *RadxDealias::_extractFieldData(const RadxVol &radxVol, string fieldName) {


  // 1st, make sure the field is in the volume
  //  vector<string> getUniqueFieldNameList() const;
  vector<string> fieldNames = radxVol.getUniqueFieldNameList();
  vector<string>::iterator s;
  bool found = false;
  for (s=fieldNames.begin(); s != fieldNames.end(); s++) {
    if (fieldName.compare(*s) == 0) {
      found = true;
    }
  }
  if (!found) {
    cerr << "ERROR - no field found in data " << fieldName << endl;
    return NULL;
  }

  // Volume
  Volume *volume = Rsl::new_volume(radxVol.getNSweeps());  

  // Sweeps
  for (int i=0; i<volume->h.nsweeps; i++) {
    Sweep *sweep = volume->sweep[i];
    sweep = Rsl::new_sweep(radxVol.getNRays());

    // Rays    
    vector<RadxRay *> radxRays = radxVol.getRays();      

    for (int j=0; j<sweep->h.nrays; j++) {

      RadxRay *radxRay = radxRays.at(j);
  
      // convert the rays
      Ray *ray = sweep->ray[j];
      ray = Rsl::new_ray(radxRay->getNGates());
      ray->h.azimuth = radxRay->getAzimuthDeg();
      ray->h.elev = radxRay->getElevationDeg();
      ray->h.nyq_vel = radxRay->getNyquistMps();
      // get the Range Geometry
      // void RadxVol::getPredomRayGeom(double &startRangeKm, double &gateSpacingKm)
      // radxRay->getGateRangeKm? radxRay->getGateSpacingKm?
      double startRangeKm;
      double gateSpacingKm;
      radxVol.getPredomRayGeom(startRangeKm, gateSpacingKm);
      ray->h.gate_size = startRangeKm; 
      ray->h.range_bin1 = gateSpacingKm;

      // keep the data as float; ignore the RANGE type
      // NO! Keep the data as is originally is; the FourDD algorithm uses scale and bias.
      // move the field data
      RadxField *velocityField = radxRay->getField(fieldName);
      // save the original data type
      Radx::DataType_t originalDataType = velocityField->getDataType();

      //velocityField->convertToSi32();
      //      Radx::fl32 *data = velocityField->getDataFl32();
      Radx::si16 *data = velocityField->getDataSi16();
      if (data == NULL) 
	cout << "data values are NULL" << endl;
      else {
	cout << "data values for " << fieldName << " " ;
	  for (int i = 0; i< 10; i++) 
	    cout << data[i] << " ";
	cout << endl;

	//	     << data[0] << " " <<  data[1] << " " << data[3] << " " << data[4] << endl;
      }

      ray->h.bias = velocityField->getOffset();  // TODO: bias
      ray->h.scale = velocityField->getScale(); // TODO: scale

      // TODO; Hmmm, copy the data, or just let the data be modified?
      //memcpy(ray->range, data, sizeof(Radx::fl32)*ray->getNGates());
      // TODO: reinstate ...  ray->range = data;
 
    } // for each ray
  } // for each sweep  

  if (_params.debug) {
    Rsl::print_volume(volume);
  }
  return volume;
}


void RadxDealias::jamesCopyright()
{
   cerr << "** RadxDealias executes 4DD which is the Four Dimensional Dealiasing algorithm\n"
        << "** developed by the  Mesoscale Group, Department of Atmospheric Sciences,\n"
        << "** University of Washington, Seattle, WA, USA. Copyright 2001.\n\n";

}




