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
// PidZdrStats.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2018
//
///////////////////////////////////////////////////////////////

#include "PidZdrStats.hh"
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxXml.hh>
#include <Ncxx/Nc3xFile.hh>
#include <Mdv/GenericRadxFile.hh>
#include <didss/LdataInfo.hh>
#include <toolsa/pmu.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/sincos.h>
#include <toolsa/TaArray.hh>
#include <toolsa/file_io.h>
#include <rapmath/RapComplex.hh>
#include <physics/IcaoStdAtmos.hh>
#include <physics/thermo.h>
#include <rapformats/WxObs.hh>
#include <Spdb/SoundingPut.hh>
#include <algorithm>
using namespace std;

// Constructor

PidZdrStats::PidZdrStats(int argc, char **argv)
  
{

  OK = TRUE;
  _outFilesOpen = false;

  // set programe name

  _progName = "PidZdrStats";
  
  // parse command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Problem with command line args." << endl;
    OK = FALSE;
    return;
  }
  
  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list, &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Problem with TDRP parameters." << endl;
    OK = FALSE;
    return;
  }

  // init process mapper registration

  if (_params.mode == Params::REALTIME) {
    PMU_auto_init( _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }

  // check the params

  for (int ii = 0; ii < _params.pid_regions_n; ii++) {
    if (_params._pid_regions[ii].pid < NcarParticleId::CLOUD ||
        _params._pid_regions[ii].pid > NcarParticleId::MISC) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  Problem with TDRP parameters." << endl;
      cerr << "  PID value out of bounds: " << _params._pid_regions[ii].pid << endl;
      cerr << "    for label: " << _params._pid_regions[ii].label << endl;
    }
  }

  // compute lookup table for PID processing

  for (int ii = 0; ii <= NcarParticleId::MISC; ii++) {
    _pidIndex[ii] = -1;
  } 
  for (int ii = 0; ii < _params.pid_regions_n; ii++) {
    _pidIndex[_params._pid_regions[ii].pid] = ii;
    _pidVals.push_back(_params._pid_regions[ii].pid);
  }

  // create polynomial distribution objects

  DistPolynomial poly;
  for (int ii = 0; ii < _params.pid_regions_n; ii++) {
    _dists.push_back(poly);
  }

  // allocate the vector for accumulating gate data for various PIDs
  
  _allocGateDataVec();

}

// destructor

PidZdrStats::~PidZdrStats()

