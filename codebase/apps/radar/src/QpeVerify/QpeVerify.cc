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
////////////////////////////////////////////////////////////////////////
// QpeVerify.cc
//
// QpeVerify object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 2015
//
///////////////////////////////////////////////////////////////
//
// QpeVerify reads precip accumulation measurements from SPDB,
// and compares these with radar-derived QPE values stored
// in gridded files.
//
///////////////////////////////////////////////////////////////////////

#include <cerrno>
#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/DateTime.hh>
#include <dsserver/DsLdataInfo.hh>
#include <rapformats/WxObs.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>
#include "QpeVerify.hh"
using namespace std;

// Constructor

QpeVerify::QpeVerify(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "QpeVerify";
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
    return;
  }

  // check mode

  if (_params.mode == Params::ARCHIVE) {
    if (_args.startTime == 0) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  Mode is ARCHIVE but start time is not set." << endl;
      cerr << "  You must set the start time in archive mode." << endl;
      isOK = FALSE;
      return;
    }
    if (_args.endTime == 0) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  Mode is ARCHIVE but end time is not set." << endl;
      cerr << "  You must set the end time in archive mode." << endl;
      isOK = FALSE;
      return;
    }
  }

  // init process mapper registration

  if (_params.mode == Params::REALTIME) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }

  return;

}

/////////////////////////////////////////////////
// destructor

QpeVerify::~QpeVerify()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run
//
// Returns 0 on success, -1 on failure

int QpeVerify::Run()
{

  if (_params.mode == Params::REALTIME) {
    return _runRealtime();
  } else {
    return _runArchive();
  }

}

//////////////////////////////////////////////////
// Run in realtime mode
//
// Returns 0 on success, -1 on failure

int QpeVerify::_runRealtime()
{

  DsLdataInfo ldata(_params.measured_precip_spdb_url);

  while (true) {

    // register with procmap
    
    PMU_auto_register("_runRealtime");

    // wait for data

    ldata.readBlocking(_params.realtime_max_valid_age_secs, 5, PMU_auto_register);

    // process

    time_t latestTime = ldata.getLatestTime();
    if (_run(latestTime - _params.realtime_lookback_secs, latestTime)) {
      return -1;
    }

  } // while

  return 0;

}

//////////////////////////////////////////////////
// Run in archive mode
//
// Returns 0 on success, -1 on failure

int QpeVerify::_runArchive()
{

  return _run(_args.startTime, _args.endTime);

}

//////////////////////////////////////////////////
// Run for specified interval
//
// Returns 0 on success, -1 on failure

int QpeVerify::_run(time_t startTime, time_t endTime)

{

  // get SPDB data

  DsSpdb spdb;
  int dataType = 0; // all stations
  if (spdb.getInterval(_params.measured_precip_spdb_url,
                       startTime, endTime, dataType)) {
    cerr << "ERROR - " << _progName << ":Run" << endl;
    cerr << "  Calling getInterval for url: " << _params.measured_precip_spdb_url << endl;
    cerr << "  Start time: " << DateTime::strm(startTime) << endl;
    cerr << "  End time: " << DateTime::strm(endTime) << endl;
    cerr << spdb.getErrStr() << endl;
    return -1;
  }
  
  // got chunks
  
  const vector<Spdb::chunk_t> &allChunks = spdb.getChunks();
  if (allChunks.size() < 1) {
    cerr << "ERROR - " << _progName << ":Run" << endl;
    cerr << "  No data found for url: " << _params.measured_precip_spdb_url << endl;
    cerr << "  Start time: " << DateTime::strm(startTime) << endl;
    cerr << "  End time: " << DateTime::strm(endTime) << endl;
    return -1;
  }

  // initialize regression

  _nCorr = 0.0;
  _sumAccum = 0.0;
  _sumAccum2 = 0.0;
  _sumQpe = 0.0;
  _sumQpe2 = 0.0;
  _sumAccumQpe = 0.0;

  // loop though the chunks, processing them in time-based batches
  // i.e all chunks of the same valid time will use the same QPE data

  vector<const Spdb::chunk_t *> validChunks;
  validChunks.push_back(&allChunks[0]);
  time_t validTime = allChunks[0].valid_time;
  
  for (size_t ichunk = 1; ichunk < allChunks.size(); ichunk++) {
    
    const Spdb::chunk_t &chunk = allChunks[ichunk];
    
    if (chunk.valid_time != validTime) {
      
      // time is changing, so process chunks assembled so far
      
      _processTime(validTime, validChunks);

      // clear working arrays
      
      validChunks.clear();

      // add this entry

      validChunks.push_back(&chunk);
      validTime = chunk.valid_time;
      
    } else {
      
      // add to list
      
      validChunks.push_back(&chunk);

    }
    
    if (ichunk == allChunks.size() - 1) {
      
      // last chunk in data
      // add and process
      
      _processTime(validTime, validChunks);
      
    }

  } // ichunk

  // compute regression and bias

  double qpeBias = _sumQpe / _sumAccum;
  double denomSq = ((_nCorr * _sumAccum2 - _sumAccum * _sumAccum) *
                    (_nCorr * _sumQpe2 - _sumQpe * _sumQpe));
  double num = _nCorr * _sumAccumQpe - _sumAccum * _sumQpe;
  double corr = 0.0;
  if (denomSq > 0) {
    corr = num / sqrt(denomSq);
  }

  cerr << "==========>> nPoints: " << _nCorr << endl;
  cerr << "==========>> qpeBias: " << qpeBias << endl;
  cerr << "==========>> correlation: " << corr << endl;

  return 0;

}

/////////////////////////////////////////
// process precip data for a select time
//
// Returns 0 on success, -1 on failure

