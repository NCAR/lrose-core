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
///////////////////////////////////////////////////////////////
// PrintMdv.cc
//
// PrintMdv object
//
// Paddy McCarthy, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 1999
//
///////////////////////////////////////////////////////////////

#include "PrintMdv.hh"
#include <didss/DsDataFile.hh>
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxChunk.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxRadar.hh>
#include <Mdv/MdvxTimeStamp.hh>
#include "X11ColorMap.h"

#include <QImage>
#include <QColor>

using namespace std;

// Constructor

PrintMdv::PrintMdv(char *filePath)
  
{
  int size = strlen(filePath);
  _filePath.assign(filePath, size);
/*
  OK = TRUE;

  // set programe name

  _progName = "PrintMdv";
  
  // parse command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args." << endl;
    OK = FALSE;
    return;
  }
  
  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list, &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = FALSE;
    return;
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _printSizes(cerr);
  }

  // parse the DateTime objects

  _readSearchTime = DateTime::parseDateTime(_params.read_search_time);
  if (_readSearchTime == DateTime::NEVER) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot parse read_search_time: "
	 << "\"" << _params.read_search_time << "\"" << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = FALSE;
  }
    
  _latestValidModTime = DateTime::parseDateTime(_params.latest_valid_mod_time);
  if (_latestValidModTime == DateTime::NEVER) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot parse latest_valid_mod_time: "
	 << "\"" << _params.latest_valid_mod_time << "\"" << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = FALSE;
  }
    
  _timeListStartTime = DateTime::parseDateTime(_params.time_list_start);
  if (_timeListStartTime == DateTime::NEVER) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot parse time_list_start time: "
	 << "\"" << _params.time_list_start << "\"" << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = FALSE;
  }
    
  _timeListEndTime = DateTime::parseDateTime(_params.time_list_end);
  if (_timeListEndTime == DateTime::NEVER) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot parse time_list_end time: "
	 << "\"" << _params.time_list_end << "\"" << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = FALSE;
  }
    
  _timeListGenTime = DateTime::parseDateTime(_params.time_list_gen);
  if (_timeListGenTime == DateTime::NEVER) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot parse time_list_gen time: "
	 << "\"" << _params.time_list_gen << "\"" << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = FALSE;
  }
    
  _timeListSearchTime = DateTime::parseDateTime(_params.time_list_search);
  if (_timeListSearchTime == DateTime::NEVER) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot parse time_list_search time: "
	 << "\"" << _params.time_list_search << "\"" << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = FALSE;
  }
    
  return;
*/
}

// destructor

PrintMdv::~PrintMdv()

{

}

//////////////////////////////////////////////////
// Run

int PrintMdv::Run()
{

  // set up Mdvx object

  DsMdvx *mdvx;
  //if (_params.threaded) {
  //  mdvx = new DsMdvxThreaded;
  //} else {
    mdvx = new DsMdvx;
  //}

  // set up the read specs

  _setupRead(mdvx, _filePath);

  // testing
  /*
  if (_params.test_no_print) {
    if (_doTest(mdvx) == 0) {
      delete mdvx;
      return 0;
    } else {
      delete mdvx;
      return -1;
    }
  }
  */
  // volume data

  int iret = 0;

  //if (_params.debug >= Params::DEBUG_VERBOSE) {
    mdvx->setDebug();
  //}
  //if (_params.debug) {
    mdvx->printReadRequest(cerr);
  //}


/*
  if (_params.get_mode == Params::GET_VOLUME ||
      _params.get_mode == Params::GET_GIS ||
      _params.get_mode == Params::GET_TABLE) {
*/
    if (_handleVolume(mdvx)) {
      iret = -1;
    }
/*
  } else if (_params.get_mode == Params::GET_VSECTION) {

    // vert section
*/
    if (_handleVsection(mdvx)) {
      iret = -1;
    }
/*
  } else if (_params.get_mode == Params::GET_ALL_HEADERS) {

    // all headers
*/
    if (_handleAllHeaders(mdvx)) {
      iret = -1;
    }
/*
  } else if (_params.get_mode == Params::GET_TIME_LIST) {
*/
    if (_handleTimeList(mdvx)) {
      iret = -1;
    }
/*
  } else if (_params.get_mode == Params::GET_TIME_HEIGHT) {
*/    
    if (_handleTimeHeight(mdvx)) {
      iret = -1;
    }

 // }
  
  delete mdvx;
  return iret;

}

//////////////////////////////////////////////////
// set up read for MDV

