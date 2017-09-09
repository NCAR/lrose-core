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
// SpdbQuery.cc
//
// SpdbQuery object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
///////////////////////////////////////////////////////////////
//
// SpdbQuery queries an SPDB data base, and prints to stdout.
//
///////////////////////////////////////////////////////////////

#include "SpdbQuery.hh"
#include "Args.hh"
#include "Print.hh"
#include "XmlPrint.hh"
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/file_io.h>
#include <toolsa/uusleep.h>
#include <cerrno>
using namespace std;

// Constructor

SpdbQuery::SpdbQuery(int argc, char **argv)

{

  // initialize

  OK = true;
  
  // set programe name
  
  _progName = "SpdbQuery";

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Problem with command line args" << endl;
    OK = false;
    return;
  }

  return;

}

// destructor

SpdbQuery::~SpdbQuery()

{

}

//////////////////////////////////////////////////
// Run

int SpdbQuery::Run()
{

  for (size_t ii = 0; ii < _args.dataTypes.size(); ii++) {

    DsSpdb *spdb;

    if (_args.threaded) {
      if (_args.debug) {
	cerr << "** THREADING ON **" << endl;
      }
      spdb = new DsSpdbThreaded;
    } else {
      spdb = new DsSpdb;
    }

    if (_args.verbose) {
      spdb->setDebug(true);
    }

    if (_args.compressDataBufOnGet) {
      spdb->setDataCompressForTransfer(DsSpdb::COMPRESSION_GZIP);
    }
    
    if (_doDataType(_args.dataTypes[ii], spdb)) {
      return -1;
      delete spdb;
    }
    
    delete spdb;

  }

  return 0;

}

//////////////////////////////////////////////////
// process a given data type

int SpdbQuery::_doDataType(int dataType,
			   DsSpdb *spdb)