{

  _closeOutputFiles();

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int PidZdrStats::Run()
{

  if (_params.mode == Params::ARCHIVE) {
    return _runArchive();
  } else if (_params.mode == Params::FILELIST) {
    return _runFilelist();
  } else {
    return _runRealtime();
  }
}

//////////////////////////////////////////////////
// Run in filelist mode

int PidZdrStats::_runFilelist()
{

  // loop through the input file list

  int iret = 0;

  for (int ii = 0; ii < (int) _args.inputFileList.size(); ii++) {

    string inputPath = _args.inputFileList[ii];
    if (_processFile(inputPath)) {
      return -1;
    }

  }

  return iret;

}

//////////////////////////////////////////////////
// Run in archive mode

int PidZdrStats::_runArchive()
{

  // get the files to be processed

  RadxTimeList tlist;
  tlist.setDir(_params.input_dir);
  tlist.setModeInterval(_args.startTime, _args.endTime);
  if (tlist.compile()) {
    cerr << "ERROR - PidZdrStats::_runFilelist()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(_args.startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(_args.endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - PidZdrStats::_runFilelist()" << endl;
    cerr << "  No files found, dir: " << _params.input_dir << endl;
    return -1;
  }
  
  // loop through the input file list
  
  int iret = 0;
  for (size_t ii = 0; ii < paths.size(); ii++) {
    if (_processFile(paths[ii])) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// Run in realtime mode

int PidZdrStats::_runRealtime()
{

  // init process mapper registration

  PMU_auto_init(_progName.c_str(), _params.instance,
                PROCMAP_REGISTER_INTERVAL);

  // watch for new data to arrive

  LdataInfo ldata(_params.input_dir,
                  _params.debug >= Params::DEBUG_VERBOSE);
  
  int iret = 0;

  while (true) {
    ldata.readBlocking(_params.max_realtime_data_age_secs,
                       1000, PMU_auto_register);
    
    const string path = ldata.getDataPath();
    if (_processFile(path)) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// Process a file
// Returns 0 on success, -1 on failure

int PidZdrStats::_processFile(const string &filePath)
{

  // check we have not already processed this file
  // in the file aggregation step

  if (_params.debug) {
    cerr << "INFO - PidZdrStats::Run" << endl;
    cerr << "  Input path: " << filePath << endl;
  }
  
  GenericRadxFile inFile;
  _setupRead(inFile);
  
  // read in file
  
  _readVol.clear();
  if (inFile.readFromPath(filePath, _readVol)) {
    cerr << "ERROR - PidZdrStats::_processFile" << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }

  // process this data set
  
  if (_processVol()) {
    cerr << "ERROR - PidZdrStats::_processFile" << endl;
    cerr << "  Cannot process data in file: " << filePath << endl;
    return -1;
  }

  // compute the statistics for the file

  _computeStats();

  // option to get site temperature
  
  if (_params.read_site_temp_from_spdb) {
    if (_retrieveSiteTempFromSpdb(_siteTempC,
                                  _timeForSiteTemp) == 0) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "==>> site tempC: " 
             << _siteTempC << " at " 
             << RadxTime::strm(_timeForSiteTemp) << endl;
      }
    }
  }

  // save out the stats to SPDB

  _writeStatsToSpdb(filePath);

  return 0;

}

//////////////////////////////////////////////////
// set up read

void PidZdrStats::_setupRead(RadxFile &file)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }

  file.addReadField(_params.PID_field_name);
  file.addReadField(_params.ZDR_field_name);
  if (strlen(_params.RHOHV_field_name) > 0) {
    file.addReadField(_params.RHOHV_field_name);
  }
  if (strlen(_params.TEMP_field_name) > 0) {
    file.addReadField(_params.TEMP_field_name);
  }

  if (_params.set_max_range) {
    file.setReadMaxRangeKm(_params.max_range_km);
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.printReadRequest(cerr);
  }

}

///////////////////////////////////////////
// process this data set

int PidZdrStats::_processVol()
  
{

  if (_params.debug) {
    cerr << "Processing volume ..." << endl;
  }

  // open output files as needed

  if (_openOutputFiles()) {
    cerr << "ERROR - PidZdrStats::_processVol()" << endl;
    cerr << "  Cannot open output files" << endl;
    return -1;
  }

  // set up geom

  _nGates = _readVol.getMaxNGates();
  _startRangeKm = _readVol.getStartRangeKm();
  _gateSpacingKm = _readVol.getGateSpacingKm();

  _radarName = _readVol.getInstrumentName();
  _radarLatitude = _readVol.getLatitudeDeg();
  _radarLongitude = _readVol.getLongitudeDeg();
  _radarAltitude = _readVol.getAltitudeKm();

  // loop through the rays

  _clearGateData();

  const vector<RadxRay *> rays = _readVol.getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {

    RadxRay *ray = rays[iray];

    // PID field

    RadxField *pidField = ray->getField(_params.PID_field_name);
    if (pidField == NULL) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING - ray does not have PID field: "
             << _params.PID_field_name << endl;
      }
      continue;
    }
    // convert to ints
    pidField->convertToFl32();
    _pid = pidField->getDataFl32();
    _pidMiss = pidField->getMissingFl32();

    // ZDR field

    RadxField *zdrField = ray->getField(_params.ZDR_field_name);
    if (zdrField == NULL) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING - ray does not have ZDR field: "
             << _params.ZDR_field_name << endl;
      }
      continue;
    }
    // convert to ints
    zdrField->convertToFl32();
    _zdr = zdrField->getDataFl32();
    _zdrMiss = zdrField->getMissingFl32();

    // RHOHV field

    _rhohv = NULL;
    if (strlen(_params.RHOHV_field_name) > 0) {
      RadxField *rhohvField = ray->getField(_params.RHOHV_field_name);
      if (rhohvField == NULL) {
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "WARNING - ray does not have RHOHV field: "
               << _params.RHOHV_field_name << endl;
        }
      } else {
        // convert to ints
        rhohvField->convertToFl32();
        _rhohv = rhohvField->getDataFl32();
        _rhohvMiss = rhohvField->getMissingFl32();
      }
    }

    // TEMP field

    _temp = NULL;
    if (strlen(_params.TEMP_field_name) > 0) {
      RadxField *tempField = ray->getField(_params.TEMP_field_name);
      if (tempField == NULL) {
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "WARNING - ray does not have TEMP field: "
               << _params.TEMP_field_name << endl;
        }
      } else {
        // convert to ints
        tempField->convertToFl32();
        _temp = tempField->getDataFl32();
        _tempMiss = tempField->getMissingFl32();
      }
    }

    // process this ray

    _processRay(ray);

  } // iray
    
  return 0;

}

