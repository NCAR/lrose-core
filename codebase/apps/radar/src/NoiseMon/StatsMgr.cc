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
// StatsMgr.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2022
//
///////////////////////////////////////////////////////////////
//
// StatsMgr manages the stats, and prints out
//
////////////////////////////////////////////////////////////////

#include "StatsMgr.hh"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cerrno>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctime>
#include <toolsa/DateTime.hh>
#include <toolsa/uusleep.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <toolsa/mem.h>
#include <toolsa/TaXml.hh>
#include <radar/RadarComplex.hh>
#include <radar/BeamHeight.hh>
#include <Spdb/DsSpdb.hh>

using namespace std;

const double StatsMgr::missingVal = -9999.0;

// Constructor

StatsMgr::StatsMgr(const string &prog_name,
                   const Args &args,
		   const Params &params) :
  _progName(prog_name),
  _args(args),
  _params(params)
  
{

  // set up field name map
  
  for (int ii = 0; ii < _params.input_fields_n; ii++) {
    Params::input_field_t &infield = _params._input_fields[ii];
    _fieldNameMap[infield.id] = infield.moments_name;
  }

  // initialize for stats

  _thisStartTime.set(RadxTime::NEVER);
  _nextStartTime.set(RadxTime::NEVER);
  
  clearStats();
  
  _sunTime.set(RadxTime::ZERO);

}

// destructor

StatsMgr::~StatsMgr()

{

}

/////////////////////////////////
// check and compute when ready

void StatsMgr::checkCompute(const RadxTime &mtime)
{

  if (mtime >= _nextStartTime) {
    
    if (computeStats() == 0) {
      
      if (_params.write_stats_to_text_file) {
        writeStats();
      }
      if (_params.write_stats_to_spdb) {
        writeStatsToSpdb();
      }

    }
    
    clearStats();
    // _azMovedStats = 0;
    // _startTimeStats = _endTimeStats;

    _thisStartTime = _nextStartTime;
    _nextStartTime += _params.stats_interval_secs;
    
  }
  
}
 
/////////////////////////////////
// process a ray of data

void StatsMgr::processRay(const RadxPlatform &radar,
                          RadxRay *ray)

