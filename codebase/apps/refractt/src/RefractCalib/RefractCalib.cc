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
/**
 *
 * @file RefractCalib.cc
 * @class RefractCalib
 * Main application class
 * @date 1/15/2009
 */

#include "RefractCalib.hh"
#include <Refract/RefractInput.hh>
#include <Refract/ParmApp.hh>
#include <Radx/RadxTimeList.hh>
#include <Mdv/MdvxTimeList.hh>
#include <Mdv/DsMdvx.hh>
#include <dsdata/DsUrlTrigger.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <assert.h>

//---------------------------------------------------------------------

RefractCalib::RefractCalib(int argc, char **argv)

{

  // Initialize the okay flag.

  okay = true;
  
  // Set the program name.

  _progName = "RefractCalib";
  
  // Display ucopyright message.

  ucopyright(_progName.c_str());
  
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

  // check that start and end time is set in archive mode
  
  if (_params.mode == Params::ARCHIVE) {

    if (_args.startTime == 0 || _args.endTime == 0) {
      cerr << "ERROR - must specify start and end dates." << endl << endl;
      _args.usage(_progName, cerr);
      okay = false;
    } else {
    }

  }

  if (!parmAppInit(_params, argc, argv)) {
    exit(0);
  }
  
  if (RefParms::isPrintMode()) {

    _refparms.print(stdout, PRINT_VERBOSE);

  } else {

    if (_refparms.load(RefParms::parmPath().c_str(), NULL,
		       !RefParms::isParmPrint(), false)) {

      LOG(ERROR) << "cant load " <<  RefParms::parmPath();
      okay = false;

    } else {

      _refparms.setOk();

    }

    if (RefParms::isPrintAndLoadMode()) {
      _refparms.print(stdout, PRINT_VERBOSE);
    }
    
  }
  
  parmAppFinish(_params, _refparms);

}

//---------------------------------------------------------------------
RefractCalib::~RefractCalib()
{

}

//---------------------------------------------------------------------

int RefractCalib::_init()
  
{

  // Create the requested colorscale files

  if (_params.create_colorscales) {
    _createStrengthColorscale();
    _createQualityColorscale();
  }
  
  if (_params.create_quality_colorscale) {
  }
  
  // Initialize the input handler

  RefractInput *input_handler = _refparms.initInput();

  // Initialize the processor object
  if (!_driver.init(_params, _refparms, input_handler)) {
    LOG(ERROR) << " initializing driver object";
    return -1;
  }

  return 0;

}

//---------------------------------------------------------------------
int RefractCalib::run()
{

  // Create colorscale files and quit, if requested

  if (_params.create_colorscales) {

    _createStrengthColorscale();
    _createQualityColorscale();

    cerr << "Created strength colorscale: " << _params.strength_colorscale_path << endl;
    cerr << "Created quality colorscale: " << _params.quality_colorscale_path << endl;

    return 0;

  } // if (_params.create_colorscales)
  
  // get vector list of files
  

  vector<string> fileList;
  
  if (_params.mode == FILELIST) {

    fileList = _args.inputFileList;

  } else {

    // get the files to be processed
    
    RadxTimeList tlist;
    tlist.setDir(_params.input_dir);
    tlist.setModeInterval(_args.startTime, _args.endTime);
    if (_params.aggregate_sweep_files_on_read) {
      tlist.setReadAggregateSweeps(true);
    }
    if (tlist.compile()) {
      cerr << "ERROR - RadxConvert::_runArchive()" << endl;
      cerr << "  Cannot compile time list, dir: " << _params.input_dir << endl;
      cerr << "  Start time: " << RadxTime::strm(_args.startTime) << endl;
      cerr << "  End time: " << RadxTime::strm(_args.endTime) << endl;
      cerr << tlist.getErrStr() << endl;
      return -1;
    }

  const vector<string> &paths = tlist.getPathList();

  }
    

  // initialize
  
  if (_init()) {
    return -1;
  }
  
  // Do the target identification step
  
  vector< string >
    target_id_file_list = _setupFiles(_params.target_id_file_list_n,
				      _params._target_id_file_list,
				      _params._target_files_time_range,
				      _params.target_files_host,
				      _params.target_files_path);
  vector< string >
    calib_file_list = _setupFiles(_params.calibration_file_list_n,
				  _params._calibration_file_list,
				  _params._calibration_files_time_range,
				  _params.calibration_files_host,
				  _params.calibration_files_path);

  if (target_id_file_list.empty() || calib_file_list.empty()) {
    return -1;
  }
  
  double input_gate_spacing;
  if (!_driver.findReliableTargets(target_id_file_list,
				   _params.target_files_host,
				   input_gate_spacing)) {
    return -1;
  }
  
  // Do the calibration step

  if (!_driver.calibTargets(calib_file_list,
                            _params.target_files_host,
			    input_gate_spacing)) {
    return -1;
  }
  
  return 0;
  
}

