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
///////////////////////////////////////////////////////////
// Calibration.cc
// 
// read and store calibration data
//
// EOL, NCAR, Boulder CO
//
// August 2007
//
// Mike Dixon
/////////////////////////////////////////////////////////////

#include <cerrno>
#include <cmath>
#include <cassert>
#include <toolsa/file_io.h>
#include <toolsa/DateTime.hh>
#include <toolsa/ReadDir.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/TaArray.hh>
#include <radar/IwrfTsInfo.hh>
#include <Spdb/DsSpdb.hh>
#include "Calibration.hh"

//////////////////
// Constructor

Calibration::Calibration(const Params &params) :
        _params(params)
{

  // initialize
  
  _calAvailable = false;
  _prevPulseWidth = -1.0;
  _prevXmitRcvMode = IWRF_XMIT_RCV_MODE_NOT_SET;
  _radarName = "unknown";
  _calTime = 0;

  _noiseMonTime = -1;
  _noiseMonZdr = -9999.0;
  _noiseMonDbmHc = -9999.0;
  _noiseMonDbmVc = -9999.0;
  _useNoiseMonCalib = false;
  
}

//////////////////
// Destructor


Calibration::~Calibration()
{
}

//////////////////////////////////////////////////////////
// get current cal values

const IwrfCalib &Calibration::getIwrfCalib() const
{

  if (_useNoiseMonCalib) {
    return _noiseMonCalib;
  } else {
    return _calib;
  }

}

//////////////////////////////////////////////////////////
// Read calibration for a given file path.
// Returns 0 on success, -1 on failure

int Calibration::readCal(const string &filePath)

{

  // read from file

  if (_readCalFromFile(filePath)) {
    return -1;
  }

  // debug print

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _calib.print(cerr);
  }

  return 0;

}

//////////////////////////////////////////////////////////
// Load calibration appropriate to a given beam
// Returns 0 on success, -1 on failure

int Calibration::loadCal(Beam *beam)
  
{
  
  if (_params.use_cal_from_time_series) {
    
    _setCalFromTimeSeries(beam);
    
  } else {
    
    // read calibration for this pulse width, if appropriate
    
    if (_params.set_cal_by_pulse_width) {
      
      if (_params.pulse_width_cals_n < 1) {
        cerr << "WARNING - Calibration::loadCal" << endl;
        cerr << "  No calibration directories specified for pulse width." << endl;
        cerr << "  Set parameter 'set_cal_by_pulse_width = false'" << endl;
        return -1;
      }
      
      if (_checkPulseWidthAndRead(beam)) {
        return -1;
      }
      
    }
    
  }

  // adjust cal receiver gains from NoiseMon data, if required
  
  if (_params.noise_mon_correct_cal_rx_gain) {
    if (_adjustCalGainFromNoiseMon(beam)) {
      cerr << "WARNING - Calibration::loadCal()" << endl;
      cerr << "  Cannot retrieve NoiseMon data to adjust rx gains" << endl;
      return -1;
    }
  }
    
  return 0;

}

/////////////////////////////////////////////////
// Set the calibration from the ops info
// that the baseDbz1km values are set

void Calibration::_setCalFromTimeSeries(Beam *beam)
  