{

  // update sun posn every 10 secs

  RadxTime rayTime = ray->getRadxTime();
  if (rayTime - _sunTime > 10) {
    // recompute every 10 secs
    _sunPosn.setLocation(radar.getLatitudeDeg(),
                         radar.getLongitudeDeg(),
                         radar.getAltitudeKm() * 1000.0);
    _sunPosn.computePosnNova(rayTime.asDouble(), _elSun, _azSun);
    _sunTime = rayTime;
  }

  // check ray is not close to sun

  if (_params.avoid_the_sun) {

    double deltaAz = fabs(ray->getAzimuthDeg() - _azSun);
    if (deltaAz > 180.0) {
      deltaAz -= 360.0;
    }
    double deltaEl = fabs(ray->getElevationDeg() - _elSun);

    if (deltaAz < _params.sun_avoidance_angle_margin_deg &&
        deltaEl < _params.sun_avoidance_angle_margin_deg) {
      if (_params.debug >= Params::DEBUG_EXTRA) {
        cerr << "StatsMgr::processRay() - avoiding ray close to sun" << endl;
        cerr << "  sun el, az: " << _elSun << ", " << _azSun << endl;
        cerr << "  ray el, az: " << ray->getElevationDeg() << ", " << ray->getAzimuthDeg() << endl;
      }
      return;
    }
    
  } // if (_params.avoid_the_sun)

  // check ray does not have strong echo

  if (_params.avoid_strong_echo) {

    double strongEchoDbzSum = _computeStrongEchoDbzSum(ray);
    // cerr << "111111111 el, az, dbzSum: "
    //      << ray->getElevationDeg() << ", "
    //      << ray->getAzimuthDeg() << ", "
    //      << strongEchoDbzSum << endl;
    if (strongEchoDbzSum > _params.strong_echo_dbz_sum_max) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "StatsMgr::processRay() - avoiding ray with strong echo" << endl;
        cerr << "  strongEchoDbzSum: " << strongEchoDbzSum << endl;
      }
      return;
    }
    
  } // if (_params.avoid_strong_echo) 

  // check time relative to processing interval
  
  if (!_nextStartTime.isValid()) {
    // first data point
    time_t initTime = ray->getRadxTime().utime();
    _thisStartTime =
      (initTime / _params.stats_interval_secs) * _params.stats_interval_secs;
    _nextStartTime = _thisStartTime + _params.stats_interval_secs;
  }

  // make sure they are floats
  
  ray->convertToFl32();

  // get the fields
  
  RadxField *dbmhcField = ray->getField(_fieldNameMap[Params::DBMHC]);
  Radx::fl32 dbmhcMiss = -9999.0;
  Radx::fl32 *dbmhcData = NULL;
  if (dbmhcField != NULL) {
    dbmhcMiss = dbmhcField->getMissingFl32();
    dbmhcData = dbmhcField->getDataFl32();
  }

  RadxField *dbmvcField = ray->getField(_fieldNameMap[Params::DBMVC]);
  Radx::fl32 dbmvcMiss = -9999.0;
  Radx::fl32 *dbmvcData = NULL;
  if (dbmvcField != NULL) {
    dbmvcMiss = dbmvcField->getMissingFl32();
    dbmvcData = dbmvcField->getDataFl32();
  }

  if (dbmhcField == NULL || dbmvcField == NULL) {
    return;
  }
  
  RadxField *dbmhxField = ray->getField(_fieldNameMap[Params::DBMHX]);
  Radx::fl32 dbmhxMiss = -9999.0;
  Radx::fl32 *dbmhxData = NULL;
  if (dbmhxField != NULL) {
    dbmhxMiss = dbmhxField->getMissingFl32();
    dbmhxData = dbmhxField->getDataFl32();
  }

  RadxField *dbmvxField = ray->getField(_fieldNameMap[Params::DBMVX]);
  Radx::fl32 dbmvxMiss = -9999.0;
  Radx::fl32 *dbmvxData = NULL;
  if (dbmvxField != NULL) {
    dbmvxMiss = dbmvxField->getMissingFl32();
    dbmvxData = dbmvxField->getDataFl32();
  }

  // set up geometry

  double elDeg = ray->getElevationDeg();
  BeamHeight beamHt;
  beamHt.setInstrumentHtKm(radar.getAltitudeKm());

  // loop through the gates
  
  double rangeKm = ray->getStartRangeKm();
  
  size_t nGates = ray->getNGates();
  for (size_t ii = 0; ii < nGates; ii++, rangeKm += ray->getGateSpacingKm()) {
    
    if (rangeKm < _params.min_range_km || rangeKm > _params.max_range_km) {
      continue;
    }
    
    double htKm = beamHt.computeHtKm(elDeg, rangeKm);
    if (htKm < _params.min_height_km) {
      continue;
    }
    
    if (dbmhcData[ii] == dbmhcMiss || dbmvcData[ii] == dbmvcMiss) {
      continue;
    }
    if (dbmhcData[ii] > _params.max_valid_noise_power_dbm ||
        dbmvcData[ii] > _params.max_valid_noise_power_dbm) {
      continue;
    }

    _countCoPol++;
    _sumDbmhc += dbmhcData[ii];
    _sumDbmvc += dbmvcData[ii];
    _sumHtKm += htKm;
    
    if (dbmhxData == NULL || dbmvxData == NULL ||
        dbmhxData[ii] == dbmhxMiss || dbmvxData[ii] == dbmvxMiss) {
      continue;
    }

    _countCrossPol++;
    _sumDbmhx += dbmhxData[ii];
    _sumDbmvx += dbmvxData[ii];
    
  } // ii

  // check if we should compute the stats at this stage

  checkCompute(ray->getRadxTime());
  
}
 
/////////////////////////////////
// compute sum of strong echo dbz

double StatsMgr::_computeStrongEchoDbzSum(RadxRay *ray)
{

  // get the DBZ field
  
  RadxField *dbzField = ray->getField(_fieldNameMap[Params::DBZ]);
  if (dbzField == NULL) {
    cerr << "ERROR - StatsMgr::_computeStrongEchoDbzSum" << endl;
    cerr << "  Cannot find DBZ field, name: " << _fieldNameMap[Params::DBZ] << endl;
    cerr << "  Cannot check for strong echo" << endl;
    return 0.0;
  }

  Radx::fl32 dbzMiss = dbzField->getMissingFl32();
  Radx::fl32 *dbzData = dbzField->getDataFl32();

  // loop through the gates
  
  double dbzSum = 0.0;
  size_t nGates = ray->getNGates();
  for (size_t ii = 0; ii < nGates; ii++) {
    if (dbzData[ii] == dbzMiss) {
      continue;
    }
    double dbz = dbzData[ii];
    if (dbz > _params.strong_echo_dbz_threshold) {
      dbzSum += dbz;
    }
  }

  return dbzSum;

}

////////////////////////////////////////////
// clear stats info

void StatsMgr::clearStats()

{

  _countCoPol = 0.0;
  _countCrossPol = 0.0;
  _sumDbmhc = 0.0;
  _sumDbmvc = 0.0;
  _sumDbmhx = 0.0;
  _sumDbmvx = 0.0;
  _sumHtKm = 0.0;

  _meanDbmhc = -9999.0;
  _meanDbmvc = -9999.0;
  _meanDbmhx = -9999.0;
  _meanDbmvx = -9999.0;
  _meanNoiseZdr = -9999.0;
  _meanHtKm = -9999.0;

}
  
//////////////////////////////////////
// compute stats for az moved so far

int StatsMgr::computeStats()
  
