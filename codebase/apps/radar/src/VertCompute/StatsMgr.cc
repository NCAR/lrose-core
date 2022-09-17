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
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2006
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

  _startTimeGlobal = 0;
  _endTimeGlobal = 0;
  _startTimeStats = 0;
  _endTimeStats = 0;
  _prevTime = 0;
  _prt = 0;
  _el = 0;
  _az = 0;
  _prevAz = -999;
  _azMoved = 0;
  _nRotations = 0;

  _sumEl = 0.0;
  _nEl = 0.0;
  _meanEl = 0.0;
  _globalSumEl = 0.0;
  _globalNEl = 0.0;
  _globalMeanEl = -9999;

  _globalCountZdrm = 0;
  _globalSumZdrm = 0;
  _globalSumSqZdrm = 0;
  _globalMeanZdrm = -9999;
  _globalSdevZdrm = -9999;
  _globalMeanOfSdevZdrm = -9999;

  // set up layers
  
  _nLayers = _params.n_layers;
  _startHt = _params.start_height;
  _deltaHt = _params.delta_height;

  for (int ii = 0; ii < _nLayers; ii++) {
    double minHt = _startHt + (ii - 0.5) * _deltaHt;
    double maxHt = minHt + _deltaHt;
    LayerStats *layer = new LayerStats(_params, minHt, maxHt);
    _layers.push_back(layer);
    _maxHt = maxHt;
  }

}

// destructor

StatsMgr::~StatsMgr()

{

  for (int ii = 0; ii < (int) _layers.size(); ii++) {
    delete _layers[ii];
  }

}

///////////////////////////////
// set the start and end time

void StatsMgr::setStartTime(double start_time)
{
  _startTimeStats = start_time;
  if (_startTimeGlobal == 0.0) {
    _startTimeGlobal = start_time;
  }
}

void StatsMgr::setEndTime(double latest_time)
{
  _endTimeStats = latest_time;
  _endTimeGlobal = latest_time;
  if (_prevTime != 0) {
    double timeGap = _endTimeStats - _prevTime;
    if (timeGap > _params.max_time_gap_for_stats) {
      clearStats();
    }
  }
}

////////////////////
// set the elevation

void StatsMgr::setEl(double el) {

  _el = el;

  _sumEl += _el;
  _nEl++;

  _globalSumEl += _el;
  _globalNEl++;

}
 
////////////////////
// set the azimuth

void StatsMgr::setAz(double az) {

  _az = az;

  if (_prevAz < -900) {
    _prevAz = _az;
  } else {
    double azDiff = RadarComplex::diffDeg(_prevAz, _az);
    _azMoved += fabs(azDiff);
    _prevAz = _az;
  }
  
}
 
/////////////////////////////////
// check and compute when ready

void StatsMgr::checkCompute() {

  if (_azMoved > _params.cumulative_azimuth_moved_for_stats) {

    computeStats();
    if (_params.write_stats_to_text_file) {
      writeStats();
    }
    if (_params.write_stats_to_spdb) {
      writeStatsToSpdb();
    }
    clearStats();
    _azMoved = 0;
    _nRotations++;
    _startTimeStats = _endTimeStats;

  }
  
}
 
/////////////////////////////////
// add data to layer

void StatsMgr::addDataPoint(double range,
			    MomentData mdata)

{
  
  double sinEl = sin(_el * DEG_TO_RAD);
  double ht = (range * sinEl);
  int layer = (int) ((ht - _startHt) / _deltaHt);
  mdata.height = ht;
  if (layer >= 0 && layer < _nLayers) {
    _layers[layer]->addData(mdata);
  }
}
 
////////////////////////////////////////////
// clear stats info

void StatsMgr::clearStats()

{
  for (int ii = 0; ii < (int) _layers.size(); ii++) {
    _layers[ii]->clearData();
  }
  _sumEl = 0.0;
  _nEl = 0.0;
  _startTimeStats = _endTimeStats;
  _prevTime = _endTimeStats;
}
  
//////////////////////////////////////
// compute stats for az moved so far

void StatsMgr::computeStats()
  
