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
#include <Radx/RadxPath.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <dsserver/DsLdataInfo.hh>
#include <didss/DsInputPath.hh>
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

}

//////////////////////////////////////////////////////////////
// 
// destructor
//
RadxDealias::~RadxDealias()

{

  //
  // unregister process
  //
  PMU_auto_unregister();

}

////////////////////////////////////////////////////////
//
// Run
//
int RadxDealias::Run()
{

  // create the dealiasing object and send it the input parameters
  _fourDD = new FourDD(_params);

  try {
  // build the list of files depending on the mode
  // then process the list of files 
  if (_params.mode == Params::ARCHIVE) {
    const vector<string> &fileList = _useCommandLineStartEndTimes();
    return _runWithCompleteFileList(fileList);
  } else if (_params.mode == Params::FILELIST) {
    vector<string> fileList = _useCommandLineFileList();
    cerr << "fileList ";
    for (vector<string>::iterator it = fileList.begin(); it < fileList.end(); it++) 
      cerr << *it << endl;
    return _runWithCompleteFileList(fileList);

  } else {

    // run real time mode 
    // register with procmap
    //
    PMU_auto_register("Run");

    if (_params.latest_data_info_avail) {
      return _runRealtimeWithLdata();
    } else {
      return _runRealtimeNoLdata();
    }

  }
  } catch (const char *errMsg) {
    cerr << errMsg << endl;
  }

  return 0;
}


vector<string>  RadxDealias::_useCommandLineFileList()
{
  
  if (_params.debug) {
    cerr << "RadxDealias is running ..." << endl;
    cerr << _args.inputFileList.size() << " input file(s) " << endl;
  }
  // loop through the input file list

  vector <string> inputFiles;
  for (int ii = 0; ii < (int) _args.inputFileList.size(); ii++) {
    RadxPath path(_args.inputFileList[ii]);
    if (!path.pathExists()) {
      path.setDirectory(_params.input_dir);
    }
    inputFiles.push_back(path.getPath());
  } // end for each file

  return inputFiles;
}


//
// in parameters:
// string filePath 
// function returns 0 on success; throws any exceptions encountered to calling method
//
int RadxDealias::_processOne(string filePath)
{
  
  int iret = 0;

  if (_params.debug) {
    cerr << "processingOne volume from file ..." << filePath << endl;
  }

  static Volume *prevVelVol = NULL;
  Volume *currVelVol = NULL;    
  Volume *currDbzVol = NULL;

  RadxVol vol;

  // read input file
  _readFile(filePath, vol);

  vol.loadFieldsFromRays();
  vol.remapToFinestGeom();

  vol.setNGatesConstant();

  vol.convertToFl32(); // does FourDD use signed ints? No, it seems to use floats

  // convert from RadxVol to Volume structures
  currDbzVol = _extractFieldData(vol, _params._required_fields[0], 0.0);
  // override Nyquist frequency if directed from params file
  currVelVol = _extractFieldData(vol, _params._required_fields[1], _params.nyquist_mps);

  if ((currDbzVol == NULL) || (currVelVol == NULL))
    throw "Error, velocity or reflectivity field could not be read from data file";

  time_t volTime = vol.getStartTimeSecs();

  _processVol(prevVelVol, currVelVol, currDbzVol, volTime);

  // move the unfolded data back into the RadxVol structure
  // first, move the data back to the rays, so that we can add a couple
  // new fields to the rays                                                                                             
  vol.loadRaysFromFields();

  // only the velocity should change; NOT the reflectivity
  _insertFieldData(&vol, _params._required_fields[1], currVelVol);

  // load the data back into the fields
  vol.loadFieldsFromRays();

  //vector<string> fieldNamesNow = vol.getUniqueFieldNameList();

  // write the volume data
  _writeVol(vol);

  // reset and free memory as needed
  Rsl::free_volume(prevVelVol);
  Rsl::free_volume(currDbzVol);

  // seed the next unfolding with the results
  prevVelVol = currVelVol;

  // clear all data on volume object
  vol.clear();

  return iret;
}



