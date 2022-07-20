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
// RadxCalUpdate.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2019
//
///////////////////////////////////////////////////////////////
//
// Update the calibration in a Radx file.
// Also ajusts the DBZ fields accordingly.
// Optionally corrects the altitude for EGM errors.
//
////////////////////////////////////////////////////////////////

#include <cmath>
#include <Radx/RadxVol.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Mdv/GenericRadxFile.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <toolsa/pmu.h>
#include <toolsa/TaXml.hh>
#include <toolsa/TaStr.hh>
#include <dsserver/DsLdataInfo.hh>
#include <Spdb/DsSpdb.hh>
#include "RadxCalUpdate.hh"
using namespace std;

// Constructor

RadxCalUpdate::RadxCalUpdate(int argc, char **argv)
  
{

  OK = TRUE;

  // set programe name

  _progName = "RadxCalUpdate";
  
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

}

// destructor

RadxCalUpdate::~RadxCalUpdate()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int RadxCalUpdate::Run()
{

  // read cal from XML file, as required
  
  if (_params.update_calibration) {
    string errStr;
    if (_fileCal.readFromXmlFile(_params.calibration_file_path,
                                 errStr)) {
      cerr << "ERROR - RadxCalUpdate::Run()" << endl;
      cerr << "  Cannot read cal from file: "
           << _params.calibration_file_path << endl;
      cerr << errStr << endl;
      return -1;
    }
  }
  _tempCorrCal = _fileCal;

  // read altitude correction table, as required
    
  if (_params.correct_altitude_for_egm) {
    if (_egm.readGeoid(_params.egm_2008_geoid_file)) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  Altitude correction for geoid." << endl;
      cerr << "  Problem reading geoid file: " 
           << _params.egm_2008_geoid_file << endl;
      return -1;
    }
  }

  // run

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

int RadxCalUpdate::_runFilelist()
{

  // loop through the input file list

  int iret = 0;

  for (int ii = 0; ii < (int) _args.inputFileList.size(); ii++) {

    string inputPath = _args.inputFileList[ii];
    if (_processFile(inputPath)) {
      iret = -1;
    }

  }

  return iret;

}

//////////////////////////////////////////////////
// Run in archive mode

int RadxCalUpdate::_runArchive()
{

  // get the files to be processed

  RadxTimeList tlist;
  tlist.setDir(_params.input_dir);
  tlist.setModeInterval(_args.startTime, _args.endTime);
  if (tlist.compile()) {
    cerr << "ERROR - RadxCalUpdate::_runFilelist()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(_args.startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(_args.endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - RadxCalUpdate::_runFilelist()" << endl;
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

int RadxCalUpdate::_runRealtime()
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

int RadxCalUpdate::_processFile(const string &filePath)
{

  if (_params.debug) {
    cerr << "INFO - RadxCalUpdate::Run" << endl;
    cerr << "  Input path: " << filePath << endl;
  }
  
  RadxFile inFile;
  _setupRead(inFile);
  
  // read in file
  
  RadxVol vol;
  if (inFile.readFromPath(filePath, vol)) {
    cerr << "ERROR - RadxCalUpdate::Run" << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }

  // correct gain for temperature if required
  
  if (_params.correct_hcr_v_rx_gain_for_temperature) {
    time_t volStartTime = vol.getStartTimeSecs();
    time_t volEndTime = vol.getEndTimeSecs();
    time_t volMidTime = volStartTime + (volEndTime - volStartTime) / 2;
    if (_correctHcrVRxGainForTemp(volMidTime) == 0) {
      string newStatus = vol.getStatusXml();
      newStatus += _deltaGainXml;
      vol.setStatusXml(newStatus);
    } else {
      // failed - use file version
      _tempCorrCal = _fileCal;
    }
  }

  // fix each ray in turn

  vector<RadxRay *> &rays = vol.getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {
    _fixRay(vol, *rays[iray]);
  }

  // set the new calib for the volume

  vol.clearRcalibs();
  RadxRcalib *newCal = new RadxRcalib(_tempCorrCal);
  vol.addCalib(newCal);

  // write the file

  if (_writeVol(vol)) {
    cerr << "ERROR - RadxCalUpdate::_processFile" << endl;
    cerr << "  Cannot write volume to file" << endl;
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// set up read

void RadxCalUpdate::_setupRead(RadxFile &file)
{

  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setDebug(true);
    file.printReadRequest(cerr);
  }

}

//////////////////////////////////////////////////
// set up write

void RadxCalUpdate::_setupWrite(RadxFile &file)
{

  if (_params.debug) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }

  if (_params.output_filename_mode == Params::START_TIME_ONLY) {
    file.setWriteFileNameMode(RadxFile::FILENAME_WITH_START_TIME_ONLY);
  } else if (_params.output_filename_mode == Params::END_TIME_ONLY) {
    file.setWriteFileNameMode(RadxFile::FILENAME_WITH_END_TIME_ONLY);
  } else {
    file.setWriteFileNameMode(RadxFile::FILENAME_WITH_START_AND_END_TIMES);
  }

  if (_params.output_compressed) {
    file.setWriteCompressed(true);
    file.setCompressionLevel(_params.compression_level);
  } else {
    file.setWriteCompressed(false);
  }

  // set output format

  file.setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL);
  file.setNcFormat(RadxFile::NETCDF4);

}

//////////////////////////////////////////////////
// write out the volume

int RadxCalUpdate::_writeVol(RadxVol &vol)
{

  // output file

  GenericRadxFile outFile;
  _setupWrite(outFile);
  
  // write to dir
  
  if (outFile.writeToDir(vol, _params.output_dir,
                         _params.append_day_dir_to_output_dir,
                         _params.append_year_dir_to_output_dir)) {
    cerr << "ERROR - FixRadxPointing::_writeVol" << endl;
    cerr << "  Cannot write file to dir: " << _params.output_dir << endl;
    cerr << outFile.getErrStr() << endl;
    return -1;
  }

  // write latest data info file if requested 
  
  if (_params.write_latest_data_info) {
    string outputPath = outFile.getPathInUse();
    DsLdataInfo ldata(_params.output_dir);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      ldata.setDebug(true);
    }
    string relPath;
    RadxPath::stripDir(_params.output_dir, outputPath, relPath);
    ldata.setRelDataPath(relPath);
    ldata.setWriter(_progName);
    if (ldata.write(vol.getEndTimeSecs())) {
      cerr << "WARNING - FixRadxPointing::_writeVol" << endl;
      cerr << "  Cannot write latest data info file to dir: "
           << _params.output_dir << endl;
    }
  }

  return 0;

}

