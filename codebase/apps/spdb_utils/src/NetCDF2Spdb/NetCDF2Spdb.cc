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
// NetCDF2Spdb.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2014
//
///////////////////////////////////////////////////////////////
//
// NetCDF2Spdb reads met data in netCDF, interprets it and
// writes it to SPDB
//
////////////////////////////////////////////////////////////////

#include <iomanip>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/Tty.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/DateTime.hh>
#include <physics/thermo.h>
#include <rapformats/ac_posn.h>
#include <rapformats/ltg.h>
#include "NetCDF2Spdb.hh"
using namespace std;

const double NetCDF2Spdb::_missing = -9999.0;

// Constructor

NetCDF2Spdb::NetCDF2Spdb(int argc, char **argv)
  
{
  
  isOK = true;
  _input = NULL;
  
  // set programe name

  _progName = "NetCDF2Spdb";
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

  // create input object

  if (_params.mode == Params::REALTIME) {
    _input = new DsInputPath(_progName,
                             _params.debug >= Params::DEBUG_VERBOSE,
                             _params.input_dir,
                             _params.max_realtime_valid_age,
                             PMU_auto_register,
                             _params.latest_data_info_avail,
                             _params.latest_file_only);
  } else if (_params.mode == Params::ARCHIVE) {
    _input = new DsInputPath(_progName, 
                             _params.debug >= Params::DEBUG_VERBOSE,
                             _params.input_dir,
                             _args.startTime, _args.endTime);
  } else if (_params.mode == Params::FILELIST) {
    _input = new DsInputPath(_progName,
                             _params.debug >= Params::DEBUG_VERBOSE,
                             _args.inputFileList);
  }

  // init process mapper registration
  
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

NetCDF2Spdb::~NetCDF2Spdb()

{

  _clearData();

  if (_input) {
    delete _input;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int NetCDF2Spdb::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // create DsSpdb object for output
  
  const char *path;
  while ((path = _input->next()) != NULL) {
    
    if (_processFile(path)) {
      cerr << "ERROR - NetCDF2Spdb::run" << endl;
      return -1;
    }

  }

  return 0;

}

//////////////////////////////////////////////////
// process specified file

int NetCDF2Spdb::_processFile(const string &path)

{

  _clearData();

  // register with procmap
  
  PMU_auto_register("Reading file");
  
  // open netscf file

  if (_ncf.openRead(path)) {
    cerr << "ERROR - NetCDF2Spdb::_processFile" << endl;
    cerr << "  Cannot open file: " << path << endl;
    cerr << _ncf.getErrStr() << endl;
    return -1;
  }

  // assume for now that all vars are in the root group
  // i.e. this is a classic-style file

  // read in times

  if (_readTimes()) {
    cerr << "ERROR - NetCDF2Spdb::_processFile" << endl;
    return -1;
  }

  // read in variables

  if (_readVars()) {
    cerr << "ERROR - NetCDF2Spdb::_processFile" << endl;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _printData(cerr);
  }

  // close
  
  _ncf.close();

  // write data

  int iret = 0;
  switch (_params.output_format) {
    case Params::SURFACE:
      iret = _writeSurface();
      break;
    case Params::SOUNDING:
      iret = _writeSounding();
      break;
    case Params::LIGHTNING:
      iret = _writeLightning();
      break;
    case Params::AIRCRAFT:
      iret = _writeAircraft();
      break;
    case Params::GEN_PT:
      iret = _writeGenPt();
      break;
  } // switch

  return iret;

}

//////////////////////////////////////////////////
// read in times

int NetCDF2Spdb::_readTimes()

{

  // register with procmap
  
  PMU_auto_register("Reading times");
  
  // assume for now that all vars are in the root group
  // i.e. this is a classic-style file

  // get time var

  NcFile *file = _ncf.getNcFile();
  NcVar timeVar = file->getVar(_params.time_var_name);

  _timeDim = timeVar.getDim(0);
  _nTimes = _timeDim.getSize();

  NcVarAtt timeUnitsAtt = timeVar.getAtt("units");
  NcType unitsAttType = timeUnitsAtt.getType();

  // get base time from units if possible
  // defaults to Jan 1 1970

  DateTime baseTime;
  baseTime.set(1970, 1, 1, 0, 0, 0);
  if (unitsAttType.getId() == NC_CHAR) {
    string timeUnits;
    timeUnitsAtt.getValues(timeUnits);
    if (baseTime.setFromW3c(timeUnits.c_str())) {
      baseTime.set(1970, 1, 1, 0, 0, 0);
    }
  }
  time_t btime = baseTime.utime();
    
  NcType timeVarType = timeVar.getType();
  if (timeVarType.getId() == NcType::nc_INT ||
      timeVarType.getId() == NcType::nc_UINT ||
      timeVarType.getId() == NcType::nc_INT64) {

    vector<int64_t> itimes;
    if (_ncf.readInt64Array(_params.time_var_name, itimes, 0, true)) {
      cerr << "ERROR - NetCDF2Spdb::_readTimes" << endl;
      cerr << "  Cannot read time var: " << _params.time_var_name << endl;
      return -1;
    }
    
    for (size_t ii = 0; ii < itimes.size(); ii++) {
      _time.push_back(itimes[ii] + btime);
      _dtime.push_back(itimes[ii] + btime);
    }

  } else {

    vector<double> dtimes;
    if (_ncf.readDoubleArray(_params.time_var_name, dtimes, 0, true)) {
      cerr << "ERROR - NetCDF2Spdb::_readTimes" << endl;
      cerr << "  Cannot read time var: " << _params.time_var_name << endl;
      return -1;
    }

    for (size_t ii = 0; ii < dtimes.size(); ii++) {
      _time.push_back((time_t) (dtimes[ii] + btime));
      _dtime.push_back(dtimes[ii] + btime);
    }

  }

  return 0;

}

//////////////////////////////////////////////////
// read in variables

int NetCDF2Spdb::_readVars()

{

  // register with procmap
  
  PMU_auto_register("Reading vars");
  
  // read in position
  
  _readVar(_params.latitude_var_name, _latitude);
  _readVar(_params.longitude_var_name, _longitude);
  _readVar(_params.altitude_var_name, _altitude);
  _readVar(_params.pressure_var_name, _pressure);
  _readVar(_params.temp_var_name, _temp);
  _readVar(_params.dewpt_var_name, _dewpt);
  _readVar(_params.rh_var_name, _rh);
  _readVar(_params.mixing_ratio_var_name, _mixingRatio);
  _readVar(_params.liquid_water_var_name, _liquidWater);
  _readVar(_params.wind_dirn_var_name, _windDirn);
  _readVar(_params.wind_speed_var_name, _windSpeed);
  _readVar(_params.wind_gust_var_name, _windGust);
  _readVar(_params.true_airspeed_var_name, _trueAirspeed);
  _readVar(_params.ground_speed_var_name, _groundSpeed);
  _readVar(_params.precip_rate_var_name, _precipRate);
  _readVar(_params.precip_accum_var_name, _precipAccum);
  _readVar(_params.visibility_var_name, _visibility);
  _readVar(_params.ceiling_var_name, _ceiling);
  _readVar(_params.rvr_var_name, _rvr);
  _readVar(_params.ltg_amplitude_var_name, _ltgAmplitude);
  _readVar(_params.ltg_ellipse_angle_var_name, _ltgEllipseAngle);
  _readVar(_params.ltg_semi_major_axis_var_name, _ltgSemiMajorAxis);
  _readVar(_params.ltg_eccentricity_var_name, _ltgEccentricity);
  _readVar(_params.ltg_chisq_var_name, _ltgChisq);
  
  return 0;

}

//////////////////////////////////////////////////
// read in a variable with a time dimension

int NetCDF2Spdb::_readVar(const char *varName,
                          vector<double> &loc)

{

  if (strlen(varName) > 0) {
    if (_ncf.readDoubleArray(varName, loc, _missing, true)) {
      for (size_t ii = 0; ii < _nTimes; ii++) {
        loc.push_back(_missing);
      }
      cerr << "ERROR - NetCDF2Spdb::_readVar" << endl;
      cerr << "  Cannot read var: " << varName << endl;
      return -1;
    }
  } else {
    for (size_t ii = 0; ii < _nTimes; ii++) {
      loc.push_back(_missing);
    }
  }

  // make sure the array has time dimension

  NcVar var = _ncf.getNcFile()->getVar(varName);
  NcDim varDim = var.getDim(0);
  if (varDim != _timeDim) {
    loc.clear();
    for (size_t ii = 0; ii < _nTimes; ii++) {
      loc.push_back(_missing);
    }
    cerr << "ERROR - NetCDF2Spdb::_readVar" << endl;
    cerr << "  Var does not have time dimension" << endl;
    cerr << "  Cannot read var: " << varName << endl;
    return -1;
  }

  return 0;

}
  
//////////////////////////////////////////////////
// clear data

void NetCDF2Spdb::_clearData()

{

  _time.clear();
  _dtime.clear();
  _latitude.clear();
  _longitude.clear();
  _altitude.clear();
  _pressure.clear();
  _temp.clear();
  _dewpt.clear();
  _rh.clear();
  _mixingRatio.clear();
  _liquidWater.clear();
  _windDirn.clear();
  _windSpeed.clear();
  _windGust.clear();
  _trueAirspeed.clear();
  _groundSpeed.clear();
  _precipRate.clear();
  _precipAccum.clear();
  _visibility.clear();
  _ceiling.clear();
  _rvr.clear();
  _ltgAmplitude.clear();
  _ltgEllipseAngle.clear();
  _ltgSemiMajorAxis.clear();
  _ltgEccentricity.clear();
  _ltgChisq.clear();

}

//////////////////////////////////////////////////
// print data

void NetCDF2Spdb::_printData(ostream &out)

{

  out << "_nTimes: " << _nTimes << endl;

  switch (_params.output_format) {

    case Params::SURFACE:
      for (size_t ii = 0; ii < _nTimes; ii++) {
        out << "ii,time,lat,lon,alt,pres,temp,dewpt,rh,"
            << "wdir,wspd,gust,prate,paccum,vis,ceil,rvr,amp: "
            << ii << ", "
            << DateTime::strm(_time[ii]) << ", "
            << _latitude[ii] << ", "
            << _longitude[ii] << ", "
            << _altitude[ii] << ", "
            << _pressure[ii] << ", "
            << _temp[ii] << ", "
            << _dewpt[ii] << ", "
            << _rh[ii] << ", "
            << _windDirn[ii] << ", "
            << _windSpeed[ii] << ", "
            << _windGust[ii] << ", "
            << _precipRate[ii] << ", "
            << _precipAccum[ii] << ", "
            << _visibility[ii] << ", "
            << _ceiling[ii] << ", "
            << _rvr[ii] << endl;
      } // ii
      break;

    case Params::SOUNDING:
      for (size_t ii = 0; ii < _nTimes; ii++) {
        out << "ii,time,lat,lon,alt,pres,temp,dewpt,rh,"
            << "wdir,wspd: "
            << ii << ", "
            << DateTime::strm(_time[ii]) << ", "
            << _latitude[ii] << ", "
            << _longitude[ii] << ", "
            << _altitude[ii] << ", "
            << _pressure[ii] << ", "
            << _temp[ii] << ", "
            << _dewpt[ii] << ", "
            << _rh[ii] << ", "
            << _windDirn[ii] << ", "
            << _windSpeed[ii] << endl;
      } // ii
      break;

    case Params::LIGHTNING:
      for (size_t ii = 0; ii < _nTimes; ii++) {
        out << "ii,time,lat,lon,alt,"
            << "amp,ellipse,axis,eccen,chisq: "
            << ii << ", "
            << DateTime::strm(_time[ii]) << ", "
            << _latitude[ii] << ", "
            << _longitude[ii] << ", "
            << _altitude[ii] << ", "
            << _ltgAmplitude[ii] << ", "
            << _ltgEllipseAngle[ii] << ", "
            << _ltgSemiMajorAxis[ii] << ", "
            << _ltgEccentricity[ii] << ", "
            << _ltgChisq[ii] << endl;
      } // ii
      break;

    case Params::AIRCRAFT:
      for (size_t ii = 0; ii < _nTimes; ii++) {
        out << "ii,time,lat,lon,alt,pres,temp,dewpt,rh,mr,"
            << "llw,wdir,wspd,tas,gs: "
            << ii << ", "
            << DateTime::strm(_time[ii]) << ", "
            << _latitude[ii] << ", "
            << _longitude[ii] << ", "
            << _altitude[ii] << ", "
            << _pressure[ii] << ", "
            << _temp[ii] << ", "
            << _dewpt[ii] << ", "
            << _rh[ii] << ", "
            << _mixingRatio[ii] << ", "
            << _liquidWater[ii] << ", "
            << _windDirn[ii] << ", "
            << _windSpeed[ii] << ", "
            << _trueAirspeed[ii] << ", "
            << _groundSpeed[ii] << endl;
      } // ii
      break;

    case Params::GEN_PT:
    default:
      for (size_t ii = 0; ii < _nTimes; ii++) {
        out << "ii,time,lat,lon,alt,pres,temp,dewpt,rh,mr,"
            << "llw,wdir,wspd,gust,tas,gs,prate,paccum,vis,"
            << "vis,ceil,rvr,"
            << "amp,ellipse,axis,eccen,chisq: "
            << ii << ", "
            << DateTime::strm(_time[ii]) << ", "
            << _latitude[ii] << ", "
            << _longitude[ii] << ", "
            << _altitude[ii] << ", "
            << _pressure[ii] << ", "
            << _temp[ii] << ", "
            << _dewpt[ii] << ", "
            << _rh[ii] << ", "
            << _mixingRatio[ii] << ", "
            << _liquidWater[ii] << ", "
            << _windDirn[ii] << ", "
            << _windSpeed[ii] << ", "
            << _windGust[ii] << ", "
            << _trueAirspeed[ii] << ", "
            << _groundSpeed[ii] << ", "
            << _precipRate[ii] << ", "
            << _precipAccum[ii] << ", "
            << _visibility[ii] << ", "
            << _ceiling[ii] << ", "
            << _rvr[ii] << ", "
            << _ltgAmplitude[ii] << ", "
            << _ltgEllipseAngle[ii] << ", "
            << _ltgSemiMajorAxis[ii] << ", "
            << _ltgEccentricity[ii] << ", "
            << _ltgChisq[ii] << endl;
      } // ii
      break;

  } // switch

}

//////////////////////////////////////////////////
// write out surface data
// returns 0 on success, -1 on failure

int NetCDF2Spdb::_writeSurface()

{

  cerr << "NOTE: NetCDF2Spdb::_writeSurface()" << endl;
  cerr << "Not yet implemented" << endl;

  return -1;

}

//////////////////////////////////////////////////
// write out sounding data
// returns 0 on success, -1 on failure

int NetCDF2Spdb::_writeSounding()

{

  cerr << "NOTE: NetCDF2Spdb::_writeSounding()" << endl;
  cerr << "Not yet implemented" << endl;

  return -1;

}

//////////////////////////////////////////////////
// write out lightning data
// returns 0 on success, -1 on failure

int NetCDF2Spdb::_writeLightning()

{

  DsSpdb spdb;
  
  for (size_t ii = 0; ii < _nTimes; ii++) {

    LTG_extended_t strike;
    LTG_init_extended(&strike);
    strike.time = (si32) _time[ii];
    strike.latitude = _latitude[ii];
    strike.longitude = _longitude[ii];
    strike.amplitude = _ltgAmplitude[ii];
    strike.type = LTG_GROUND_STROKE;
    strike.n_sensors = 0;
    strike.semi_major_axis = _ltgSemiMajorAxis[ii];
    if (_ltgSemiMajorAxis[ii] > -9990 &&
        _ltgEccentricity[ii] > -9990) {
      strike.semi_minor_axis = _ltgSemiMajorAxis[ii] / _ltgEccentricity[ii];
    }
    strike.ellipse_angle = _ltgEllipseAngle[ii];
    strike.chi_sq = _ltgChisq[ii];
    
    if (_params.debug >= Params::DEBUG_EXTRA) {
      LTG_print_extended(stderr, &strike);
    }
    
    si32 dataType = 0;
    LTG_extended_to_BE(&strike);
    
    time_t validTime = _time[ii];
    spdb.addPutChunk(dataType,
                     validTime,
                     validTime + _params.valid_period_secs,
                     sizeof(LTG_extended_t),
                     &strike,
                     SPDB_LTG_ID);
    
  } // ii
  
  // put data
  
  int iret = 0;
  if (spdb.put(_params.output_url,
               SPDB_LTG_ID,
               SPDB_LTG_LABEL)) {
    cerr << "ERROR - NetCDF2Spdb::_writeLtg" << endl;
    cerr << spdb.getErrStr() << endl;
    iret = -1;
  }

  if (_params.debug) {
    cerr << "Wrote n lightning entries: " << _nTimes << endl;
    cerr << "  to URL: " << _params.output_url << endl;
  }
  
  spdb.clearPutChunks();

  return iret;

}

//////////////////////////////////////////////////
// write out aircraft data
// returns 0 on success, -1 on failure

int NetCDF2Spdb::_writeAircraft()

{
  
  DsSpdb spdb;
  
  for (size_t ii = 0; ii < _nTimes; ii++) {

    ac_posn_wmod_t posn;
    MEM_zero(posn);
    posn.lat = _latitude[ii];
    posn.lon = _longitude[ii];
    posn.alt = _altitude[ii];
    posn.tas = _trueAirspeed[ii];
    posn.gs = _groundSpeed[ii];
    posn.temp = _temp[ii];
    posn.dew_pt = _dewpt[ii];
    posn.lw = _liquidWater[ii];
    posn.fssp = AC_POSN_MISSING_FLOAT;
    posn.rosemount = AC_POSN_MISSING_FLOAT;
    time_t validTime = _time[ii];
    STRncopy(posn.callsign, _params.aircraft_callsign, AC_POSN_N_CALLSIGN);
    
    if (_params.debug >= Params::DEBUG_EXTRA) {
      ac_posn_wmod_print(stderr, "", &posn);
    }
  
    si32 dataType = Spdb::hash4CharsToInt32(posn.callsign);
    BE_from_ac_posn_wmod(&posn);
    
    spdb.addPutChunk(dataType,
                     validTime,
                     validTime + _params.valid_period_secs,
                     sizeof(ac_posn_wmod_t),
                     &posn,
                     SPDB_AC_POSN_WMOD_ID);
    
  } // ii
  
  // put data
  
  int iret = 0;
  if (spdb.put(_params.output_url,
               SPDB_AC_POSN_ID,
               SPDB_AC_POSN_LABEL)) {
    cerr << "ERROR - NetCDF2Spdb" << endl;
    cerr << spdb.getErrStr() << endl;
    iret = -1;
  }
  
  if (_params.debug) {
    cerr << "Wrote n aircraft entries: " << _nTimes << endl;
    cerr << "  to URL: " << _params.output_url << endl;
  }
  
  spdb.clearPutChunks();

  return iret;

}

//////////////////////////////////////////////////
// write out genpt data
// returns 0 on success, -1 on failure

int NetCDF2Spdb::_writeGenPt()

{

  cerr << "NOTE: NetCDF2Spdb::_writeGenPt()" << endl;
  cerr << "Not yet implemented" << endl;

  return -1;

}

