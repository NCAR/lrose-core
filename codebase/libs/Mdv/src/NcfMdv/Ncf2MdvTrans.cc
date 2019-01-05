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
// Ncf2MdvTrans.cc
//
// Sue Dettling, Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2008 
//
///////////////////////////////////////////////////////////////
//
// Ncf2MdvTrans class.
// Translate a CF NetCDF file to Mdvx object
//
///////////////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <toolsa/mem.h>
#include <toolsa/TaStr.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pjg.h>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxChunk.hh>
#include <Mdv/Mdvx.hh>
#include <Mdv/NcfMdv.hh>
#include <Mdv/Ncf2MdvTrans.hh>
#include <Mdv/Ncf2MdvField.hh>
#include <Radx/NcfRadxFile.hh>
#include <Radx/ForayNcRadxFile.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxRcalib.hh>
#include <Radx/RadxSweep.hh>
#include <rapformats/DsRadarParams.hh>
#include <rapformats/DsRadarCalib.hh>
#include <cstring>
#include <cmath>
#include <cstdlib>

/////////////////////////////////
// Constructor

Ncf2MdvTrans::Ncf2MdvTrans()

{

  // initialize data members

  _debug = false;
  _ncFile = NULL;
  _ncErr = NULL;
  _forecast_reference_time_found = false;
  _timesInspected = false;
  _expectedNumTimes = -1;
  _timeIndex = -1;
  _isRhi = false;
  clearErrStr();

}

/////////////////////////////////////
// Destructor

Ncf2MdvTrans::~Ncf2MdvTrans()

{
  _clear();
}
  
/////////////////////////////////////////
// Read in Ncf data, determine times
// Returns number of times, or -1 for no consistent times dimension

int Ncf2MdvTrans::inspectTimes(const string &path, Mdvx &mdv)

{
  _timesInspected = false;
  _expectedNumTimes = -1;
  _timeIndex = -1;

  // check if this is a Radx-supported file
  // if so do not do the time inspection, Radx cannot have multiple times
  
  RadxFile radxFile;
  if (radxFile.isSupported(path)) {
    return -1;
  }
  
  // initialize 
  if (_initializeForRead(path, mdv)) {
    return -1;
  }

  // parse the NetCDF file for time info
  if (_setTimeInfo()) {
    TaStr::AddStr(_errStr, "ERROR - Ncf2MdvTrans::inspectTimes");
    TaStr::AddStr(_errStr, "  Cannot set time info");
  }
  else
  {
    // figure out which times are actually used in gridded fields
    // we hope for exactly one set of times
    _expectedNumTimes = _matchTimeInfoToData();
    if (_expectedNumTimes > 0) {
      _timesInspected = true;
    }
    else {
      TaStr::AddStr(_errStr, "ERROR - Ncf2MdvTrans::inspectTimes");
      TaStr::AddStr(_errStr, "  Cannot match time info to data");
    }
  }

  // close the NetCDF file
  _closeNcFile();

  return _expectedNumTimes;
}

  
/////////////////////////////////////////
// Parse Ncf data, populate Mdvx object
// Returns 0 on success, -1 on failure

int Ncf2MdvTrans::translate(const string &path, Mdvx &mdv)

{

  // check if this is a Radx-supported file
  // if so translate using Radx
  
  RadxFile radxFile;
  if (radxFile.isSupported(path)) {
    if (translateRadx(path, mdv)) {
      TaStr::AddStr(_errStr, "ERROR - Ncf2MdvTrans::translate");
      return -1;
    } else {
      return 0;
    }
  }
  
  // initialize

  if (_initializeForRead(path, mdv)) {
    return -1;
  }

  // parse the NetCDF file
  
  int status = _parseNc();
  if (status) {
    TaStr::AddStr(_errStr, "ERROR - Ncf2MdvTrans::translate");
    TaStr::AddStr(_errStr, "  Parsing Nc File, path: ", path);
  }

  // close the NetCDF file
  _closeNcFile();

  return status;

}

///////////////////////////////////////////////////////////////
// Translate Radx file
// Returns 0 on success, -1 on failure

int Ncf2MdvTrans::translateRadx(const string &path, Mdvx &mdv)

{

  // initialize Mdvx object

  _initMdv(path, mdv);

  ///////////////
  // set up read

  RadxFile radxFile;
  
  // set field names
  
  if (_mdv->_readFieldNames.size() > 0) {
    for (int ii = 0; ii < (int) _mdv->_readFieldNames.size(); ii++) {
      radxFile.addReadField(_mdv->_readFieldNames[ii]);
    }
  }

  // set vertical limits

  if (_mdv->_readVlevelLimitsSet) {
    radxFile.setReadFixedAngleLimits(_mdv->_readMinVlevel,
				     _mdv->_readMaxVlevel);
    radxFile.setReadStrictAngleLimits(false);
  } else if (_mdv->_readPlaneNumLimitsSet) {
    radxFile.setReadSweepNumLimits(_mdv->_readMinPlaneNum,
				   _mdv->_readMaxPlaneNum);
    radxFile.setReadStrictAngleLimits(false);
  }

  // ignore rays with antenna transitions

  radxFile.setReadIgnoreTransitions(true);

  // perform read of CfRadial file into RadxVol
  
  RadxVol vol;
  if (radxFile.readFromPath(path, vol)) {
    TaStr::AddStr(_errStr, "ERROR - Ncf2MdvTrans::_translateRadx");
    TaStr::AddStr(_errStr, "  Reading Nc File, path: ", path);
    TaStr::AddStr(_errStr, radxFile.getErrStr());
    return -1;
  }

  // translate the vol to MDV

  if (_translateRadxVol(path, vol)) {
    return -1;
  }

  return 0;

}

///////////////////////////////////////////////////////////////
// Translate from RadxVol
// Returns 0 on success, -1 on failure

int Ncf2MdvTrans::translateRadxVol(const string &path,
				   RadxVol &vol,
				   Mdvx &mdv)
  
{

  // initialize Mdvx object
  
  _initMdv(path, mdv);
  
  // translate the vol to MDV
  
  if (_translateRadxVol(path, vol)) {
    return -1;
  }

  return 0;

}

///////////////////////////////
// handle error string

void Ncf2MdvTrans::clearErrStr()

{
  _errStr = "";
  TaStr::AddStr(_errStr, "Time for following error: ", DateTime::str());
}

/////////////////////////////////
// Initialization step before reading, return 0 for good, -1 for error

int Ncf2MdvTrans::_initializeForRead(const string &path, Mdvx &mdv)

{
  // initialize Mdvx object

  _initMdv(path, mdv);

  // make sure this object is clear

  _clear();

  // open the Nc file

  if (_openNcFile(path)) {
    TaStr::AddStr(_errStr, "ERROR - Ncf2MdvTrans::_initializeForRead");
    TaStr::AddStr(_errStr, "  Opening Nc File, path: ", path);
    return -1;
  }

  return 0;
}


/////////////////////////////////
// Clear the memory

void Ncf2MdvTrans::_clear()

{
  
  // close file if open, delete ncFile and ncError

  _closeNcFile();
  clearErrStr();
  _timeDims.clear();
  _validTimes.clear();

  // Note: must leave the 'time info' values alone:
  // _timesInspected, _expectedNumTimes, _timeIndex
}

//////////////////////////////////////////
// Init mdv object

void Ncf2MdvTrans::_initMdv(const string &path, Mdvx &mdv)

{

  _mdv = &mdv;
  _mdv->clearMasterHeader();
  _mhdr = _mdv->getMasterHeader();
  _mdv->clearFields();
  _mdv->clearChunks();
  _mdv->clearErrStr();
  _mdv->_pathInUse = path;

}
  
//////////////////////////////////////////
// Parse NcFile
// Returns 0 on success, -1 on failure

int Ncf2MdvTrans::_parseNc()

{

  // set the master header

  if (_setMasterHeader()) {
    TaStr::AddStr(_errStr, "ERROR - Ncf2MdvTrans::_parseNc");
    TaStr::AddStr(_errStr, "  Cannot set master header");
    return -1;
  }

  // set times from CF-style variables
  
  if (_setTimeInfo()) {
    TaStr::AddStr(_errStr, "ERROR - Ncf2MdvTrans::_parseNc");
    TaStr::AddStr(_errStr, "  Cannot set time info");
    return -1;
  }

  // add the data fields
  // Note that for data sets with multiple times, the fields
  // for all times will be added. These will need to be separated
  // out later for writing Mdv files

  if (_addDataFields()) {
    TaStr::AddStr(_errStr, "ERROR - Ncf2MdvTrans::_parseNc");
    TaStr::AddStr(_errStr, "  Adding data fields");
    return -1;
  }

  // set number of data times in master header
  // and set the time_centroid to the first valid time

  _mhdr.num_data_times = _validTimes.size();
  if (_validTimes.size() > 0) {
    _mhdr.time_centroid = *_validTimes.begin();
  }

  // add Mdv chunks
  
  if (_addChunks()) {
    TaStr::AddStr(_errStr, "ERROR - Ncf2MdvTrans::_parseNc");
    TaStr::AddStr(_errStr, "  Adding MDV chunks");
    return -1;
  }
  _addGlobalAttrXmlChunk();

  // for forecast data, set the forecast_time
  // and set data collection type to forecast data.
  
  if (_forecast_reference_time_found && _validTimes.size() == 1) {
    _mhdr.forecast_time = _mhdr.time_centroid;
    _mhdr.data_collection_type = Mdvx::DATA_FORECAST;
    if (_mhdr.forecast_time != _mhdr.time_gen + _mhdr.forecast_delta) {
      cerr << "WARNING - Ncf2MdvTrans::_parseNc" << endl;
      cerr << "  Times mismatch" << endl;
      cerr << "  Valid time: " << DateTime::strm(_mhdr.time_centroid) << endl;
      cerr << "  Gen time: " << DateTime::strm(_mhdr.time_gen) << endl;
      cerr << "  Forecast time: "
           << DateTime::strm(_mhdr.forecast_time) << endl;
      cerr << "  Forecast lead time: " << _mhdr.forecast_delta << endl;
    }
  }

  // set the master header in the mdv object
  
  _mdv->setMasterHeader(_mhdr);
  _mdv->setDataSetInfo(_dataSetInfo.c_str());
  _mdv->updateMasterHeader();

  return 0;

}

//////////////////////////////////////
// open netcdf file
//
// Returns 0 on success, -1 on failure

int Ncf2MdvTrans::_openNcFile(const string &path)
  