{
  
  const IwrfTsInfo &tsInfo = beam->getOpsInfo();

  if (tsInfo.isDerivedFromRvp8()) {

    // set RVP8 specific data
    
    if (beam->getIsSwitchingReceiver()) {
      
      _calib.setNoiseDbmHc(tsInfo.get_rvp8_f_noise_dbm(0));
      _calib.setNoiseDbmHx(tsInfo.get_rvp8_f_noise_dbm(1));
      _calib.setNoiseDbmVc(tsInfo.get_rvp8_f_noise_dbm(0));
      _calib.setNoiseDbmVx(tsInfo.get_rvp8_f_noise_dbm(1));
      
    } else {
      
      _calib.setNoiseDbmHc(tsInfo.get_rvp8_f_noise_dbm(0));
      _calib.setNoiseDbmHx(tsInfo.get_rvp8_f_noise_dbm(0));
      _calib.setNoiseDbmVc(tsInfo.get_rvp8_f_noise_dbm(1));
      _calib.setNoiseDbmVx(tsInfo.get_rvp8_f_noise_dbm(1));
      
    }
    
    _calib.setBaseDbz1kmHc(tsInfo.get_rvp8_f_dbz_calib());
    _calib.setBaseDbz1kmHx(tsInfo.get_rvp8_f_dbz_calib());
    _calib.setBaseDbz1kmVc(tsInfo.get_rvp8_f_dbz_calib());
    _calib.setBaseDbz1kmVx(tsInfo.get_rvp8_f_dbz_calib());
    
  } else {
    
    tsInfo.setIwrfCalib(_calib);
    
  } // if (tsInfo.isDerivedFromRvp8())

  // apply corrections as appropriate

  _applyCorrections();

}


//////////////////////////////////////////////////////////
// Read calibration for a specific pulse width,
// appropriate to a given beam.
// Returns 0 on success, -1 on failure

int Calibration::_checkPulseWidthAndRead(Beam *beam)

{

  // check for change in pulse width
  
  double beamPulseWidthUs = beam->getPulseWidth() * 1.0e6;
  iwrf_xmit_rcv_mode_t xmitRcvMode = beam->getXmitRcvMode();
  
  if (fabs(beamPulseWidthUs - _prevPulseWidth) < 0.000001 &&
      xmitRcvMode == _prevXmitRcvMode) {
    // don't need to read
    return 0;
  }

  // pulse width has changed - reset
  
  _calAvailable = false;
  _calTime = 0;
  
  // set directory for pulse width
  
  double minDiff = 1.0e99;
  int index = 0;
  _calDirForPulseWidth = _params._pulse_width_cals[0].cal_dir;
  for (int ii = 0; ii < _params.pulse_width_cals_n; ii++) {
    
    Params::pulse_width_cal_t entry = _params._pulse_width_cals[ii];

    if (entry.check_xmit_rcv_mode) {
      if (xmitRcvMode != _getXmitRcvMode(entry.xmit_rcv_mode)) {
        // xmit rcv mode does not match so do not use this entry
        continue;
      }
    }
    
    double pwidth = entry.pulse_width_us;
    double diff = fabs(pwidth - beamPulseWidthUs);
    if (diff < minDiff) {
      _calDirForPulseWidth = entry.cal_dir;
      minDiff = diff;
      index = ii;
    }
    
  } // ii
  
  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "Reading new cal, pulse width (us): " << beamPulseWidthUs << endl;
    cerr << "  Cal dir: " << _calDirForPulseWidth << endl;
  }
  
  _prevPulseWidth = beamPulseWidthUs;
  _prevXmitRcvMode = xmitRcvMode;

  // read the calibration

  Params::pulse_width_cal_t entry = _params._pulse_width_cals[index]; 
  if (_readCal(beam->getTimeSecs(), _calDirForPulseWidth)) {
    return -1;
  }
  
  // override from entry as required
  
  if (_params.override_cal_dbz_correction) {
    _calib.setDbzCorrection(_params.dbz_correction);
  }

  if (_params.override_cal_zdr_correction) {
    if (entry.zdr_correction_db > -9990) {
      _calib.setZdrCorrectionDb(entry.zdr_correction_db);
    }
  }
  
  if (_params.override_cal_system_phidp) {
    if (entry.system_phidp_deg > -9990) {
      _calib.setSystemPhidpDeg(entry.system_phidp_deg);
    }
  }
    
  return 0;

}


////////////////////////////////////////////////////////////////////////
// search for and read a cal file, given the time

int Calibration::_readCal(time_t utime, const string &calDir)