//////////////////////////////////////////////////
// fix a ray

void RadxCalUpdate::_fixRay(RadxVol &vol, RadxRay &ray)
{

  if (_params.update_calibration) {
    _fixRayCalibration(vol, ray);
  }
  
  if (_params.correct_altitude_for_egm) {
    _fixRayAltitude(vol, ray);
  }
  
}

//////////////////////////////////////////////////
// fix a ray's calibration

void RadxCalUpdate::_fixRayCalibration(RadxVol &vol, RadxRay &ray)
{

  if (vol.getNRcalibs() < 1) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - RadxCalUpdate::_fixRayCalibration" << endl;
      cerr << "  No calibration for this volume" << endl;
    }
    return;
  }
  
  // get the calibration for this ray
  
  int calibIndex = ray.getCalibIndex();
  if (calibIndex > (int) vol.getNRcalibs() - 1) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - RadxCalUpdate::_fixRayCalibration" << endl;
      cerr << "  Calib index too high: " << calibIndex << endl;
      cerr << "  nCalibs: " << vol.getNRcalibs() << endl;
    }
    calibIndex = 0;
  }
  RadxRcalib *rayCal = vol.getRcalibs()[calibIndex];

  // check the time

  if (rayCal->getCalibTime() == _tempCorrCal.getCalibTime()) {
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "WARNING - RadxCalUpdate::_fixRayCalibration" << endl;
      cerr << "  Calibrations have same time: "
           << RadxTime::strm(_tempCorrCal.getCalibTime()) << endl;
      cerr << "  No change" << endl;
    }
    return;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "===== Working on ray, time: " << ray.getRadxTime().asString(3)
         << " =====" << endl;
  }

  // correct the cal for temp

  _correctHcrVRxGainForTemp(ray.getTimeSecs());

  // correct the DBZ fields for the new cal

  for (int ifld = 0; ifld < _params.dbz_fields_for_update_n; ifld++) {

    // find the field we want
    
    string fieldName = _params._dbz_fields_for_update[ifld].name;
    RadxField *dbzFld = ray.getField(fieldName);
    if (dbzFld == NULL) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING - RadxCalUpdate::_fixRayCalibration" << endl;
        cerr << "==>> ignoring DBZ field, cannot find: " << fieldName << endl;
      }
      continue;
    }
    
    // compute the deltas for the calibration
    
    double oldBaseDbz = rayCal->getBaseDbz1kmHc();
    double newBaseDbz = _tempCorrCal.getBaseDbz1kmHc();
    if (_params._dbz_fields_for_update[ifld].channel == Params::CHANNEL_VC) {
      oldBaseDbz = rayCal->getBaseDbz1kmVc();
      newBaseDbz = _tempCorrCal.getBaseDbz1kmVc();
    }
    double deltaDbz = newBaseDbz - oldBaseDbz;
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "==>> changing DBZ cal, field, deltaDbz = " 
           << fieldName << ", " << deltaDbz << endl;
    }

    // store the data type
    // then convert to floats

    Radx::DataType_t dtype = dbzFld->getDataType();
    dbzFld->convertToFl32();
    Radx::fl32 *dbz = dbzFld->getDataFl32();
    size_t nGates = dbzFld->getNPoints();
    
    // adjust the reflectivity for the change in cal

    for (size_t igate = 0; igate < nGates; igate++) {
      dbz[igate] = dbz[igate] + deltaDbz;
    } // igate

    // convert back to original type
    
    dbzFld->convertToType(dtype);

  } // ifld

  // correct the DBM fields for the new cal

  for (int ifld = 0; ifld < _params.dbm_fields_for_update_n; ifld++) {

    // find the field we want
    
    string fieldName = _params._dbm_fields_for_update[ifld].name;
    RadxField *dbmFld = ray.getField(fieldName);
    if (dbmFld == NULL) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING - RadxCalUpdate::_fixRayCalibration" << endl;
        cerr << "==>> ignoring DBM field, cannot find: " << fieldName << endl;
      }
      continue;
    }
    
    // compute the deltas for the calibration
    
    double oldGainDb = rayCal->getReceiverGainDbHc();
    double newGainDb = _tempCorrCal.getReceiverGainDbHc();
    if (_params._dbm_fields_for_update[ifld].channel == Params::CHANNEL_VC) {
      oldGainDb = rayCal->getReceiverGainDbVc();
      newGainDb = _tempCorrCal.getReceiverGainDbVc();
    } else if (_params._dbm_fields_for_update[ifld].channel == Params::CHANNEL_HX) {
      oldGainDb = rayCal->getReceiverGainDbHx();
      newGainDb = _tempCorrCal.getReceiverGainDbHx();
    } else if (_params._dbm_fields_for_update[ifld].channel == Params::CHANNEL_VX) {
      oldGainDb = rayCal->getReceiverGainDbVx();
      newGainDb = _tempCorrCal.getReceiverGainDbVx();
    }
    double deltaGainDb = newGainDb - oldGainDb;
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "==>> changing DBM cal, field, deltaGainDb = "
           << fieldName << ", " << deltaGainDb << endl;
    }

    // store the data type
    // then convert to floats

    Radx::DataType_t dtype = dbmFld->getDataType();
    dbmFld->convertToFl32();
    Radx::fl32 *dbm = dbmFld->getDataFl32();
    size_t nGates = dbmFld->getNPoints();
    
    // adjust the dbm for the change in cal

    for (size_t igate = 0; igate < nGates; igate++) {
      dbm[igate] = dbm[igate] - deltaGainDb;
    } // igate

    // convert back to original type
    
    dbmFld->convertToType(dtype);

  } // ifld

}