{

  if (_args.horizLimitsSet) {
    spdb->setHorizLimits(_args.minLat, _args.minLon,
			_args.maxLat, _args.maxLon);
  }
  if (_args.vertLimitsSet) {
    spdb->setVertLimits(_args.minHt, _args.maxHt);
  }
  if (_args.auxXmlPath.size() > 0) {
    _setAuxXml(spdb);
  }

  // get times is a special case

  if (_args.mode == Args::timesMode) {

    time_t firstTime;
    time_t lastTime;
    time_t lastValidTime;
    
    if (spdb->getTimes(_args.urlStr, firstTime, lastTime, lastValidTime)) {
      cerr << "ERROR - " << _progName << ":Run" << endl;
      cerr << "  Calling getTimes for url: " << _args.urlStr << endl;
      cerr << spdb->getErrStr() << endl;
      return (-1);
    }
    
    cout << "==================================================" << endl;
    cout << "url: " << _args.urlStr << endl;
    cout << "Prod label: " << spdb->getProdLabel() << endl;
    cout << "Prod id:    " << spdb->getProdId() << endl;
    cout << "DataType: " << dataType << endl;
    
    cout << "  First time:      " << DateTime::str(firstTime, false) << endl;
    cout << "  Last time:       " << DateTime::str(lastTime, false) << endl;
    cout << "  Last valid time: "
	 << DateTime::str(lastValidTime, false) << endl;

    return (0);

  }

  if (_args.unique == Args::uniqueLatest) {
    spdb->setUniqueLatest();
  } else if (_args.unique == Args::uniqueEarliest) {
    spdb->setUniqueEarliest();
  }

  if (_args.checkWriteTimeOnGet) {
    spdb->setCheckWriteTimeOnGet(_args.latestWriteTime);
  }

  switch (_args.mode) {

  case Args::exactMode:
    if (spdb->getExact(_args.urlStr, _args.requestTime,
		      dataType, _args.dataType2,
		      _args.refsOnly, _args.respectZeroTypes)) {
      cerr << "ERROR - " << _progName << ":Run" << endl;
      cerr << "  Calling getExact for url: " << _args.urlStr << endl;
      cerr << spdb->getErrStr() << endl;
      return (-1);
    }
    break;
    
  case Args::closestMode:
    if (spdb->getClosest(_args.urlStr,
			_args.requestTime, _args.timeMargin,
			dataType, _args.dataType2,
			_args.refsOnly, _args.respectZeroTypes)) {
      cerr << "ERROR - " << _progName << ":Run" << endl;
      cerr << "  Calling getClosest for url: " << _args.urlStr << endl;
      cerr << spdb->getErrStr() << endl;
      return (-1);
    }
    break;
    
  case Args::intervalMode:
    if (spdb->getInterval(_args.urlStr,
			 _args.startTime, _args.endTime,
			 dataType, _args.dataType2,
			 _args.refsOnly, _args.respectZeroTypes)) {
      cerr << "ERROR - " << _progName << ":Run" << endl;
      cerr << "  Calling getInterval for url: " << _args.urlStr << endl;
      cerr << spdb->getErrStr() << endl;
      return (-1);
    }
    break;
    
  case Args::validMode:
    if (spdb->getValid(_args.urlStr, _args.requestTime,
		      dataType, _args.dataType2,
		      _args.refsOnly, _args.respectZeroTypes)) {
      cerr << "ERROR - " << _progName << ":Run" << endl;
      cerr << "  Calling getValid for url: " << _args.urlStr << endl;
      cerr << spdb->getErrStr() << endl;
      return (-1);
    }
    break;
    
  case Args::latestMode:
    if (spdb->getLatest(_args.urlStr, _args.timeMargin,
		       dataType, _args.dataType2,
		       _args.refsOnly, _args.respectZeroTypes)) {
      cerr << "ERROR - " << _progName << ":Run" << endl;
      cerr << "  Calling getLatest for url: " << _args.urlStr << endl;
      cerr << spdb->getErrStr() << endl;
      return (-1);
    }
    break;
    
  case Args::firstBeforeMode:
    if (spdb->getFirstBefore(_args.urlStr,
			    _args.requestTime, _args.timeMargin,
			    dataType, _args.dataType2,
			    _args.refsOnly, _args.respectZeroTypes)) {
      cerr << "ERROR - " << _progName << ":Run" << endl;
      cerr << "  Calling getFirstBefore for url: " << _args.urlStr << endl;
      cerr << spdb->getErrStr() << endl;
      return (-1);
    }
    break;
    
  case Args::firstAfterMode:
    if (spdb->getFirstAfter(_args.urlStr,
			   _args.requestTime, _args.timeMargin,
			   dataType, _args.dataType2,
			   _args.refsOnly, _args.respectZeroTypes)) {
      cerr << "ERROR - " << _progName << ":Run" << endl;
      cerr << "  Calling getFirstAfter for url: " << _args.urlStr << endl;
      cerr << spdb->getErrStr() << endl;
      return (-1);
    }
    break;
    
  case Args::headerMode:
    if (spdb->printHeader(_args.urlStr,
			 _args.requestTime, cout)) {
      cerr << "ERROR - " << _progName << ":Run" << endl;
      cerr << "  Calling printHeader for url: " << _args.urlStr << endl;
      cerr << "  Request time: "
	   << DateTime::str(_args.requestTime, false) << endl;
      cerr << spdb->getErrStr() << endl;
      return (-1);
    }
    return 0;
    break;
    
  case Args::timeListMode:
    if (spdb->compileTimeList(_args.urlStr,
			     _args.startTime, _args.endTime,
			     _args.timeListMinInterval)) {
      cerr << "ERROR - " << _progName << ":Run" << endl;
      cerr << "  Calling compileTimeList for url: " << _args.urlStr << endl;
      cerr << spdb->getErrStr() << endl;
      return (-1);
    }
    break;
    
  default:
    break;

  }

  if (_args.threaded) {
    DsSpdbThreaded *tspdb = (DsSpdbThreaded *) spdb;
    while (!tspdb->getThreadDone()) {
      if (_args.debug) {
	cerr << " ... waiting for thread to complete, % done: "
	     << tspdb->getPercentComplete() << endl;
	cerr << "     NbytesExpected: " << tspdb->getNbytesExpected() << endl;
	cerr << "     NbytesDone: " << tspdb->getNbytesDone() << endl;
      }
      umsleep(500);
    }
    if (tspdb->getThreadRetVal()) {
      cerr << "ERROR - " << _progName << ":Run" << endl;
      cerr << "  Thread ret val: " << tspdb->getThreadRetVal() << endl;
      cerr << tspdb->getErrStr() << endl;
      return (-1);
    }
  }

  // time list?

  if (_args.mode == Args::timeListMode) {
    _printTimeList(spdb);
    return 0;
  }

  // is XML supported

  if (_args.printAsXml) {
    if (_printAsXml(dataType, spdb) == 0) {
      return 0;
    }
  }

  // default to normal print

  _printNormal(dataType, spdb);
  
  return 0;

}