{

  if (utime == 0) {
    if (_params.debug) {
      cerr << "WARNING - Calibration::_readCal" << endl;
      cerr << "  Cannot read cal files, data time still 0" << endl;
    }
    return -1;
  }

  // compile file list

  FileMap fileMap;
  int iret = _compileFileList(calDir, fileMap);
  if (iret && _params.debug) {
    cerr << "WARNING - Calibration::_readCal" << endl;
    cerr << "  Cannot read cal files" << endl;
  }
  if (!_calAvailable && fileMap.size() == 0) {
    cerr << "ERROR - Calibration::_readCal" << endl;
    cerr << "  No calibration files available" << endl;
    return -1;
  }

  // find best file

  double minDiff = 1.0e99;
  string bestPath = "";
  time_t bestTime = 0;
  for (FileMap::iterator it = fileMap.begin(); it != fileMap.end(); it++) {
    const FilePair &filePair = *it;
    double fileTime = (double) filePair.first;
    double diff = fabs((double) utime - fileTime);
    if (diff < minDiff) {
      bestTime = filePair.first;
      const string &path = filePair.second;
      bestPath = path;
      minDiff = diff;
    }
  } // it

  if (bestPath.size() == 0 && _params.debug) {
    cerr << "WARNING - Calibration::_readCal" << endl;
    cerr << "  No cal files available" << endl;
    return -1;
  }
  
  // print out cal file list

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Data time: " << DateTime::strm(utime) << endl;
    cerr << "Reading in calibration - available files: " << endl;
    for (FileMap::iterator it = fileMap.begin(); it != fileMap.end(); it++) {
      const FilePair &filePair = *it;
      cerr << "  cal path: " << filePair.second << endl;
      cerr << "      time: " << DateTime::strm(filePair.first) << endl;
    } // it
    cerr << "Best path: " << bestPath << ", time diff (days): " << minDiff / 86400.0 << endl;
  }

  bool success = false;

  if (bestTime != _calTime) {

    if (_readCalFromFile(bestPath) == 0) {

      _calTime = bestTime;
      success = true;
      
    } else {
      
      cerr << "WARNING - Calibration::_readCal" << endl;
      cerr << "  Cannot read best file: " << bestPath << endl;
      
      // try the files in reverse order
      
      for (FileMap::reverse_iterator it = fileMap.rbegin();
	   it != fileMap.rend(); it++) {
	const FilePair &filePair = *it;
	const string &path = filePair.second;
	if (_params.debug) {
	  cerr << "  Trying cal file: " << path << endl;
	}
	if (_readCalFromFile(path) == 0) {
	  if (_params.debug) {
	    cerr << "SUCCESS - Calibration::_readCal" << endl;
	    cerr << "  Read this file instead: " << path << endl;
	  }
	  success = true;
	  continue;
	}
      } // it
    }
      
  } else {

    if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
      cerr << "Skipping cal file - " << bestPath 
	   << " - File utime same as previous cal" << endl;
    }

  } // if (bestTime != _calTime)

  if (success) {

    if (_params.debug) {
      cerr << "Read calibration, time: " << DateTime::strm(_calTime) << endl;
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        _calib.print(cerr);
      }
    }

    return 0;

  } else {

    if (bestTime != _calTime) {
      cerr << "ERROR - Calibration::_readCal" << endl;
      cerr << "  Could not find new valid param file" << endl;
    }
    return -1;

  }

}

////////////////////////////////////////////////////////////////////////
// read a given cal file

int Calibration::_readCalFromFile(const string &calPath)

{

  string errStr;
  if (_calib.readFromXmlFile(calPath, errStr)) {
    cerr << "ERROR - Calibration::_readCalFromFile" << endl;
    cerr << "  Cannot decode cal file: " << calPath << endl;
    cerr << errStr;
    _calAvailable = false;
    return -1;
  }
  
  _calFilePath = calPath;
  _calAvailable = true;

  // apply corrections as appropriate

  _applyCorrections();

  if (_params.debug) {
    cerr << "Done reading calibration file: " << calPath << endl;
  }

  return 0;

}