void PrintMdv::_setupRead(DsMdvx *mdvx, string filePath)
{
/*
  if (_params.set_latest_valid_mod_time) {
    mdvx->setCheckLatestValidModTime(_latestValidModTime);
  }
  
  if (_params.file_format == Params::FORMAT_XML) {
    mdvx->setReadFormat(Mdvx::FORMAT_XML);
    mdvx->setWriteFormat(Mdvx::FORMAT_XML);
    if (_params.debug) {
      cerr << "Using XML format" << endl;
    }    
  } else if (_params.file_format == Params::FORMAT_NCF) {
    mdvx->setReadFormat(Mdvx::FORMAT_NCF);
    mdvx->setWriteFormat(Mdvx::FORMAT_NCF);
    if (_params.debug) {
      cerr << "Using NCF format" << endl;
    }    
  }

  if (_params.debug) {
    cerr << "================== URL ================" << endl;
    DsURL url(_params.url);
    url.print(cerr);
    cerr << "=======================================" << endl;
  }
    
  if (_params.get_mode != Params::GET_TIME_LIST &&
      _params.get_mode != Params::GET_TIME_HEIGHT) {

    if (_params.specify_file_by_time) {

      mdvx->setReadTime((Mdvx::read_search_mode_t)
			_params.read_search_mode,
			_params.url,
			_params.read_search_margin,
			_readSearchTime,
			_params.read_forecast_lead_time);
      
    } else {
      
      string path;
      if (strlen(_params.path) > 0 &&
	  (_params.path[0] == '/' || _params.path[0] == '.')) {
	path = _params.path;
      } else {
	path = "./";
	path += _params.path;
      }
      */
      //string path = "/Users/me/rainfall_estimation_workflow/elle_basic/gfs/mdv/20170602/000000.mdv";
      string path = filePath; // "/Users/me/rainfall_estimation_workflow/elle_basic/cart/mdv/20170602/20170602_001311.mdv";

      mdvx->setReadPath(path);
      
    //} // if (_params.specify_file_by_time)

    mdvx->setReadFieldFileHeaders();
 /*   
  } // if (_params.get_mode != Params::GET_TIME_LIST ...

  if (_params.set_valid_time_search_wt) {
    mdvx->setValidTimeSearchWt(_params.valid_time_search_wt);
  }

  if (_params.constrain_forecast_lead_times) {
    mdvx->setConstrainFcastLeadTimes
      (_params.forecast_constraints.min_lead_time,
       _params.forecast_constraints.max_lead_time,
       _params.forecast_constraints.request_by_gen_time);
  }
  
  if (_params.read_set_horiz_limits) {
    mdvx->setReadHorizLimits(_params.read_horiz_limits.min_lat,
			     _params.read_horiz_limits.min_lon,
			     _params.read_horiz_limits.max_lat,
			     _params.read_horiz_limits.max_lon);
  }
  
  if (_params.read_set_vlevel_limits) {
    mdvx->setReadVlevelLimits(_params.read_lower_vlevel,
			      _params.read_upper_vlevel);
  }

  if (_params.read_set_plane_num_limits) {
    mdvx->setReadPlaneNumLimits(_params.read_lower_plane_num,
				_params.read_upper_plane_num);
  }
  
  if (_params.read_set_vlevel_type) {
    switch(_params.read_vlevel_type) {
    case Params::VERT_TYPE_Z:
      mdvx->setReadVlevelType(Mdvx::VERT_TYPE_Z);
      break;
    case Params::VERT_TYPE_PRESSURE:
      mdvx->setReadVlevelType(Mdvx::VERT_TYPE_PRESSURE);
      break;
    case Params::VERT_FLIGHT_LEVEL:
      mdvx->setReadVlevelType(Mdvx::VERT_FLIGHT_LEVEL);
      break;
    }
  }

  mdvx->setReadEncodingType((Mdvx::encoding_type_t)
			    _params.read_encoding_type);

  mdvx->setReadCompressionType((Mdvx::compression_type_t)
			       _params.read_compression_type);

  mdvx->setReadScalingType((Mdvx::scaling_type_t) _params.read_scaling_type,
			   _params.read_scale,
			   _params.read_bias);

  if (_params.read_composite) {
    mdvx->setReadComposite();
  }

  if (_params.read_set_fill_missing) {
    mdvx->setReadFillMissing();
  }

  if (_params.read_set_field_names) {

    for (int i = 0; i < _params.read_field_names_n; i++) {
      mdvx->addReadField(_params._read_field_names[i]);
    }

  } else if (_params.read_set_field_nums) {

    for (int i = 0; i < _params.read_field_nums_n; i++) {
      mdvx->addReadField(_params._read_field_nums[i]);
    }

  }

  if (_params.read_set_decimation) {
    mdvx->setReadDecimate(_params.decimation_max_nxy);
  }

  if (_params.read_time_list_also || 
      _params.get_mode == Params::GET_TIME_LIST ||
      _params.get_mode == Params::GET_TIME_HEIGHT) {
    _setTimeListMode(mdvx);
  }
  
  if (_params.read_time_list_also) {
    mdvx->setReadTimeListAlso();
  }

  if (_params.read_as_single_part) {
    mdvx->setReadAsSinglePart();
  }

  if (_params.read_field_file_headers_also) {
    mdvx->setReadFieldFileHeaders();
  }

  if (_params.get_mode == Params::GET_VSECTION ||
      _params.get_mode == Params::GET_TIME_HEIGHT) {
    
    // waypoints
    
    for (int i = 0; i < _params.read_vsect_waypts_n; i++) {
      mdvx->addReadWayPt(_params._read_vsect_waypts[i].lat,
			 _params._read_vsect_waypts[i].lon);
    }

    if (_params.disable_vsection_interp) {
      mdvx->setReadVsectDisableInterp();
    }

  }

  if (_params.read_using_32_bit_headers) {
    mdvx->setRead32BitHeaders(true);
  } else {
    mdvx->setRead32BitHeaders(false);
  }
  */
}

