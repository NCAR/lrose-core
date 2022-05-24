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

#include <stdexcept>
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
#include "FirstGuess.hh"

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

bool RadxDealias::tdrp_bool_t_to_bool(tdrp_bool_t value) {  // convert from debug_t
  return (value == pTRUE); 
}

////////////////////////////////////////////////////////
//
// Run
//
int RadxDealias::Run()
{

  // create the dealiasing object and send it the input parameters
  /* these are the input parameters FourDD needs ...
  params = parameters;
  (params.debug)
   params.sounding_url );
   params.sounding_look_back*60,
   params.wind_alt_min*1000,
   params.wind_alt_max*1000,
   params.avg_wind_u,
   params.avg_wind_v );
   params.prep) {
   params.filt, &unfoldSuccess);
   params.output_soundVol) {
   params.max_shear &&                                
   params.sign<0)        
   params.del_num_bins; i++) {
   params.no_dbz_rm_rv==1)
   params.low_dbz)||
   params.high_dbz))&&
   params.angle_variance;
   params.comp_thresh>1.0 || 
   params.comp_thresh2;
   params.strict_first_pass) {                                                        
   params.max_count)                                            
   params.ck_val) {
   params.proximity;   
   params.min_good)           
   params.std_thresh*NyqVelocity) 
   params.epsilon
  */
  // note: missing data value is set when extracting the velocity field  
  //  _fourDD = new FourDD(_params);

  bool param_debug = (_params.debug > Params::DEBUG_NORM);  // convert from debug_t
  bool param_prep = tdrp_bool_t_to_bool(_params.prep);   
  bool param_filt = tdrp_bool_t_to_bool(_params.filt); 
  bool param_dbz_rm_rv = tdrp_bool_t_to_bool(_params.dbz_rm_rv);
  bool param_strict_first_pass = tdrp_bool_t_to_bool(_params.strict_first_pass);

  if (sizeof(Radx::fl32) != sizeof(float))
    throw "Incompatible size of float and size of Radx::fl32";


 _fourDD = new FourDD(
		      param_debug,
		      _params.sounding_url,
		      (float) _params.sounding_look_back,
		      (float) _params.wind_alt_min,
		      (float) _params.wind_alt_max,
		      (float) _params.avg_wind_u,
		      (float) _params.avg_wind_v,
		      param_prep,
		      param_filt,
		      (float) _params.max_shear,         
		      _params.sign,
		      _params.del_num_bins,
		      param_dbz_rm_rv,
		      (float) _params.low_dbz,
		      (float) _params.high_dbz,
		      (float) _params.angle_variance,
		      (float) _params.comp_thresh,
		      (float) _params.comp_thresh2,
                      (float) _params.thresh,
		      param_strict_first_pass,
		      _params.max_count,                   
		      (float) _params.ck_val,
		      _params.proximity,
		      _params.min_good,
		      (float) _params.std_thresh);


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

  vol.sortSweepRaysByAzimuth();

  vol.loadFieldsFromRays();

  // TODO: force same geometry ...
  //void remapToPredomGeom(); 
  //then                      
  //remapRangeGeom(double startRangeKm,
  //             double gateSpacingKm, 
  //             bool interp = false);  
  vol.remapToFinestGeom();

  vol.setNGatesConstant();

  vol.convertToFl32(); // does FourDD use signed ints? No, it seems to use floats
 
  string velocityFieldName = _params._required_fields[1];
  float nyquist_mps = 0.0; 
  if (_params.nyquist_mps != 0.0) { // then use the Nyquist frequency from the the param file
    nyquist_mps = _params.nyquist_mps;
  } else { // estimate the Nyquist frequency from the max velocity of the volume
    vol.estimateSweepNyquistFromVel(velocityFieldName);
    // this puts the estimate in each ray; to be used in the extractFieldData step below
  }

  // convert from RadxVol to Volume structures
  
  // extract the reflectivity field 
  currDbzVol = _extractFieldData(vol, _params._required_fields[0]);
  // override Nyquist frequency if directed from params file
  //  currVelVol = _extractFieldData(vol, _params._required_fields[1], _params.nyquist_mps);

  /*  
  // override missing values             
  // missing values are kept with a RadxField                                                                               
  if (_params.override_missing_field_values) {
    // TODO: use these variables? or missing_velocity?
    Radx::fl32 missingValue = _params.missing_field_fl32; 
    RadxField *field;
    field->setMissingFl32(missingValue);
    cout << "overriding missing value from " << vol.getMissingFl32();
    vol.setMissingFl32(missingValue);
    cout << " to " << vol.getMissingFl32 << endl;
  }
  */
  
  currVelVol = _extractVelocityFieldData(vol, velocityFieldName, nyquist_mps, 
    _params.override_missing_field_values, _params.velocity_field_missing_value);
  
  if ((currDbzVol == NULL) || (currVelVol == NULL))
    throw "ERROR, velocity or reflectivity field could not be read from data file";

  time_t volTime = vol.getStartTimeSecs();

  bool param_output_soundVol = tdrp_bool_t_to_bool(_params.output_soundVol);

  _processVol(prevVelVol, currVelVol, currDbzVol, volTime, param_output_soundVol);

  // move the unfolded data back into the RadxVol structure
  // first, move the data back to the rays, so that we can add a couple
  // new fields to the rays                                                                                             
  //vol.loadRaysFromFields();

  try {
  // only the velocity should change; NOT the reflectivity
  _insertFieldData(&vol, _params._required_fields[1], currVelVol);
  } catch (const char* ex) {
    cout << ex << endl;
  }

  // load the data back into the fields
  vol.loadFieldsFromRays();

  //vector<string> fieldNamesNow = vol.getUniqueFieldNameList();

  // write the volume data
  _writeVol(vol);

  // reset and free memory as needed
  Rsl::free_volume(prevVelVol);
  Rsl::free_volume(currDbzVol);

  // TODO: HERE <<=== trying to figure out why previous volume is all missing
  // seed the next unfolding with the results
  //prevVelVol = currVelVol;

  // TODO: does this affect currVelVol?
  bool copy_with_debug = false;
  prevVelVol = Rsl::copy_volume(currVelVol, copy_with_debug);
  //Volume *Rsl::copy_volume(Volume *v, bool debug)
  // TODO: free prevVelVol?? 

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
    } catch (...) { // const char*  errorMsg) {
      iret = -1;
      nError++;
      //      cout << errorMsg << endl;
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
			      Volume *currDbzVol, time_t volTime,
			      bool output_soundVol) {

  if (_params.debug) {
    cerr << "_processVol(): " << endl;
  }
 
  if (currVelVol == NULL) {
    throw std::invalid_argument("currVelVol is NULL");
  }

  // If there is a previous volume, make 
  // sure the previous and the current volumes are the same
  // size before dealiasing. The James dealiaser presently 
  // requires it.

  if (prevVelVol != NULL) {
    if (prevVelVol->h.nsweeps != currVelVol->h.nsweeps) {
      fprintf(stderr, "Cannot dealias velocity volumes of different sizes. Time associated with volume %ld\n", volTime);
      return; 
    }
  }

  Volume *soundVolume = NULL;
  // copy velocity to soundVolume
  soundVolume = Rsl::copy_volume(currVelVol);

  FirstGuess firstGuess(
			_params.debug >= Params::DEBUG_VERBOSE,
			_params.sounding_url,
			(float) _params.sounding_look_back,
			(float) _params.wind_alt_min,
			(float) _params.wind_alt_max,
			(float) _params.avg_wind_u,
			(float) _params.avg_wind_v,
			(float) _params.max_shear,         
			_params.sign);
  // try to find a good sounding
  bool firstGuessSuccess = firstGuess.firstGuess(soundVolume, volTime); 

  bool firstGuessFailed = !firstGuessSuccess;
  if (firstGuessFailed) {
    // clean up
      Rsl::free_volume(soundVolume);
      soundVolume = NULL;
  }

  if (output_soundVol) {
    // write and exit; do not unfold
    if (soundVolume != NULL) {
      _replaceVelocityWithSounding(currVelVol, soundVolume);
      Rsl::free_volume(soundVolume);
    }
  } else {
    // Unfold Volume if we have either a previous volume or VAD data
    if (soundVolume != NULL  || prevVelVol != NULL) {
      _fourDD->Dealias(prevVelVol, currVelVol, currDbzVol, soundVolume);
    }
  }

  // TODO: maybe we need a final clean up to free any memory from Rsl::Volume structure

    /* -------

  if (output_soundVol) {
    // write sound volume and exit; do not unfold
    if (soundVolume != NULL) {
      _outputSoundVolume(currVelVol, soundVolume);
      Rsl::free_volume(soundVolume);
    } else {
      fprintf(stderr, "\nFirst guess using sounding failed\n");
    } 
  } else {
    // try to unfold
    if (!firstGuessSuccess) {
      Rsl::free_volume(soundVolume);
      soundVolume = NULL;
    }

    //
    // Unfold Volume if we have either a previous volume or VAD data
    //
    if (firstGuessSuccess  || prevVelVol != NULL) {
      _fourDD->Dealias(prevVelVol, currVelVol, currDbzVol, soundVolume);
    }
  }
    */

}