////////////////////////
// _checkDirForParams()
//
// Recurse down the directory tree, checking for existence
// of _DsFileDist files
//
// The findLdataInfo argument indicates whether we are looking
// for (a) _DsFileDist params files (false) or
//     (b) _latest_data_info files (true)
//
// Returns 0 on success, -1 on failure.

int Calibration::_compileFileList(const string &dirPath,
                                  FileMap &fileMap)
  
{
  
  // Try to open the directory
  
  ReadDir rdir;
  if (rdir.open(dirPath.c_str())) {
    cerr << "ERROR - " << "Calibration::_compileFileList." << endl;
    cerr << "  Cannot open dirPath: \'" << dirPath << "\'" << endl;
    cerr << "  " << strerror(errno) << endl;
    return (-1);
  }

  // Loop thru directory looking for sub-directories
  
  struct dirent *dp;
  for (dp = rdir.read(); dp != NULL; dp = rdir.read()) {
    
    // exclude the '.' and '..' entries
    
    if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) {
      continue;
    }

    // stat the file
    
    string path = dirPath;
    path += PATH_DELIM;
    path += dp->d_name;
    struct stat fileStat;

    if (stat(path.c_str(), &fileStat)) {
      // file has disappeared, so ignore
      continue;
    }

    // is this an XML file? If not, skip

    if (strstr(dp->d_name, ".xml") == NULL) {
      continue;
    }
    
    // check for directory
    
    if (S_ISDIR(fileStat.st_mode)) {

      // this is a directory, so recurse into it
      
      _compileFileList(path, fileMap);
      
    } else {
      
      // read the file, get the time from it

      IwrfCalib cal;
      string errStr;
      if (cal.readFromXmlFile(path, errStr) == 0) {
        // add to map
        FilePair filePair(cal.getCalibTime(), path);
        fileMap.insert(filePair);
      }

    } // if (S_ISDIR ...
    
  } // dp
    
  rdir.close();
  
  return 0;

}

////////////////////////////////////////////////////////////////
// get the IWRF xmit/rcv mode from the parameter file enum

iwrf_xmit_rcv_mode_t Calibration::_getXmitRcvMode(Params::xmit_rcv_mode_t mode)
  
{

  switch (mode) {
    
    case Params::SINGLE_POL: 
      return IWRF_SINGLE_POL;
    case Params::SINGLE_POL_V: 
      return IWRF_SINGLE_POL_V;
    case Params::DP_ALT_HV_CO_ONLY:
      return IWRF_ALT_HV_CO_ONLY;
    case Params::DP_ALT_HV_CO_CROSS:
      return IWRF_ALT_HV_CO_CROSS;
    case Params::DP_ALT_HV_FIXED_HV:
      return IWRF_ALT_HV_FIXED_HV;
    case Params::DP_SIM_HV_FIXED_HV:
      return IWRF_SIM_HV_FIXED_HV;
    case Params::DP_SIM_HV_SWITCHED_HV:
      return IWRF_SIM_HV_SWITCHED_HV;
    case Params::DP_H_ONLY_FIXED_HV:
      return IWRF_H_ONLY_FIXED_HV;
    case Params::DP_V_ONLY_FIXED_HV:
      return IWRF_V_ONLY_FIXED_HV;
    default:
      return IWRF_SINGLE_POL;
      
  } // switch

}

////////////////////////////////////////////////////////////////
// apply corrections

void Calibration::_applyCorrections()
  