////////////////////////
// handle volume

int PrintMdv::_handleVolume(DsMdvx *mdvx)

{

  if (!_needData()) {
    // just read all headers
    return _handleAllHeaders(mdvx);
  }

  // read the volume

  if (_getVolume(mdvx)) {
    cerr << "could not read volume\n";
    return -1;
  }
/*  
  if (_params.save_to_file) {

    int iret = 0;
    const Mdvx::master_header_t &mhdr = mdvx->getMasterHeader();
    if (mhdr.data_collection_type == Mdvx::DATA_EXTRAPOLATED ||
        mhdr.data_collection_type == Mdvx::DATA_FORECAST ||
        mhdr.forecast_time != 0) {
      mdvx->setWriteAsForecast();
    }
    if (mdvx->writeToDir(_params.save_url)) {
      cerr << "ERROR - PrintMdv" << endl;
      cerr << "  Cannot save file to url: " << _params.save_url << endl;
      cerr << mdvx->getErrStr() << endl;
      iret = -1;
    }
    if (_params.debug) {
      cerr << "Wrote file: " << mdvx->getPathInUse() << endl;
    }
    if (_params.no_print) {
      return iret;
    }

  } else if (mdvx->getInternalFormat() == Mdvx::FORMAT_NCF) {

    // save to tmp file, so it can be printed
    
    time_t now = time(NULL);
    DateTime dnow(now);
    pid_t pid = getpid();
    char tmpFilePath[FILENAME_MAX];
    sprintf(tmpFilePath,
            "/tmp/PrintMdv_vol_tmp_%.4d%.2d%.2d_%.2d%.2d%.2d_%.5d",
            dnow.getYear(), dnow.getMonth(), dnow.getDay(),
            dnow.getHour(), dnow.getMin(), dnow.getSec(), pid);
    if (mdvx->writeToPath(tmpFilePath)) {
      cerr << "ERROR - PrintMdv" << endl;
      cerr << "  Cannot save tmp file: " << tmpFilePath << endl;
      cerr << mdvx->getErrStr() << endl;
      return -1;
    }
    if (_params.debug) {
      cerr << "Wrote file: " << mdvx->getPathInUse() << endl;
    }

  }

  MdvxTimeList::time_list_mode_t tlistMode =
    getTimeListMode(_params.time_list_mode);
  
  if (_params.get_mode == Params::GET_GIS) {
    Mdvx::printVolGis(cout, mdvx, _params.start_at_top);
  } else if (_params.get_mode == Params::GET_TABLE) {
    Mdvx::printVolTable(cout, mdvx);
  } else if (_params.print_summary) {
    if (_params.read_time_list_also) {
      Mdvx::printTimeList(cout, mdvx, tlistMode,
                          _params.url,
                          _timeListStartTime,
                          _timeListEndTime,
                          _timeListSearchTime,
                          _timeListGenTime,
                          _params.time_list_margin);
    }
    Mdvx::printVolSummary(cout, mdvx);
  } else {
    if (_params.read_time_list_also) {
      Mdvx::printTimeList(cout, mdvx, tlistMode,
                          _params.url,
                          _timeListStartTime,
                          _timeListEndTime,
                          _timeListSearchTime,
                          _timeListGenTime,
                          _params.time_list_margin);
    }
    _printVolume(mdvx);
  }
*/

  //_printVolume(mdvx);
  readColorMap();
  _plotVolume(mdvx);
  return 0;

}

///////////////////////////////////
// check that we need to read data

bool PrintMdv::_needData()

{
  return true;
/*
  if (_params.save_to_file) {
    return true;
  }
  if (_params.get_mode == Params::GET_GIS) {
    return true;
  }
  if (_params.get_mode == Params::GET_TABLE) {
    return true;
  }
  if (_params.print_summary) {
    return true;
  }
  if (_params.print_data) {
    return true;
  }
  if (_params.print_canonical) {
    return true;
  }
  if (_params.print_chunks) {
    return true;
  }
  if (_params.read_as_single_part) {
    return true;
  }
  if (_params.read_set_decimation) {
    return true;
  }
  if (_params.read_set_field_nums) {
    return true;
  }
  if (_params.read_set_field_names) {
    return true;
  }
  if (_params.read_set_vlevel_type) {
    return true;
  }
  if (_params.read_set_vlevel_limits) {
    return true;
  }
  if (_params.read_transform_to_linear) {
    return true;
  }
  if (_params.read_composite) {
    return true;
  }
  */
  return false;
}
  