void RadxDealias::_replaceVelocityWithSounding(Volume *currVelVol, Volume *soundVolume) {

  int nSweeps = currVelVol->h.nsweeps;

  /*
(lldb) p nSweeps
(int) $0 = 11
(lldb) p nRays
(int) $1 = 482
(lldb) p nBins
(int) $2 = 996
  */
  // currVelVol->sweep[1]->ray[481] = NULL = soundVolume->sweep[i]->ray[j]
  // (lldb) p currVelVol->sweep[i]->ray[j]
  // (Ray *) $7 = 0x0000000000000000

     // NOTE: the number of rays can be different for each sweep!
     // TODO: do we need to make the number of rays the same for each sweep?
     // Q: Will the FourDD algorithm work if the number of rays differ among the sweeps?
     // A: Maybe.  Let's try it.
     // NOTE: the number of gates can vary between different sweeps
     for (int i = 0; i < nSweeps;  i++) {
       int nRays = currVelVol->sweep[i]->h.nrays;

       for (int j = 0; j < nRays; j ++) {
	 int nBins = currVelVol->sweep[i]->ray[j]->h.nbins;

	 for (int k = 0; k < nBins; k++) {
	   //cout << "currVelVol->sweep[" << i << "]->ray[" << j << "]->range[" << k <<"] = " <<
	   //currVelVol->sweep[i]->ray[j]->range[k] <<
	   //" => " <<
	   //"soundVolume->sweep[" << i << "]->ray[" << j << "]->range[" << k << "]= " <<
	   //soundVolume->sweep[i]->ray[j]->range[k] << endl;

  	   currVelVol->sweep[i]->ray[j]->range[k] = soundVolume->sweep[i]->ray[j]->range[k];
	 }
       }
     }

  fprintf(stderr, "\nREPLACED VELOCITY DATA WITH SOUNDING DATA!!\n");

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

Volume *RadxDealias::_extractFieldData(const RadxVol &radxVol, string fieldName) {

  char message[1024];

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
    sprintf(message, "ERROR - no field %s found in data", fieldName.c_str());
    cout << message << endl;
    throw std::invalid_argument(message);
  }

  // Volume
  Volume *volume = Rsl::new_volume(radxVol.getNSweeps());  

  // Sweeps
  vector<RadxSweep *> radxSweeps = radxVol.getSweeps();
  size_t rslVolumeIdx = 0;
  std::vector<RadxSweep *>::iterator itSweep;
  for (itSweep = radxSweeps.begin(); itSweep != radxSweeps.end(); ++itSweep) {
    //for (int i=0; i<volume->h.nsweeps; i++) {

    // TODO: sometimes the sweeps are numbered 0 ... n-1, sometimes 1 ... n
    // So, get a vector of the sweeps?
    // The problem is Rsl::Volume has the array index tied to the sweep number
    //  Fix? Maybe just loop through the sweeps in order and NOT index by sweep number???
    // 
    const RadxSweep *radxSweep = *itSweep; // radxVol.getSweepByNumber(i);
    //    if (radxSweep == NULL) {
    //  sprintf(message, "ERROR - sweep %d not found for field %s in data", i, fieldName.c_str());
    //  cout << message << endl;
      // TODO: free memory allocated; clean up ...
    //  throw message;
    //}

    Sweep **sweeps = volume->sweep;
    Sweep *newSweep = Rsl::new_sweep(radxSweep->getNRays());
    sweeps[rslVolumeIdx] = newSweep;

    // Rays    
    Ray **rays = newSweep->ray;

    size_t startRayIndex = radxSweep->getStartRayIndex();
    size_t endRayIndex = radxSweep->getEndRayIndex();
    if (_params.debug) {
      cout << "for RadxSweep " << rslVolumeIdx << ": startRayIndex=" << startRayIndex <<
        " endRayIndex=" << endRayIndex << endl;
    }

    vector<RadxRay *> radxRays = radxVol.getRays();      
    for (int j=0; j<newSweep->h.nrays; j++) {

      if (startRayIndex+j > endRayIndex)
        throw "ERROR: _extractFieldData: Ray index out of bounds";
      RadxRay *radxRay = radxRays.at(startRayIndex+j);

      size_t nGates = radxRay->getNGates();
      Rsl::setMaxBinsInSweep(newSweep, nGates);

      // convert the rays
      Ray *newRay = Rsl::new_ray(radxRay->getNGates());
      rays[j] = newRay;
      newRay->h.azimuth = radxRay->getAzimuthDeg();
      newRay->h.elev = radxRay->getElevationDeg();

      /*
      if (override_nyquist_vel != 0.0) {
	newRay->h.nyq_vel = override_nyquist_vel;
      } else {
	newRay->h.nyq_vel = radxRay->getNyquistMps();
      }
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cout << "Using " << newRay->h.nyq_vel << " as Nyquist Velocity" << endl;
      }
      */

      // TRMM RSL wants altitude in meters
      newRay->h.alt = radxVol.getAltitudeKm()*1000.0;

      // get the Range Geometry
      if (!radxRay->getRangeGeomSet())
	radxRay->copyRangeGeomFromFields();
      // trmm rsl expects gate size and distance to first gate in meters 
      newRay->h.gate_size = radxRay->getStartRangeKm() * 1000.0; 
      newRay->h.range_bin1 = radxRay->getGateSpacingKm() * 1000.0;

      // move the field data
      RadxField *radxField = radxRay->getField(fieldName);
      // check the original data type
      Radx::DataType_t originalDataType = radxField->getDataType();
      if (originalDataType != Radx::FL32)
        throw "Error - Expected float 32 data";
      Radx::fl32 *data = radxField->getDataFl32();

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

      newRay->h.bias = 0.0; //  radxField->getOffset();
      newRay->h.scale = 1.0; // radxField->getScale();

      // pull the missing value  from the associated RadxField
      // TODO: this gets set multiple times, but I cannot think
      // of a better way to do this right now. 
      volume->h.missing = radxField->getMissingFl32();

      // copy the data ...

      newRay->range = new Range[newRay->h.nbins]; // (Range *) malloc(sizeof(Range) * newRay->h.nbins);
      newRay->h.binDataAllocated = true;
      memcpy(newRay->range, data, sizeof(Range) * newRay->h.nbins);

    } // for each ray
    rslVolumeIdx += 1;

  } // for each sweep  

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    Rsl::print_volume(volume);
  }
  return volume;
}