{
  
  for (int ii = 0; ii < (int) _layers.size(); ii++) {
    _layers[ii]->computeStats();
  }

  // compute Zdr for this rotation
  // and accumulate stats for computing mean Zdr

  double sumValid = 0.0;
  double sumZdrm = 0.0;
  double sum2Zdrm = 0.0;

  for (int ii = 0; ii < (int) _layers.size(); ii++) {
    LayerStats &layer = *(_layers[ii]);
    double ht = layer.getMeanHt();
    double snr = layer.getMean().snr;
    if (ht >= _params.min_ht_for_stats &&
	ht <= _params.max_ht_for_stats &&
        snr >= _params.min_snr) {
      sumValid += layer.getNValid();
      sumZdrm += layer.getSum().zdrm;
      sum2Zdrm += layer.getSum2().zdrm;
    }
  }

  _meanEl = _sumEl / _nEl;

  _countZdrm = sumValid;
  _meanZdrm = -9999;
  _sdevZdrm = -9999;

  if (_countZdrm > 0) {
    _meanZdrm = sumZdrm / _countZdrm;
    _globalCountZdrm++;
    _globalSumZdrm += _meanZdrm;
    _globalSumSqZdrm += _meanZdrm * _meanZdrm;
  }
  if (_countZdrm > 2) {
    _meanZdrm = sumZdrm / _countZdrm;
    double variance =
      (sum2Zdrm - (sumZdrm * sumZdrm) / _countZdrm) / (_countZdrm - 1.0);
    _sdevZdrm = 0.000001;
    if (variance >= 0.0) {
      _sdevZdrm = sqrt(variance);
    }
  }

}

/////////////////////////
// compute global stats

void StatsMgr::computeGlobalStats()
  
{

  _globalMeanEl = _globalSumEl / _globalNEl;

  for (int ii = 0; ii < (int) _layers.size(); ii++) {
    _layers[ii]->computeGlobalStats();
  }

  // compute global Zdrm stats

  if (_globalCountZdrm > 0) {
    _globalMeanZdrm = _globalSumZdrm / _globalCountZdrm;
  }
  if (_globalCountZdrm > 2) {
    _globalMeanZdrm = _globalSumZdrm / _globalCountZdrm;
    double variance =
      (_globalSumSqZdrm - (_globalSumZdrm * _globalSumZdrm) / 
       _globalCountZdrm) / (_globalCountZdrm - 1.0);
    _globalSdevZdrm = 0.000001;
    if (variance >= 0.0) {
      _globalSdevZdrm = sqrt(variance);
    }
    _globalMeanOfSdevZdrm = _globalSdevZdrm / sqrt(_globalCountZdrm);
  }

}

//////////////////////////////////////
// write out 360 deg stats to files

int StatsMgr::writeStats()