{
  
  if (_ncFile) {
    _ncFile->close();
    delete _ncFile;
  }

  _ncFile = new Nc3File(path.c_str(), Nc3File::ReadOnly);

  // Check that constructor succeeded

  if (!_ncFile->is_valid()) {
    TaStr::AddStr(_errStr, "ERROR - Ncf2MdvTrans::_openNcFile");
    TaStr::AddStr(_errStr, "  Opening file, path: ", path);
    return 1;
  }
  
  // Change the error behavior of the netCDF C++ API by creating an
  // Nc3Error object. Until it is destroyed, this Nc3Error object will
  // ensure that the netCDF C++ API returns error codes
  // on any failure, and leaves any other error handling to the
  // calling program.
  
  _ncErr = new Nc3Error(Nc3Error::silent_nonfatal);

  if (_debug) {
    cerr << "SUCCESS - opened file: " << path << endl;
  }
 
  return 0;

}

//////////////////////////////////////
// close netcdf file if open
// remove error object if it exists

void Ncf2MdvTrans::_closeNcFile()
  
{
  
  // close file if open, delete ncFile
  
  if (_ncFile) {
    _ncFile->close();
    delete _ncFile;
    _ncFile = NULL;
  }

  if (_ncErr) {
    delete _ncErr;
    _ncErr = NULL;
  }

}

//////////////////////////
// set the master header

int Ncf2MdvTrans::_setMasterHeader()

{

  // data set info is built up incrementally

  _dataSetInfo.clear();
  _dataSetInfo += "Converted from NetCDF to MDV, ";
  _dataSetInfo += DateTime::strm(time(NULL));
  _dataSetInfo += "\n";

  // Loop through the global attributes, use the ones which make sense

  for (int i = 0; i < _ncFile->num_atts(); i++) {
    
    Nc3Att* att = _ncFile->get_att(i);
    
    if (att == NULL) {
      continue;
    }
    
    // data set name, source and info

    if (!strcmp(att->name(), NcfMdv::title)) {
      STRncopy(_mhdr.data_set_name, asString(att).c_str(), MDV_NAME_LEN);
    }
      
    if (!strcmp(att->name(), NcfMdv::source)) {
      STRncopy(_mhdr.data_set_source, asString(att).c_str(), MDV_NAME_LEN);
    }

    _addAttr2Str(att, NcfMdv::history, _dataSetInfo, "  Ncf:history: ");
    _addAttr2Str(att, NcfMdv::institution, _dataSetInfo, "  Ncf:institution: ");
    _addAttr2Str(att, NcfMdv::references, _dataSetInfo, "  Ncf:references: ");
    _addAttr2Str(att, NcfMdv::comment, _dataSetInfo, "  Ncf:comment: ");

    // Caller must delete attribute

    delete att;
    
  } // i

  // set data set info
  
  STRncopy(_mhdr.data_set_info, _dataSetInfo.c_str(), MDV_INFO_LEN);

  // find the master header variable, and decode it it available

  Nc3Var *mhdrVar = _ncFile->get_var(NcfMdv::mdv_master_header);
  if (mhdrVar != NULL) {
    
    for (int i = 0; i < mhdrVar->num_atts(); i++) {
      
      Nc3Att* att = mhdrVar->get_att(i);
      
      if (att == NULL) {
        continue;
      }
      
      // ints
      
      _setSi32FromAttr(att, NcfMdv::mdv_revision_number, _mhdr.revision_number);
      _setSi32FromAttr(att, NcfMdv::mdv_epoch, _mhdr.epoch);
      
      _setSi32FromAttr(att, NcfMdv::mdv_time_centroid, _mhdr.time_centroid);
      _setSi32FromAttr(att, NcfMdv::mdv_time_gen, _mhdr.time_gen);
      _setSi32FromAttr(att, NcfMdv::mdv_time_begin, _mhdr.time_begin);
      _setSi32FromAttr(att, NcfMdv::mdv_time_end, _mhdr.time_end);
      _setSi32FromAttr(att, NcfMdv::mdv_user_time, _mhdr.user_time);
      _setSi32FromAttr(att, NcfMdv::mdv_time_expire, _mhdr.time_expire);
      _setSi32FromAttr(att, NcfMdv::mdv_time_written, _mhdr.time_written);
      _setSi32FromAttr(att, NcfMdv::mdv_forecast_time, _mhdr.forecast_time);
      _setSi32FromAttr(att, NcfMdv::mdv_forecast_delta, _mhdr.forecast_delta);
      
      _setSi32FromAttr(att, NcfMdv::mdv_data_collection_type,
                       _mhdr.data_collection_type);
      _setSi32FromAttr(att, NcfMdv::mdv_user_data, _mhdr.user_data);
      _setSi32FromAttr(att, NcfMdv::mdv_vlevel_type, _mhdr.vlevel_type);
      _setSi32FromAttr(att, NcfMdv::mdv_native_vlevel_type,
                       _mhdr.native_vlevel_type);
      
      _setSi32FromAttr(att, NcfMdv::mdv_user_data_si32_0,
                       _mhdr.user_data_si32[0]);
      _setSi32FromAttr(att, NcfMdv::mdv_user_data_si32_1,
                       _mhdr.user_data_si32[1]);
      _setSi32FromAttr(att, NcfMdv::mdv_user_data_si32_2,
                       _mhdr.user_data_si32[2]);
      _setSi32FromAttr(att, NcfMdv::mdv_user_data_si32_3,
                       _mhdr.user_data_si32[3]);
      _setSi32FromAttr(att, NcfMdv::mdv_user_data_si32_4,
                       _mhdr.user_data_si32[4]);
      _setSi32FromAttr(att, NcfMdv::mdv_user_data_si32_5,
                       _mhdr.user_data_si32[5]);
      _setSi32FromAttr(att, NcfMdv::mdv_user_data_si32_6,
                       _mhdr.user_data_si32[6]);
      _setSi32FromAttr(att, NcfMdv::mdv_user_data_si32_7,
                       _mhdr.user_data_si32[7]);
      
      // floats
      
      _setFl32FromAttr(att, NcfMdv::mdv_user_data_fl32_0,
                       _mhdr.user_data_fl32[0]);
      _setFl32FromAttr(att, NcfMdv::mdv_user_data_fl32_1,
                       _mhdr.user_data_fl32[1]);
      _setFl32FromAttr(att, NcfMdv::mdv_user_data_fl32_2,
                       _mhdr.user_data_fl32[2]);
      _setFl32FromAttr(att, NcfMdv::mdv_user_data_fl32_3,
                       _mhdr.user_data_fl32[3]);
      _setFl32FromAttr(att, NcfMdv::mdv_user_data_fl32_4,
                       _mhdr.user_data_fl32[4]);
      _setFl32FromAttr(att, NcfMdv::mdv_user_data_fl32_5,
                       _mhdr.user_data_fl32[5]);
      
      _setFl32FromAttr(att, NcfMdv::mdv_sensor_lon, _mhdr.sensor_lon);
      _setFl32FromAttr(att, NcfMdv::mdv_sensor_lat, _mhdr.sensor_lat);
      _setFl32FromAttr(att, NcfMdv::mdv_sensor_alt, _mhdr.sensor_alt);
      
      // Caller must delete attribute
      
      delete att;
      
    } // i
    
  } // if (mhdrVar != NULL)
    
  // set standard items
    
  _mhdr.vlevel_included = 1;
  _mhdr.num_data_times = 1;
  _mhdr.index_number = 0;
  _mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  _mhdr.data_ordering = Mdvx::ORDER_XYZ;
  _mhdr.struct_id = Mdvx::MASTER_HEAD_MAGIC_COOKIE;
  _mhdr.revision_number = 1;
  _mhdr.record_len1 = sizeof(Mdvx::master_header_t) - (2 * sizeof(si32));
  _mhdr.record_len2 = _mhdr.record_len1;

  if (_debug) {
    cerr << "SUCCESS - setting master header" << endl;
  }
 
  return 0;

}

//////////////////////////////////////////////////////////////////
// set time information from Ncf file, return 0 for good, -1 for error
//
// There are two standard names relating to time attributes
// in the standard name table:
//   'time'
//   'forecast_reference_time'
//
// Also, 'forecast_period' is the standard name for the forecast lead time.
// 

int Ncf2MdvTrans::_setTimeInfo()

{

  // clear the time dimensions vector

  _timeDims.clear();

  // loop through the variables, looking for time dimensions and adding to state

  for (int ivar = 0; ivar < _ncFile->num_vars(); ivar++) {
    
    Nc3Var* var = _ncFile->get_var(ivar);
    if (var == NULL) {
      continue;
    }
    _setTimeInfoForVar(var);
  }

  // check we have at least one time dimension

  if (_timeDims.size() < 1) {
    TaStr::AddStr(_errStr, "ERROR - Ncf2MdvTrans::_setTimeInfo");
    TaStr::AddStr(_errStr, "  Cannot find time dimension and coord variable");
    if (_debug) {
      cerr << "ERROR - cannot find time coord variable" << endl;
    }
    return -1;
  }
  
  // find the time dimension with the smallest number of times
  // set _timeDim to that one

  size_t maxTimes = _timeDims[0].times.size();
  _timeDim = _timeDims[0];
  
  for (int ii = 1; ii < (int) _timeDims.size(); ii++) {
    if (_timeDims[ii].times.size() < maxTimes) {
      _timeDim = _timeDims[ii];
      maxTimes = _timeDims[ii].times.size();
    }
  }

  if (_debug) {
    cerr << "Default time dimension: " << _timeDim.name << endl;
    cerr << "  time: " << DateTime::strm(_timeDim.times[0]) << endl;
    if (_timeDims.size() > 1) {
      cerr << "List of time dimensions: " << endl;
      for (int ii = 0; ii < (int) _timeDims.size(); ii++) {
        cerr << "  name: " << _timeDims[ii].name << endl;
        cerr << "  values: " << endl;
        for (int jj = 0; jj < (int) _timeDims[ii].times.size(); jj++) {
          cerr << "        "
               << DateTime::strm(_timeDims[ii].times[jj]) << endl;
        } // jj
      } // ii
    } // if (_timeDims.size() > 1)
  } // debug
  
  // if time_bounds exists, use it to set time_begin and time_end
  
  Nc3Att* boundsAtt = _timeDim.var->get_att(NcfMdv::bounds);
  if (boundsAtt != NULL) {
    string boundsName = asString(boundsAtt);
    delete boundsAtt;
    Nc3Var* boundsVar = _ncFile->get_var(boundsName.c_str());
    if (boundsVar != NULL) {
      double timeBounds[2];
      Nc3Bool ok = boundsVar->get(timeBounds, 1, 2);
      if (ok) {
        if (timeBounds[0] != 0.0) {
          _mhdr.time_begin = (si32) timeBounds[0];
        }
        if (timeBounds[1] != 0.0) {
          _mhdr.time_end = (si32) timeBounds[1];
        }
      } else {
        cerr << "WARNING - Ncf2MdvTrans::_setTimeInfo" << endl;
        cerr << "  Could not decode time bounds, var: " << boundsName << endl;
      }
    }
  }

  // if start_time and stop_time variables exist, use them to set
  // time_begin and time_end

  if (_debug) {
    cerr << "SUCCESS - setting time coord variable" << endl;
  }

  return 0;

}

