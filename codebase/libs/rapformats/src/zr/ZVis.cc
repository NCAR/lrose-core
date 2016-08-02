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
// ZVis.cc
//
// C++ class for dealing with radar-visibility calibration.
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// July 2002
//////////////////////////////////////////////////////////////


#include <rapformats/ZVis.hh>
#include <dataport/bigend.h>
#include <toolsa/DateTime.hh>
#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <cmath>
using namespace std;

///////////////
// constructor

ZVis::ZVis()

{

  _versionNum = 1;
  clear();

}

/////////////
// destructor

ZVis::~ZVis()

{

}

//////////////////////////
// clear all data members

void ZVis::clear()

{

  _visSensorName = "";
  _visSensorLat = 0;
  _visSensorLon = 0;

  _validTime = 0;
  _calibTime = 0;
  _calibSecs = 0;
  _calibStatus = (zv_calib_status_t) 0;

  _trecU = 0.0;
  _trecV = 0.0;
  _surfaceU = 0.0;
  _surfaceV = 0.0;

  _rmse = 0.0;
  _fallSecs = 0.0;
  _trecWeight = 0.0;
  _surfaceWeight = 0.0;
  _calibCoeff = 0.0;
  _calibExpon = 0.0;

  _smoothedRmse = 0.0;
  _smoothedFallSecs = 0.0;
  _smoothedTrecWeight = 0.0;
  _smoothedSurfaceWeight = 0.0;
  _smoothedCalibCoeff = 0.0;
  _smoothedCalibExpon = 0.0;

  _calibE.clear();
  _calibDbz.clear();

  _dbzMeasured = 0.0;
  _eMeasured = 0.0;

  _eTrendSecs = 0;
  _eTrendSlope = 0.0;

  _forecastDeltaSecs = 0;
  _forecastStartTime = 0;
  _forecastE.clear();
  _forecastDbz.clear();

}

///////////////////////////////////////////
// assemble()
// Load up the buffer from the object.
// Handles byte swapping.

void ZVis::assemble()
  
{

  // free up mem buffer

  _memBuf.free();

  // load up header struct

  zv_calib_hdr_t hdr;
  _loadHeader(hdr);

  // byte swamp to bigendian, add to buffer

  calib_hdr_to_BE(hdr);
  _memBuf.add(&hdr, sizeof(hdr));

  // add calib ez data
  
  for (size_t ii = 0; ii < _calibE.size(); ii++) {

    fl32 ee = _calibE[ii];
    BE_from_array_32(&ee, sizeof(fl32));
    _memBuf.add(&ee, sizeof(fl32));

    fl32 dbz = _calibDbz[ii];
    BE_from_array_32(&dbz, sizeof(fl32));
    _memBuf.add(&dbz, sizeof(fl32));

  } // ii
  
  // add forecast ez data
  
  for (size_t ii = 0; ii < _forecastE.size(); ii++) {

    fl32 e_fcast = _forecastE[ii];
    BE_from_array_32(&e_fcast, sizeof(fl32));
    _memBuf.add(&e_fcast, sizeof(fl32));

    fl32 dbz_fcast = _forecastDbz[ii];
    BE_from_array_32(&dbz_fcast, sizeof(fl32));
    _memBuf.add(&dbz_fcast, sizeof(fl32));

  } // ii
  
}

///////////////////////////////////////////////////////////
// disassemble()
// Disassembles a buffer, sets the values in the object.
// Handles byte swapping.
// Returns 0 on success, -1 on failure

int ZVis::disassemble(const void *buf, int len)

{

  int minLen = (int) (sizeof(zv_calib_hdr_t));
  if (len < minLen) {
    cerr << "ERROR - ZVis::disassemble" << endl;
    cerr << "  Buffer too small for disassemble" << endl;
    cerr << "  Buf len: " << len << endl;
    cerr << "  Min len: " << minLen << endl;
    return -1;
  }

  // header

  int offset = 0;
  zv_calib_hdr_t hdr;
  memcpy(&hdr, (ui08 *) buf + offset, sizeof(zv_calib_hdr_t));
  calib_hdr_from_BE(hdr);
  _unloadHeader(hdr);
  offset += sizeof(zv_calib_hdr_t);

  // calib ze data

  _calibE.clear();
  _calibDbz.clear();
  for (int ii = 0; ii < hdr.n_ze_pairs; ii++) {

    fl32 ee;
    memcpy(&ee, (ui08 *) buf + offset, sizeof(fl32));
    BE_to_array_32(&ee, sizeof(fl32));
    _calibE.push_back((double) ee);
    offset += sizeof(fl32);

    fl32 dbz;
    memcpy(&dbz, (ui08 *) buf + offset, sizeof(fl32));
    BE_to_array_32(&dbz, sizeof(fl32));
    _calibDbz.push_back((double) dbz);
    offset += sizeof(fl32);

  }

  // forecast data

  _forecastE.clear();
  _forecastE.clear();
  for (int ii = 0; ii < hdr.n_forecasts; ii++) {
    
    fl32 e_fcast;
    memcpy(&e_fcast, (ui08 *) buf + offset, sizeof(fl32));
    BE_to_array_32(&e_fcast, sizeof(fl32));
    _forecastE.push_back((double) e_fcast);
    offset += sizeof(fl32);

    fl32 dbz_fcast;
    memcpy(&dbz_fcast, (ui08 *) buf + offset, sizeof(fl32));
    BE_to_array_32(&dbz_fcast, sizeof(fl32));
    _forecastDbz.push_back((double) dbz_fcast);
    offset += sizeof(fl32);

  }

  return 0;

}