///////////////////////////////////////////
// process a ray

int PidZdrStats::_processRay(RadxRay *ray)
  
{

  double elev = ray->getElevationDeg();

  double rangeKm = _startRangeKm;
  for (size_t igate = 0; igate < ray->getNGates();
       igate++, rangeKm += _gateSpacingKm) {

    // get PID and region
    int pid = _pid[igate];
    if (pid == _pidMiss) {
      continue;
    }

    int pidIndex = _pidIndex[pid];
    const Params::pid_region_t &region =
      _params._pid_regions[pidIndex];
    
    // check for elevation angle

    if (elev < region.min_elev_deg || elev > region.max_elev_deg) {
      continue;
    }

    // check RHOHV
    
    double rhohv = _rhohvMiss;
    if (_rhohv != NULL) {
      rhohv = _rhohv[igate];
    }

    // check TEMP
    
    double temp = _tempMiss;
    if (_temp != NULL) {
      temp = _temp[igate];
    }
    if (temp != _tempMiss &&
        (temp < region.min_temp_c || temp > region.max_temp_c)) {
      continue;
    }
    
    double zdr = _zdr[igate];
    if (zdr == _zdrMiss) {
      continue;
    }

    // print out

    fprintf(_outFilePtrs[pidIndex], "  %8.2f %8.2f %4d %8.2f %8.2f %8.2f\n",
            elev, rangeKm, pid, temp, rhohv, zdr);

    // append to gate data

    gate_data_t gdat;
    gdat.zdr = zdr;
    gdat.rhohv = rhohv;
    gdat.temp = temp;

    _gateData[pidIndex].push_back(gdat);

  } // igate

  return 0;

}

//////////////////////////////
// open the output files

int PidZdrStats::_openOutputFiles()
  
{

  if (_outFilesOpen) {
    return 0;
  }

  // compute output dir
  
  RadxTime fileTime(_readVol.getStartTimeSecs());

  string outDir(_params.output_dir);
  char dayStr[BUFSIZ];
  sprintf(dayStr, "%s%.4d%.2d%.2d", PATH_DELIM,
          fileTime.getYear(), fileTime.getMonth(), fileTime.getDay());
  outDir += dayStr;

  // make sure output subdir exists
  
  if (ta_makedir_recurse(outDir.c_str())) {
    cerr << "ERROR - PidZdrStats::_openOutputFiles" << endl;
    cerr << "  Cannot create output dir: " << outDir << endl;
    return -1;
  }

  // open files for each pid regiod

  for (int ii = 0; ii < _params.pid_regions_n; ii++) {

    // compute file name
    
    char fileName[BUFSIZ];
    sprintf(fileName,
            "zdr_stats.%.4d%.2d%.2d_%.2d%.2d%.2d.%s.txt",
            fileTime.getYear(), fileTime.getMonth(), fileTime.getDay(),
            fileTime.getHour(), fileTime.getMin(), fileTime.getSec(),
            _params._pid_regions[ii].label);
    
    char outPath[BUFSIZ];
    sprintf(outPath, "%s%s%s",
            outDir.c_str(), PATH_DELIM,  fileName);

    // open file

    FILE *out = fopen(outPath, "w");
    if (out == NULL) {
      cerr << "ERROR - PidZdrStats::_openOutputFiles" << endl;
      cerr << "  Cannot open output file: " << outPath << endl;
      return -1;
    }

    _outFilePtrs.push_back(out);
    _outFilePaths.push_back(outPath);
    
    if (_params.debug) {
      cerr << "Opened output file: " << outPath << endl;
    }

    // write header

    
    fprintf(out, "# %8s %8s %4s %8s %8s %8s\n",
            "elev", "range", "pid", "temp", "rhohv", "zdr");
    
  } // ii

  _outFilesOpen = true;

  return 0;

}
  
//////////////////////////////
// close the output files

void PidZdrStats::_closeOutputFiles()
  
{

  for (size_t ii = 0; ii < _outFilePtrs.size(); ii++) {
    fclose(_outFilePtrs[ii]);
  } // ii

}
//////////////////////////////
// alloc gate data vector