//////////////////////////////////////////////////////////////////
// set time information from one variable in Ncf file
//
// There are two standard names relating to time attributes
// in the standard name table:
//   'time'
//   'forecast_reference_time'
//
// Also, 'forecast_period' is the standard name for the forecast lead time.
// 

void Ncf2MdvTrans::_setTimeInfoForVar(Nc3Var *var)
{
  if ( var->num_vals() == 0 ) {
    return;
  }
 
  // get name and standard name
    
  string name = var->name();
  string stdName;
  Nc3Att* stdNameAtt = var->get_att(NcfMdv::standard_name);
  if (stdNameAtt != NULL) {
    stdName = asString(stdNameAtt);
    delete stdNameAtt;
  }
    
  // check for forecast period
    
  if (name.compare(NcfMdv::forecast_period) == 0) {
    double forecast_period = var->as_double(0);
    _mhdr.forecast_delta = (si32) (forecast_period + 0.5);
    return;
  }

  // get units attribute
    
  Nc3Att* unitsAtt = var->get_att(NcfMdv::units);
  if (unitsAtt == NULL) {
    // time variables must have units
    return;
  }
  string units = asString(unitsAtt);
  delete unitsAtt;

#ifdef USE_UDUNITS

  // check if this is a time variable, using units
    
  ut_unit *udUnit = ut_parse(_udunits.getSystem(), units.c_str(), UT_ASCII);
  if (udUnit == NULL) {
    // cannot parse as a unit
    return;
  }
    
  // is this a time variable?
    
  if (ut_are_convertible(udUnit, _udunits.getEpoch()) == 0) {
    // not a time variable
    ut_free(udUnit);
    return;
  }
      
  // convert to unix time 
  // cv_converter and udUnit freed below, we may need 
  // if we have more than one forecast time
    
  cv_converter *conv = ut_get_converter(udUnit, _udunits.getEpoch());
  double storedTime = var->as_double(0);
  double unixTime = cv_convert_double(conv, storedTime);

#else

  DateTime rtime;
  double mult = 1.0;
  if (units.find("seconds") != string::npos) {
    mult = 1.0;
  } else if (units.find("minutes") != string::npos) {
    mult = 60.0;
  } else if (units.find("hours") != string::npos) {
    mult = 3600.0;
  } else if (units.find("days") != string::npos) {
    mult = 86400.0;
  } else {
    // not a time variable
    return;
  }
  if (rtime.setFromW3c(units.c_str())) {
    // not a time variable
    return;
  }
  double storedTime = var->as_double(0) * mult;
  double unixTime = storedTime + rtime.getTimeAsDouble();

#endif

  // forecast_reference_time?
    
  if (name.compare(NcfMdv::forecast_reference_time) == 0) {
    _forecast_reference_time_found = true;
    _mhdr.time_gen = (si32) (unixTime + 0.5);
    return;
  }
      
  // start time? stop_time?
    
  if (name.compare(NcfMdv::start_time) == 0) {
    if (unixTime != 0) {
      _mhdr.time_begin = (si32) (unixTime + 0.5);
      return;
    }
  }
      
  if (name.compare(NcfMdv::stop_time) == 0) {
    if (unixTime != 0) {
      _mhdr.time_end = (si32) (unixTime + 0.5);
      return;
    }
  }
      
  // determine whether this is the time coordinate variable
  // by checking its name is the same as its dimension
    
  int numDims = var->num_dims();
  if (numDims != 1) {
    return;
  }
    
  Nc3Dim* dim = var->get_dim(0);
  if (dim == NULL) {
    return;
  }

  // add to the timeDims vector
    
  TimeDim tdim;
  tdim.name = name;
  tdim.dim = dim;
  tdim.var = var;
  for (int jj = 0; jj < dim->size(); jj++){
    double storedForecastTime = var->as_double(jj);
#ifdef USE_UDUNITS
    double unixForecastTime = cv_convert_double(conv, storedForecastTime);
#else
    double unixForecastTime = storedForecastTime + rtime.getTimeAsDouble();
#endif
    tdim.times.push_back((time_t) (unixForecastTime + 0.5));
  }
  _timeDims.push_back(tdim);

#ifdef USE_UDUNITS
  cv_free(conv);
  ut_free(udUnit);
#endif
    
}
  
///////////////////////////////////////////////////////////////
// search for data fields with time, hoping for exactly one consistent
// time dimension
//
// returns time dimension, or -1 if more than one or error

int Ncf2MdvTrans::_matchTimeInfoToData()

{
  _validTimes.clear();
  
  int fieldNum = 0;
  vector<int> nTimes;

  // loop through the variables, adding to nTimes vector as appropriate

  for (int ivar = 0; ivar < _ncFile->num_vars(); ivar++) {
    
    if (_adjustTimeInfo(_ncFile->get_var(ivar), fieldNum, nTimes)) {
      return -1;
    }
  }
 
  if (nTimes.empty()) {
    TaStr::AddStr(_errStr, "ERROR - Ncf2MdvTrans::__matchTimeInfoToData");
    TaStr::AddStr(_errStr, "  No time dimension found");
    return -1;
  }

  return nTimes[0];
  
}

//////////////////////////
// add the data fields
//
// returns 0 on success, -1 on failure

int Ncf2MdvTrans::_addDataFields()

{
  time_t t0 = time(0);

  _validTimes.clear();
  
  // loop through the variables, adding data fields as appropriate

  int fieldNum = 0;
  for (int ivar = 0; ivar < _ncFile->num_vars(); ivar++) {
    
    if (_addOneField(_ncFile->get_var(ivar), fieldNum))
    {
      return -1;
    }
  }
  if (_mdv->getNFields() < 1) {
    TaStr::AddStr(_errStr, "ERROR - Ncf2MdvTrans::_addDataFields");
    TaStr::AddStr(_errStr, "  No fields found");
    return -1;
  }

  // finalize fields
  // (a) convert to vert section as needed
  // (b) compress appropriately
  
  if (_finalizeFields()) {
    return -1;
  }

  time_t t1 = time(0);
  if (_debug) {
    cerr << "Ncf2MdvTrans::addDataFields";
    cerr << "Time elapsed = " << t1 - t0 << endl;
  }
  return 0;
}

//////////////////////////
//
//  Add one field to mdv state, return 0 for good, -1 for bad

int Ncf2MdvTrans::_addOneField(Nc3Var *dataVar, int &fieldNum)
{
  ArrayDim arrayDim;

  if (_addOneFieldInit(dataVar, fieldNum, arrayDim)) {
    return 0;
  }

  TimeDim *tdim = _findTimeDim(dataVar);


  // go for it.

  if (_debug) {
    cerr << "Ncf2MdvTrans::_addOneField" << endl;
    string fieldName = dataVar->name();
    cerr << "  -->> adding field: " << fieldName << endl;
  }

  // How many times are associated with this netCDF var?
	
  int numTimes = (int) tdim->times.size();
	
  if (_timesInspected && _timeIndex >= 0) {

    // do just the current index

    if (numTimes == _expectedNumTimes) {

      if (_addOneTimeDataField(_timeIndex, tdim, dataVar, arrayDim)) {
	return -1;
      }
      // a bit of a hack to adjust forecast delta here, but needed.
      if (_validTimes.size() == 1) {
	_mhdr.forecast_delta = *_validTimes.begin() - _mhdr.time_gen;
      }
      else {
	cerr << "ERROR expected 1 valid time got " << _validTimes.size() <<endl;
	return -1;
      }
    }	 
    else {
      cerr << "ERROR mismatch in number of times for " << dataVar->name() <<
	" dataNtimes" << numTimes << " expected " << 
	_expectedNumTimes << endl;
      return -1;
    }
  }
  else
  {

    // Create data for for each and every time.
    for (int itime = 0 ; itime < numTimes; itime++) {
      if (_addOneTimeDataField(itime, tdim, dataVar, arrayDim)) {
	return -1;
      }
    }
  }
  return 0;
}


//////////////////////////
//
// adjust nTimes for a field return -1 for bad, 0 for good

int Ncf2MdvTrans::_adjustTimeInfo(Nc3Var *dataVar, int &fieldNum,
				  vector<int> &nTimes)
{
  ArrayDim arrayDim;

  if (_addOneFieldInit(dataVar, fieldNum, arrayDim)) {
    return 0;
  }

  TimeDim *tdim = _findTimeDim(dataVar);


  // here a field that is a grid that is wanted

  if (_debug) {
    cerr << "Ncf2MdvTrans::_adjustTimeInfo" << endl;
    string fieldName = dataVar->name();
    cerr << "  -->> adding field: " << fieldName << endl;
  }

  // How many times are associated with this netCDF var?
	
  int numTimes = (int) tdim->times.size();
  if (nTimes.empty())
  {
    nTimes.push_back(numTimes);
  }
  else
  {
    if (nTimes[0] != numTimes)
    {
      // we do not allow more than one time, so indicate this is the case
      cerr << "ERROR Ncf2MdvTrans::_adjustTimeInfo" << endl;
      cerr << "  More than one time dimension " << nTimes[0] << 
	" " << numTimes  << endl;
      return -1;
    }
  }
  return 0;
}

//////////////////////////
// Initialization for adding one field, check if it is a grid
// return 0 if field should be added, -1 if not

int Ncf2MdvTrans::_addOneFieldInit(Nc3Var *dataVar, int &fieldNum,
				   ArrayDim &arrayDim)

{
  if (!_shouldAddField(dataVar, fieldNum)) {
    return -1;
  }

  arrayDim = ArrayDim();

  int numDims = dataVar->num_dims();

  // inspect the dimensions in reverse order,
  // which should be x,y,z,t
  for (int jdim = numDims - 1; jdim >= 0; jdim--) {
    _inspectDim(dataVar->get_dim(jdim), jdim, arrayDim);
  }

  // look through dimensions again, looking for dimensions 
  // that are not yet used, by inspecting the var name
  for (int jdim = numDims - 1; jdim >= 0; jdim--) {
    _reInspectDim(dataVar->get_dim(jdim), jdim, arrayDim);
  }

  if (!arrayDim.isGrid()) {

    if (_debug) {
      cerr << "NOTE: this is not a data field" << endl;
      cerr << "  X and/or Y coord missing" << endl;
    }
    return -1;
  }
  return 0;
}