//////////////////////////////////////////////////
// fix a ray's altitude

void RadxCalUpdate::_fixRayAltitude(RadxVol &vol, RadxRay &ray)
{

  // get the georef
  
  RadxGeoref *georef = ray.getGeoreference();
  if (georef == NULL) {
    // no georef, so this step does not apply
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "WARNING - RadxCalUpdate::_fixRayAltitude" << endl;
      cerr << "==>> no georefs, so no altitude correction applied" << endl;
    }
    return;
  }

  // get the geoid correction in meters
  
  double geoidM = _egm.getInterpGeoidM(georef->getLatitude(),
                                       georef->getLongitude());
  
  // the altitude correction has the opposite sign, since it
  // is added to the measured altitude

  double altCorrM = geoidM * -1.0;
  double altKmMsl = georef->getAltitudeKmMsl() + altCorrM / 1000.0;
  georef->setAltitudeKmMsl(altKmMsl);

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "==>> changing altitude, geoidM = " << geoidM << endl;
  }

}

////////////////////////////////////////////////////////
// Correct HCR V RX calibration gains for temperature
// the reads in the correction from SPDB

int RadxCalUpdate::_correctHcrVRxGainForTemp(time_t timeSecs)
  
{

  // init

  _deltaGainXml.clear();

  // get gain correction data from SPDB
  
  DsSpdb spdb;
  if (spdb.getClosest(_params.hcr_delta_gain_spdb_url,
                      timeSecs,
                      _params.hcr_delta_gain_search_margin_secs)) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - RadxCalUpdate::_correctHcrVRxGainForTemp()" << endl;
      cerr << "  Cannot get delta gain from URL: "
           << _params.hcr_delta_gain_spdb_url << endl;
      cerr << "  Search time: " << DateTime::strm(timeSecs) << endl;
      cerr << "  Search margin (secs): "
           << _params.hcr_delta_gain_search_margin_secs << endl;
      cerr << spdb.getErrStr() << endl;
    }
    return -1;
  }
  
  // got chunks
  
  const vector<Spdb::chunk_t> &chunks = spdb.getChunks();
  if (chunks.size() < 1) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - RadxCalUpdate::_correctHcrVRxGainForTemp()" << endl;
      cerr << "  No suitable gain data from URL: "
           << _params.hcr_delta_gain_spdb_url << endl;
      cerr << "  Search time: " << DateTime::strm(timeSecs) << endl;
      cerr << "  Search margin (secs): "
           << _params.hcr_delta_gain_search_margin_secs << endl;
    }
    return -1;
  }
  const Spdb::chunk_t &chunk = chunks[0];

  // set xml string with gain results
  
  _deltaGainXml = std::string((char *) chunk.data, chunk.len - 1);

  // find delta gain in xml
  
  double deltaGainVc =
    _getDeltaGainFromXml(_deltaGainXml, _params.hcr_v_rx_delta_gain_tag_list);
  
  if (std::isnan(deltaGainVc)) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - RadxCalUpdate::_correctHcrVRxGainForTemp()" << endl;
      cerr << "  Cannot find deltaGain in XML: " << _deltaGainXml << endl;
    }
    return -1;
  }

  // copy the new cal from the input file

  _tempCorrCal = _fileCal;
  
  // compute base dbz if needed

  if (!_tempCorrCal.isMissing(_tempCorrCal.getReceiverGainDbVc())) {
    double rconst = _fileCal.getRadarConstantV();
    double noise = _fileCal.getNoiseDbmVc();
    // we assume the noise does not change based on temp
    // not quite accurate, but good enough
    double noiseCorr = noise;
    double gain = _fileCal.getReceiverGainDbVc();
    double gainCorr = gain + deltaGainVc;
    double baseDbz1km = _fileCal.getBaseDbz1kmVc();
    double baseDbz1kmCorr = noiseCorr - gainCorr + rconst;
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "==>> Vc noise, corr, delta: "
           << noise << ", "
           << noiseCorr << ", "
           << noiseCorr - noise << endl;
      cerr << "==>> Vc gain, corr, delta: "
           << gain << ", " 
           << gainCorr << ", "
           << gainCorr - gain << endl;
      cerr << "==>> Vc baseDbz1Km, corr, delta: "
           << baseDbz1km << ", "
           << baseDbz1kmCorr << ", "
           << baseDbz1kmCorr - baseDbz1km << endl;
    }
    if (!_tempCorrCal.isMissing(noise) && !_tempCorrCal.isMissing(rconst)) {
      _tempCorrCal.setBaseDbz1kmVc(baseDbz1kmCorr);
    }
    _tempCorrCal.setReceiverGainDbVc(gainCorr);
    _tempCorrCal.setNoiseDbmVc(noiseCorr);
    _tempCorrCal.setCalibTime(timeSecs);
  }
  
  return 0;
  
}