int RadxDealias::_runWithCompleteFileList(vector<string> fileList)
{
  
  int iret = 0;

  if (_params.debug) {
    cerr << "RadxDealias is running ..." << endl;
    cerr << fileList.size() << " input file(s) " << endl;
  }

  int nGood = 0;
  int nError = 0;

  // loop through the file list

  for (int ii = 0; ii < (int) fileList.size(); ii++) {
    if (_params.debug) {
      cerr << "reading " << fileList[ii] << endl;
    }
            
    try {
      _processOne(fileList[ii]);
      nGood++;
    } catch (const char*  errorMsg) {
      iret = -1;
      nError++;
      cerr << errorMsg << endl;
    }

    statusReport(nError, nGood);

  } // end for each file

  if (_params.debug) {
    cerr << "RadxDealias done" << endl;
    cerr << "====>> n good files processed: " << nGood << endl;
  }

  return iret;

}


//////////////////////////////////////////////////
// Run in filelist mode
void RadxDealias::statusReport(int nError, int nGood) {

  if (_params.debug) {
    cerr << "  ====>> n good files so far: " << nGood << endl;
    cerr << "  ====>> n errors     so far: " << nError << endl;
    cerr << "  ====>> sum          so far: " << nGood + nError << endl;
  }
}



//////////////////////////////////////////////////
// Run in archive mode

vector<string> RadxDealias::_useCommandLineStartEndTimes()
{

  // get the files to be processed

  RadxTimeList tlist;
  tlist.setDir(_params.input_dir);
  tlist.setModeInterval(_args.startTime, _args.endTime);
 
  if (tlist.compile()) {
    string errMsg("ERROR - RadxDealias::_runFilelist()\n");
    errMsg.append("  Cannot compile time list, dir: ");
    errMsg.append(_params.input_dir);
    errMsg.append("\n  Start time: ");
    errMsg.append(RadxTime::strm(_args.startTime));
    errMsg.append("\n  End time: ");
    errMsg.append(RadxTime::strm(_args.endTime));
    errMsg.append("\n");;
    errMsg.append(tlist.getErrStr());
    errMsg.append("\n");;
    throw errMsg.c_str();
  }

  vector<string> paths = tlist.getPathList();
  if (paths.size() < 1) {
    string errMsg("ERROR - RadxDealias::_runFilelist()\n");
    errMsg.append("  No files found, dir: ");
    errMsg.append(_params.input_dir);
    errMsg.append("\n");;
    throw errMsg.c_str();
  }
  
  return paths;

}


//////////////////////////////////////////////////
// Run in realtime mode with latest data info

int RadxDealias::_runRealtimeWithLdata()
{

  // init process mapper registration

  PMU_auto_init(_progName.c_str(), _params.instance,
                PROCMAP_REGISTER_INTERVAL);

  // watch for new data to arrive

  LdataInfo ldata(_params.input_dir,
                  _params.debug >= Params::DEBUG_VERBOSE);
  if (strlen(_params.search_ext) > 0) {
    ldata.setDataFileExt(_params.search_ext);
  }

  int iret = 0;
  int nGood = 0;
  int nError = 0;

  int msecsWait = _params.wait_between_checks * 1000;
  while (true) {
    ldata.readBlocking(_params.max_realtime_data_age_secs,
                       msecsWait, PMU_auto_register);
    const string path = ldata.getDataPath();

      try {
	_processOne(path);	
	nGood++;
      } catch (const char*  errorMsg) {
	  iret = -1;
	  nError++;
          cerr << errorMsg << endl;
      }

      statusReport(nError, nGood);

  } // end while true

  return iret;

}

//////////////////////////////////////////////////
// Run in realtime mode without latest data info

int RadxDealias::_runRealtimeNoLdata()
{

  // init process mapper registration

  PMU_auto_init(_progName.c_str(), _params.instance,
                PROCMAP_REGISTER_INTERVAL);
  
  // Set up input path

  DsInputPath input(_progName,
		    _params.debug >= Params::DEBUG_VERBOSE,
		    _params.input_dir,
		    _params.max_realtime_data_age_secs,
		    PMU_auto_register,
		    _params.latest_data_info_avail,
		    false);

  input.setFileQuiescence(_params.file_quiescence);
  input.setSearchExt(_params.search_ext);
  input.setRecursion(_params.search_recursively);
  input.setMaxRecursionDepth(_params.max_recursion_depth);
  input.setMaxDirAge(_params.max_realtime_data_age_secs);

  int iret = 0;
  int nGood = 0;
  int nError = 0;

  while(true) {

    // check for new data
    
    char *path = input.next(false);
    
    if (path == NULL) {
      
      // sleep a bit
      
      PMU_auto_register("Waiting for data");
      umsleep(_params.wait_between_checks * 1000);

    } else {

      try {
	_processOne(path);	
	nGood++;
      } catch (const char*  errorMsg) {
	  iret = -1;
	  nError++;
          cerr << errorMsg << endl;
      }

      statusReport(nError, nGood);
  
    } // end else path != NULL

  } // while

  return iret;

}

