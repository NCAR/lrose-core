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
  
  clearStats();
  
}

// destructor

StatsMgr::~StatsMgr()

{

}

///////////////////////////////
// set the start and end time

// void StatsMgr::setStartTime(double start_time)
// {
//   _startTimeStats = start_time;
// }

// void StatsMgr::setEndTime(double latest_time)
// {
//   _endTimeStats = latest_time;
//   if (_prevTime != 0) {
//     double timeGap = _endTimeStats - _prevTime;
//     if (timeGap > _params.max_time_gap_secs) {
//       clearStats();
//     }
//   }
// }

////////////////////
// set the elevation

// void StatsMgr::setEl(double el) {

//   _el = el;

//   _sumEl += _el;
//   _nEl++;

// }
 
// ////////////////////
// // set the azimuth

// void StatsMgr::setAz(double az) {

//   _az = az;

//   if (_prevAz < -900) {
//     _prevAz = _az;
//   } else {
//     double azDiff = fabs(RadarComplex::diffDeg(_prevAz, _az));
//     if (azDiff < 10.0) {
//       _azMovedStats += azDiff;
//     }
//     _prevAz = _az;
//   }
  
// }
 
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
// add data to layer

// void StatsMgr::addDataPoint(RadxTime mtime,
//                             double range,
// 			    MomentData mdata)

// {

//   if (!_nextStartTime.isValid()) {
//     // first data point
//     time_t initTime = mtime.utime();
//     _thisStartTime = (initTime / _params.stats_interval_secs) * _params.stats_interval_secs;
//     _nextStartTime = _thisStartTime + _params.stats_interval_secs;
//   }
  
//   double sinEl = sin(_el * DEG_TO_RAD);
//   double ht = (range * sinEl);
//   mdata.height = ht;

// }
 
/////////////////////////////////
// process a ray of data

void StatsMgr::processRay(const RadxPlatform &radar,
                          RadxRay *ray)

{

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

  
  
}
 
////////////////////////////////////////////
// clear stats info

void StatsMgr::clearStats()

{

  // _sumEl = 0.0;
  // _nEl = 0.0;
  // _meanEl = 0.0;

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

  // for (int ii = 0; ii < (int) _layers.size(); ii++) {
  //   _layers[ii]->clearData();
  // }

  // _sumEl = 0.0;
  // _nEl = 0.0;
  // _startTimeStats = _endTimeStats;
  // _prevTime = _endTimeStats;

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
  fprintf(out, "   mean dbmhc (dBm)      : %8.3f\n", _meanDbmhc);
  fprintf(out, "   mean dbmvc (dBm)      : %8.3f\n", _meanDbmvc);
  if (_meanDbmhx > -9990) {
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

  xml += TaXml::writeDouble("meanHtKm", 1, _meanHtKm);
  xml += TaXml::writeDouble("meanNoiseZdr", 1, _meanNoiseZdr);
  xml += TaXml::writeDouble("meanDbmhc", 1, _meanDbmhc);
  xml += TaXml::writeDouble("meanDbmvc", 1, _meanDbmvc);
  if (_meanDbmhx > -9990) {
    xml += TaXml::writeDouble("meanDbmhx", 1, _meanDbmhx);
    xml += TaXml::writeDouble("meanDbmvx", 1, _meanDbmvx);
  }
  
  xml += TaXml::writeEndTag("NoiseMonitoring", 0);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Writing XML stats to SPDB:" << endl;
    cerr << xml << endl;
  }

  DsSpdb spdb;
  time_t validTime = _thisStartTime.utime();
  time_t expireTime = _nextStartTime.utime();
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