{

  // print to stdout

  printStats(stdout);

  // create the directory for the output files, if needed

  if (ta_makedir_recurse(_params.text_output_dir)) {
    int errNum = errno;
    cerr << "ERROR - StatsMgr::_writeStats";
    cerr << "  Cannot create output dir: " << _params.text_output_dir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // compute output file path

  time_t fileTime = (time_t) _startTimeStats;
  DateTime ftime(fileTime);
  char outPath[1024];
  sprintf(outPath, "%s/vert_zdr_cal_%.4d%.2d%.2d_%.2d%.2d%.2d.txt",
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

  // check we have some valid stats to print

  bool statsFound = false;
  for (int ii = 0; ii < (int) _layers.size(); ii++) {
    const LayerStats &layer = *(_layers[ii]);
    if (layer.getMean().snr > -9990) {
      statsFound = true;
      break;
    }
  }
  if (!statsFound) {
    return;
  }
  
  time_t startTime = (time_t) _startTimeStats;
  
  fprintf(out,
          " ===================================="
          "============================================\n");
  fprintf(out, " Vertical-pointing ZDR calibration\n");
  fprintf(out, "   Time: %s\n", DateTime::strm(startTime).c_str());
  fprintf(out, "   az moved (deg)        : %8g\n", _azMoved);
  fprintf(out, "   n samples             : %8d\n", _params.n_samples);
  fprintf(out, "   n valid               : %8d\n", (int) (_countZdrm + 0.5));
  fprintf(out, "   min snr (dB)          : %8.3f\n", _params.min_snr);
  fprintf(out, "   max snr (dB)          : %8.3f\n", _params.max_snr);
  fprintf(out, "   min vel (m/s)         : %8.3f\n", _params.min_vel);
  fprintf(out, "   max vel (m/s)         : %8.3f\n", _params.max_vel);
  fprintf(out, "   min rhohv             : %8.3f\n", _params.min_rhohv);
  fprintf(out, "   max ldr               : %8.3f\n", _params.max_ldr);
  fprintf(out, "   zdr_n_sdev            : %8.3f\n", _params.zdr_n_sdev);
  fprintf(out, "   min ht for stats (km) : %8.3f\n", _params.min_ht_for_stats);
  fprintf(out, "   max ht for stats (km) : %8.3f\n", _params.max_ht_for_stats);
  fprintf(out, "   mean elevation (deg)  : %8.3f\n", _meanEl);
  fprintf(out, "   mean ZDRm (dB)        : %8.3f\n", _meanZdrm);
  fprintf(out, "   sdev ZDRm (dB)        : %8.3f\n", _sdevZdrm);
  fprintf(out, "   ZDR correction (dB)   : %8.3f\n", _meanZdrm * -1.0);
  fprintf(out,
          " ===================================="
          "============================================\n");
  fprintf(out, " %5s %7s %7s %7s %5s %8s %6s %7s %7s %6s %6s\n",
          "Ht", "npts", "snr", "dBZ", "vel",
          "ldr", "rhohv", "zdrMean", "zdrSdev", "gof", "rmse");
  for (int ii = 0; ii < (int) _layers.size(); ii++) {
    const LayerStats &layer = *(_layers[ii]);
    if (layer.getMean().snr > -9990) {
      fprintf(out,
              " %5.2f %7d %7.3f %7.3f %5.1f %8.3f %6.3f %7.3f %7.3f %6.3f %6.3f\n",
              layer.getMeanHt(),
              layer.getNValid(),
              layer.getMean().snr,
              layer.getMean().dbz,
              layer.getMean().vel,
              layer.getMean().ldrh,
              layer.getMean().rhohv,
              layer.getMean().zdrm,
              layer.getSdev().zdrm,
              layer.getDist().getGof(),
              layer.getDist().getRmsePdf());
    }
  }
  fprintf(out,
          " ===================================="
          "============================================\n");
  
}

///////////////////////////////
// write stats to SPDB

int StatsMgr::writeStatsToSpdb()

{

  // check we have some valid stats to print

  bool statsFound = false;
  for (int ii = 0; ii < (int) _layers.size(); ii++) {
    const LayerStats &layer = *(_layers[ii]);
    if (layer.getMean().snr > -9990) {
      statsFound = true;
      break;
    }
  }
  if (!statsFound) {
    return 0;
  }

  // create XML string

  string xml;

  xml += TaXml::writeStartTag("VertPointingStats", 0);

  xml += TaXml::writeDouble("meanElevation", 1, _meanEl);
  xml += TaXml::writeDouble("meanZdrm", 1, _meanZdrm);
  xml += TaXml::writeDouble("sdevZdrm", 1, _sdevZdrm);
  xml += TaXml::writeDouble("countZdrm", 1, _countZdrm);

  for (int ii = 0; ii < (int) _layers.size(); ii++) {
    const LayerStats &layer = *(_layers[ii]);
    // if (layer.getMean().snr > -9990) {

      xml += TaXml::writeStartTag("LayerStats", 1);
      xml += TaXml::writeDouble("meanHt", 2, layer.getMeanHt());
      xml += TaXml::writeInt("nValid", 2, layer.getNValid());
      xml += TaXml::writeDouble("meanSnr", 2, layer.getMean().snr);
      xml += TaXml::writeDouble("meanDbz", 2, layer.getMean().dbz);
      xml += TaXml::writeDouble("meanVel", 2, layer.getMean().vel);
      xml += TaXml::writeDouble("meanZdrm", 2, layer.getMean().zdrm);
      xml += TaXml::writeDouble("sdevZdrm", 2, layer.getSdev().zdrm);
      xml += TaXml::writeDouble("meanLdrh", 2, layer.getMean().ldrh);
      xml += TaXml::writeDouble("meanLdrv", 2, layer.getMean().ldrv);
      xml += TaXml::writeDouble("meanRhohv", 2, layer.getMean().rhohv);
      xml += TaXml::writeEndTag("LayerStats", 1);

      // }
  }
  
  xml += TaXml::writeEndTag("VertPointingStats", 0);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Writing XML stats to SPDB:" << endl;
    cerr << xml << endl;
  }

  DsSpdb spdb;
  time_t validTime = (time_t) _startTimeStats;
  si32 dataType = Spdb::hash4CharsToInt32(_params.radar_name_for_spdb);
  spdb.addPutChunk(dataType, validTime, validTime, xml.size() + 1, xml.c_str());
  if (spdb.put(_params.spdb_output_url,
               SPDB_XML_ID, SPDB_XML_LABEL)) {
    cerr << "ERROR - StatsMgr::writeStats360ToSpdb" << endl;
    cerr << spdb.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Wrote stats to spdb, url: " << _params.spdb_output_url << endl;
    cerr << "  Valid time: " << DateTime::strm(validTime) << endl;
  }

  return 0;

}

///////////////////////////////
// write out stats to files

int StatsMgr::writeGlobalStats()

{

  printGlobalStats(stdout);

  // create the directory for the output files, if needed

  if (ta_makedir_recurse(_params.text_output_dir)) {
    int errNum = errno;
    cerr << "ERROR - StatsMgr::writeGlobalStats";
    cerr << "  Cannot create output dir: " << _params.text_output_dir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // compute output file path

  time_t startTime = (time_t) _startTimeGlobal;
  DateTime ftime(startTime);
  char outPath[1024];
  sprintf(outPath, "%s/vert_zdr_global_cal_%.4d%.2d%.2d_%.2d%.2d%.2d.txt",
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
    cerr << "ERROR - StatsMgr::_writeFile";
    cerr << "  Cannot create file: " << outPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // print to file

  printGlobalStats(out);

  if (_params.debug) {
    cerr << "-->> Writing global stats file: " << outPath << endl;
  }

  // close file

  fclose(out);
  return 0;

}

///////////////////////////////
// print global stats

void StatsMgr::printGlobalStats(FILE *out)

{
  
  time_t startTime = (time_t) _startTimeGlobal;
  time_t endTime = (time_t) _endTimeGlobal;

  fprintf(out,
          " ===================================="
          "============================================\n");
  fprintf(out, " Vertical-pointing ZDR calibration - global\n");
  fprintf(out, " Start time: %s\n", DateTime::strm(startTime).c_str());
  fprintf(out, " End time  : %s\n", DateTime::strm(endTime).c_str());
  fprintf(out, "   az moved (deg)        : %8g\n", _azMoved);
  fprintf(out, "   n samples             : %8d\n", _params.n_samples);
  fprintf(out, "   n complete rotations  : %8d\n", _nRotations);
  fprintf(out, "   min snr (dB)          : %8.3f\n", _params.min_snr);
  fprintf(out, "   max snr (dB)          : %8.3f\n", _params.max_snr);
  fprintf(out, "   min vel (m/s)         : %8.3f\n", _params.min_vel);
  fprintf(out, "   max vel (m/s)         : %8.3f\n", _params.max_vel);
  fprintf(out, "   min rhohv             : %8.3f\n", _params.min_rhohv);
  fprintf(out, "   max ldr               : %8.3f\n", _params.max_ldr);
  fprintf(out, "   min ht for stats (km) : %8.3f\n", _params.min_ht_for_stats);
  fprintf(out, "   max ht for stats (km) : %8.3f\n", _params.max_ht_for_stats);
  fprintf(out, "   mean elevation (deg)  : %8.3f\n", _globalMeanEl);
  fprintf(out, "   mean ZDRm (dB)        : %8.3f\n", _globalMeanZdrm);
  fprintf(out, "   sdev ZDRm (dB)        : %8.3f\n", _globalSdevZdrm);
  fprintf(out, "   ZDR correction (dB)   : %8.3f\n", _globalMeanZdrm * -1.0);
  fprintf(out,
          " ===================================="
          "============================================\n");
  fprintf(out, " %5s %7s %7s %7s %5s %8s %6s %7s %7s %6s %6s\n",
          "Ht", "npts", "snr", "dBZ", "vel",
          "ldr", "rhohv", "zdrMean", "zdrSdev", "gof", "rmse");
  for (int ii = 0; ii < (int) _layers.size(); ii++) {
    const LayerStats &layer = *(_layers[ii]);
    if (layer.getMean().snr > -9990) {
      fprintf(out,
              " %5.2f %7d %7.3f %7.3f %5.1f %8.3f %6.3f %7.3f %7.3f %6.3f %6.3f\n",
              layer.getMeanHt(),
              layer.getGlobalNValid(),
              layer.getGlobalMean().snr,
              layer.getGlobalMean().dbz,
              layer.getGlobalMean().vel,
              layer.getGlobalMean().ldrh,
              layer.getGlobalMean().rhohv,
              layer.getGlobalMean().zdrm,
              layer.getGlobalSdev().zdrm,
              layer.getGlobalDist().getGof(),
              layer.getGlobalDist().getRmsePdf());
    }
  } // ii
  fprintf(out,
          " ===================================="
          "============================================\n");

}

////////////////////////////////////////////////////////
// write out zdr and height data for individual points

int StatsMgr::writeZdrPoints()

{
  
  if (ta_makedir_recurse(_params.zdr_points_output_dir)) {
    int errNum = errno;
    cerr << "ERROR - StatsMgr::_writeStats";
    cerr << "  Cannot create output dir: " << _params.zdr_points_output_dir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // compute output file path

  time_t startTime = (time_t) _startTimeGlobal;
  time_t endTime = (time_t) _endTimeGlobal;
  DateTime ftime(startTime);
  char outPath[1024];
  sprintf(outPath, "%s/zdr_points_%.4d%.2d%.2d_%.2d%.2d%.2d.txt",
          _params.zdr_points_output_dir,
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
    cerr << "ERROR - StatsMgr::_writeFile";
    cerr << "  Cannot create file: " << outPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // write header

  fprintf(out, "# height snr dbz vel zdrm ldr rhohv\n");
  fprintf(out, "#============================================\n");
  fprintf(out, "# Table produced by VertCompute\n");
  fprintf(out, "# Start time: %s\n", DateTime::strm(startTime).c_str());
  fprintf(out, "# End time  : %s\n", DateTime::strm(endTime).c_str());
  fprintf(out, "#------------ Table column list -------------\n");
  fprintf(out, "#    col 000: height\n");
  fprintf(out, "#    col 001: snr\n");
  fprintf(out, "#    col 002: dbz\n");
  fprintf(out, "#    col 003: vel\n");
  fprintf(out, "#    col 004: zdrm\n");
  fprintf(out, "#    col 005: ldr\n");
  fprintf(out, "#    col 006: rhohv\n");
  fprintf(out, "#--------------------------------------------\n");
  fprintf(out, "#============================================\n");

  // write 

  for (int ii = 0; ii < (int) _layers.size(); ii++) {
    const LayerStats &layer = *(_layers[ii]);
    const vector<MomentData> &momentData = layer.getMomentData();
    for (size_t jj = 0; jj < momentData.size(); jj++) {
      const MomentData &mdata = momentData[jj];
      if (mdata.snr > -9990) {
        fprintf(out,
                "%9.3f %9.3f %9.3f %9.3f %9.3f %9.3f %9.3f\n",
                mdata.height,
                mdata.snr,
                mdata.dbz,
                mdata.vel,
                mdata.zdrm,
                (mdata.ldrh + mdata.ldrv) / 2.0,
                mdata.rhohv);
      }
    } // jj
  } // ii

  if (_params.debug) {
    cerr << "-->> Wrote zdr points file: " << outPath << endl;
  }

  // close file

  fclose(out);
  return 0;

}