void PidZdrStats::_allocGateDataVec()
{
  _gateData.clear();
  for (int ii = 0; ii < _params.pid_regions_n; ii++) {
    vector<gate_data_t> gvec;
    _gateData.push_back(gvec);
  }
}

//////////////////////////////
// clear gate data

void PidZdrStats::_clearGateData()
{
  for (size_t ii = 0; ii < _gateData.size(); ii++) {
    _gateData[ii].clear();
  }
}

//////////////////////////////
// compute the statistics

void PidZdrStats::_computeStats()
{

  for (int ii = 0; ii < _params.pid_regions_n; ii++) {

    const Params::pid_region_t &region = _params._pid_regions[ii];

    DistPolynomial &poly = _dists[ii];
    poly.clearValues();

    vector<gate_data_t> &gateData = _gateData[ii];
    for (size_t jj = 0; jj < gateData.size(); jj++) {
      poly.addValue(gateData[jj].zdr);
    }

    poly.setHistNBins(_params.zdr_hist_n_bins);
    poly.setHistRange(region.zdr_hist_lower_limit, region.zdr_hist_upper_limit);
    poly.computeHistogram();
    poly.setOrder(_params.zdr_dist_poly_order);
    poly.performFit();

  }

}

//////////////////////////////
// write the stats to spdb

void PidZdrStats::_writeStatsToSpdb(const string &filePath)
{

  DsSpdb spdb;
  time_t validTime = _readVol.getStartTimeSecs();

  // loop through the PID categories

  for (int ii = 0; ii < _params.pid_regions_n; ii++) {

    const Params::pid_region_t &region = _params._pid_regions[ii];
    DistPolynomial &poly = _dists[ii];

    // check we have enough data for good stats

    if ((int) poly.getNValues() < _params.min_npts_for_valid_stats) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING - pid: " << region.label << endl;
        cerr << "  too few points for stats: " << poly.getNValues() << endl;
      }
      continue;
    }

    // get the XML for this PID

    string xml = _getStatsXml(filePath, region.label, region.pid, poly);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "========================= pid: " << region.pid << endl;
      cerr << xml;
      cerr << "=====================================" << endl;
    }

    // add chunk to SPDB

    spdb.addPutChunk(region.pid, validTime, validTime, xml.size() + 1, xml.c_str());

  } // ii

  // write it out
  
  if (spdb.put(_params.spdb_output_url,
               SPDB_XML_ID, SPDB_XML_LABEL)) {
    cerr << "ERROR - PidZdrStats::_writeStatsToSpdb" << endl;
    cerr << spdb.getErrStr() << endl;
    return;
  }
  
  if (_params.debug) {
    cerr << "Wrote ZDR stats to spdb, url: " << _params.spdb_output_url << endl;
    cerr << "  time: " << RadxTime::strm(validTime) << endl;
  }

}

/////////////////////////////////////////////////////////////
// get XML stats for specified PID

string PidZdrStats::_getStatsXml(const string &filePath,
                                 string pidLabel,
                                 int pid,
                                 DistPolynomial &poly)