int QpeVerify::_processTime(time_t validTime,
                            vector<const Spdb::chunk_t *> &validChunks)
  
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Processing valid time: " << DateTime::strm(validTime) << endl;
  }

  // read in QPE data

  DsMdvx inMdvx;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    inMdvx.setDebug(true);
  }
  inMdvx.addReadField(_params.qpe_field_name);
  inMdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  inMdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  inMdvx.setReadTime(Mdvx::READ_FIRST_BEFORE,
                     _params.qpe_url,
                     _params.qpe_read_time_margin_secs,
                     validTime + _params.qpe_read_time_offset_secs);
  inMdvx.addReadField(_params.qpe_field_name);
  if (_params.debug >= Params::DEBUG_EXTRA) {
    inMdvx.printReadRequest(cerr);
  }

  if (inMdvx.readVolume()) {
    cerr << "ERROR - QpeVerify::_processTime" << endl;
    cerr << "  Cannot read QPE data for time: " << DateTime::strm(validTime) << endl;
    cerr << inMdvx.getErrStr() << endl;
    return -1;
  }

  MdvxField *qpeField = inMdvx.getField(_params.qpe_field_name);
  if (qpeField == NULL) {
    cerr << "ERROR - QpeVerify::_processTime" << endl;
    cerr << "  Cannot find QPE field: " << _params.qpe_field_name << endl;
    cerr << "  URL: " << _params.qpe_url << endl;
    return -1;
  }
  MdvxProj gridProj(inMdvx);

  // loop through chunks

  for (size_t ichunk = 0; ichunk < validChunks.size(); ichunk++) {
    
    const Spdb::chunk_t *chunk = validChunks[ichunk];
    WxObs obs;
    obs.disassemble(chunk->data, chunk->len);
    
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "=======================================" << endl;
      cerr << "Precip accum data:\n" << endl;
      obs.print(cerr, "   ");
      cerr << "=======================================" << endl;
    }

    const WxObsField &precipAccum = obs.getPrecipLiquidMmField();
    
    for (int ii = 0; ii < precipAccum.getSize(); ii++) {
      
      double accumPeriodRatio =
        precipAccum.getQualifier(ii) / (double) _params.accum_period_secs;
      
      if (accumPeriodRatio > 0.95 && accumPeriodRatio < 1.05) {

        // good to go

        _compareWithQpe(obs.getStationId(),
                        obs.getObservationTime(),
                        obs.getLatitude(),
                        obs.getLongitude(),
                        precipAccum.getValue(ii),
                        precipAccum.getQualifier(ii),
                        qpeField,
                        gridProj);
        
      }
      
    }

  }

  return 0;

}

////////////////////////////////////
// compare measurement with QPE
//
// Returns 0 on success, -1 on failure

int QpeVerify::_compareWithQpe(string stationId,
                               time_t obsTime,
                               double lat,
                               double lon,
                               double accumDepth,
                               double accumSecs,
                               MdvxField *qpeField,
                               const MdvxProj &gridProj)

{
  
  fl32 *qpeData = (fl32 *) qpeField->getVol();
  Mdvx::field_header_t fhdr = qpeField->getFieldHeader();

  if (accumDepth <= 0 || accumDepth > _params.max_valid_accum_mm) {
    return 0;
  }

  // check bounding box
  
  if (_params.constrain_using_bounding_box) {
    if (_params.bounding_box_min_lon < _params.bounding_box_max_lon) {
      if (lon < _params.bounding_box_min_lon ||
          lon > _params.bounding_box_max_lon) {
        return 0;
      }
    } else {
      if (lon < _params.bounding_box_min_lon &&
          lon > _params.bounding_box_max_lon) {
        return 0;
      }
    }
    if (lat < _params.bounding_box_min_lat ||
        lat > _params.bounding_box_max_lat) {
      return 0;
    }
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "=======>> stationId, time, lat, lon, depth, period: "
         << stationId << ", "
         << DateTime::strm(obsTime) << ", "
         << lat << ", "
         << lon << ", "
         << accumDepth << ", "
         << accumSecs << endl;
  }

  fl32 missing = fhdr.missing_data_value;
  int margin = _params.qpe_grid_margin;
  int xIndex, yIndex;
  if (gridProj.latlon2xyIndex(lat, lon, xIndex, yIndex) == 0) {

    double sum = 0.0;
    double count = 0.0;

    for (int iy = yIndex - margin; iy <= yIndex + margin; iy++) {

      if (iy < 0 || iy >= fhdr.ny) {
        continue;
      }

      for (int ix = xIndex - margin; ix <= xIndex + margin; ix++) {

        if (ix < 0 || ix >= fhdr.nx) {
          continue;
        }

        int arrayIndex = iy * fhdr.nx + ix;
        fl32 qpeVal = qpeData[arrayIndex];
        if (qpeVal != missing) {
          sum += qpeVal;
          count += 1.0;
        }
        
      } // ix

    } // iy
    
    if (count > 0) {

      DateTime dtime(obsTime);
      double qpeMean = sum / count;

      if (qpeMean > 0 && qpeMean < _params.max_valid_qpe_mm) {

        fprintf(stdout,
                "%s %s %10.4f %10.4f %8g %10.2f %10.2f\n",
                dtime.getDateStrPlain().c_str(),
                dtime.getTimeStrPlain().c_str(),
                lat,
                lon,
                accumSecs,
                accumDepth,
                qpeMean);
        
        _nCorr++;
        _sumAccum += accumDepth;
        _sumAccum2 += accumDepth * accumDepth;
        _sumQpe += qpeMean;
        _sumQpe2 += qpeMean * qpeMean;
        _sumAccumQpe += accumDepth * qpeMean;

      }

    }
    
  } // if (gridProj.latlon2xyIndex ...

  return 0;

}
                              