//////////////////////////
// Look a a field and adjust arrayDim if appropriate

void Ncf2MdvTrans::_inspectDim(Nc3Dim *dim, int jdim, ArrayDim &arrayDim)
{
  if (dim == NULL) {
    if (_debug) {
      cerr << "REJECT var as field: no dim, jdim: " << jdim << endl;
    }
    return;
  }
      
  Nc3Var *coordVar = _ncFile->get_var(dim->name());
  if (coordVar == NULL) {
    if (_debug) {
      cerr << "REJECT var as field: no coords" << endl;
    }
    return;
  }
      
  Nc3Att *standardNameAtt = coordVar->get_att(NcfMdv::standard_name);
  string stdName;
  if (standardNameAtt != NULL) {
    stdName = asString(standardNameAtt);
    delete standardNameAtt;
  } else {
    if (_debug) {
      cerr << "NOTE: no standard_name attribute found" << endl;
    }
  }

  Nc3Att *longNameAtt = coordVar->get_att(NcfMdv::long_name);
  string longName;
  if (longNameAtt != NULL) {
    longName = asString(longNameAtt);
    delete longNameAtt;
  }

  if (stdName.compare(NcfMdv::projection_x_coordinate) == 0 ||
      longName.compare(NcfMdv::projection_x_coordinate) == 0 ||
      stdName.compare(NcfMdv::longitude) == 0) {
    arrayDim.xVar = coordVar;
    arrayDim.xDim = dim;
    if (_debug) {
      cerr << "SUCCESS - FIELD has X coordinate" << endl;
    }
    return;
  }

  if (stdName.compare(NcfMdv::projection_y_coordinate) == 0 ||
      longName.compare(NcfMdv::projection_y_coordinate) == 0 ||
      stdName.compare(NcfMdv::latitude) == 0) {
    arrayDim.yVar = coordVar;
    arrayDim.yDim = dim;
    if (_debug) {
      cerr << "SUCCESS - FIELD has Y coordinate" << endl;
    }
    return;
  }
  Nc3Att *positiveAtt = coordVar->get_att(NcfMdv::positive);
  if (positiveAtt != NULL) {
    arrayDim.zVar = coordVar;
    arrayDim.zDim = dim;
    delete positiveAtt;
    if (_debug) {
      cerr << "NOTE - FIELD has Z coordinate" << endl;
    }
    return;
  }
 }

//////////////////////////
// Look a a field in another way and adjust arrayDim if appropriate

void Ncf2MdvTrans::_reInspectDim(Nc3Dim *dim, int jdim, ArrayDim &arrayDim)
{
  if (dim == NULL) {
    return;
  }
  string name = dim->name();
  Nc3Var *coordVar = _ncFile->get_var(name.c_str());
  if (coordVar == NULL) {
    return;
  }
      
  if (arrayDim.xDim == NULL) {
    if (name == "x" || name == "X") {
      arrayDim.xVar = coordVar;
      arrayDim.xDim = dim;
      if (_debug) {
	cerr << "SUCCESS - FIELD has X coordinate" << endl;
      }
      return;
    }
  }

  if (arrayDim.yDim == NULL) {
    if (name == "y" || name == "Y") {
      arrayDim.yVar = coordVar;
      arrayDim.yDim = dim;
      if (_debug) {
	cerr << "SUCCESS - FIELD has Y coordinate" << endl;
      }
      return;
    }
  }

  if (arrayDim.zDim == NULL) {
    if (name == "z" || name == "Z") {
      arrayDim.zVar = coordVar;
      arrayDim.zDim = dim;
      if (_debug) {
	cerr << "SUCCESS - FIELD has Z coordinate" << endl;
      }
      return;
    }
  }
}


//////////////////////////
// Check if variable is a field that should be added
//
// returns true if yes.

bool Ncf2MdvTrans::_shouldAddField(Nc3Var *dataVar, int &fieldNum)
{
  if (dataVar == NULL) {
    return false;
  }
    
  // we need fields with 2, 3 or 4 dimensions

  int numDims = dataVar->num_dims();
  if (numDims < 2 || numDims > 4) {
    return false;
  }

  // check if we need this field
    
  fieldNum++;
  string fieldName = dataVar->name();
  bool needField = true;
    
  if (_mdv->_readFieldNums.size() > 0) {
    needField = false;
    for (int kk = 0; kk < (int) _mdv->_readFieldNums.size(); kk++) {
      if (_mdv->_readFieldNums[kk] == fieldNum) {
	needField = true;
	break;
      }
    }
  }
  if (_mdv->_readFieldNames.size() > 0) {
    needField = false;
    for (int kk = 0; kk < (int) _mdv->_readFieldNames.size(); kk++) {
      if (_mdv->_readFieldNames[kk] == fieldName) {
	needField = true;
	break;
      }
    }
  }

  if (_debug) {
    cerr << "Ncf2MdvTrans::_shouldAddField" << endl;
    if (needField) {
      cerr << "  -->> adding field: " << fieldName << endl;
    } else {
      cerr << "  -->> rejecting field: " << fieldName << endl;
    }
  }

  if (!needField) {
    if (_debug) {
      cerr << "Ncf2MdvTrans::_shouldAddField" << endl;
      cerr << "  -->> rejecting field: " << fieldName << endl;
    }
    return false;
  }
    
  if (_debug) {
    cerr << "Ncf2MdvTrans::_shouldAddField" << endl;
    cerr << "  Checking variable for field data: " << fieldName << endl;
  }
  return true;
}

/////////////////////////////////////////
// find the time dimension for this variable
// if set, it will be the first dimension

Ncf2MdvTrans::TimeDim * Ncf2MdvTrans::_findTimeDim(Nc3Var *dataVar)
{
  // find the time dimension for this variable
  // if set, it will be the first dimension

  TimeDim *ret = NULL;
  Nc3Dim* vdim = dataVar->get_dim(0);
  for (int ii = 0; ii < (int) _timeDims.size(); ii++) {
    string tname = vdim->name();
    if (tname == _timeDims[ii].name) {
      ret = &_timeDims[ii];
    }
  } // ii
  if (ret == NULL) {
    // variable has no time dimesion, use the default
    // NOTE assumes that to be here the default var has been set to something
    ret = &_timeDim;
  }
  return ret;
}


//////////////////////////
// Add infor for one field at one time to _mdv object
// return 0 for good, -1 for error

int Ncf2MdvTrans::_addOneTimeDataField(int itime, TimeDim *tdim, 
				       Nc3Var* dataVar, ArrayDim &arrayDim)
{
  time_t validTime = tdim->times[itime];
        
  // Set the forecast delta and forecast time to zero 
  // if we do not have forecast data, else set the
  // forecast time and delta appropriately
        
  si32 forecastDelta;
  si32 forecastTime;
        
  if(_forecast_reference_time_found){
    forecastDelta = validTime - _mhdr.time_gen;
    forecastTime = validTime;
  } else {
    forecastDelta = 0;
    forecastTime = 0;
  }
        
  Ncf2MdvField *field = new Ncf2MdvField(_debug,
					 validTime,
					 itime,
					 forecastTime,
					 forecastDelta,
#ifdef USE_UDUNITS
					 _udunits.getSystem(),
#endif
					 _ncFile, _ncErr,
					 dataVar,
					 tdim->dim, tdim->var,
					 arrayDim.zDim, arrayDim.zVar,
					 arrayDim.yDim, arrayDim.yVar,
					 arrayDim.xDim, arrayDim.xVar);
        
  MdvxField *mdvxField = field->createMdvxField();

  if (mdvxField != NULL) {
    mdvxField->setFieldHeaderFile(mdvxField->getFieldHeader());
    mdvxField->setVlevelHeaderFile(mdvxField->getVlevelHeader());
    _mdv->addField(mdvxField);
  } else {
    _errStr += field->getErrStr();
    TaStr::AddStr(_errStr, "ERROR - Ncf2MdvTrans::_addDataFields");
    string fieldName = dataVar->name();
    TaStr::AddStr(_errStr, "  Adding field: ", fieldName);
    TaStr::AddStr(_errStr, "  Cannot find field");
    delete field;
    return -1;
  }
  _validTimes.insert(validTime);
  delete field;
  return 0;
}


//////////////////////////
// finalize fields
// (a) convert to vert section as needed
// (b) compress appropriately

int Ncf2MdvTrans::_finalizeFields()

{

  // try RHI first

  if (_isRhi) {

    // RHI file

    return _finalizeFieldsRhi();
    
  }

  // then try vsection waypt

  if (_mdv->_vsectWayPts.size() > 0) {

    // transformation lut

    MdvxRemapLut remapLut;
    MdvxVsectLut vsectLut;
    
    // this is a vsection
    // compute number of samples
    
    int n_samples = _mdv->_computeNVsectSamples();
    
    // compute longitude limits

    double minLon = 360.0;
    double maxLon = -360.0;
    if (_mdv->_vsectWayPts.size() > 0) {
      for (size_t i = 0; i < _mdv->_vsectWayPts.size(); i++) {
	minLon = MIN(minLon, _mdv->_vsectWayPts[i].lon);
	maxLon = MAX(maxLon, _mdv->_vsectWayPts[i].lon);
      }
    }

    for (int ifield = 0; ifield < (int) _mdv->getNFields(); ifield++) {

      MdvxField *fld = _mdv->getField(ifield);
      
      // apply read constraints, do not compress etc
      
      fld->_apply_read_constraints(*_mdv, false, false, false,
				   remapLut, true, minLon, maxLon);
      
      // convert to vert section
      
      fld->convert2Vsection(_mhdr,
			    _mdv->_vsectWayPts,
			    n_samples,
			    vsectLut,
			    _mdv->_readFillMissing,
			    !_mdv->_vsectDisableInterp,
			    _mdv->_readSpecifyVlevelType,
			    _mdv->_readVlevelType,
			    false);
      
      fld->compress(Mdvx::COMPRESSION_GZIP);

    } // ifield
      
      // add vsect-specific members
    
    _mdv->_vsectWayPts = vsectLut.getWayPts();
    _mdv->_vsectSamplePts = vsectLut.getSamplePts();
    _mdv->_vsectSegments = vsectLut.getSegments();
    _mdv->_vsectDxKm = vsectLut.getDxKm();
    _mdv->_vsectTotalLength = vsectLut.getTotalLength();
    _mdv->_addVsectChunks();

    return 0;

  }

  // not a vsection, just apply read contraints, compression etc
  
  MdvxRemapLut remapLut;
  for (int ifield = 0; ifield < (int) _mdv->getNFields(); ifield++) {
    MdvxField *fld = _mdv->getField(ifield);
    fld->_apply_read_constraints(*_mdv, _mdv->_readFillMissing,
				 _mdv->_readDecimate, true,
				 remapLut, false, -360.0, 360.0);
    fld->compress(Mdvx::COMPRESSION_GZIP);
  }

  return 0;
  
}