//////////////////////////////////////////////////
// Read in a file
// accounting for special cases such as gematronik
// Returns 0 on success
//         1 if already read,
//         -1 on failure

void RadxDealias::_readFile(const string &readPath,
                           RadxVol &vol)
{

  PMU_auto_register("Processing file");

  if (_params.debug) {
    cerr << "INFO - RadxDealias::_readFile" << endl;
    cerr << "  Input path: " << readPath << endl;
  }

  if (!file_exists(readPath)) {
    throw "ERROR - RadxDealias::_readFile\n  Cannot access file";
  }

  // At this point, the file must be a cfradial file;
  // otherwise, use RadxConvert to get it to cfradial    
  RadxFile inFile;
  //_setupRead(inFile);
  
  // read in file
  if (inFile.readFromPath(readPath, vol)) {  
    string errorMsg;
    errorMsg.append("ERROR - RadxDealias::_readFile\n");
    errorMsg.append("  path: ");
    errorMsg.append(readPath);
    errorMsg.append(inFile.getErrStr());
    throw errorMsg.c_str();
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "  ==>> read in file: " << readPath << endl;
  }

}

//////////////////////////////////////////////////
// write out the volume
// if errors encountered, throw a string exception
void RadxDealias::_writeVol(RadxVol &vol)
{
  
  if (_params.debug) {
    cerr << "Writing Volume ..." << endl;
  }

  RadxFile outFile;  
  string outputDir = _params.output_dir;
    
  if (_params.output_filename_mode == Params::SPECIFY_FILE_NAME) {    
    string outPath = outputDir;
    outPath += PATH_DELIM;
    outPath += _params.output_filename;

    if (_params.debug) cerr << " to " << outPath << endl;

    if (outFile.writeToPath(vol, outPath)) {
      char errMsg[1024];
      string systemErrorMsg = outFile.getErrStr();
      sprintf(errMsg, "ERROR - RadxDealias::_writeVol\n   Cannot write file to path: %s\n   %s\n",
	      outPath.c_str(), systemErrorMsg.c_str());
      throw errMsg;
    }    
  } else {
    if (_params.debug) cerr << " to " << outputDir << endl;

    if (outFile.writeToDir(vol, outputDir,
                           _params.append_day_dir_to_output_dir,
                           _params.append_year_dir_to_output_dir)) {
      char errMsg[1024];
      string systemErrorMsg = outFile.getErrStr();
      sprintf(errMsg, "ERROR - RadxDealias::_writeVol\n  Cannot write file to dir: %s\n   %s\n",
	      outputDir.c_str(), systemErrorMsg.c_str());
      throw errMsg;
    }
  }

  if (_params.debug) {
    cerr << "Finished writing volume" << endl;
  }
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

Volume *RadxDealias::_extractFieldData(const RadxVol &radxVol, string fieldName, float override_nyquist_vel) {

  if (_params.debug) {
    cerr << " looking for data in field " << fieldName << endl;
  }

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
    Sweep **sweeps = volume->sweep;
    Sweep *newSweep = Rsl::new_sweep(radxVol.getNRays());
    sweeps[i] = newSweep;

    // Rays    
    Ray **rays = newSweep->ray;

    vector<RadxRay *> radxRays = radxVol.getRays();      
    for (int j=0; j<newSweep->h.nrays; j++) {

      RadxRay *radxRay = radxRays.at(j);

      // convert the rays
      Ray *newRay = Rsl::new_ray(radxRay->getNGates());
      rays[j] = newRay;
      newRay->h.azimuth = radxRay->getAzimuthDeg();
      newRay->h.elev = radxRay->getElevationDeg();

      if (override_nyquist_vel != 0.0) {
	newRay->h.nyq_vel = override_nyquist_vel;
      } else {
	newRay->h.nyq_vel = radxRay->getNyquistMps();
      }

      // TRMM RSL wants altitude in meters
      newRay->h.alt = radxVol.getAltitudeKm()*1000.0;

      // get the Range Geometry
      if (!radxRay->getRangeGeomSet())
	radxRay->copyRangeGeomFromFields();
      // trmm rsl expects gate size and distance to first gate in meters 
      newRay->h.gate_size = radxRay->getStartRangeKm() * 1000.0; 
      newRay->h.range_bin1 = radxRay->getGateSpacingKm() * 1000.0;

      // move the field data
      RadxField *velocityField = radxRay->getField(fieldName);
      // check the original data type
      Radx::DataType_t originalDataType = velocityField->getDataType();
      if (originalDataType != Radx::FL32)
        throw "Error - Expected float 32 data";
      Radx::fl32 *data = velocityField->getDataFl32();

      if (data == NULL) 
	cout << "data values are NULL" << endl;
      else {
        if (0) { // _params.debug) {
	  cout << "data values for " << fieldName << " " ;
	  for (int i = 0; i< 10; i++) 
	    cout << data[i] << " ";
	  cout << endl;
	}
      }

      newRay->h.bias = velocityField->getOffset();
      newRay->h.scale = velocityField->getScale();

      // copy the data ...

      newRay->range = (Range *) malloc(sizeof(Range) * newRay->h.nbins);
      newRay->h.binDataAllocated = true;
      memcpy(newRay->range, data, sizeof(Range) * newRay->h.nbins);

    } // for each ray
  } // for each sweep  

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    Rsl::print_volume(volume);
  }
  return volume;
}