{

  char text[1024];
  RadxPath path(filePath);
  string xml;

  xml += RadxXml::writeStartTag("ZdrStats", 0);

  xml += RadxXml::writeString("CfRadialFileName", 1, path.getFile());
  xml += RadxXml::writeBoolean("IsRhi", 1, _readVol.checkIsRhi());
  xml += RadxXml::writeString("PidLabel", 1, pidLabel);
  xml += RadxXml::writeInt("PidVal", 1, pid);
  
  xml += RadxXml::writeDouble("Mean", 1, poly.getMean());
  xml += RadxXml::writeDouble("Sdev", 1, poly.getSdev());
  xml += RadxXml::writeDouble("Skewness", 1, poly.getSkewness());
  xml += RadxXml::writeDouble("Kurtosis", 1, poly.getKurtosis());

  // polygon coefficients

  xml += RadxXml::writeDouble("nPoly", 1, poly.getOrder());

  string coeffStr;
  const vector<double> &coeffs = poly.getCoeffs();
  for (size_t ii = 0; ii < coeffs.size(); ii++) {
    snprintf(text, 1024, "%g", coeffs[ii]);
    if (ii != 0) {
      coeffStr += ",";
    }
    coeffStr += text;
  }
  xml += RadxXml::writeString("PolyCoeffs", 1, coeffStr);

  // histogram details
  
  xml += RadxXml::writeDouble("HistNBins", 1, poly.getHistNBins());
  xml += RadxXml::writeDouble("HistMin", 1, poly.getHistMin());
  xml += RadxXml::writeDouble("HistMax", 1, poly.getHistMax());
  xml += RadxXml::writeDouble("HistDelta", 1, poly.getHistDelta());
  xml += RadxXml::writeDouble("HistMedian", 1, poly.getHistMedian());
  xml += RadxXml::writeDouble("HistMode", 1, poly.getHistMode());
  xml += RadxXml::writeDouble("GoodnessOfFit", 1, poly.getGof());
  
  string countStr, densityStr, pdfStr, cdfStr;
  const vector<double> &counts = poly.getHistCount();
  const vector<double> &density = poly.getHistDensity();
  const vector<double> &pdf = poly.getHistPdf();
  const vector<double> &cdf = poly.getHistCdf();
  for (size_t ii = 0; ii < poly.getHistNBins(); ii++) {
    if (ii != 0) {
      countStr += ",";
      densityStr += ",";
      pdfStr += ",";
      cdfStr += ",";
    }
    snprintf(text, 1024, "%g", counts[ii]);
    countStr += text;
    snprintf(text, 1024, "%g", density[ii]);
    densityStr += text;
    snprintf(text, 1024, "%g", pdf[ii]);
    pdfStr += text;
    snprintf(text, 1024, "%g", cdf[ii]);
    cdfStr += text;
  }
  xml += RadxXml::writeString("HistCounts", 1, countStr);
  xml += RadxXml::writeString("HistDensity", 1, densityStr);
  xml += RadxXml::writeString("PdfCounts", 1, pdfStr);
  xml += RadxXml::writeString("CdfCounts", 1, cdfStr);
  
  if (_params.read_site_temp_from_spdb) {
    xml += RadxXml::writeDouble("TempSite", 1, _siteTempC);
    RadxTime tempTime(_timeForSiteTemp);
    xml += RadxXml::writeString("TempTime", 1, tempTime.getW3cStr());
  }
  
  xml += RadxXml::writeEndTag("ZdrStats", 0);

  return xml;

}

//////////////////////////////////////////////////
// retrieve site temp from SPDB for volume time

int PidZdrStats::_retrieveSiteTempFromSpdb(double &tempC,
                                           time_t &timeForTemp)
  
{

  // get surface data from SPDB

  DsSpdb spdb;
  si32 dataType = 0;
  if (strlen(_params.site_temp_station_name) > 0) {
    dataType =  Spdb::hash4CharsToInt32(_params.site_temp_station_name);
  }
  time_t searchTime = _readVol.getStartTimeSecs();

  if (spdb.getClosest(_params.site_temp_spdb_url,
                      searchTime,
                      _params.site_temp_search_margin_secs,
                      dataType)) {
    cerr << "WARNING - PidZdrStats::_retrieveSiteTempFromSpdb" << endl;
    cerr << "  Cannot get temperature from URL: " << _params.site_temp_spdb_url << endl;
    cerr << "  Station name: " << _params.site_temp_station_name << endl;
    cerr << "  Search time: " << RadxTime::strm(searchTime) << endl;
    cerr << "  Search margin (secs): " << _params.site_temp_search_margin_secs << endl;
    cerr << spdb.getErrStr() << endl;
    return -1;
  }
  
  // got chunks
  
  const vector<Spdb::chunk_t> &chunks = spdb.getChunks();
  if (chunks.size() < 1) {
    cerr << "WARNING - PidZdrStats::_retrieveSiteTempFromSpdb" << endl;
    cerr << "  No suitable temp data from URL: " << _params.site_temp_spdb_url << endl;
    cerr << "  Search time: " << RadxTime::strm(searchTime) << endl;
    cerr << "  Search margin (secs): " << _params.site_temp_search_margin_secs << endl;
    return -1;
  }

  const Spdb::chunk_t &chunk = chunks[0];
  WxObs obs;
  if (obs.disassemble(chunk.data, chunk.len)) {
    cerr << "WARNING - PidZdrStats::_retrieveSiteTempFromSpdb" << endl;
    cerr << "  SPDB data is of incorrect type, prodLabel: " << spdb.getProdLabel() << endl;
    cerr << "  Should be station data type" << endl;
    cerr << "  URL: " << _params.site_temp_spdb_url << endl;
    return -1;
  }

  tempC = obs.getTempC();
  timeForTemp = obs.getObservationTime();

  return 0;

}