//////////////////////////
// finalize fields for RHI
//
// returns 0 on success, -1 on error

int Ncf2MdvTrans::_finalizeFieldsRhi(bool respectUserDistance /* = false*/)

{

  // master header

  _mdv->setMasterHeader(_mhdr);

  // in an RHI request has been made, then load up the
  // closest RHI to the way pts requested

  if (_mdv->_vsectWayPts.size() >= 2) {
    if (_mdv->_load_closest_rhi(respectUserDistance)) {
      return -1;
    }
  }

  // finalize

  MdvxRemapLut remapLut;
  for (int ifield = 0; ifield < (int) _mdv->getNFields(); ifield++) {
    MdvxField *fld = _mdv->getField(ifield);
    fld->_apply_read_constraints(*_mdv, _mdv->_readFillMissing,
				 _mdv->_readDecimate, true,
				 remapLut, false, -360.0, 360.0);
    fld->compress(Mdvx::COMPRESSION_GZIP);
  }

  _mdv->updateMasterHeader();
  return 0;

}
    
//////////////////////////
// add chunks
// returns 0 on success, -1 on failure

int Ncf2MdvTrans::_addChunks()

{

  // look for up to 10000 chunks
  
  for (int ii = 0; ii < 10000; ii++) {

    // dimension

    char dimName[128];
    sprintf(dimName, "%s_%.4d", NcfMdv::nbytes_mdv_chunk, ii);
    Nc3Dim *dim = _ncFile->get_dim(dimName);
    if (dim == NULL) {
      return 0;
    }

    // variable
    
    char varName[128];
    sprintf(varName, "%s_%.4d", NcfMdv::mdv_chunk, ii);
    Nc3Var *var = _ncFile->get_var(varName);
    if (var == NULL) {
      return 0;
    }

    // id

    int id = 0;
    Nc3Att *idAtt = var->get_att(NcfMdv::id);
    if (idAtt != NULL) {
      id = idAtt->as_int(0);
      delete idAtt;
    }
    
    // info

    string info;
    Nc3Att *infoAtt = var->get_att(NcfMdv::info);
    if (infoAtt != NULL) {
      info = asString(infoAtt);
      delete infoAtt;
    }

    // data
    
    TaArray<ncbyte> data_;
    ncbyte *data = data_.alloc(dim->size());
    if (var->get(data, 1, dim->size()) != 0) {
      MdvxChunk *chunk = new MdvxChunk();
      chunk->setData(data, dim->size());
      chunk->setId(id);
      chunk->setInfo(info.c_str());
      _mdv->addChunk(chunk);
      if (_debug) {
	cerr << "Adding chunk: " << info << endl;
      }
    }

    
  }
  
  return 0;

}
    
//////////////////////////////////////////
// add chunk for global attributes in XML

void Ncf2MdvTrans::_addGlobalAttrXmlChunk()
  
{

  if (_debug) {
    cerr << "Ncf2MdvTrans::addGlobalAttrXmlChunk()" << endl;
  }
  
  string xml = _getGlobalAttrXml();
  
  MdvxChunk *chunk = new MdvxChunk();
  chunk->setData(xml.c_str(), xml.size() + 1);
  chunk->setId(Mdvx::CHUNK_TEXT_DATA);
  chunk->setInfo("NetCDF file global attributes");
  _mdv->addChunk(chunk);

}

///////////////////////////////////
// create XML for global attributes

string Ncf2MdvTrans::_getGlobalAttrXml()

{

  string xml;
  
  xml += TaXml::writeStartTag("netcdf-global-attributes", 0);

  // Loop through the global attributes

  for (int i = 0; i < _ncFile->num_atts(); i++) {
    
    Nc3Att* att = _ncFile->get_att(i);
    
    if (att == NULL) {
      continue;
    }

    // TODO - fix this for STRING values
    
    if (att->values() != NULL) {
      char *val = att->as_string(0);
      xml += TaXml::writeString(att->name(), 1, val);
      delete[] val;
    } else {
      xml += TaXml::writeString(att->name(), 1, "unknown");
    }

    delete att;
    
  } // i

  xml += TaXml::writeEndTag("netcdf-global-attributes", 0);

  return xml;

}

////////////////////////////////////////
// add attribute to string

void Ncf2MdvTrans::_addAttr2Str(Nc3Att *att, const string &requiredName,
				string &str, const string &label)
  
{
  if (requiredName.compare(att->name()) == 0) {
    str += label;
    if (strlen(att->name()) > 0) {
      str += asString(att);
    }
    str += "\n";
  }
}

////////////////////////////////////////
// set si32 from attribute

void Ncf2MdvTrans::_setSi32FromAttr(Nc3Att *att, const string &requiredName, si32 &val)

{
  if (requiredName.compare(att->name()) == 0) {
    val = att->as_int(0);
  }
}

////////////////////////////////////////
// set fl32 from attribute

void Ncf2MdvTrans::_setFl32FromAttr(Nc3Att *att, const string &requiredName, fl32 &val)

{
  if (requiredName.compare(att->name()) == 0) {
    val = att->as_float(0);
  }
}

    
///////////////////////////////////////////
// get string representation of component

string Ncf2MdvTrans::asString(const Nc3TypedComponent *component,
			      int index /* = 0 */)
  
{

  // TODO - fix this for STRING values

  if (component->values() != NULL) {
    const char* strc = component->as_string(index);
    string strs(strc);
    delete[] strc;
    return strs;
  }

  return "unknown";

}

/////////////////////////////////////////////////////////
// translate RadxVol to Mdv
// Assumes _initMdv has been called

int Ncf2MdvTrans::_translateRadxVol(const string &path,
				    RadxVol &vol)
  
{

  // ensure sweep angles are in ascending order
  
  vol.reorderSweepsAsInFileAscendingAngle();
  vol.reorderSweepsAscendingAngle();

  // make sure gate geom is constant

  vol.remapToPredomGeom();

  // set the number of gates constant

  vol.setNGatesConstant();

  // load the fields from the rays to the volume
  
  vol.loadFieldsFromRays();

  // save data byte width in RadxFields
  
  vector<int> radxByteWidths;
  for (int ifield = 0; ifield < (int) vol.getFields().size(); ifield++) {
    RadxField &field = *vol.getFields()[ifield];
    int radxByteWidth = field.getByteWidth();
    radxByteWidths.push_back(radxByteWidth);
  }
  
  // convert volume to 4-byte floats
  
  vol.convertToFl32();

  // compute the angular resolution of the data, and the number of angles

  _computeAngRes(vol);

  // set master header
  
  _setMasterHeaderCfRadial(path, vol);

  // add fields

  for (int ifield = 0; ifield < (int) vol.getFields().size(); ifield++) {
    RadxField &fieldx = *vol.getFields()[ifield];
    _addFieldCfRadial(vol, fieldx, radxByteWidths[ifield]);
  } // ifield
  
  // finalize fields
  // (a) convert to vert section as needed
  // (b) compress appropriately
  
  if (_finalizeFields()) {
    return -1;
  }

  // add chunks

  _addRadarParamsCfRadial(vol);
  _addCalibCfRadial(vol);
  _addElevArrayCfRadial(vol);

  // set the master header, make consistent with rest of object

  _mdv->setMasterHeader(_mhdr);
  _mdv->setDataSetInfo(_dataSetInfo.c_str());
  _mdv->updateMasterHeader();

  return 0;

}

//////////////////////////////////////////////////
// determine the angular resolution for the volume

void Ncf2MdvTrans::_computeAngRes(const RadxVol &vol)
  
{
  
  _minAngleRes = 5.0;
  _maxAngleRes = 0.0;
  _isRhi = false;
  for (int ii = 0; ii < (int) vol.getSweeps().size(); ii++) {
    const RadxSweep &sweep = *vol.getSweeps()[ii];
    double angleRes = sweep.getAngleResDeg();
    if (angleRes > 0.05) {
      if (angleRes < _minAngleRes) {
        _minAngleRes = angleRes;
      }
      if (angleRes > _maxAngleRes) {
        _maxAngleRes = angleRes;
      }
    }
    if (sweep.getSweepMode() == Radx::SWEEP_MODE_RHI ||
	sweep.getSweepMode() == Radx::SWEEP_MODE_MANUAL_RHI) {
      _isRhi = true;
    }
  }

  if (_isRhi) {

    // compute min and max elevation

    double minElev = 360.0;
    double maxElev = -360.0;
    for (int ii = 0; ii < (int) vol.getRays().size(); ii++) {
      double elev = vol.getRays()[ii]->getElevationDeg();
      if (elev < minElev) {
	minElev = elev;
      } 
      if (elev > maxElev) {
	maxElev = elev;
      }
    }

    double elevRange = maxElev - minElev;
    _nAngles = (int) (elevRange / _minAngleRes + 0.5);
    _deltaAngle = elevRange / _nAngles;
    int startIndex = (int) floor(minElev / _deltaAngle);
    // _minAngle = (startIndex + 0.5) * _deltaAngle;
    _minAngle = startIndex * _deltaAngle;

  } else {
    
    // PPI
    
    _nAngles = (int) (360.0 / _minAngleRes + 0.5);
    _deltaAngle = 360.0 / _nAngles;
    // _minAngle = 0.5 * _deltaAngle;
    _minAngle = 0.0;

    _findEmptySectors(vol);

    if (_trimToSector) {
      _nAngles = _nAzTrimmed;
      // _minAngle = (_dataStartIaz + 0.5) * _deltaAngle;
      _minAngle = _dataStartIaz * _deltaAngle;
    }
    if (_minAngle < 0) {
      _minAngle += 360.0;
    } else if (_minAngle > 360) {
      _minAngle -= 360.0;
    }

  } 

  _halfGap = (int) ((_maxAngleRes / _deltaAngle) / 2.0) + 1;

}

///////////////////////////////////////////////////////////////
// Set the master header from CfRadial

void Ncf2MdvTrans::_setMasterHeaderCfRadial(const string &path, 
					    const RadxVol &vol)