{

  
  if (_params.override_cal_dbz_correction) {
    _calib.setDbzCorrection(_params.dbz_correction);
    if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
      cerr << "Calibration::_applyCorrections()" << endl;
      cerr << "  setting dbz_correction: " << _params.dbz_correction << endl;
    }
  }
  
  if (_params.override_cal_zdr_correction) {
    _calib.setZdrCorrectionDb(_params.zdr_correction_db);
    if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
      cerr << "Calibration::_applyCorrections()" << endl;
      cerr << "  setting zdr_correction_db: " << _params.zdr_correction_db << endl;
    }
  }
  
  if (_params.override_cal_ldr_corrections) {
    _calib.setLdrCorrectionDbH(_params.ldr_correction_db_h);
    _calib.setLdrCorrectionDbV(_params.ldr_correction_db_v);
    if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
      cerr << "Calibration::_applyCorrections()" << endl;
      cerr << "  setting ldr_correction_db_h: " << _params.ldr_correction_db_h << endl;
      cerr << "  setting ldr_correction_db_v: " << _params.ldr_correction_db_v << endl;
    }
  }
  
  if (_params.override_cal_system_phidp) {
    _calib.setSystemPhidpDeg(_params.system_phidp_deg);
    if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
      cerr << "Calibration::_applyCorrections()" << endl;
      cerr << "  setting system_phidp_deg: " << _params.system_phidp_deg << endl;
    }
  }

}

////////////////////////////////////////////////////////////////
// adjust cal rx gain for noise mon

int Calibration::_adjustCalGainFromNoiseMon(Beam *beam)
  