////////////////////////
// handle vsection

int PrintMdv::_handleVsection(DsMdvx *mdvx)

{

  // vert section
  
  if (_getVsection(mdvx)) {
    return -1;
  }
/*
  if (_params.save_to_file) {

    int iret = 0;
    const Mdvx::master_header_t &mhdr = mdvx->getMasterHeader();
    if (mhdr.data_collection_type == Mdvx::DATA_EXTRAPOLATED ||
        mhdr.data_collection_type == Mdvx::DATA_FORECAST ||
        mhdr.forecast_time != 0) {
      mdvx->setWriteAsForecast();
    }
    if (mdvx->writeToDir(_params.save_url)) {
      cerr << "ERROR - PrintMdv" << endl;
      cerr << "  Cannot save file to url: " << _params.save_url << endl;
      cerr << mdvx->getErrStr() << endl;
      iret = -1;
    }
    if (_params.debug) {
      cerr << "Wrote file: " << mdvx->getPathInUse() << endl;
    }
    if (_params.no_print) {
      return iret;
    }

  } else if (mdvx->getInternalFormat() == Mdvx::FORMAT_NCF) {

    // save to tmp file, so it can be printed
    
    time_t now = time(NULL);
    DateTime dnow(now);
    pid_t pid = getpid();
    char tmpFilePath[FILENAME_MAX];
    sprintf(tmpFilePath,
            "/tmp/PrintMdv_vsect_tmp_%.4d%.2d%.2d_%.2d%.2d%.2d_%.5d",
            dnow.getYear(), dnow.getMonth(), dnow.getDay(),
            dnow.getHour(), dnow.getMin(), dnow.getSec(), pid);
    if (mdvx->writeToPath(tmpFilePath)) {
      cerr << "ERROR - PrintMdv" << endl;
      cerr << "  Cannot save tmp file: " << tmpFilePath << endl;
      cerr << mdvx->getErrStr() << endl;
      return -1;
    }
    if (_params.debug) {
      cerr << "Wrote file: " << mdvx->getPathInUse() << endl;
    }

  }
  
  if (_params.read_time_list_also) {
    MdvxTimeList::time_list_mode_t tlistMode =
      getTimeListMode(_params.time_list_mode);
    Mdvx::printTimeList(cout, mdvx, tlistMode,
                        _params.url,
                        _timeListStartTime,
                        _timeListEndTime,
                        _timeListSearchTime,
                        _timeListGenTime,
                        _params.time_list_margin);
  }
*/
  _printVsection(mdvx);

  return 0;

}

////////////////////////
// handle all headers

int PrintMdv::_handleAllHeaders(DsMdvx *mdvx)

{

  if (_getAllHeaders(mdvx)) {
      return -1;
  }
/*
  if (_params.debug) {
    cerr << "Read file path: " << mdvx->getPathInUse() << endl;
  }

  if (_params.read_time_list_also) {
    MdvxTimeList::time_list_mode_t tlistMode =
      getTimeListMode(_params.time_list_mode);
    Mdvx::printTimeList(cout, mdvx, tlistMode,
                        _params.url,
                        _timeListStartTime,
                        _timeListEndTime,
                        _timeListSearchTime,
                        _timeListGenTime,
                        _params.time_list_margin);
  }
  */
  Mdvx::printAllHeaders(cout, mdvx);

  return 0;

}

////////////////////////
// handle time list

int PrintMdv::_handleTimeList(DsMdvx *mdvx)

{

  if (_getTimeList(mdvx)) {
      return -1;
  }
/*
  MdvxTimeList::time_list_mode_t tlistMode =
    getTimeListMode(_params.time_list_mode);
  Mdvx::printTimeList(cout, mdvx, tlistMode,
                      _params.url,
                      _timeListStartTime,
                      _timeListEndTime,
                      _timeListSearchTime,
                      _timeListGenTime,
                      _params.time_list_margin);
*/
  return 0;

}

////////////////////////
// handle time height

int PrintMdv::_handleTimeHeight(DsMdvx *mdvx)

{

  if (_getTimeHeight(mdvx)) {
    return -1;
  }
/*
  Mdvx::printTimeHeight(cout,
                        mdvx,
                        _params.print_data,
                        _params.read_transform_to_linear,
                        _params.print_native);
*/
  return 0;
    
}
  
////////////////////////
// read volume

int PrintMdv::_getVolume(DsMdvx *mdvx)
{

  if (mdvx->readVolume()) {
    cerr << mdvx->getErrStr();
    return -1;
  }
  /*
  if (_params.threaded) {
    DsMdvxThreaded *mdvxt = (DsMdvxThreaded *) mdvx;
    while (!mdvxt->getThreadDone()) {
      if (_params.debug) {
	cerr << "Waiting for threaded read to complete, % done:"
	     << mdvxt->getPercentReadComplete() << endl;
      }
      umsleep(500);
    }
  }
  
  if (_params.debug) {
    cerr << "Read file path: " << mdvx->getPathInUse() << endl;
  }

  if (_params.remap_z_to_constant_grid) {
    for (size_t ii = 0; ii < mdvx->getNFields(); ii++) {
      MdvxField *field = mdvx->getField(ii);
      if (field) {
        field->remapVlevels(_params.remap_z_grid.nz,
                            _params.remap_z_grid.minz,
                            _params.remap_z_grid.dz);
      }
    }
  }
*/
  return 0;
  
}
  