Volume *RadxDealias::_extractVelocityFieldData(const RadxVol &radxVol, string fieldName,
					       float override_nyquist_vel,
					       bool override_missing_field_values,
					       float velocity_field_missing_value) {

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

  Radx::fl32 missing = 0.0;

  // Volume
  Volume *volume = Rsl::new_volume(radxVol.getNSweeps());  


  // Sweeps
  vector<RadxSweep *> radxSweeps = radxVol.getSweeps();
  size_t rslVolumeIdx = 0;
  std::vector<RadxSweep *>::iterator itSweep;
  for (itSweep = radxSweeps.begin(); itSweep != radxSweeps.end(); ++itSweep) {
    //for (int i=0; i<volume->h.nsweeps; i++) {

    // TODO: sometimes the sweeps are numbered 0 ... n-1, sometimes 1 ... n
    // So, get a vector of the sweeps?
    // The problem is Rsl::Volume has the array index tied to the sweep number
    //  Fix? Maybe just loop through the sweeps in order and NOT index by sweep number???
    // 
    const RadxSweep *radxSweep = *itSweep; // radxVol.getSweepByNumber(i);

    Sweep **sweeps = volume->sweep;
    Sweep *newSweep = Rsl::new_sweep(radxSweep->getNRays());
    sweeps[rslVolumeIdx] = newSweep;

    // Rays    
    Ray **rays = newSweep->ray;

    size_t startRayIndex = radxSweep->getStartRayIndex();
    size_t endRayIndex = radxSweep->getEndRayIndex();
    if (_params.debug) {
      cout << "for RadxSweep " << rslVolumeIdx << ": startRayIndex=" << startRayIndex <<
        " endRayIndex=" << endRayIndex << endl;
    }

    vector<RadxRay *> radxRays = radxVol.getRays();      
    for (int j=0; j<newSweep->h.nrays; j++) {
 
      if (j == 481)
        bool here = TRUE;

      if (startRayIndex+j > endRayIndex) 
	throw "ERROR: _extractVelocityFieldData: Ray index out of bounds";
      RadxRay *radxRay = radxRays.at(startRayIndex+j);

      size_t nGates = radxRay->getNGates();
      Rsl::setMaxBinsInSweep(newSweep, nGates);

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
      //      if (_params.debug >= Params::DEBUG_VERBOSE) {
      //	cout << "Using " << newRay->h.nyq_vel << " as Nyquist Velocity" << endl;
      //      }

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
        if (_params.debug) {
	  // print the first 10 rays and the last ray's data
	  if ((j < 10) || (j == newSweep->h.nrays-1)) {
	    cout << "data values for sweep=" << rslVolumeIdx << 
	      " ray=" << j << " fieldName=" << fieldName << " " ;
	    for (int i = 0; i< 50; i++) 
	      cout << data[i] << " ";
	    cout << endl;
	  }
	}
      }

      // pull the missing value from the associated RadxField
      missing = velocityField->getMissingFl32();

      // These should not be needed, or set to 1.0 & 0.0
      // because we are pulling the Fl32 data which have
      // the scale and bias applied
     
      newRay->h.bias = 0.0; // velocityField->getOffset();
      newRay->h.scale = 1.0; // velocityField->getScale();

      // copy the data ...

      // TODO: be sure to free this memory!! 
      newRay->range = (Range *) malloc(sizeof(Range) * newRay->h.nbins);
      newRay->h.binDataAllocated = true;
      memcpy(newRay->range, data, sizeof(Range) * newRay->h.nbins);

    } // for each ray
    rslVolumeIdx += 1;
  } // for each sweep  

  if (override_missing_field_values) {
    volume->h.missing = velocity_field_missing_value;
  } else {
    // pull the missing value from the associated RadxField
    volume->h.missing = missing; 
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    Rsl::print_volume(volume);
  }
  return volume;
}