{
  
  _mhdr.index_number = vol.getVolumeNumber(); 
  _mhdr.time_gen = 0;
  _mhdr.time_begin = vol.getStartTimeSecs(); 
  _mhdr.time_end = vol.getEndTimeSecs(); 
  _mhdr.time_centroid = vol.getEndTimeSecs(); 
  _mhdr.data_collection_type = Mdvx::DATA_MEASURED;
  if (_isRhi) {
    _mhdr.native_vlevel_type = Mdvx::VERT_TYPE_AZ;
    _mhdr.vlevel_type = Mdvx::VERT_TYPE_AZ;
  } else {
    _mhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
    _mhdr.vlevel_type = Mdvx::VERT_TYPE_ELEV;
  }
  _mhdr.vlevel_included = true;
  _mhdr.n_fields = vol.getFields().size();
  _mhdr.max_nx = vol.getMaxNGates();
  _mhdr.max_ny = _nAngles;
  _mhdr.max_nz = vol.getSweeps().size();
  _mhdr.n_chunks = 0;
  _mhdr.field_grids_differ = false;
  struct stat fstat;
  if (ta_stat(path.c_str(), &fstat) == 0) {
    _mhdr.time_written = fstat.st_mtime;
  } else {
    _mhdr.time_written = time(NULL);
  }
  _mhdr.sensor_lon = vol.getLongitudeDeg();
  _mhdr.sensor_lat = vol.getLatitudeDeg();
  _mhdr.sensor_alt = vol.getAltitudeKm();
    
  _dataSetInfo.clear();
  _dataSetInfo += "Converted from CfRadial to MDV, ";
  _dataSetInfo += DateTime::strm(time(NULL));
  _dataSetInfo += "\n";
  _dataSetInfo += vol.getHistory();
  _dataSetInfo += "\n";
  _dataSetInfo += vol.getInstitution();
  _dataSetInfo += "\n";
  _dataSetInfo += vol.getReferences();
  _dataSetInfo += "\n";
  _dataSetInfo += vol.getComment();
  _dataSetInfo += "\n";
  STRncopy(_mhdr.data_set_info, _dataSetInfo.c_str(), MDV_INFO_LEN);
  STRncopy(_mhdr.data_set_name, vol.getTitle().c_str(), MDV_NAME_LEN);
  STRncopy(_mhdr.data_set_source, vol.getSource().c_str(), MDV_NAME_LEN);

}

    
///////////////////////////////////////////////////////////////
// add field from CfRadial
// NOTE: data has already been converted to fl32, and the original
// byte width is passed in

void Ncf2MdvTrans::_addFieldCfRadial(const RadxVol &vol,
				     const RadxField &radxField,
				     int origByteWidth)

{
  
  // load field header
  
  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);
    
  fhdr.nx = radxField.getMaxNGates();
  fhdr.ny = _nAngles;
  fhdr.nz = vol.getSweeps().size();
  if (_isRhi) {
    fhdr.proj_type = Mdvx::PROJ_RHI_RADAR;
  } else {
    fhdr.proj_type = Mdvx::PROJ_POLAR_RADAR;
  }
  fhdr.data_element_nbytes = sizeof(fl32);
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  int nPoints = fhdr.nx * fhdr.ny * fhdr.nz;
  fhdr.volume_size = nPoints * fhdr.data_element_nbytes;

  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_DYNAMIC;
  if (_isRhi) {
    fhdr.native_vlevel_type = Mdvx::VERT_TYPE_AZ;
    fhdr.vlevel_type = Mdvx::VERT_TYPE_AZ;
  } else {
    fhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
    fhdr.vlevel_type = Mdvx::VERT_TYPE_ELEV;
  }
  fhdr.dz_constant = false;
  fhdr.data_dimension = 3;
  fhdr.proj_origin_lat = vol.getLatitudeDeg();
  fhdr.proj_origin_lon = vol.getLongitudeDeg();
  fhdr.grid_dx = radxField.getGateSpacingKm();
  fhdr.grid_dy = _deltaAngle;
  fhdr.grid_dz = 1.0;
  fhdr.grid_minx = radxField.getStartRangeKm();
  fhdr.grid_miny = _minAngle;
  if (vol.getSweeps().size() > 0) {
    fhdr.grid_minz = vol.getSweeps()[0]->getFixedAngleDeg();
  }
  fhdr.scale = 1.0;
  fhdr.bias = 0.0;
  fhdr.bad_data_value = radxField.getMissingFl32();
  fhdr.missing_data_value = fhdr.bad_data_value;
  fhdr.proj_rotation = 0;
  radxField.computeMinAndMax();
  fhdr.min_value = radxField.getMinValue();
  fhdr.max_value = radxField.getMaxValue();
  fhdr.min_value_orig_vol = fhdr.min_value;
  fhdr.max_value_orig_vol = fhdr.max_value;

  if (radxField.getStandardName().size() > 0) {
    STRncopy(fhdr.field_name_long, radxField.getStandardName().c_str(),
	     MDV_LONG_FIELD_LEN);
  } else {
    STRncopy(fhdr.field_name_long, radxField.getLongName().c_str(),
	     MDV_LONG_FIELD_LEN);
  }
  STRncopy(fhdr.field_name, radxField.getName().c_str(), MDV_SHORT_FIELD_LEN);
  string units = radxField.getUnits();
  STRncopy(fhdr.units, units.c_str(), MDV_UNITS_LEN);
  if (units == "dbz" || units == "dBZ" || units == "dBz" || units == "DBZ") {
    STRncopy(fhdr.transform, "dBZ", MDV_TRANSFORM_LEN);
  } else if (units == "db" || units == "dB" || units == "DB") {
    STRncopy(fhdr.transform, "dB", MDV_TRANSFORM_LEN);
  }

  // set vlevel header

  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  for (int isweep = 0; isweep < (int) vol.getSweeps().size(); isweep++) {
    const RadxSweep &sweep = *vol.getSweeps()[isweep];
    double fixedAngle = sweep.getFixedAngleDeg();
    vhdr.level[isweep] = fixedAngle;
    if (_isRhi) {
      vhdr.type[isweep] = Mdvx::VERT_TYPE_AZ;
    } else {
      vhdr.type[isweep] = Mdvx::VERT_TYPE_ELEV;
    }
  }

  // create array of floats for field data
  
  fl32 *mdvData = new fl32[nPoints];
  fl32 missing = fhdr.missing_data_value;
  for (int ii = 0; ii < nPoints; ii++) {
    mdvData[ii] = missing;
  }

  // create array for storing ray indices

  TaArray<int> rayLut_;
  int *rayLut = rayLut_.alloc(_nAngles);

  // copy data into array, using ray information

  int nxy = fhdr.nx * fhdr.ny;
  Radx::fl32 *radxData = (Radx::fl32 *) radxField.getData();
  for (int isweep = 0; isweep < (int) vol.getSweeps().size(); isweep++) {

    const RadxSweep &sweep = *vol.getSweeps()[isweep];

    // load up ray lookup locations
    
    _loadRayLookup(vol, sweep, rayLut);

    // load up data

    for (int iangle = 0; iangle < _nAngles; iangle++) {

      // get ray index from lookup

      int iray = rayLut[iangle];
      if (iray < 0) {
        continue;
      }

      // compute location

      const RadxRay &ray = *vol.getRays()[iray];
      int radxStart = iray * ray.getNGates();
      int mdvxStart = isweep * nxy + iangle * fhdr.nx;

      // copy data

      memcpy(mdvData + mdvxStart, radxData + radxStart, fhdr.nx * sizeof(fl32));

    } // iangle

  } // isweep
  
    // create Mdvx field
  
  MdvxField *mdvxField = new MdvxField(fhdr, vhdr, mdvData);

  // headers as in file

  int nz = vol.getSweepsAsInFile().size();
  if (nz > MDV_MAX_VLEVELS) {
    nz = MDV_MAX_VLEVELS;
  }
  fhdr.nz = nz;

  mdvxField->setFieldHeaderFile(fhdr);
  for (int isweep = 0; isweep < nz; isweep++) {
    const RadxSweep &sweep = *vol.getSweepsAsInFile()[isweep];
    double fixedAngle = sweep.getFixedAngleDeg();
    vhdr.level[isweep] = fixedAngle;
    if (_isRhi) {
      vhdr.type[isweep] = Mdvx::VERT_TYPE_AZ;
    } else {
      vhdr.type[isweep] = Mdvx::VERT_TYPE_ELEV;
    }
  }
  mdvxField->setVlevelHeaderFile(vhdr);

  // clean up

  delete[] mdvData;

  // convert mdv field based on byte width, compress
  
  if (origByteWidth == 1) {
    mdvxField->convertType(Mdvx::ENCODING_INT8, Mdvx::COMPRESSION_GZIP);
  } else if (origByteWidth == 2) {
    mdvxField->convertType(Mdvx::ENCODING_INT16, Mdvx::COMPRESSION_GZIP);
  } else {
    mdvxField->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_GZIP);
  }

  // add to mdv volume

  _mdv->addField(mdvxField);

}

///////////////////////////////////////////////////////////////
// load ray index lookup

void Ncf2MdvTrans::_loadRayLookup(const RadxVol &vol,
                                  const RadxSweep &sweep,
                                  int *rayLut)
  
{
  
  // initialize

  for (int ii = 0; ii < _nAngles; ii++) {
    rayLut[ii] = -1;
  }

  // load indexes at ray locations
  
  int nAnglesPer360 = (int) floor(360.0 / _deltaAngle + 0.5);
  for (int iray = (int) sweep.getStartRayIndex();
       iray <= (int) sweep.getEndRayIndex(); iray++) {
    const RadxRay &ray = *vol.getRays()[iray];
    double angle = 0.0;
    if (_isRhi) {
      angle = ray.getElevationDeg();
    } else {
      angle = ray.getAzimuthDeg();
    }
    // compute angle index
    int iangle = (int) floor((angle - _minAngle) / _deltaAngle + 0.5);
    if (iangle < 0) {
      iangle += nAnglesPer360;
    } else if (iangle > _nAngles - 1) {
      iangle -= nAnglesPer360;
    }
    if (iangle > _nAngles - 1) {
      continue;
    }
    if (iangle < 0) {
      continue;
    }
    rayLut[iangle] = iray;
  } // iray

  // make copy

  TaArray<int> copy_;
  int *copy = copy_.alloc(_nAngles);
  memcpy(copy, rayLut, _nAngles * sizeof(int));
  
  // fill in gaps with nearest neighbor
  
  for (int kk = 1; kk <= _halfGap; kk++) {
    for (int ii = 0; ii < _nAngles; ii++) {
      if (copy[ii] >= 0) {
        int jj = ii - kk;
        jj = ii + kk;
        if (jj >= 0) {
          if (rayLut[jj] < 0) {
            rayLut[jj] = copy[ii];
          }
        }
        if (jj < _nAngles) {
          if (rayLut[jj] < 0) {
            rayLut[jj] = copy[ii];
          }
        }
      } // if (copy[ii] >= 0)
    } // ii
  } // kk

}