////////////////////////
// read all headers

int PrintMdv::_getAllHeaders(DsMdvx *mdvx)
{
  
  if (mdvx->readAllHeaders()) {
    cerr << mdvx->getErrStr();
    return -1;
  }
  /*
  if (_params.threaded) {
    DsMdvxThreaded *mdvxt = (DsMdvxThreaded *) mdvx;
    while (!mdvxt->getThreadDone()) {
      if (_params.debug) {
	cerr << "Waiting for threaded read to complete, % done:"
	     << mdvxt->getPercentReadComplete() << endl;
      }
      umsleep(500);
    }
  }

  if (_params.debug) {
    cerr << "Read file path: " << mdvx->getPathInUse() << endl;
  }
*/
  return 0;
  
}
  
////////////////////////
// get vsection

int PrintMdv::_getVsection(DsMdvx *mdvx)
{
  
  if (mdvx->readVsection()) {
    cerr << mdvx->getErrStr();
    return -1;
  }
  /*
  if (_params.threaded) {
    DsMdvxThreaded *mdvxt = (DsMdvxThreaded *) mdvx;
    while (!mdvxt->getThreadDone()) {
      if (_params.debug) {
	cerr << "Waiting for threaded read to complete, % done:"
	     << mdvxt->getPercentReadComplete() << endl;
      }
      umsleep(500);
    }
  }

  if (_params.debug) {
    cerr << "Read file path: " << mdvx->getPathInUse() << endl;
  }
*/
  return 0;

}

////////////////////////
// get time list

int PrintMdv::_getTimeList(DsMdvx *mdvx)
{

  if (mdvx->compileTimeList()) {
    cerr << mdvx->getErrStr();
    return -1;
  }

  return 0;

}

////////////////////////
// get time height

int PrintMdv::_getTimeHeight(DsMdvx *mdvx)
{

  if (mdvx->compileTimeHeight()) {
    cerr << mdvx->getErrStr();
    return -1;
  }

  return 0;

}

//////////////////////////
// set the time list mode

void PrintMdv::_setTimeListMode(DsMdvx *mdvx)
{
  
  mdvx->clearTimeListMode();
/*
  if (_params.set_latest_valid_mod_time) {
    mdvx->setCheckLatestValidModTime(_latestValidModTime);
  }

  switch(_params.time_list_mode) {
  case Params::TIME_LIST_VALID:
    mdvx->setTimeListModeValid(_params.url,
			       _timeListStartTime,_timeListEndTime);
    break;
  case Params::TIME_LIST_GEN:
    mdvx->setTimeListModeGen(_params.url,
			     _timeListStartTime,_timeListEndTime);
    break;
  case Params::TIME_LIST_FORECAST:
    mdvx->setTimeListModeForecast(_params.url, _timeListGenTime);
    break;
  case Params::TIME_LIST_GEN_PLUS_FORECASTS:
    mdvx->setTimeListModeGenPlusForecasts(_params.url,
					  _timeListStartTime,_timeListEndTime);
    break;
  case Params::TIME_LIST_VALID_MULT_GEN:
    mdvx->setTimeListModeValidMultGen(_params.url,
				      _timeListStartTime,_timeListEndTime);
    break;
  case Params::TIME_LIST_FIRST:
    mdvx->setTimeListModeFirst(_params.url);
    break;
  case Params::TIME_LIST_LAST:
    mdvx->setTimeListModeLast(_params.url);
    break;
  case Params::TIME_LIST_CLOSEST:
    mdvx->setTimeListModeClosest(_params.url, _timeListSearchTime,
				 _params.time_list_margin);
    break;
  case Params::TIME_LIST_FIRST_BEFORE:
    mdvx->setTimeListModeFirstBefore(_params.url, _timeListSearchTime,
				     _params.time_list_margin);
    break;
  case Params::TIME_LIST_FIRST_AFTER:
    mdvx->setTimeListModeFirstAfter(_params.url, _timeListSearchTime,
				    _params.time_list_margin);
    break;
  case Params::TIME_LIST_BEST_FORECAST:
    mdvx->setTimeListModeBestForecast(_params.url, _timeListSearchTime,
				      _params.time_list_margin);
    break;
  case Params::TIME_LIST_SPECIFIED_FORECAST:
    {
      if (_timeListSearchTime == 0) {
        _timeListSearchTime =
          _timeListGenTime + _params.read_forecast_lead_time;
      }
      mdvx->setTimeListModeSpecifiedForecast(_params.url,
                                             _timeListGenTime,
                                             _timeListSearchTime,
                                             _params.time_list_margin);
    }
    break;
  }
*/
}