// insert in the same order as the extraction; the rays should NOT have moved
void RadxDealias::_insertFieldData(RadxVol *radxVol, string fieldName, Volume *volume) {

  Radx::fl32 missing = 0.0;

  string unfoldedName = fieldName + "_UNF";


  // Sweeps
  vector<RadxSweep *> radxSweeps = radxVol->getSweeps();
  size_t rslVolumeIdx = 0;
  std::vector<RadxSweep *>::iterator itSweep;
  for (itSweep = radxSweeps.begin(); itSweep != radxSweeps.end(); ++itSweep) {
    //for (int i=0; i<volume->h.nsweeps; i++) {

    // TODO: sometimes the sweeps are numbered 0 ... n-1, sometimes 1 ... n
    // So, get a vector of the sweeps?
    // The problem is Rsl::Volume has the array index tied to the sweep number
    //  Fix? Maybe just loop through the sweeps in order and NOT index by sweep number???
    // 
    const RadxSweep *radxSweep = *itSweep; // radxVol.getSweepByNumber(i);

    // Rays    
    //Ray **rays = newSweep->ray;
    Sweep *newSweep = volume->sweep[rslVolumeIdx];

    size_t startRayIndex = radxSweep->getStartRayIndex();
    size_t endRayIndex = radxSweep->getEndRayIndex();
    if (_params.debug) {
      cout << "for RadxSweep " << rslVolumeIdx << ": startRayIndex=" << startRayIndex <<
        " endRayIndex=" << endRayIndex << endl;
    }

    vector<RadxRay *> radxRays = radxVol->getRays();      
    for (int j=0; j<newSweep->h.nrays; j++) {

      int sweepNumber = rslVolumeIdx;
      int rayNumRadx = startRayIndex+j;
      int rayNumVolume = j;

      if (rayNumRadx > endRayIndex)
        throw "ERROR: _insertFieldData: Ray index out of bounds";
      RadxRay *radxRay = radxRays.at(rayNumRadx);

      Radx::fl32 *newData = volume->sweep[sweepNumber]->ray[rayNumVolume]->range;
      int nGates = volume->sweep[sweepNumber]->ray[rayNumVolume]->h.nbins;
      // pull the missing value  from the associated RadxField
      RadxField *radxField = radxRay->getField(fieldName);

      //  pull the missing and nyquist values from the RSL structures; they should have propogated ...
      double missingValue = volume->h.missing; // (double) radxField->getMissingFl32();
      
      // get the units; this should be pulled from the associate RadxRay
      string units = radxField->getUnits();
      bool isLocal = TRUE;
      // Note: discarding the field returned, after adding it to the ray, since we don't need it.
      radxRay->addField(unfoldedName, units, nGates, missingValue, newData, isLocal);
      //---------

    } // for each ray
    rslVolumeIdx += 1;
  } // for each sweep  
}