///////////////////////////////////////////////////////////////
// add radar params chunk from CfRadial

void Ncf2MdvTrans::_addRadarParamsCfRadial(const RadxVol &vol)

{

  // load radar params

  DsRadarParams rparams;

  rparams.radarId = 0;
  rparams.radarType = _getDsRadarType(vol.getPlatformType());
  rparams.numFields = vol.getFields().size();
  rparams.numGates = vol.getMaxNGates();
  if (vol.getRays().size() > 0) {
    rparams.samplesPerBeam = vol.getRays()[0]->getNSamples();
  }
  rparams.scanType = 0;
  if (vol.getSweeps().size() > 0) {
    const RadxSweep &sweep0 = *(vol.getSweeps()[0]);
    rparams.scanMode = _getDsScanMode(sweep0.getSweepMode());
    rparams.followMode = _getDsFollowMode(sweep0.getFollowMode());
    rparams.polarization =
      _getDsPolarizationMode(sweep0.getPolarizationMode());
    rparams.prfMode = _getDsPrfMode(sweep0.getPrtMode());
    rparams.scanTypeName =
      Radx::sweepModeToStr(sweep0.getSweepMode());
  }

  if (vol.getRcalibs().size() > 0) {
    rparams.radarConstant = vol.getRcalibs()[0]->getRadarConstantH();
  }

  rparams.altitude = vol.getAltitudeKm();
  rparams.latitude = vol.getLatitudeDeg();
  rparams.longitude = vol.getLongitudeDeg();
  rparams.gateSpacing = vol.getGateSpacingKm();
  rparams.startRange = vol.getStartRangeKm();
  rparams.horizBeamWidth = vol.getRadarBeamWidthDegH();
  rparams.vertBeamWidth = vol.getRadarBeamWidthDegV();
  rparams.antennaGain = vol.getRadarAntennaGainDbH();
  rparams.wavelength = vol.getWavelengthM() * 100.0;

  if (vol.getRays().size() > 0) {
    const RadxRay &ray = *vol.getRays()[0];
    rparams.pulseWidth = ray.getPulseWidthUsec();
    rparams.pulseRepFreq = 1.0 / ray.getPrtSec();
    double prtRatio = ray.getPrtRatio();
    rparams.prt = ray.getPrtSec();
    rparams.prt2 = rparams.prt / prtRatio;
    rparams.unambigRange = ray.getUnambigRangeKm();
    rparams.unambigVelocity = ray.getNyquistMps();
    if (fabs(prtRatio - 0.667) < 0.01) {
      rparams.prfMode = DS_RADAR_PRF_MODE_STAGGERED_2_3;
    } else if (fabs(prtRatio - 0.75) < 0.01) {
      rparams.prfMode = DS_RADAR_PRF_MODE_STAGGERED_3_4;
    } else if (fabs(prtRatio - 0.80) < 0.01) {
      rparams.prfMode = DS_RADAR_PRF_MODE_STAGGERED_4_5;
    }
  }

  if (vol.getRcalibs().size() > 0) {
    const RadxRcalib &cal = *vol.getRcalibs()[0];
    rparams.xmitPeakPower = pow(10.0, cal.getXmitPowerDbmH() / 10.0) / 1000.0;
    rparams.receiverGain = cal.getReceiverGainDbHc();
    rparams.receiverMds = cal.getNoiseDbmHc() - rparams.receiverGain;
    rparams.systemGain = rparams.antennaGain + rparams.receiverGain;
    rparams.measXmitPowerDbmH = cal.getXmitPowerDbmH();
    rparams.measXmitPowerDbmV = cal.getXmitPowerDbmV();
  }

  rparams.radarName = vol.getInstrumentName() + "/" + vol.getSiteName();
  rparams.scanTypeName = vol.getScanName();

  if (_debug) {
    rparams.print(cerr);
  }

  // add a radar params chunk

  MdvxChunk *chunk = new MdvxChunk;
  chunk->setId(Mdvx::CHUNK_DSRADAR_PARAMS);
  chunk->setInfo("DsRadar params");
  DsRadarParams_t rparamsStruct;
  rparams.encode(&rparamsStruct);
  chunk->setData(&rparamsStruct, sizeof(DsRadarParams_t));
  _mdv->addChunk(chunk);

}
  
////////////////////////////////////////////////////////////////
// add calibration chunk from CfRadial, if cal data is available

void Ncf2MdvTrans::_addCalibCfRadial(const RadxVol &vol)

{

  if (vol.getRcalibs().size() < 1) {
    // no cal data
    return;
  }

  // use first calibration
  
  DsRadarCalib calOut;
  const RadxRcalib &calIn = *vol.getRcalibs()[0];

  calOut.setCalibTime(calIn.getCalibTime());

  calOut.setWavelengthCm(vol.getWavelengthM() * 100.0);
  calOut.setBeamWidthDegH(vol.getRadarBeamWidthDegH());
  calOut.setBeamWidthDegV(vol.getRadarBeamWidthDegV());
  calOut.setAntGainDbH(vol.getRadarAntennaGainDbH());
  calOut.setAntGainDbV(vol.getRadarAntennaGainDbV());

  calOut.setPulseWidthUs(calIn.getPulseWidthUsec());
  calOut.setXmitPowerDbmH(calIn.getXmitPowerDbmH());
  calOut.setXmitPowerDbmV(calIn.getXmitPowerDbmV());
  
  calOut.setTwoWayWaveguideLossDbH(calIn.getTwoWayWaveguideLossDbH());
  calOut.setTwoWayWaveguideLossDbV(calIn.getTwoWayWaveguideLossDbV());
  calOut.setTwoWayRadomeLossDbH(calIn.getTwoWayRadomeLossDbH());
  calOut.setTwoWayRadomeLossDbV(calIn.getTwoWayRadomeLossDbV());
  calOut.setReceiverMismatchLossDb(calIn.getReceiverMismatchLossDb());
  
  calOut.setRadarConstH(calIn.getRadarConstantH());
  calOut.setRadarConstV(calIn.getRadarConstantV());
  
  calOut.setNoiseDbmHc(calIn.getNoiseDbmHc());
  calOut.setNoiseDbmHx(calIn.getNoiseDbmHx());
  calOut.setNoiseDbmVc(calIn.getNoiseDbmVc());
  calOut.setNoiseDbmVx(calIn.getNoiseDbmVx());
  
  calOut.setReceiverGainDbHc(calIn.getReceiverGainDbHc());
  calOut.setReceiverGainDbHx(calIn.getReceiverGainDbHx());
  calOut.setReceiverGainDbVc(calIn.getReceiverGainDbVc());
  calOut.setReceiverGainDbVx(calIn.getReceiverGainDbVx());
  
  calOut.setReceiverSlopeDbHc(calIn.getReceiverSlopeDbHc());
  calOut.setReceiverSlopeDbHx(calIn.getReceiverSlopeDbHx());
  calOut.setReceiverSlopeDbVc(calIn.getReceiverSlopeDbVc());
  calOut.setReceiverSlopeDbVx(calIn.getReceiverSlopeDbVx());
  
  calOut.setBaseDbz1kmHc(calIn.getBaseDbz1kmHc());
  calOut.setBaseDbz1kmHx(calIn.getBaseDbz1kmHx());
  calOut.setBaseDbz1kmVc(calIn.getBaseDbz1kmVc());
  calOut.setBaseDbz1kmVx(calIn.getBaseDbz1kmVx());
  
  calOut.setSunPowerDbmHc(calIn.getSunPowerDbmHc());
  calOut.setSunPowerDbmHx(calIn.getSunPowerDbmHx());
  calOut.setSunPowerDbmVc(calIn.getSunPowerDbmVc());
  calOut.setSunPowerDbmVx(calIn.getSunPowerDbmVx());
  
  calOut.setNoiseSourcePowerDbmH(calIn.getNoiseSourcePowerDbmH());
  calOut.setNoiseSourcePowerDbmV(calIn.getNoiseSourcePowerDbmV());
  
  calOut.setPowerMeasLossDbH(calIn.getPowerMeasLossDbH());
  calOut.setPowerMeasLossDbV(calIn.getPowerMeasLossDbV());
  
  calOut.setCouplerForwardLossDbH(calIn.getCouplerForwardLossDbH());
  calOut.setCouplerForwardLossDbV(calIn.getCouplerForwardLossDbV());
  
  calOut.setZdrCorrectionDb(calIn.getZdrCorrectionDb());
  calOut.setLdrCorrectionDbH(calIn.getLdrCorrectionDbH());
  calOut.setLdrCorrectionDbV(calIn.getLdrCorrectionDbV());
  calOut.setSystemPhidpDeg(calIn.getSystemPhidpDeg());
  
  calOut.setTestPowerDbmH(calIn.getTestPowerDbmH());
  calOut.setTestPowerDbmV(calIn.getTestPowerDbmV());
  
  if (_debug) {
    calOut.print(cerr);
  }
  
  // put calibration
  
  MdvxChunk *chunk = new MdvxChunk;
  chunk->setId(Mdvx::CHUNK_DSRADAR_CALIB);
  chunk->setInfo("DsRadar calib");
  ds_radar_calib_t calibStruct;
  calOut.encode(&calibStruct);
  chunk->setData(&calibStruct, sizeof(ds_radar_calib_t));
  _mdv->addChunk(chunk);

}

///////////////////////////////////////////////////////////////
// add elevation angle chunk from CfRadial

void Ncf2MdvTrans::_addElevArrayCfRadial(const RadxVol &vol)

{

  MdvxChunk *chunk = new MdvxChunk;

  if (_isRhi) {
    chunk->setId(Mdvx::CHUNK_DSRADAR_AZIMUTHS);
    chunk->setInfo("RHI azimuth angles");
  } else {
    chunk->setId(Mdvx::CHUNK_DSRADAR_ELEVATIONS);
    chunk->setInfo("Radar Elevation angles");
  }
  
  MemBuf angBuf;
  si32 numAng = vol.getSweeps().size();
  angBuf.add(&numAng, sizeof(si32));
  
  for(int ii = 0; ii < numAng; ii++) {
    fl32 ang = vol.getSweeps()[ii]->getFixedAngleDeg();
    angBuf.add(&ang, sizeof(fl32));
  }
  
  BE_from_array_32(angBuf.getPtr(), angBuf.getLen());
  chunk->setData(angBuf.getPtr(), angBuf.getLen());

  _mdv->addChunk(chunk);
  
}

////////////////////////////////////////////////////////////////
// find empty sectors
//
// Sets the data region

void Ncf2MdvTrans::_findEmptySectors(const RadxVol &vol)