void PrintMdv::readColorMap() {


static string colorName[] =
{
"DarkSlateGray1  ",
"SkyBlue1  ",
"deep sky blue",
"medium slate blue",
"slate blue ",
"blue4  ",
"MediumPurple4  ",
"DarkOrchid4  ",
"MediumOrchid4  ",
"dark orchid ",
"purple  ",
"hot pink ",
"pink  ",
"DarkOrchid4  ",
"RoyalBlue4  ",
"gray  ",
"SpringGreen2  ",
"SpringGreen3  ",
"SpringGreen4  ",
"yellow1  ",
"yellow3  ",
"gold1  ",
"DarkGoldenrod1  ",
"orange  ",
"sienna1  ",
"coral1  ",
"red1  "  
};

  _nbins = 27; // sizeof(cmin);

  _colorMapRGB = new unsigned int[_nbins];
  // need a color scale 
  // TODO: integrate with color scale class
  try {
    for (int i=0; i<_nbins; i++) {
      _colorMapRGB[i] = X11ColorMap::instance()->x11Name2Rgb(colorName[i]);
    }
  } catch (std::invalid_argument &ex) {
    cerr << ex.what() << endl;
    exit(1);
  }

}

unsigned int PrintMdv::_mapToColorScale(float value, float range, float minArg) {

  unsigned int rgb;


  float step = abs(cmax[0] - cmin[0]);
  //unsigned int maxRGB = 0xffffffff;
  //unsigned int minRGB = 0x00000000;
  int startingIdx = (value - cmin[0]) / step;
  while (cmin[startingIdx] > value) startingIdx -= 1;
  while (cmax[startingIdx] < value) startingIdx += 1;
  int foundIdx = startingIdx;
  QColor qColor;
  // cerr << colorName[foundIdx];
  // QString qColorName(colorName[foundIdx].c_str());


  // TODO: convert colormap to rgb BEFORE using it!!!

  rgb = _colorMapRGB[foundIdx]; // X11ColorMap::instance()->x11Name2Rgb(colorName[foundIdx]);
  //if (qColor.isValidColor(qColorName)) {
  //  cerr << "is valid" << endl;
  //} else {
  //  cerr << "NOT valid" << endl;
  //}
  //qColor.setNamedColor();
  //rgb = (unsigned int) qColor.rgba;
  /*
  int idx = value*step;
  if (idx < 0) {
    rgb = qColor.setNamedColor(QString(colorName[0])).rgb();
  } else if (idx <= nbins) {
    rgb = qColor.setNamedColor(QString(colorName[nbins-1])).rgb();  
  } else {
    rgb =  qColor.setNamedColor(QString(colorName[idx])).rgb();  
  }
  */
  return rgb;
}

void PrintMdv::_plotVolume(const DsMdvx *mdvx) {


  for (size_t ifld = 0; ifld < mdvx->getNFields(); ifld++) {

    MdvxField *field = mdvx->getField(ifld);
    Mdvx::field_header_t fhdr = field->getFieldHeader();
    char *fieldName = fhdr.field_name;

    field->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE); 
    field->setPlanePtrs();
    
    int planeSize = field->getPlaneSize(0);
    int volumeSize = planeSize*fhdr.nz;
    ui08 *tmpVol = new ui08[volumeSize];
    Mdvx::vlevel_header_t vh = field->getVlevelHeader();    
    Mdvx::vlevel_header_t tmpvh(vh);

    for(int iv = 0; iv < fhdr.nz; iv++) {
        const void* plane = field->getPlane(iv);  // what does this do? what does it return?
        int planeOffset = field->getPlaneOffset(fhdr.nz-iv-1);
        
        const float* plane2 = (const float*) field->getPlane(iv);
        cerr << "first values ";
        for (int idx = 0; idx < 10; idx++) {
          cerr << plane2[idx] << " ";
        }
        cerr << endl;



        // ------

        QImage *image;

        int width = fhdr.nx;
        int height = fhdr.ny; 
        int bytesPerLine;
        bytesPerLine = width * sizeof(unsigned int); 

        cout << "sizeof unsigned long " << sizeof(unsigned long);  // 8 bytes
        cout << "sizeof unsigned int" << sizeof(unsigned int) << endl; 

        unsigned int *data = new unsigned int[width*height]; // [width*height*sizeof(unsigned int)];
        //for (int r = 0; r<height; r++) {
        //  for (int c = 0; c<width; c++) {
        // int idx = r*width + c;
        for (int idx = 0; idx < width*height; idx++) {
          //cout << "idx = " << idx << endl;
            data[idx] = _mapToColorScale(plane2[idx],3,3); // 0xff76F2E5; // 0xffRRGGBB;
          //}
        }
        cout << "after filling data array" << endl;
        
        image = new QImage( (unsigned char *) data, 
          width, 
          height, 
          bytesPerLine, 
          QImage::Format_RGB32); // QImage::Format format, 
          //QImageCleanupFunction cleanupFunction = nullptr, 
          //void *cleanupInfo = nullptr);

        // bool QImage::save ( const QString & fileName, const char * format = 0, int quality = -1 ) const
        char outFileName[150];
        sprintf(outFileName, "%s_%s_%1d%s", "/tmp/testImage", fieldName, iv, ".png");
          image->save(QString(outFileName), "PNG", 50);


        //memcpy(&tmpVol[planeOffset], plane, planeSize);
        //tmpvh.level[fhdr.nz-iv-1] = vh.level[iv];
    }

    //field->setVolData(tmpVol,volumeSize, Mdvx::ENCODING_FLOAT32);
    //field->setVlevelHeader(tmpvh);  

  }

}