//////////////////////////////////////////////////////
// time list print

void SpdbQuery::_printTimeList(const DsSpdb *spdb)
  
{
  
  cout << "==================================================" << endl;
  cout << "url: " << _args.urlStr << endl;
  cout << "Prod label: " << spdb->getProdLabel() << endl;
  cout << "Prod id:    " << spdb->getProdId() << endl;
  cout << "Time list:" << endl;
  const vector<time_t> &times = spdb->getTimeList();
  for (size_t ii = 0; ii < times.size(); ii++) {
    cout << "  " << DateTime::str(times[ii]) << endl;
  }
  
}

//////////////////////////////////////////////////////
// normal print

void SpdbQuery::_printNormal(int dataType,
                             const DsSpdb *spdb)
  
{

  Print doPrint(stdout, cout, _args.debug);
  
  if (!_args.noHeader && !_args.timeLabel) {

    cout << "==================================================" << endl;
    cout << "url: " << _args.urlStr << endl;
    cout << "Prod label: " << spdb->getProdLabel() << endl;
    cout << "Prod id:    " << spdb->getProdId() << endl;
    cout << "DataType: " << dataType << endl;
    cout << "N Chunks:   " << spdb->getNChunks() << endl;
    
  }

  // How many items to print?
  
  const vector<Spdb::chunk_t> &chunks = spdb->getChunks();
  int nChunks = (int) chunks.size();
  int startPos = 0;
  if ((_args.doLastN > 0) && (_args.doLastN < nChunks)) {
    startPos = nChunks - _args.doLastN;
  }

  // load up print vector

  vector<Spdb::chunk_t> printChunks;
  for (int ii = startPos; ii < nChunks; ii++) {
    printChunks.push_back(chunks[ii]);
  }
  int nPrint = (int) printChunks.size();

  // Print out the results. Loop through either in forward or reverse order
  
  for (int jj = 0; jj < nPrint; jj++) {

    int ii = jj;
    if (_args.doReverse) {
      ii = nPrint - 1 - jj;
    }
    
    const Spdb::chunk_t &chunk = printChunks[ii];
    
    if (_args.timeLabel) {
      cout << DateTime::str(chunk.valid_time) << endl;
    } else if (!_args.noHeader) {
      fprintf(stdout, "\n");
      fprintf(stdout, "Header for chunk ---> %d <---\n", ii);
      doPrint.chunk_hdr(chunk);
    }
    
    if (_args.blankLine) {
      fprintf(stdout, "\n");
    }

    if (_args.refsOnly) {
      continue;
    }

    int data_len = chunk.len;
    void *chunk_data = chunk.data;

    switch (spdb->getProdId()) {
      
    case SPDB_ASCII_ID :
    case SPDB_XML_ID :
    case SPDB_RAW_METAR_ID :
    case SPDB_WAFS_SIGWX_ID:
    case SPDB_WAFS_SIGWX_CLOUD_ID:
    case SPDB_WAFS_SIGWX_JETSTREAM_ID:
    case SPDB_WAFS_SIGWX_TROPOPAUSE_ID:
    case SPDB_WAFS_SIGWX_TURBULENCE_ID:
    case SPDB_WAFS_SIGWX_VOLCANO_ID:
      doPrint.ascii(data_len, chunk_data);
      break;
      
    case SPDB_GENERIC_POINT_ID :
      doPrint.generic_pt(data_len, chunk_data);
      break;

    case SPDB_AC_VECTOR_ID :
      doPrint.ac_vector(data_len, chunk_data);
      break;

    case SPDB_COMBO_POINT_ID :
      doPrint.combo_pt(data_len, chunk_data);
      break;
      
    case SPDB_STATION_REPORT_ID :
      // doPrint.stn_report(chunk_data);
      doPrint.wx_obs(data_len, chunk_data);
      break;
      
    case SPDB_TREC_PT_FORECAST_ID :
      doPrint.history_forecast(data_len, chunk_data);
      break;
       
    case SPDB_SYMPROD_ID :
      doPrint.symprod(data_len, chunk_data);
      break;
    
    case SPDB_SIGMET_ID :
      doPrint.sigmet(chunk_data);
      break;
    
    case SPDB_SIGAIRMET_ID :
      doPrint.sig_air_met(data_len, chunk_data);
      break;
    
    case SPDB_TAF_ID :
      doPrint.taf(data_len, chunk_data);
      break;
    
    case SPDB_BDRY_ID :
      doPrint.bdry(chunk_data);
      break;
    
    case SPDB_AMDAR_ID :
      doPrint.amdar(data_len, chunk_data);
      break;
    
    case SPDB_ACARS_ID :
      doPrint.acars(data_len, chunk_data);
      break;
      
    case SPDB_AC_POSN_ID :
      if (chunk.data_type2 ==  SPDB_AC_POSN_WMOD_ID) {
	doPrint.ac_posn_wmod(data_len, chunk_data);
      } else {
	doPrint.ac_posn(data_len, chunk_data);
      }
      break;
      
    case SPDB_AC_POSN_WMOD_ID :
      doPrint.ac_posn_wmod(data_len, chunk_data);
      break;
    
    case SPDB_AC_GEOREF_ID :
      doPrint.ac_georef(data_len, chunk_data);
      break;
    
    case SPDB_AC_ROUTE_ID :
      doPrint.ac_route(chunk_data);
      break;

    case SPDB_AC_DATA_ID :
      doPrint.ac_data(chunk_data);
      break;
      
    case SPDB_FLT_PATH_ID :
      doPrint.flt_path(chunk_data);
      break;
    
    case SPDB_LTG_ID :
      doPrint.ltg(data_len, chunk_data);
      break;
      
    case SPDB_TSTORMS_ID :
      doPrint.tstorms(chunk_data);
      break;
    
    case SPDB_TREC_GAUGE_ID :
      doPrint.trec_gauge(data_len, chunk_data);
      break;
    
    case SPDB_ZR_PARAMS_ID :
      doPrint.zr_params(chunk_data);
      break;
    
    case SPDB_ZRPF_ID :
      doPrint.zrpf(data_len, chunk_data);
      break;
    
    case SPDB_ZVPF_ID :
      doPrint.zvpf(data_len, chunk_data);
      break;
    
    case SPDB_ZVIS_CAL_ID :
      doPrint.zvis_cal(data_len, chunk_data);
      break;
    
    case SPDB_ZVIS_FCAST_ID :
      doPrint.zvis_fcast(data_len, chunk_data);
      break;
    
    case SPDB_VERGRID_REGION_ID :
      doPrint.vergrid_region(data_len, chunk_data);
      break;

    case SPDB_SNDG_ID:
      doPrint.sndg(chunk_data);
      break;

    case SPDB_SNDG_PLUS_ID:
      doPrint.sndg_plus(data_len, chunk_data);
      break;
   
    case SPDB_EDR_VER2_POINT_ID:
      doPrint.EDR_point(data_len, chunk_data);
      break;

    case SPDB_EDR_POINT_ID:
      doPrint.edr_point(data_len, chunk_data);
      break;

    case SPDB_PIREP_ID:
      doPrint.pirep(data_len, chunk_data);
      break;

    case SPDB_POSN_RPT_ID:
      doPrint.posn_rpt (chunk_data);
      break;

    case SPDB_GENERIC_POLYLINE_ID:
      doPrint.gen_poly (data_len, chunk_data);
      break;

    case SPDB_USGS_ID:
      doPrint.usgs_data (data_len, chunk_data);
      break;

    case SPDB_DS_RADAR_SWEEP_ID:
      doPrint.ds_radar_sweep(data_len, chunk_data);
      break;

    case SPDB_DS_RADAR_POWER_ID:
      doPrint.ds_radar_power(data_len, chunk_data);
      break;
      
    case SPDB_RADAR_SPECTRA_ID:
      doPrint.radar_spectra(data_len, chunk_data);
      break;
      
    case SPDB_NWS_WWA_ID:
      doPrint.nws_wwa(data_len, chunk_data);
      break;

    case SPDB_TAIWAN_AWOS_REPORT_ID:
      doPrint.taiwan_awos(data_len, chunk_data);
      break;

    case SPDB_CHECKTIME_ID:
      doPrint.checktimes(data_len, chunk_data);
      break;

    } // endswitch - ProductId

  } // jj

}
  