/////////////////////////////////////////////////////////////////
// get delta gain from XML string, given the tag list
// returns delta gain, NAN on failure

double RadxCalUpdate::_getDeltaGainFromXml(const string &xml,
                                           const string &tagList) const
  
{
  
  // get tags in list
  
  vector<string> tags;
  TaStr::tokenize(tagList, "<>", tags);
  if (tags.size() == 0) {
    // no tags
    cerr << "WARNING - RadxCalUpdate::_getDeltaGainFromXml()" << endl;
    cerr << "  deltaGain tag not found: " << tagList << endl;
    return NAN;
  }
  
  // read through the outer tags in status XML
  
  string buf(xml);
  for (size_t jj = 0; jj < tags.size(); jj++) {
    string val;
    if (TaXml::readString(buf, tags[jj], val)) {
      cerr << "WARNING - RadxCalUpdate::_getDeltaGainFromXml()" << endl;
      cerr << "  Bad tags found in status xml, expecting: "
           << tagList << endl;
      return NAN;
    }
    buf = val;
  }

  // read delta gain
  
  double deltaGain = NAN;
  if (TaXml::readDouble(buf, deltaGain)) {
    cerr << "WARNING - RadxCalUpdate::_getDeltaGainFromXml()" << endl;
    cerr << "  Bad deltaGain found in status xml, buf: " << buf << endl;
    return NAN;
  }
  
  return deltaGain;

}
  