////////////////////////
// print volume

void PrintMdv::_printVolume(const DsMdvx *mdvx) const
{

  cout << "=============== VOLUME ==============" << endl;
  _doPrintVol(mdvx);

}
  
///////////////////////////////////////////////////////
// print vert section
  
void PrintMdv::_printVsection(const DsMdvx *mdvx) const
  
{

  cout << "=============== VERTICAL SECION ==============" << endl;
  _doPrintVol(mdvx);

}

///////////////////////////////////////////////////////
// do print on volume
  
void PrintMdv::_doPrintVol(const DsMdvx *mdvx) const
  
{

  mdvx->printFormats(cout);
  
  if (mdvx->getInternalFormat() != Mdvx::FORMAT_NCF) {
    /*
    Mdvx::printVol(cout,
                   mdvx,
                   _params.read_field_file_headers_also,
                   _params.print_data,
                   _params.read_transform_to_linear,
                   _params.print_native,
                   _params.print_canonical,
                   _params.print_nlines_data);
*/
  } else {

    mdvx->printNcfInfo(cout);
    char command[2048];
    sprintf(command, "ncdump %s", mdvx->getPathInUse().c_str());
    system(command);
    /*
    if (!_params.save_to_file) {
      // remove tmp file
      unlink(mdvx->getPathInUse().c_str());
    }
    */

  }

}

///////////////////////////////////////////////////////
// get mdvx time list mode from params
/* 
MdvxTimeList::time_list_mode_t
  PrintMdv::getTimeListMode() // Params::time_list_mode_t mode)
  
{
  
  switch(_params.time_list_mode) {
    case Params::TIME_LIST_VALID:
      return MdvxTimeList::MODE_VALID;
    case Params::TIME_LIST_GEN:
      return MdvxTimeList::MODE_GENERATE;
    case Params::TIME_LIST_FORECAST:
      return MdvxTimeList::MODE_FORECAST;
    case Params::TIME_LIST_GEN_PLUS_FORECASTS:
      return MdvxTimeList::MODE_GEN_PLUS_FCASTS;
    case Params::TIME_LIST_VALID_MULT_GEN:
      return MdvxTimeList::MODE_VALID_MULT_GEN;
    case Params::TIME_LIST_FIRST:
      return MdvxTimeList::MODE_FIRST;
    case Params::TIME_LIST_LAST:
      return MdvxTimeList::MODE_LAST;
    case Params::TIME_LIST_CLOSEST:
      return MdvxTimeList::MODE_CLOSEST;
    case Params::TIME_LIST_FIRST_BEFORE:
      return MdvxTimeList::MODE_FIRST_BEFORE;
    case Params::TIME_LIST_FIRST_AFTER:
      return MdvxTimeList::MODE_FIRST_AFTER;
    case Params::TIME_LIST_BEST_FORECAST:
      return MdvxTimeList::MODE_BEST_FCAST;
    case Params::TIME_LIST_SPECIFIED_FORECAST:
      return MdvxTimeList::MODE_SPECIFIED_FCAST;
    default:
      return MdvxTimeList::MODE_VALID;
  } // switch
 
}
*/
//////////////////////////////
// do test for retrieval speed

int PrintMdv::_doTest(DsMdvx *mdvx)

{
/*
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    mdvx->setDebug();
    mdvx->printReadRequest(cerr);
  }

  if (_params.debug) {
    cerr << "Testing MDV retrieval" << endl;
    cerr << "  n retreivals: " << _params.test_n_retrievals << endl;
  }
  
  for (int i = 0; i < _params.test_n_retrievals; i++) {

    if (_params.get_mode == Params::GET_VOLUME ||
	_params.get_mode == Params::GET_GIS ||
	_params.get_mode == Params::GET_TABLE) {
      if (mdvx->readVolume()) {
	cerr << mdvx->getErrStr();
	cerr << "******* ABORTING TEST!!!! *******" << endl;
	return -1;
      }
    } else if (_params.get_mode == Params::GET_VSECTION) {
      if (mdvx->readVsection()) {
	cerr << mdvx->getErrStr();
	cerr << "******* ABORTING TEST!!!! *******" << endl;
	return -1;
      }
    }

  }
  */
  return 0;

}

//////////////////////////////
// print data sizes

void PrintMdv::_printSizes(ostream &out)