{
  
  if (_debug) {
    cerr << "==== Searching for empty sectors ====" << endl;
    cerr << "==== _nAngles: " << _nAngles << endl;
  }

  // load up array to indicate which azimuths are active

  TaArray<bool> active_;
  bool *active = active_.alloc(_nAngles);
  
  for (int iaz = 0; iaz < _nAngles; iaz++) {
    active[iaz] = false;
  }

  for (int ii = 0; ii < (int) vol.getRays().size(); ii++) {
    double az = vol.getRays()[ii]->getAzimuthDeg();
    if (az > 360.0) {
      az -= 360.0;
    } else if (az < 0) {
      az += 360.0;
    }
    int iaz = 0;
    if (az >= _minAngle) {
      iaz = (int) ((az - _minAngle) / _deltaAngle);
    } else {
      iaz = (int) ((az + 360.0 - _minAngle) / _deltaAngle);
    }
    active[iaz] = true;
  }

  // load up a vector of empty sectors

  vector<az_sector_t> emptySectors;
  int startIaz = 0;
  bool inEmptySector = false;
  if (!active[0]) {
    inEmptySector = true;
  }
  for (int iaz = 1; iaz < _nAngles; iaz++) {
    if (iaz < _nAngles - 1) {
      // not the last azimuth
      if (active[iaz]) {
	// this azimuth is active
	if (inEmptySector) {
	  az_sector_t sector;
	  sector.startIaz = startIaz;
	  sector.endIaz = iaz - 1;
	  if (sector.endIaz != sector.startIaz) {
	    emptySectors.push_back(sector);
	  }
	}
	inEmptySector = false;
      } else {
	// this azimuth is not active
	if (!inEmptySector) {
	  inEmptySector = true;
	  startIaz = iaz;
	}
      }
    } else {
      // last azimuth
      if (active[iaz]) {
	// this azimuth is active
	if (inEmptySector) {
	  az_sector_t sector;
	  sector.startIaz = startIaz;
	  sector.endIaz = iaz - 1;
	  if (sector.endIaz != sector.startIaz) {
	    emptySectors.push_back(sector);
	  }
	}
      } else {
	// this azimuth is not active
	if (inEmptySector) {
	  az_sector_t sector;
	  sector.startIaz = startIaz;
	  sector.endIaz = iaz;
	  if (sector.endIaz != sector.startIaz) {
	    emptySectors.push_back(sector);
	  }
	} else {
	  az_sector_t sector;
	  sector.startIaz = iaz;
	  sector.endIaz = iaz;
	  if (sector.endIaz != sector.startIaz) {
	    emptySectors.push_back(sector);
	  }
	}
      }
    }
  } // iaz

  int nEmpty = (int) emptySectors.size();
  if (_debug) {
    cerr << "===========================" << endl;
    for (int ii = 0; ii < nEmpty; ii++) {
      cerr << "------>> empty sector: " << ii << endl;
      cerr << "  start iaz, az: " << emptySectors[ii].startIaz << ", "
	   << emptySectors[ii].startIaz * _deltaAngle << endl;
      cerr << "  end iaz, az: " << emptySectors[ii].endIaz << ", "
	   << emptySectors[ii].endIaz * _deltaAngle << endl;
    }
    cerr << "===========================" << endl;
  }

  // loop through the empty sectors, finding the largest
  // contiguous block of azimuths
  // we ignore data blocks only 1 wide
  
  _dataStartIaz = 0;
  _dataEndIaz = 0;
  
  if (nEmpty == 0) {

    // no empty areas, use full circle
    
    _dataStartIaz = 0;
    _dataEndIaz = _nAngles - 1;
    _trimToSector = false;

  } else if (nEmpty == 1) {

    // only 1 empty sector

    _dataStartIaz = emptySectors[0].endIaz + 1;
    _dataEndIaz = emptySectors[0].startIaz - 1;
    if (_dataStartIaz > _nAngles) {
      _dataStartIaz -= _nAngles;
    }
    if (_dataEndIaz < 0) {
      _dataEndIaz += _nAngles;
    }
    _trimToSector = true;
    
  } else {

    // multiple empty sectors
    // search through them for the largest contiguous empty sector
    // with azimuth gaps of no greater than 3

    int maxNaz = 0;
    
    for (int ii = 0; ii < nEmpty; ii++) {

      int startIaz = emptySectors[ii].startIaz;
      int endIaz = 0;
      
      for (int jj = 0; jj < nEmpty; jj++) {
	
	int kk = (ii + jj) % nEmpty;
	int mm = (ii + jj + 1) % nEmpty;
	
	const az_sector_t &thisSec = emptySectors[kk];
	const az_sector_t &nextSec = emptySectors[mm];
	
	endIaz = thisSec.endIaz;
	
	int gap = 0;
	if (nextSec.startIaz >= thisSec.endIaz) {
	  gap = nextSec.startIaz - thisSec.endIaz;
	} else {
	  gap = nextSec.startIaz - thisSec.endIaz + _nAngles;
	}
	
	if (gap > 3) {
	  break;
	}

      } // jj

      int nAz = endIaz - startIaz;
      if (nAz < 0) {
	nAz += _nAngles;
      }
      
      if (nAz > maxNaz) {
	_dataStartIaz = endIaz + 1;
	_dataEndIaz = startIaz - 1;
	maxNaz = nAz;
	_trimToSector = true;
      }
      
    } // ii
    
  } // if (nEmpty == 0)

  if (_dataStartIaz > _dataEndIaz) {
    _dataStartIaz -= _nAngles;
  }
  _nAzTrimmed = _dataEndIaz - _dataStartIaz + 1;

  if (_debug) {
    cerr << "  _trimToSector: " << (_trimToSector? "Y":"N") << endl;
    cerr << "  data startIaz, az: " << _dataStartIaz << ", "
	 << _dataStartIaz * _deltaAngle << endl;
    cerr << "  data endIaz, az: " << _dataEndIaz << ", "
	 << _dataEndIaz * _deltaAngle << endl;
    cerr << "===========================" << endl;
  }

}

//////////////////////////////////////////////////
// get Dsr enums from Radx enums

int Ncf2MdvTrans::_getDsRadarType(Radx::PlatformType_t ptype)

{
  switch (ptype) {
  case Radx::PLATFORM_TYPE_VEHICLE:
    return DS_RADAR_VEHICLE_TYPE;
  case Radx::PLATFORM_TYPE_SHIP:
    return DS_RADAR_SHIPBORNE_TYPE;
  case Radx::PLATFORM_TYPE_AIRCRAFT_FORE:
    return DS_RADAR_AIRBORNE_FORE_TYPE;
  case Radx::PLATFORM_TYPE_AIRCRAFT_AFT:
    return DS_RADAR_AIRBORNE_AFT_TYPE;
  case Radx::PLATFORM_TYPE_AIRCRAFT_TAIL:
    return DS_RADAR_AIRBORNE_TAIL_TYPE;
  case Radx::PLATFORM_TYPE_AIRCRAFT_BELLY:
    return DS_RADAR_AIRBORNE_LOWER_TYPE;
  case Radx::PLATFORM_TYPE_AIRCRAFT_ROOF:
    return DS_RADAR_AIRBORNE_UPPER_TYPE;
  default:
    return DS_RADAR_GROUND_TYPE;
  }
}

int Ncf2MdvTrans::_getDsScanMode(Radx::SweepMode_t mode)

{
  switch (mode) {
  case Radx::SWEEP_MODE_SECTOR:
    return DS_RADAR_SECTOR_MODE;
  case Radx::SWEEP_MODE_COPLANE:
    return DS_RADAR_COPLANE_MODE;
  case Radx::SWEEP_MODE_RHI:
    return DS_RADAR_RHI_MODE;
  case Radx::SWEEP_MODE_VERTICAL_POINTING:
    return DS_RADAR_VERTICAL_POINTING_MODE;
  case Radx::SWEEP_MODE_IDLE:
    return DS_RADAR_IDLE_MODE;
  case Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE:
    return DS_RADAR_SURVEILLANCE_MODE;
  case Radx::SWEEP_MODE_SUNSCAN:
    return DS_RADAR_SUNSCAN_MODE;
  case Radx::SWEEP_MODE_POINTING:
    return DS_RADAR_POINTING_MODE;
  case Radx::SWEEP_MODE_MANUAL_PPI:
    return DS_RADAR_MANUAL_MODE;
  case Radx::SWEEP_MODE_MANUAL_RHI:
    return DS_RADAR_MANUAL_MODE;
  case Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE:
  default:
    return DS_RADAR_SURVEILLANCE_MODE;
  }
}

int Ncf2MdvTrans::_getDsFollowMode(Radx::FollowMode_t mode)

{
  switch (mode) {
  case Radx::FOLLOW_MODE_SUN:
    return DS_RADAR_FOLLOW_MODE_SUN;
  case Radx::FOLLOW_MODE_VEHICLE:
    return DS_RADAR_FOLLOW_MODE_VEHICLE;
  case Radx::FOLLOW_MODE_AIRCRAFT:
    return DS_RADAR_FOLLOW_MODE_AIRCRAFT;
  case Radx::FOLLOW_MODE_TARGET:
    return DS_RADAR_FOLLOW_MODE_TARGET;
  case Radx::FOLLOW_MODE_MANUAL:
    return DS_RADAR_FOLLOW_MODE_MANUAL;
  default:
    return DS_RADAR_FOLLOW_MODE_NONE;
  }
}

int Ncf2MdvTrans::_getDsPolarizationMode(Radx::PolarizationMode_t mode)

{
  switch (mode) {
  case Radx::POL_MODE_HORIZONTAL:
    return DS_POLARIZATION_HORIZ_TYPE;
  case Radx::POL_MODE_VERTICAL:
    return DS_POLARIZATION_VERT_TYPE;
  case Radx::POL_MODE_HV_ALT:
    return DS_POLARIZATION_DUAL_HV_ALT;
  case Radx::POL_MODE_HV_SIM:
    return DS_POLARIZATION_DUAL_HV_SIM;
  case Radx::POL_MODE_CIRCULAR:
    return DS_POLARIZATION_RIGHT_CIRC_TYPE;
  default:
    return DS_POLARIZATION_HORIZ_TYPE;
  }
}

int Ncf2MdvTrans::_getDsPrfMode(Radx::PrtMode_t mode)

{
  switch (mode) {
  case Radx::PRT_MODE_FIXED:
    return DS_RADAR_PRF_MODE_FIXED;
  case Radx::PRT_MODE_STAGGERED:
    return DS_RADAR_PRF_MODE_STAGGERED_2_3;
  default:
    return DS_RADAR_PRF_MODE_FIXED;
  }
}