//---------------------------------------------------------------------
void RefractCalib::_createStrengthColorscale() const
{

  FILE *file;

  if ((file = fopen(_params.strength_colorscale_path, "w")) == 0) {
    LOG(ERROR) << "Error opening colorscale file for output";
    perror(_params.strength_colorscale_path);
    return;
  }
  
  for (int i = 0; i < 120; ++i) {

    double step = 6.0 * ((double)i / 120.0);
    
    short int red, green, blue;

    red = (short int)(step * 255.0 / 6.0);
    green = red;
    blue = red;
    
    double min_value = ((80.0 / 120.0) * (double)i) - 10.0;
    double max_value = ((80.0 / 120.0) * (double)(i+1)) - 10.0;
    
    fprintf(file, "%8.7f  %8.7f   #%02x%02x%02x\n",
	    min_value, max_value, red, green, blue);

  }
  
  fclose(file);
  
}

//---------------------------------------------------------------------
void RefractCalib::_createQualityColorscale() const
{

  FILE *file;

  if ((file = fopen(_params.quality_colorscale_path, "w")) == 0) {
    LOG(ERROR) << "Error opening colorscale file for output";
    perror(_params.quality_colorscale_path);
    return;
  }
  
  for (int i = 0; i < 120; ++i) {
    
    double step = 6.0 * ((double)i / 120.0);
    
    short int red, green, blue;

    red = (short int)(step * 255.0 / 6.0);
    green = red;
    blue = red;
    
    double min_value = ((1.0 / 120.0) * (double)i);
    double max_value = ((1.0 / 120.0) * (double)(i+1));
    
    fprintf(file, "%8.7f  %8.7f   #%02x%02x%02x\n",
	    min_value, max_value, red, green, blue);
  }
  
  fclose(file);

}

//---------------------------------------------------------------------
std::vector<string> RefractCalib::_setupFiles(int numFiles,
					      char ** fileList,
					      Params::Time_t *timeRange,
					      const std::string &host,
					      const std::string &filesPath)
{

  vector<string> ret;

  if (_params.file_list_inputs) {
    // load the files into the vectors from the params
    for (int i = 0; i < numFiles; ++i) {
      ret.push_back(fileList[i]);
    }
  } else {
    time_t t0 = _timeFromParams(timeRange[0]);
    time_t t1 = _timeFromParams(timeRange[1]);
    if (!_identifyFiles(host, filesPath, t0, t1, ret)) {    
      LOG(ERROR) << "Cannot compile target id files time list ";
      ret.clear();
    }
  }

  return ret;

}

//---------------------------------------------------------------------
time_t RefractCalib::_timeFromParams(const Params::Time_t &p) const
{
  DateTime dt(p.year, p.month, p.day, p.hour, p.min, p.sec);
  return dt.utime();
}

//---------------------------------------------------------------------
bool RefractCalib::_identifyFiles(const std::string &host,
				  const std::string &path, const time_t &t0,
				  const time_t &t1,
				  std::vector<string> &files) const
{

  string url = "mdvp:://";
  url = url + host;
  url = url + "::";
  url = url + path;
  DsUrlTrigger trigger(t0, t1, url, DsUrlTrigger::OBS, false);
  time_t ttime;

  while (trigger.nextTime(ttime)) {

    LOG(DEBUG) << "Checking for file at " << DateTime::strn(ttime);
    DsMdvx d;
    d.clearTimeListMode();
    d.setTimeListModeValid(url, ttime - 10000, ttime-1);
    d.compileTimeList();
    vector<time_t> times = d.getTimeList();

    if (times.empty()) {
      LOG(ERROR) << "No input data near time " << DateTime::strn(ttime)
		 << " path=" << url;
    } else {
      time_t lastT = *(times.rbegin());
      d.setReadTime(Mdvx::READ_FIRST_BEFORE, url, 0, lastT);
      if (d.readAllHeaders()) {
	LOG(ERROR) << "Cannot read the data in";
      } else {
	files.push_back(d.getPathInUse());
      }
    }

  } // while
  
  return !files.empty();

}