/*

// insert in the same order as the extraction; the rays should NOT have moved
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

  // first, move the data back to the rays, so that we can add a couple
  // new fields to the rays                                                                                             
  //radxVol->loadRaysFromFields();


  // loop over the rays in the RadxVol, and pull the specific data from the trmml Volume->Sweep... structure
  // fetch the total number of rays (#sweeps * #rays)                                                                   
  vector<RadxRay *> currentRays = radxVol->getRays();

  int rayNum = 0;
  string unfoldedName = fieldName + "_UNF";
  char errorMsg[1024];

  // for each sweep ...
  int radxVolNSweeps = radxVol->getNSweeps();
  if (radxVolNSweeps != volume->h.nsweeps)
    throw "ERROR: _insertFieldData: number of sweeps in RadxVol and unfolded volume are different\n";

  int sweepNumber = 0;

  // for each ray of data                                                                                             
  for (vector<RadxRay *>::iterator r=currentRays.begin(); r<currentRays.end(); r++) {
    RadxRay *radxRay = *r;

    // TODO: verify this ... we may want the management to go to the RadxVol?
    bool isLocal = true;
    // TODO: the sweepNumber is NOT in the radxRay 
    int sweepNumber = radxRay->getSweepNumber();
    int rayNumber = radxRay->getRayNumber();
    cout << "sweepNumber=" << sweepNumber << " rayNumber=" << rayNumber << endl;

    //for (int sweepNumber=0; sweepNumber<volume->h.nsweeps; sweepNumber++) {

      // validate data bounds before access
      if ((sweepNumber < 0) || (sweepNumber >= volume->h.nsweeps)) {
	sprintf(errorMsg, "_insertFieldData: sweepNumber %d out of bounds %d", sweepNumber, volume->h.nsweeps);
	throw errorMsg; 
      }
      int maxRays = volume->sweep[sweepNumber]->h.nrays;
      if ((rayNum < 0) || (rayNum >= maxRays)) {
	sprintf(errorMsg, "_insertFieldData: rayNum %d out of bounds %d", rayNum, maxRays);
	throw errorMsg; 
      }
      Radx::fl32 *newData = volume->sweep[sweepNumber]->ray[rayNum]->range;
      int nGates = volume->sweep[sweepNumber]->ray[rayNum]->h.nbins;
      // pull the missing value  from the associated RadxField
      RadxField *radxField = radxRay->getField(fieldName);

      //  pull the missing and nyquist values from the RSL structures; they should have propogated ...
      double missingValue = volume->h.missing; // (double) radxField->getMissingFl32();
      
      // get the units; this should be pulled from the associate RadxRay
      string units = radxField->getUnits();

      // Note: discarding the field returned, after adding it to the ray, since we don't need it.
      radxRay->addField(unfoldedName, units, nGates, missingValue, newData, isLocal);

      if (newData == NULL) 
	cout << "data values are NULL" << endl;
      else {
	if (_params.debug) {
	  cout << "data values for sweep=" << sweepNumber << " ray=" << rayNum << " fieldName=" << unfoldedName << " " ;
	  for (int i = 0; i< 10; i++) 
	    cout << newData[i] << " ";
	  cout << endl;
	}
      }

      // }
    rayNum += 1;
    sweepNumber += 1;
  } // for each ray

  vector<string> fieldNamesNew = radxVol->getUniqueFieldNameList();

}
*/
void RadxDealias::jamesCopyright()
{
   cerr << "** RadxDealias executes 4DD which is the Four Dimensional Dealiasing algorithm\n"
        << "** developed by the  Mesoscale Group, Department of Atmospheric Sciences,\n"
        << "** University of Washington, Seattle, WA, USA. Copyright 2001.\n\n";

}