//////////////////////
// printing object


void ZVis::print(ostream &out, string spacer /* = ""*/ ) const

{

  out << "===================================" << endl;
  out << spacer << "ZViz object" << endl;
  out << spacer << "  Version number: " << _versionNum << endl;

  out << spacer << "  --- Sensor details ---" << endl;
  out << spacer << "    visSensorName: " << _visSensorName << endl;
  out << spacer << "    visSensorLat: " << _visSensorLat << endl;
  out << spacer << "    visSensorLon: " << _visSensorLon << endl;

  out << spacer << "  --- Calib details ---" << endl;
  out << spacer << "    validTime: " << DateTime::strn(_validTime) << endl;
  out << spacer << "    calibTime: " << DateTime::strn(_calibTime) << endl;
  out << spacer << "    calibSecs: " << _calibSecs << endl;
  out << spacer << "    calibStatus: " << _calibStatus << " = ";
  switch (_calibStatus) {
  case ZV_STATUS_CALIBRATED:
    out << "CALIBRATED" << endl;
    break;
  case ZV_STATUS_COASTING:
    out << "COASTING" << endl;
    break;
  case ZV_STATUS_DEFAULT:
    out << "DEFAULT" << endl;
    break;
  case ZV_STATUS_PERSIST:
    out << "PERSIST" << endl;
    break;
  }

  out << spacer << "  --- Winds ---" << endl;
  out << spacer << "    TREC U,V: " << _trecU << ", " << _trecV << endl;
  out << spacer << "    SURF U,V: " << _surfaceU << ", " << _surfaceV << endl;

  out << spacer << "  --- Calib results ---" << endl;
  out << spacer << "    RMSE: " << _rmse << endl;
  out << spacer << "    fallSecs: " << _fallSecs << endl;
  out << spacer << "    trecWeight, surfaceWeight: "
      << _trecWeight << ", " << _surfaceWeight << endl;
  out << spacer << "    calibCoeff, calibExpon: "
      << _calibCoeff << ", " << _calibExpon << endl;

  out << spacer << "  --- Calib results smoothed ---" << endl;
  out << spacer << "    smoothed RMSE: " << _smoothedRmse << endl;
  out << spacer << "    smoothedFallSecs: "
      << _smoothedFallSecs << endl;
  out << spacer << "    smoothedTrecWeight, smoothedSurfaceWeight: "
      << _smoothedTrecWeight << ", "
      << _smoothedSurfaceWeight << endl;
  out << spacer << "    smoothedCalibCoeff, smoothedCalibExpon: "
      << _smoothedCalibCoeff << ", "
      << _smoothedCalibExpon << endl;

  out << spacer << "  --- ZE pairs ---" << endl;
  out << spacer << "  (dBZ,E):";
  for (size_t ii = 0; ii < _calibE.size(); ii++) {
    out << " (" << _calibDbz[ii] << "," << _calibE[ii] << ")";
  }
  out << endl;

  out << spacer << "  --- Measured values ---" << endl;
  out << spacer << "    dbzMeasured, eMeasured: "
      << _dbzMeasured << ", " << _eMeasured << endl;

  out << spacer << "  --- Trend forecast ---" << endl;
  out << spacer << "    eTrendSecs, eTrendSlope(1/s): "
      << _eTrendSecs << ", " << _eTrendSlope << endl;
  
  out << spacer << "  --- E forecasts ---" << endl;
  out << spacer << "    nForecasts: " << _forecastE.size() << endl;
  out << spacer << "    forecastDeltaSecs: " << _forecastDeltaSecs << endl;
  out << spacer << "    forecastStartTime: "
      << DateTime::strn(_forecastStartTime) << endl;
  out << spacer << "    Forecast (dbz,E): ";
  for (size_t ii = 0; ii < _forecastE.size(); ii++) {
    out << " (";
    if (_forecastDbz[ii] > -9990) {
      out << _forecastDbz[ii];
    } else {
      out << "MISS";
    }
    out << ",";
    if (_forecastE[ii] > -9990) {
      out << _forecastE[ii];
    } else {
      out << "MISS";
    }
    out << ")";
  }
  out << endl;
  
}

/////////////////
// byte swapping