//////////////////////////////////////////////////////
// XML format print
//
// returns 0 on success, -1 on failure

int SpdbQuery::_printAsXml(int dataType,
                           const DsSpdb *spdb)
  
{
  // check for XML support
  // return with error if not supported
  
  switch (spdb->getProdId()) {
    case SPDB_ASCII_ID:
    case SPDB_RAW_METAR_ID:
    case SPDB_WAFS_SIGWX_ID:
    case SPDB_WAFS_SIGWX_CLOUD_ID:
    case SPDB_WAFS_SIGWX_JETSTREAM_ID:
    case SPDB_WAFS_SIGWX_TROPOPAUSE_ID:
    case SPDB_WAFS_SIGWX_TURBULENCE_ID:
    case SPDB_WAFS_SIGWX_VOLCANO_ID:
    case SPDB_GENERIC_POINT_ID:
    case SPDB_AC_VECTOR_ID:
    case SPDB_TREC_PT_FORECAST_ID:
    case SPDB_SYMPROD_ID:
    case SPDB_SIGMET_ID:
    case SPDB_BDRY_ID:
    case SPDB_ACARS_ID:
    case SPDB_AC_POSN_ID:
    case SPDB_AC_POSN_WMOD_ID:
    case SPDB_AC_ROUTE_ID:
    case SPDB_AC_DATA_ID:
    case SPDB_FLT_PATH_ID:
    case SPDB_TSTORMS_ID:
    case SPDB_TREC_GAUGE_ID:
    case SPDB_ZR_PARAMS_ID:
    case SPDB_ZRPF_ID:
    case SPDB_ZVPF_ID:
    case SPDB_ZVIS_CAL_ID:
    case SPDB_ZVIS_FCAST_ID:
    case SPDB_VERGRID_REGION_ID:
    case SPDB_SNDG_ID:
    case SPDB_SNDG_PLUS_ID:
    case SPDB_EDR_VER2_POINT_ID:
    case SPDB_EDR_POINT_ID:
    case SPDB_PIREP_ID:
    case SPDB_POSN_RPT_ID:
    case SPDB_GENERIC_POLYLINE_ID:
    case SPDB_USGS_ID:
    case SPDB_DS_RADAR_SWEEP_ID:
    case SPDB_DS_RADAR_POWER_ID:
    case SPDB_RADAR_SPECTRA_ID:
    case SPDB_NWS_WWA_ID:
    case SPDB_TAIWAN_AWOS_REPORT_ID:
    case SPDB_CHECKTIME_ID:
      return -1;
    default: {}
  } // endswitch - ProductId


  // return if there aren't chunks to print or print header request
  if ((spdb->getNChunks()== 0) && _args.noHeader) {
    return 0;
  }

  // preamble

  cout << TaXml::writeStartTag("SpdbQuery", 0);
  if (!_args.noHeader && !_args.timeLabel) {
    cout << TaXml::writeString("url", 1, _args.urlStr);
    cout << TaXml::writeString("product_label", 1, spdb->getProdLabel());
    cout << TaXml::writeInt("product_id", 1, spdb->getProdId());
    cout << TaXml::writeInt("specified_data_type", 1, dataType);
    cout << TaXml::writeInt("specified_data_type2", 1, _args.dataType2);
    // cout << TaXml::writeInt("n_entries", 1, spdb->getNChunks());
    switch (_args.mode) {
      case Args::exactMode:
        cout << TaXml::writeTime("request_time", 1, _args.requestTime);
        break;
      case Args::closestMode:
        cout << TaXml::writeTime("request_time", 1, _args.requestTime);
        cout << TaXml::writeInt("time_margin", 1, _args.timeMargin);
        break;
      case Args::intervalMode:
        cout << TaXml::writeTime("start_time", 1, _args.startTime);
        cout << TaXml::writeTime("end_time", 1, _args.endTime);
        break;
      case Args::validMode:
        cout << TaXml::writeTime("request_time", 1, _args.requestTime);
        break;
      case Args::latestMode:
        break;
      case Args::firstBeforeMode:
        cout << TaXml::writeTime("request_time", 1, _args.requestTime);
        cout << TaXml::writeInt("time_margin", 1, _args.timeMargin);
        break;
      case Args::firstAfterMode:
        cout << TaXml::writeTime("request_time", 1, _args.requestTime);
        cout << TaXml::writeInt("time_margin", 1, _args.timeMargin);
        break;
      default: {}
    }
    if (_args.horizLimitsSet) {
      cout << TaXml::writeBoolean("horiz_limits_set", 1, _args.horizLimitsSet);
      cout << TaXml::writeDouble("min_lat", 1, _args.minLat);
      cout << TaXml::writeDouble("min_lon", 1, _args.minLon);
      cout << TaXml::writeDouble("max_lat", 1, _args.maxLat);
      cout << TaXml::writeDouble("max_lon", 1, _args.maxLon);
    }
    if (_args.vertLimitsSet) {
      cout << TaXml::writeBoolean("vert_limits_set", 1, _args.vertLimitsSet);
      cout << TaXml::writeDouble("min_ht", 1, _args.minHt);
      cout << TaXml::writeDouble("max_ht", 1, _args.maxHt);
    }
  }

  // How many items to print?
  
  const vector<Spdb::chunk_t> &chunks = spdb->getChunks();
  int nChunks = (int) chunks.size();
  int startPos = 0;
  if ((_args.doLastN > 0) && (_args.doLastN < nChunks)) {
    startPos = nChunks - _args.doLastN;
  }

  // load up print vector

  vector<Spdb::chunk_t> printChunks;
  for (int ii = startPos; ii < nChunks; ii++) {
    printChunks.push_back(chunks[ii]);
  }
  int nPrint = (int) printChunks.size();

  // Print out the results. Loop through either in forward or reverse order
  
  XmlPrint doPrint(stdout, cout, _args);
  
  for (int jj = 0; jj < nPrint; jj++) {
    
    int ii = jj;
    if (_args.doReverse) {
      ii = nPrint - 1 - jj;
    }
    
    const Spdb::chunk_t &chunk = printChunks[ii];
    
    if (_args.refsOnly) {
      cout << TaXml::writeStartTag("entry", 1);
      doPrint.chunkHdr(chunk, 2);
      cout << TaXml::writeEndTag("entry", 1);
      continue;
    }
    
    switch (spdb->getProdId()) {
      
    case SPDB_LTG_ID:
      doPrint.ltg(chunk, 1);
      break;
        
    case SPDB_AC_GEOREF_ID:
      doPrint.acGeoref(chunk, 1);
      break;
        
    case SPDB_TAF_ID :
      doPrint.taf(chunk, 1);
      // doPrint.xml(chunk, 1); // TAFS are stored as XML 
      break;
    
    case SPDB_SIGAIRMET_ID :
      doPrint.sigmet(chunk, 1);
      break;
      
    case SPDB_AMDAR_ID :
      doPrint.amdar(chunk, 1);
      break;
      
    case SPDB_STATION_REPORT_ID :
      doPrint.wxObs(chunk, 1);
      break;
      
    case SPDB_XML_ID:
      doPrint.xml(chunk, 1);
      break;

    default: {}
        
    } // endswitch - ProductId


  } // jj
    
  cout << TaXml::writeEndTag("SpdbQuery", 0);
  
  return 0;
  
}
  