{

  cerr << "MDV64_CHUNK_INFO_LEN   : " << MDV64_CHUNK_INFO_LEN << endl;
  cerr << "MDV64_INFO_LEN         : " << MDV64_INFO_LEN << endl;
  cerr << "MDV64_LONG_FIELD_LEN   : " << MDV64_LONG_FIELD_LEN << endl;
  cerr << "MDV64_MAX_PROJ_PARAMS  : " << MDV64_MAX_PROJ_PARAMS << endl;
  cerr << "MDV64_MAX_VLEVELS      : " << MDV64_MAX_VLEVELS << endl;
  cerr << "MDV64_NAME_LEN         : " << MDV64_NAME_LEN << endl;
  cerr << "MDV64_SHORT_FIELD_LEN  : " << MDV64_SHORT_FIELD_LEN << endl;
  cerr << "MDV64_TRANSFORM_LEN    : " << MDV64_TRANSFORM_LEN << endl;
  cerr << "MDV64_UNITS_LEN        : " << MDV64_UNITS_LEN << endl;
  cerr << "MDV64_N_COORD_LABELS   : " << MDV64_N_COORD_LABELS << endl;
  cerr << "MDV64_COORD_UNITS_LEN  : " << MDV64_COORD_UNITS_LEN << endl;
  cerr << "MDV64_PROJ4_STR_LEN    : " << MDV64_PROJ4_STR_LEN << endl;
  
  cerr << "MDV32_CHUNK_INFO_LEN   : " << MDV32_CHUNK_INFO_LEN << endl;
  cerr << "MDV32_INFO_LEN         : " << MDV32_INFO_LEN << endl;
  cerr << "MDV32_LONG_FIELD_LEN   : " << MDV32_LONG_FIELD_LEN << endl;
  cerr << "MDV32_MAX_PROJ_PARAMS  : " << MDV32_MAX_PROJ_PARAMS << endl;
  cerr << "MDV32_MAX_VLEVELS      : " << MDV32_MAX_VLEVELS << endl;
  cerr << "MDV32_NAME_LEN         : " << MDV32_NAME_LEN << endl;
  cerr << "MDV32_SHORT_FIELD_LEN  : " << MDV32_SHORT_FIELD_LEN << endl;
  cerr << "MDV32_TRANSFORM_LEN    : " << MDV32_TRANSFORM_LEN << endl;
  cerr << "MDV32_UNITS_LEN        : " << MDV32_UNITS_LEN << endl;
  cerr << "MDV32_N_COORD_LABELS   : " << MDV32_N_COORD_LABELS << endl;
  cerr << "MDV32_COORD_UNITS_LEN  : " << MDV32_COORD_UNITS_LEN << endl;
  
  cerr << "sizeof(master_header_64_t) : " << sizeof(Mdvx::master_header_64_t) << endl;
  cerr << "sizeof(field_header_64_t)  : " << sizeof(Mdvx::field_header_64_t) << endl;
  cerr << "sizeof(vlevel_header_64_t) : " << sizeof(Mdvx::vlevel_header_64_t) << endl;
  cerr << "sizeof(chunk_header_64_t)  : " << sizeof(Mdvx::chunk_header_64_t) << endl;
  
  cerr << "sizeof(master_header_32_t) : " << sizeof(Mdvx::master_header_32_t) << endl;
  cerr << "sizeof(field_header_32_t)  : " << sizeof(Mdvx::field_header_32_t) << endl;
  cerr << "sizeof(vlevel_header_32_t) : " << sizeof(Mdvx::vlevel_header_32_t) << endl;
  cerr << "sizeof(chunk_header_32_t)  : " << sizeof(Mdvx::chunk_header_32_t) << endl;
  
  cerr << "64-bit-words(master_header_64_t) : " 
       << (double) sizeof(Mdvx::master_header_64_t) / (double) sizeof(si64) << endl;
  cerr << "64-bit-words(field_header_64_t)  : " 
       << (double) sizeof(Mdvx::field_header_64_t) / (double) sizeof(si64) << endl;
  cerr << "64-bit-words(vlevel_header_64_t) : "
       << (double) sizeof(Mdvx::vlevel_header_64_t) / (double) sizeof(si64) << endl;
  cerr << "64-bit-words(chunk_header_64_t)  : "
       << (double) sizeof(Mdvx::chunk_header_64_t) / (double) sizeof(si64) << endl;
  
  cerr << "64-bit-words(master_header_32_t) : "
       << (double) sizeof(Mdvx::master_header_32_t) / (double) sizeof(si64) << endl;
  cerr << "64-bit-words(field_header_32_t)  : "
       << (double) sizeof(Mdvx::field_header_32_t) / (double) sizeof(si64) << endl;
  cerr << "64-bit-words(vlevel_header_32_t) : "
       << (double) sizeof(Mdvx::vlevel_header_32_t) / (double) sizeof(si64) << endl;
  cerr << "64-bit-words(chunk_header_32_t)  : "
       << (double) sizeof(Mdvx::chunk_header_32_t) / (double) sizeof(si64) << endl;
  
}