{

  // check if this was done recently, i.e. within the last 10 secs

  if (fabs((double) beam->getTimeSecs() - (double) _noiseMonTime) < 10 &&
      _noiseMonZdr > -9990.0 &&
      _noiseMonDbmHc > -9990.0 &&
      _noiseMonDbmVc > -9990.0) {
    // we have good recent data
    if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
      cerr << "====>> have good noiseMon data, time: "
           << DateTime::strm(beam->getTimeSecs()) << endl;
    }
    return 0;
  }
      
  // retrieve noise monitoring results in interval around beam time
  
  DsSpdb spdb;
  time_t startTime = beam->getTimeSecs() - _params.noise_mon_search_margin_secs;
  time_t endTime = beam->getTimeSecs() + _params.noise_mon_search_margin_secs;
  if (spdb.getInterval(_params.noise_mon_spdb_url, startTime, endTime)) {
    if (_params.debug) {
      cerr << "WARNING - Calibration::loadCal()" << endl;
      cerr << "  Cannot get NoiseMon data from URL: "
           << _params.noise_mon_spdb_url << endl;
      cerr << "  Search start time time: " << DateTime::strm(startTime) << endl;
      cerr << "  Search end   time time: " << DateTime::strm(endTime) << endl;
      cerr << spdb.getErrStr() << endl;
    }
    return -1;
  }
  
  // got chunks
  
  const vector<Spdb::chunk_t> &chunks = spdb.getChunks();
  if (chunks.size() < 1) {
    if (_params.debug) {
      cerr << "ERROR -  Calibration::loadCal()" << endl;
      cerr << "  No suitable noise mon data from URL: "
           << _params.noise_mon_spdb_url << endl;
      cerr << "  Search time: " << DateTime::strm(beam->getTimeSecs()) << endl;
      cerr << "  Search margin (secs): "
           << _params.noise_mon_search_margin_secs << endl;
    }
    return -1;
  }

  // compute means from data in interval
  
  double sumNoiseZdr = 0.0;
  double sumNoiseDbmHc = 0.0;
  double sumNoiseDbmVc = 0.0;
  double noiseCount = 0.0;
  
  double sumSiteTempC = 0.0;
  double tempCount = 0.0;
  
  for (size_t ichunk = 0; ichunk < chunks.size(); ichunk++) {
    
    const Spdb::chunk_t &chunk = chunks[ichunk];
    string noiseMonXml((char *) chunk.data, chunk.len - 1);
  
    // find values from XML
    
    double noiseZdr = _getValFromXml(noiseMonXml,
                                     _params.noise_mon_tag_list_zdr);
    double noiseDbmHc = _getValFromXml(noiseMonXml,
                                       _params.noise_mon_tag_list_dbmhc);
    double noiseDbmVc = _getValFromXml(noiseMonXml,
                                       _params.noise_mon_tag_list_dbmvc);
    double siteTempC = _getValFromXml(noiseMonXml,
                                      _params.noise_mon_tag_list_site_temp);
    
    // sum up
    
    if (!std::isnan(noiseZdr) && !std::isnan(noiseDbmHc) && !std::isnan(noiseDbmVc)) {
      sumNoiseZdr += noiseZdr;
      sumNoiseDbmHc += noiseDbmHc;
      sumNoiseDbmVc += noiseDbmVc;
      noiseCount++;
    }
    
    if (!std::isnan(siteTempC)) {
      sumSiteTempC += siteTempC;
      tempCount++;
    }

  } // ichunk
  
  if (noiseCount < 1) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - Calibration::loadCal()" << endl;
      cerr << "  Cannot find NoiseMon data from URL: "
           << _params.noise_mon_spdb_url << endl;
      cerr << "  Search start time time: " << DateTime::strm(startTime) << endl;
      cerr << "  Search end   time time: " << DateTime::strm(endTime) << endl;
      cerr << spdb.getErrStr() << endl;
    }
    return -1;
  }

  double meanNoiseZdr = NAN;
  double meanNoiseDbmHc = NAN;
  double meanNoiseDbmVc = NAN;
  meanNoiseZdr = sumNoiseZdr / noiseCount;
  meanNoiseDbmHc = sumNoiseDbmHc / noiseCount;
  meanNoiseDbmVc = sumNoiseDbmVc / noiseCount;
  
  double meanSiteTempC = NAN;
  if (tempCount > 0) {
    meanSiteTempC = sumSiteTempC / tempCount;
  }

  // save

  _noiseMonCount = noiseCount;
  _noiseMonTime = beam->getTimeSecs();
  _noiseMonZdr = meanNoiseZdr;
  _noiseMonDbmHc = meanNoiseDbmHc;
  _noiseMonDbmVc = meanNoiseDbmVc;

  _tempCount = tempCount;
  _noiseMonSiteTempC = meanSiteTempC;

  // create XML string to add to status string
  
  string xml;
  xml += TaXml::writeStartTag("NoiseMonitoring", 0);
  xml += TaXml::writeTime("time", 1, _noiseMonTime);
  xml += TaXml::writeDouble("count", 1, _noiseMonCount);
  xml += TaXml::writeDouble("meanNoiseZdr", 1, _noiseMonZdr);
  xml += TaXml::writeDouble("meanDbmhc", 1, _noiseMonDbmHc);
  xml += TaXml::writeDouble("meanDbmvc", 1, _noiseMonDbmVc);
  if (!std::isnan(_noiseMonSiteTempC)) {
    xml += TaXml::writeDouble("siteTempC", 1, _noiseMonSiteTempC);
    xml += TaXml::writeDouble("tempCount", 1, _tempCount);
  }
  xml += TaXml::writeEndTag("NoiseMonitoring", 0);

  // augment status xml in beam
  
  beam->appendStatusXml(xml);
    
  // compute cal adjusted for noise
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
    cerr << "+++++ CALIBRATION BEFORE RX GAIN CORRECTION +++++++++++++" << endl;
    _calib.print(cerr);
    cerr << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
  }
  
  // copy from main cal
  
  _noiseMonCalib = _calib;

  // compute delta noise
  
  double calNoiseDbmHc = _calib.getNoiseDbmHc();
  double calNoiseDbmVc = _calib.getNoiseDbmVc();

  double deltaNoiseDbHc = _noiseMonDbmHc - calNoiseDbmHc;
  double deltaNoiseDbVc = _noiseMonDbmVc - calNoiseDbmVc;

  // compute corrected gains
  
  double receiverGainDbHc = _calib.getReceiverGainDbHc();
  double receiverGainDbVc = _calib.getReceiverGainDbVc();

  double corrGainDbHc = receiverGainDbHc + deltaNoiseDbHc;
  double corrGainDbVc = receiverGainDbVc + deltaNoiseDbVc;

  _noiseMonCalib.setNoiseDbmHc(_noiseMonDbmHc);
  _noiseMonCalib.setNoiseDbmVc(_noiseMonDbmVc);
  
  _noiseMonCalib.setReceiverGainDbHc(corrGainDbHc);
  _noiseMonCalib.setReceiverGainDbVc(corrGainDbVc);

  _noiseMonZdrm = _noiseMonZdr + _params.noise_mon_zdrm_corr;
  if (!std::isnan(_noiseMonSiteTempC)) {
    _noiseMonZdrm += ((_noiseMonSiteTempC - _params.noise_mon_mean_site_temp) *
                      _params.noise_mon_zdr_temp_slope);
  }
  _noiseMonCalib.setZdrCorrectionDb(_noiseMonZdrm * -1.0);
  
  _useNoiseMonCalib = true;
  
  if (_params.debug) {
    cerr << "Calibration::loadCal() - noise monitoring" << endl;
    cerr << "=========================================================" << endl;
    cerr << "noiseMonXml:" << endl;
    cerr << xml;
    cerr << "==>> _noiseMonZdr: " << _noiseMonZdr << endl;
    cerr << "==>> _noiseMonSiteTempC: " << _noiseMonSiteTempC << endl;
    cerr << "==>> _noiseMonZdrm: " << _noiseMonZdrm << endl;
    cerr << "==>> calNoiseDbmHc: " << calNoiseDbmHc << endl;
    cerr << "==>> calNoiseDbmVc: " << calNoiseDbmVc << endl;
    cerr << "==>> noiseMonDbmHc: " << _noiseMonDbmHc << endl;
    cerr << "==>> noiseMonDbmVc: " << _noiseMonDbmVc << endl;
    cerr << "==>> deltaNoiseDbHc: " << deltaNoiseDbHc << endl;
    cerr << "==>> deltaNoiseDbVc: " << deltaNoiseDbVc << endl;
    cerr << "==>> receiverGainDbHc: " << receiverGainDbHc << endl;
    cerr << "==>> receiverGainDbVc: " << receiverGainDbVc << endl;
    cerr << "==>> corrGainDbHc: " << corrGainDbHc << endl;
    cerr << "==>> corrGainDbVc: " << corrGainDbVc << endl;
    cerr << "==>> zdrm: " << _noiseMonZdrm << endl;
    cerr << "=========================================================" << endl;
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "****** CALIBRATION AFTER RX GAIN CORRECTION *************" << endl;
    _noiseMonCalib.print(cerr);
    cerr << "*********************************************************" << endl;
  }
  
  return 0;
  
}
  
/////////////////////////////////////////////////////////////////
// get value from XML string, given the tag list
// returns val, NAN on failure

double Calibration::_getValFromXml(const string &xml,
                                   const string &tagList) const
  
{
  
  // get tags in list
  
  vector<string> tags;
  TaStr::tokenize(tagList, "<>", tags);
  if (tags.size() == 0) {
    // no tags
    cerr << "WARNING - Calibration::_getValFromXml()" << endl;
    cerr << "  deltaGain tag not found: " << tagList << endl;
    return NAN;
  }
  
  // read through the outer tags in status XML
  
  string buf(xml);
  for (size_t jj = 0; jj < tags.size(); jj++) {
    string val;
    if (TaXml::readString(buf, tags[jj], val)) {
      cerr << "WARNING - Calibration::_getValFromXml()" << endl;
      cerr << "  Bad tags found in status xml, expecting: "
           << tagList << endl;
      return NAN;
    }
    buf = val;
  }

  // read delta gain
  
  double val = NAN;
  if (TaXml::readDouble(buf, val)) {
    cerr << "WARNING - Calibration::_getValFromXml()" << endl;
    cerr << "  Bad deltaGain found in status xml, buf: " << buf << endl;
    return NAN;
  }
  
  return val;

}
  