void RadxDealias::_insertFieldData(RadxVol *radxVol, string fieldName, Volume *volume) {

  if (_params.debug) {
    cerr << " inserting data into field " << fieldName << endl;
  }

  // 1st, make sure the field is in the volume
  vector<string> fieldNames = radxVol->getUniqueFieldNameList();
  vector<string>::iterator s;
  bool found = false;
  for (s=fieldNames.begin(); s != fieldNames.end(); s++) {
    if (fieldName.compare(*s) == 0) {
      found = true;
    }
  }
  if (!found) {
    cerr << "ERROR - no field found in data " << fieldName << endl;
    return;
  }

  // loop over the rays in the RadxVol, and pull the specific data from the trmml Volume->Sweep... structure
  // fetch the total number of rays (#sweeps * #rays)                                                                   
  vector<RadxRay *> currentRays = radxVol->getRays();

  int rayNum = 0;
  string unfoldedName = fieldName + "_UNF";

  // for each ray of data                                                                                             
  for (vector<RadxRay *>::iterator r=currentRays.begin(); r<currentRays.end(); r++) {
      RadxRay *radxRay = *r;

	// TODO: verify this ... we may want the management to go to the RadxVol?
      bool isLocal = true;
      int sweepNumber = radxRay->getSweepNumber() - 1;
      Radx::fl32 *newData = volume->sweep[sweepNumber]->ray[rayNum]->range;
      int nGates = volume->sweep[sweepNumber]->ray[rayNum]->h.nbins;
      // pull the missing value  from the associated RadxField
      RadxField *radxField = radxRay->getField(fieldName);
      double missingValue = (double) radxField->getMissingFl32();
      // get the units; this should be pulled from the associate RadxRay
      string units = radxField->getUnits();

      // Note: discarding the field returned, after adding it to the ray, since we don't need it.
      radxRay->addField(unfoldedName, units, nGates, missingValue, newData, isLocal);

      rayNum += 1;

    } // for each ray

    vector<string> fieldNamesNew = radxVol->getUniqueFieldNameList();

}

void RadxDealias::jamesCopyright()
{
   cerr << "** RadxDealias executes 4DD which is the Four Dimensional Dealiasing algorithm\n"
        << "** developed by the  Mesoscale Group, Department of Atmospheric Sciences,\n"
        << "** University of Washington, Seattle, WA, USA. Copyright 2001.\n\n";

}