void ZVis::calib_hdr_to_BE(zv_calib_hdr_t &hdr)
{
  BE_from_array_32(&hdr, ZV_CALIB_HDR_NBYTES_32);
}

void ZVis::calib_hdr_from_BE(zv_calib_hdr_t &hdr)
{
  BE_to_array_32(&hdr, ZV_CALIB_HDR_NBYTES_32);
}

////////////////////////////////////////////
// load up header struct

void ZVis::_loadHeader(zv_calib_hdr_t &hdr) const

{

  MEM_zero(hdr);

  hdr.version_num = (si32) _versionNum;

  hdr.valid_time = (ti32) _validTime;
  hdr.calib_time = (ti32) _calibTime;
  hdr.calib_secs = (si32) _calibSecs;
  hdr.status = (si32) _calibStatus;
  hdr.n_ze_pairs = (si32) _calibE.size();

  hdr.e_trend_secs = _eTrendSecs;
  hdr.n_forecasts = (si32) _forecastE.size();
  hdr.forecast_start_time = (ti32) _forecastStartTime;
  hdr.forecast_delta_secs = _forecastDeltaSecs;

  hdr.trec_u = (fl32) _trecU;
  hdr.trec_v = (fl32) _trecV;
  hdr.surf_u = (fl32) _surfaceU;
  hdr.surf_v = (fl32) _surfaceV;

  hdr.rmse = (fl32) _rmse;
  hdr.fall_secs = (fl32) _fallSecs; 
  hdr.trec_wt = (fl32) _trecWeight;
  hdr.surf_wt = (fl32) _surfaceWeight;   
  hdr.coeff = (fl32) _calibCoeff;     
  hdr.expon = (fl32) _calibExpon;     

  hdr.smoothed_rmse = (fl32) _smoothedRmse;
  hdr.smoothed_fall_secs = (fl32) _smoothedFallSecs; 
  hdr.smoothed_trec_wt = (fl32) _smoothedTrecWeight;
  hdr.smoothed_surf_wt = (fl32) _smoothedSurfaceWeight;   
  hdr.smoothed_coeff = (fl32) _smoothedCalibCoeff;     
  hdr.smoothed_expon = (fl32) _smoothedCalibExpon;     

  hdr.dbz_measured = (fl32) _dbzMeasured;
  hdr.e_measured = (fl32) _eMeasured;

  hdr.e_trend_slope = (fl32) _eTrendSlope;

  double dfrac, dint;
  dfrac = modf(_visSensorLat, &dint);
  hdr.vis_sensor_lat_deg = (fl32) dint;
  hdr.vis_sensor_lat_frac_deg = (fl32) dfrac;

  dfrac = modf(_visSensorLon, &dint);
  hdr.vis_sensor_lon_deg = (fl32) dint;
  hdr.vis_sensor_lon_frac_deg =  (fl32) dfrac;

  STRcopy(hdr.vis_sensor_name, _visSensorName.c_str(), ZV_GAUGE_NAME_LEN);

}

////////////////////////////////////////////
// un load header struct

void ZVis::_unloadHeader(const zv_calib_hdr_t &hdr)

{

  _versionNum = (int) hdr.version_num;

  _validTime = (time_t) hdr.valid_time;
  _calibTime = (time_t) hdr.calib_time;
  _calibSecs = (int) hdr.calib_secs;
  _calibStatus = (zv_calib_status_t) hdr.status;

  _eTrendSecs = hdr.e_trend_secs;
  _forecastStartTime = (time_t) hdr.forecast_start_time;
  _forecastDeltaSecs = hdr.forecast_delta_secs;

  _trecU = hdr.trec_u;
  _trecV = hdr.trec_v;
  _surfaceU = hdr.surf_u;
  _surfaceV = hdr.surf_v;

  _rmse = hdr.rmse;
  _fallSecs = hdr.fall_secs;
  _trecWeight = hdr.trec_wt;
  _surfaceWeight = hdr.surf_wt;
  _calibCoeff = hdr.coeff;
  _calibExpon = hdr.expon;

  _smoothedRmse = hdr.smoothed_rmse;
  _smoothedFallSecs = hdr.smoothed_fall_secs;
  _smoothedTrecWeight = hdr.smoothed_trec_wt;
  _smoothedSurfaceWeight = hdr.smoothed_surf_wt;
  _smoothedCalibCoeff = hdr.smoothed_coeff;
  _smoothedCalibExpon = hdr.smoothed_expon;

  _dbzMeasured = hdr.dbz_measured;
  _eMeasured = hdr.e_measured;

  _eTrendSlope = hdr.e_trend_slope;

  _visSensorLat = hdr.vis_sensor_lat_deg + hdr.vis_sensor_lat_frac_deg;
  _visSensorLon = hdr.vis_sensor_lon_deg + hdr.vis_sensor_lon_frac_deg;

  _visSensorName = hdr.vis_sensor_name;

}