//////////////////////////////////////////////////////
// set the auxiliary XML by reading in XML from a file

int SpdbQuery::_setAuxXml(DsSpdb *spdb)
  
{

  // Stat the file to get length
  
  struct stat fileStat;
  if (ta_stat(_args.auxXmlPath.c_str(), &fileStat)) {
    int errNum = errno;
    cerr << "ERROR - SpdbQuery::_setAuxXml" << endl;
    cerr << "  Cannot stat file: " << _args.auxXmlPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  int fileLen = fileStat.st_size;
  
  // open file
  
  FILE *xmlFile;
  if ((xmlFile = fopen(_args.auxXmlPath.c_str(), "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - SpdbQuery::_setAuxXml" << endl;
    cerr << "  Cannot open file: " << _args.auxXmlPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // create buffer
  
  TaArray<char> bufArray;
  char *xmlBuf = bufArray.alloc(fileLen + 1);
  memset(xmlBuf, 0, fileLen + 1);
  
  // read in buffer, close file
  
  if (ta_fread(xmlBuf, 1, fileLen, xmlFile) != fileLen) {
    int errNum = errno;
    cerr << "ERROR - SpdbQuery::_setAuxXml" << endl;
    cerr << "  Cannot read file: " << _args.auxXmlPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    fclose(xmlFile);
    return -1;
  }
  fclose(xmlFile);
  
  // set XML

  spdb->setAuxXml(xmlBuf);
  return 0;

}