{

  if (_countCoPol < _params.min_valid_count) {
    return -1;
  }

  _meanHtKm = _sumHtKm / _countCoPol;

  _meanDbmhc = _sumDbmhc / _countCoPol;
  _meanDbmvc = _sumDbmvc / _countCoPol;
  _meanNoiseZdr = _meanDbmhc - _meanDbmvc;
  
  if (_countCrossPol < _params.min_valid_count) {
    return 0;
  }
  _meanDbmhx = _sumDbmhx / _countCrossPol;
  _meanDbmvx = _sumDbmvx / _countCrossPol;
  
  return 0;

}

//////////////////////////////////////
// write out stats to files

int StatsMgr::writeStats()

{

  // print to stdout

  printStats(stdout);

  // create the directory for the output files, if needed

  if (ta_makedir_recurse(_params.text_output_dir)) {
    int errNum = errno;
    cerr << "ERROR - NoiseMon::StatsMgr::_writeStats";
    cerr << "  Cannot create output dir: " << _params.text_output_dir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // compute output file path

  time_t fileTime = _thisStartTime.utime();
  DateTime ftime(fileTime);
  char outPath[1024];
  sprintf(outPath, "%s/NoiseMon_%.4d%.2d%.2d_%.2d%.2d%.2d.txt",
          _params.text_output_dir,
          ftime.getYear(),
          ftime.getMonth(),
          ftime.getDay(),
          ftime.getHour(),
          ftime.getMin(),
          ftime.getSec());

  // open file
  
  FILE *out;
  if ((out = fopen(outPath, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - StatsMgr::writeStats";
    cerr << "  Cannot create file: " << outPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  printStats(out);
  
  if (_params.debug) {
    cerr << "-->> Writing stats to file: " << outPath << endl;
  }

  // close file

  fclose(out);
  return 0;

}

///////////////////////////////
// print out stats as text

void StatsMgr::printStats(FILE *out)

{

  fprintf(out,
          " ====================================="
          "============================================\n");
  fprintf(out, " Noise Monitoring\n");
  fprintf(out, "   Start time: %s\n", _thisStartTime.asString().c_str());
  fprintf(out, "   End   time: %s\n", _nextStartTime.asString().c_str());
  fprintf(out, "   count copol           : %8.0f\n", _countCoPol);
  fprintf(out, "   mean dbmhc (dBm)      : %8.3f\n", _meanDbmhc);
  fprintf(out, "   mean dbmvc (dBm)      : %8.3f\n", _meanDbmvc);
  if (_meanDbmhx > -9990) {
    fprintf(out, "   count crosspol        : %8.0f\n", _countCrossPol);
    fprintf(out, "   mean dbmhx (dBm)      : %8.3f\n", _meanDbmhx);
    fprintf(out, "   mean dbmvx (dBm)      : %8.3f\n", _meanDbmvx);
  }
  fprintf(out, "   meanNoiseZdr (dB)     : %8.3f\n", _meanNoiseZdr);
  fprintf(out, "   meanHeight   (km)     : %8.3f\n", _meanHtKm);
  fprintf(out,
          " ====================================="
          "============================================\n");

}

///////////////////////////////
// write stats to SPDB

int StatsMgr::writeStatsToSpdb()

{

  // create XML string

  string xml;

  xml += TaXml::writeStartTag("NoiseMonitoring", 0);

  xml += TaXml::writeTime("intervalStartTime", 1, _thisStartTime.utime());
  xml += TaXml::writeTime("intervalEndTime", 1, _nextStartTime.utime());

  xml += TaXml::writeDouble("countCoPol", 1, _countCoPol);
  xml += TaXml::writeDouble("meanHtKm", 1, _meanHtKm);
  xml += TaXml::writeDouble("meanNoiseZdr", 1, _meanNoiseZdr);
  xml += TaXml::writeDouble("meanDbmhc", 1, _meanDbmhc);
  xml += TaXml::writeDouble("meanDbmvc", 1, _meanDbmvc);
  if (_meanDbmhx > -9990) {
    xml += TaXml::writeDouble("countCrossPol", 1, _countCrossPol);
    xml += TaXml::writeDouble("meanDbmhx", 1, _meanDbmhx);
    xml += TaXml::writeDouble("meanDbmvx", 1, _meanDbmvx);
  }
  
  xml += TaXml::writeEndTag("NoiseMonitoring", 0);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Writing XML stats to SPDB:" << endl;
    cerr << xml << endl;
  }

  DsSpdb spdb;
  time_t validTime = _thisStartTime.utime() + _params.stats_interval_secs / 2;
  time_t expireTime = _nextStartTime.utime() + _params.stats_interval_secs / 2;
  si32 dataType = Spdb::hash4CharsToInt32(_params.radar_name);
  spdb.addPutChunk(dataType, validTime, expireTime, xml.size() + 1, xml.c_str());
  if (spdb.put(_params.spdb_output_url,
               SPDB_XML_ID, SPDB_XML_LABEL)) {
    cerr << "ERROR - StatsMgr::writeStatsToSpdb" << endl;
    cerr << spdb.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Wrote stats to spdb, url: " << _params.spdb_output_url << endl;
    cerr << "  Valid time: " << DateTime::strm(validTime) << endl;
  }

  return 0;

}



