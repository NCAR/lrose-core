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
// TempProfile.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2011
//
///////////////////////////////////////////////////////////////
//
// Gets temperature profile from sounding
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <radar/TempProfile.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaFile.hh>
#include <toolsa/TaStr.hh>
#include <Spdb/DsSpdb.hh>
#include <Spdb/SoundingGet.hh>
#include <rapformats/Sndg.hh>
#include <physics/thermo.h>
using namespace std;

const double TempProfile::missingValue = -9999.0;

// Constructor

TempProfile::TempProfile()
{

  _debug = false;
  _verbose = false;

  _soundingSearchTimeMarginSecs = 86400;

  _checkPressureRange = true;
  _soundingRequiredMinPressureHpa = 300;
  _soundingRequiredMaxPressureHpa = 950;

  _checkHeightRange = true;
  _soundingRequiredMinHeightM = 500;
  _soundingRequiredMaxHeightM = 15000;

  _checkTempRange = false;
  _soundingRequiredMinTempC = -5;
  _soundingRequiredMaxTempC = 5;

  _heightCorrectionKm = 0.0;
  _freezingLevel = TempProfile::missingValue;

  _checkPressureMonotonicallyDecreasing = false;
  _useWetBulbTemp = false;

  _lutByMeterHt.clear();
  _profile.clear();

}

// destructor

TempProfile::~TempProfile()

{

}

////////////////////////////////////////////////////////////////////////
// Load a valid temperature profile from SPDB
// returns 0 on success, -1 on failure
// on failure, tmpProfile will be empty

int TempProfile::loadFromSpdb(const string &url,
                              time_t dataTime,
                              time_t &soundingTime)

{

  _lutByMeterHt.clear();
  _profile.clear();
  _soundingSpdbUrl = url;
  time_t earliestTime = dataTime - _soundingSearchTimeMarginSecs;
  time_t searchTime = dataTime;
  
  if (_debug) {
    cerr << "Searching for sounding, dataTime: "
	 << DateTime::strm(dataTime) << endl;
    cerr << "  marginSecs: " << _soundingSearchTimeMarginSecs << endl;
  }

  while (searchTime >= earliestTime) {

    // get a temperature profile from spdb
    
    if (_getTempProfile(searchTime)) {
      // failed - move back in time and try again
      searchTime -= 3600;
      if (_debug) {
        cerr << "TempProfile::getTempProfile, url: " << url << endl;
        cerr << "ERROR - could not retrieve sounding" << endl;
        cerr << "  Moving back to search time: "
             << DateTime::strm(searchTime) << endl;
      }
      continue;
    }
    
    // check temp profile for QC

    if (_checkTempProfile()) {
      // failed - move back in time and try again
      searchTime -= 3600;
      if (_debug) {
        cerr << "TempProfile::getTempProfile, url: " << url << endl;
        cerr << "ERROR - check failed" << endl;
        cerr << "  Moving back to search time: "
             << DateTime::strm(searchTime) << endl;
      }
      continue;
    }

    // accept the current profile

    soundingTime = _soundingTime;
    
    if (_debug) {
      cerr << "TempProfile::getTempProfile, url: " << url << endl;
      cerr << "  Got profile at time: "
           << DateTime::strm(_soundingTime) << endl;
    }

    // compute the freezing level

    _computeFreezingLevel();

    // success
    
    return 0;
  
  } // while
  
  // failure

  _profile.clear();
  return -1;
  
}

////////////////////////////////////////////////////////////////////////
// Load from a PID thresholds file
// returns 0 on success, -1 on failure

int TempProfile::loadFromPidThresholdsFile(const string &pidThresholdsPath)
  
{
  
  _lutByMeterHt.clear();
  _profile.clear();
  _soundingSpdbUrl = pidThresholdsPath;
  
  if (_debug) {
    cerr << "Reading temperatures from threshold file: "
         << pidThresholdsPath << endl;
  }

  TaFile inFile;
  FILE *in = inFile.fopen(pidThresholdsPath.c_str(), "r");
  if (in == NULL) {
    int errNum = errno;
    cerr << "ERROR - TempProfile::getProfileForPid" << endl;
    cerr << "  Cannot open PID thresholds file for reading" << endl;
    cerr << "  File path: " << pidThresholdsPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // in file, look for 'tpf' line and decode it

  char line[8192];
  while (!feof(in)) {
    if (fgets(line, 8192, in) == NULL) {
      break;
    }
    // ignore comments
    if (line[0] == '#') {
      continue;
    }
    if (strlen(line) < 2) {
      continue;
    }
    if (_verbose) {
      cerr << line;
    }
    // force lower case
    for (int ii = 0; ii < (int) strlen(line); ii++) {
      line[ii] = tolower(line[ii]);
    }
    // check for temperature line
    if (strncmp(line, "tpf", 3) == 0) {
      if (_setTempProfileFromPidLine(line)) {
        cerr << "ERROR - TempProfile::getProfileForPid" << endl;
        cerr << "  Cannot set profile from tpf line" << endl;
        cerr << "  Path: " << pidThresholdsPath << endl;
        cerr << "  Line: " << line << endl;
        return -1;
      }
    }
  } // while (!feof(in) ...
  inFile.fclose();

  // compute the freezing level

  _computeFreezingLevel();

  return 0;
  
}

/////////////////////////////////////
// prepare profile for use

void TempProfile::prepareForUse() {

  // compute the freezing level

  _computeFreezingLevel();

}

/////////////////////////////////////
// get the freezing level height

double TempProfile::getFreezingLevel() const {

  return _freezingLevel;

}

/////////////////////////////////////////////////////
// set the temperature profile from PID line

int TempProfile::_setTempProfileFromPidLine(const char *line)
  
{

  // find the first and last paren

  const char *firstOpenParen = strchr(line, '(');
  const char *lastCloseParen = strrchr(line, ')');

  if (firstOpenParen == NULL || lastCloseParen == NULL) {
    return -1;
  }
  
  string sdata(firstOpenParen, lastCloseParen - firstOpenParen + 1);

  // tokenize the line on '.'

  vector<string> toks;
  TaStr::tokenize(sdata, "()", toks);
  if (toks.size() < 2) {
    return -1;
  }

  // scan in profile data

  for (int ii = 0; ii < (int) toks.size(); ii++) {
    double ht, tmp;
    if (sscanf(toks[ii].c_str(), "%lg,%lg", &ht, &tmp) == 2) {
      PointVal tmpPt(ht, tmp);
      _profile.push_back(tmpPt); 
    }
  }

  return 0;

}

/////////////////////////////////////////////////////
// set the full profile

void TempProfile::setProfile(const vector<PointVal> &profile)
{
  _profile = profile;
}
  
////////////////////////////////////////////////////////////////////////
// get temp profile from first sounding before given time.
// returns 0 on success, -1 on failure

int TempProfile::_getTempProfile(time_t searchTime)

{

  // read in sounding

  int dataType = 0;
  if (_soundingLocationName.size() > 0) {
    dataType = Spdb::hash4CharsToInt32(_soundingLocationName.c_str());
  }

  DsSpdb spdb;
  if (spdb.getClosest(_soundingSpdbUrl,
                      searchTime, _soundingSearchTimeMarginSecs,
                      dataType, 0)) {
    cerr << "ERROR - getTempProfile()" << endl;
    cerr << "  Calling getFirstBefore for url: "
         << _soundingSpdbUrl << endl;
    cerr << "  Search time: " << DateTime::strm(searchTime) << endl;
    cerr << "  margin (secs): " << _soundingSearchTimeMarginSecs << endl;
    cerr << spdb.getErrStr() << endl;
    return -1;
  }
  
  if (_verbose) {
    cerr << "=======>> Got spdb sounding" << endl;
    cerr << "  Search time: " << DateTime::strm(searchTime) << endl;
    cerr << "  margin (secs): " << _soundingSearchTimeMarginSecs << endl;
    cerr << "  url: " << _soundingSpdbUrl << endl;
    cerr << "  Prod label: " << spdb.getProdLabel() << endl;
    cerr << "  Prod id:    " << spdb.getProdId() << endl;
    cerr << "  N Chunks:   " << spdb.getNChunks() << endl;
  }
  
  const vector<Spdb::chunk_t> &chunks = spdb.getChunks();
  int nChunks = (int) chunks.size();
  if (nChunks < 1) {
    cerr << "ERROR - getTempProfile()" << endl;
    cerr << "  No sounding, time: " << DateTime::strm(searchTime) << endl;
    cerr << "               name: " << _soundingLocationName << endl;
    return -1;
  }

  _profile.clear();
  int prodId = spdb.getProdId();
  _soundingTime = 0;
  
  if (prodId == SPDB_SNDG_ID) {

    // older type sounding
    
    SNDG_spdb_product_t *sounding = (SNDG_spdb_product_t *) chunks[0].data;
    SNDG_spdb_product_from_BE(sounding);
    if (_debug) {
      cerr << "INFO - overriding temp profile with sounding:" << endl;
      cerr << "  url: " << _soundingSpdbUrl << endl;
      cerr << "  launchTime: " << DateTime::strm(sounding->launchTime) << endl; 
      cerr << "  nPoints: " << sounding->nPoints << endl;
      cerr << "  site: " << sounding->siteName << endl;
    }
    
    _soundingTime = sounding->launchTime;
    
    int dataOffset =
      sizeof(SNDG_spdb_product_t) - sizeof(SNDG_spdb_point_t);
    SNDG_spdb_point_t *dataPtr = 
      (SNDG_spdb_point_t *) ((char *) sounding + dataOffset);
    for(int ipoint = 0; ipoint < sounding->nPoints; ipoint++ ) {
        
      if (dataPtr->pressure > -999 &&
          dataPtr->altitude > -999 &&
          dataPtr->temp > -999) {
        double pressHpa = dataPtr->pressure;
        double htKm = dataPtr->altitude / 1000.0 + _heightCorrectionKm;
        double tempC = dataPtr->temp;
        double rh = dataPtr->rh;
        if (_useWetBulbTemp && rh >= 0) {
          double dewptC = PHYrhdp(tempC, rh);
          double tWet = PHYtwet(pressHpa, tempC, dewptC);
          PointVal tmpPt(pressHpa, htKm, tWet);
          _profile.push_back(tmpPt);
        } else {
          PointVal tmpPt(pressHpa, htKm, tempC);
          _profile.push_back(tmpPt);
        }
      }
      dataPtr++;
    }
    
  } else if (prodId == SPDB_SNDG_PLUS_ID) {
    
    Sndg sounding;
    if (sounding.disassemble(chunks[0].data, chunks[0].len)) {
      cerr << "ERROR - getTempProfile" << endl;
      cerr << "  Cannot disassemble chunk" << endl;
      cerr << "url: " << _soundingSpdbUrl << endl;
      return -1;
    }
    
    if (_debug) {
      const Sndg::header_t &hdr = sounding.getHeader();
      cerr << "INFO - overriding temp profile with sounding:" << endl;
      cerr << "  url: " << _soundingSpdbUrl << endl;
      cerr << "  launchTime: " << DateTime::strm(hdr.launchTime) << endl; 
      cerr << "  nPoints: " << hdr.nPoints << endl;
      cerr << "  site: " << hdr.siteName << endl;
    } 

    _soundingTime = sounding.getHeader().launchTime;

    const vector<Sndg::point_t> &pts = sounding.getPoints();
    for (size_t ii = 0; ii < pts.size(); ii++) {
      const Sndg::point_t &pt = pts[ii];
      if (pt.pressure > -9999 &&
          pt.altitude > -999 &&
          pt.temp > -999) {
        double pressHpa = pt.pressure;
        double htKm = pt.altitude / 1000.0;
        double tempC = pt.temp;
        double dewptC = pt.dewpt;
        if (_useWetBulbTemp && dewptC > -999) {
          double tWet = PHYtwet(pressHpa, tempC, dewptC);
          PointVal tmpPt(pressHpa, htKm, tWet);
          _profile.push_back(tmpPt);
        } else {
          PointVal tmpPt(pressHpa, htKm, tempC);
          _profile.push_back(tmpPt);
        }
      }
    }

  }

  return 0;
  
}

////////////////////////////////////////////////////////////////////////
// check the temperature profile for quality
// returns 0 on success, -1 on failure

int TempProfile::_checkTempProfile()

{

  // should have at least 5 points

  if (_profile.size() < 5) {
    if (_debug) {
      cerr << "WARNING - checkTempProfile()" << endl;
      cerr << "  Too few points in profile: " << _profile.size() << endl;
    }
    return -1;
  }

  // check for monotonically decreasing pressure

  if (_checkPressureMonotonicallyDecreasing) {
    
    double prevPressure = _profile[0].getPressHpa();
    size_t nSteps = 20;
    size_t intv = _profile.size() / nSteps;
    for (size_t ii = 1; ii < nSteps; ii++) {
      size_t jj = ii * intv; 
      if (jj > _profile.size() - 1) {
        jj = _profile.size() - 1;
      }
      double pressure = _profile[jj].getPressHpa();
      if (pressure > prevPressure) {
        if (_debug) {
          cerr << "WARNING - checkTempProfile()" << endl;
          cerr << "Pressure not monotonically decreasing" << endl;
          cerr << "  index: " << jj << endl;
          cerr << "  prev pressure: " << prevPressure << endl;
          cerr << "  this pressure: " << pressure << endl;
        }
        return -1;
      }
      if (jj == _profile.size() - 1) {
        break;
      }
      prevPressure = pressure;
    }

  }

  // check pressure and height ranges

  double minHt = 1.0e99;
  double maxHt = -1.0e99;
  double minPress = 1.0e99;
  double maxPress = -1.0e99;
  double minTemp = 1.0e99;
  double maxTemp = -1.0e99;

  for (size_t ii = 1; ii < _profile.size(); ii++) {

    double press = _profile[ii].getPressHpa();
    if (press < minPress) minPress = press;
    if (press > maxPress) maxPress = press;
      
    double htM = _profile[ii].getHtKm() * 1000.0;
    if (htM < minHt) minHt = htM;
    if (htM > maxHt) maxHt = htM;
    
    double tempC = _profile[ii].getTmpC();
    if (tempC < minTemp) minTemp = tempC;
    if (tempC > maxTemp) maxTemp = tempC;
    
  }

  if (_checkPressureRange) {
    if (minPress > _soundingRequiredMinPressureHpa) {
      if (_debug) {
	cerr << "WARNING - checkTempProfile()" << endl;
	cerr << "  Min pressure not low enough" << endl;
	cerr << "  Min pressure: " << minPress << endl;
	cerr << "  Should be below: "
	     << _soundingRequiredMinPressureHpa << endl;
      }
      return -1;
    }
    if (maxPress < _soundingRequiredMaxPressureHpa) {
      if (_debug) {
	cerr << "WARNING - checkTempProfile()" << endl;
	cerr << "  Max pressure not high enough" << endl;
	cerr << "  Max pressure: " << maxPress << endl;
	cerr << "  Should be above: "
	     << _soundingRequiredMaxPressureHpa << endl;
      }
      return -1;
    }
  } // if (_checkPressureRange)
  
  if (_checkHeightRange) {
    if (minHt > _soundingRequiredMinHeightM) {
      if (_debug) {
	cerr << "WARNING - checkTempProfile()" << endl;
	cerr << "  Min height not low enough" << endl;
	cerr << "  Min height: " << minHt << endl;
	cerr << "  Should be below: "
	     << _soundingRequiredMinHeightM << endl;
      }
      return -1;
    }
    if (maxHt < _soundingRequiredMaxHeightM) {
      if (_debug) {
	cerr << "WARNING - checkTempProfile()" << endl;
	cerr << "  Max height not high enough" << endl;
	cerr << "  Max height: " << maxHt << endl;
	cerr << "  Should be above: "
	     << _soundingRequiredMaxHeightM << endl;
      }
      return -1;
    }
  } // if (_checkHeightRange)

  if (_checkTempRange) {
    if (minHt > _soundingRequiredMinTempC) {
      if (_debug) {
	cerr << "WARNING - checkTempProfile()" << endl;
	cerr << "  Min temp not low enough" << endl;
	cerr << "  Min temp: " << minHt << endl;
	cerr << "  Should be below: "
	     << _soundingRequiredMinTempC << endl;
      }
      return -1;
    }
    if (maxHt < _soundingRequiredMaxTempC) {
      if (_debug) {
	cerr << "WARNING - checkTempProfile()" << endl;
	cerr << "  Max temp not high enough" << endl;
	cerr << "  Max temp: " << maxHt << endl;
	cerr << "  Should be above: "
	     << _soundingRequiredMaxTempC << endl;
      }
      return -1;
    }
  } // if (_checkTempRange)

  return 0;
  
}

////////////////////////////////////////////////////////////////////////
// compute the freezing level in km

void TempProfile::_computeFreezingLevel()

{
  _freezingLevel = getHtKmForTempC(0.0);
}

///////////////////////////////////////////////
// get height for a given temp
// returns missingVal if no temp profile available

double TempProfile::getHtKmForTempC(double tempC) const

{

  for (size_t ii = 1; ii < _profile.size(); ii++) {

    double ht1 = _profile[ii-1].getHtKm();
    double ht2 = _profile[ii].getHtKm();
    double tmp1 = _profile[ii-1].getTmpC();
    double tmp2 = _profile[ii].getTmpC();
    double dtmp1 = tmp1 - tempC;
    double dtmp2 = tmp2 - tempC;
    
    if (dtmp1 * dtmp2 <= 0) {
      
      // change in sign, so straddles desired temperature level
      
      double fraction = dtmp1 / (dtmp1 - dtmp2);
      double dht = fraction * (ht2 - ht1);

      return (ht1 + dht);
      
    }

  } // ii

  return TempProfile::missingValue;

}

///////////////////////////////////////////////
// get temperature at a given height
// returns missingValue if no temp profile available

double TempProfile::getTempForHtKm(double htKm) const

{

  // create LUT by height if it does not already exist
  
  if (_lutByMeterHt.size() < 1) {
    _createLutByMeterHt();
  }

  // check LUT - if zero size then return missing

  if (_lutByMeterHt.size() < 1) {
    return TempProfile::missingValue;
  }

  // get temp for requested height

  int htMeters = (int) (htKm * 1000.0 + 0.5);
  
  if (htMeters <= _tmpMinHtMeters) {
    return _tmpBottomC;
  } else if (htMeters >= _tmpMaxHtMeters) {
    return _tmpTopC;
  }

  int kk = htMeters - _tmpMinHtMeters;
  return _lutByMeterHt[kk];

}

//////////////////////////////////////////////////////////////////
// create a lookup table by height
// one entry per meter

void TempProfile::_createLutByMeterHt() const

{

  _lutByMeterHt.clear();
  if (_profile.size() < 1) {
    return;
  }
  
  _tmpMinHtMeters =
    (int) (_profile[0].getHtKm() * 1000.0 + 0.5);
  _tmpMaxHtMeters =
    (int) (_profile[_profile.size()-1].getHtKm() * 1000.0 + 0.5);

  _tmpBottomC = _profile[0].getTmpC();
  _tmpTopC = _profile[_profile.size()-1].getTmpC();

  // fill out temp array, every meter

  int nHt = (_tmpMaxHtMeters - _tmpMinHtMeters) + 1;
  _lutByMeterHt.resize(nHt);
  
  for (int ii = 1; ii < (int) _profile.size(); ii++) {

    int minHtMeters = (int) (_profile[ii-1].getHtKm() * 1000.0 + 0.5);
    double minTmp = _profile[ii-1].getTmpC();

    int maxHtMeters = (int) (_profile[ii].getHtKm() * 1000.0 + 0.5);
    double maxTmp = _profile[ii].getTmpC();

    double deltaMeters = maxHtMeters - minHtMeters;
    double deltaTmp = maxTmp - minTmp;
    double gradient = deltaTmp / deltaMeters;
    double tmp = minTmp;
    int kk = minHtMeters - _tmpMinHtMeters;
    
    for (int jj = minHtMeters; jj <= maxHtMeters; jj++, kk++, tmp += gradient) {
      if (kk >= 0 && kk < nHt) {
	_lutByMeterHt[kk] = tmp;
      }
    }
    
  } // ii

}

//////////////////////////////////////////////////////////////
// PointVal interior class

// Constructors

TempProfile::PointVal::PointVal()

{

  pressHpa = TempProfile::missingValue;
  htKm = TempProfile::missingValue;
  tmpC = TempProfile::missingValue;
  rhPercent = TempProfile::missingValue;

}

TempProfile::PointVal::PointVal(double ht, double tmp)

{

  pressHpa = TempProfile::missingValue;
  htKm = ht;
  tmpC = tmp;
  rhPercent = TempProfile::missingValue;

}

TempProfile::PointVal::PointVal(double press, double ht, double tmp)

{

  pressHpa = press;
  htKm = ht;
  tmpC = tmp;
  rhPercent = TempProfile::missingValue;

}

/////////////////////////////////////////////////////////
// destructor

TempProfile::PointVal::~PointVal()

{

}

/////////////////////////////////////////////////////////
// print

void TempProfile::PointVal::print(ostream &out) const

{

  out << "---- Temp point ----" << endl;
  out << "  Pressure Hpa: " << getPressHpa() << endl;
  out << "  Height Km: " << getHtKm() << endl;
  out << "  Temp    C: " << getTmpC() << endl;
  out << "  RH      %: " << getRhPercent() << endl;
  out << "------------------" << endl;

}

/////////////////////////////////////////////////////////
// print

void TempProfile::print(ostream &out) const

{

  out << "======= Temperature Profile =========" << endl;

  if (_soundingTime != 0) {
    out << "  soundingTime: " << DateTime::strm(_soundingTime) << endl;
  }
  if (_soundingSpdbUrl.size() > 0) {
    out << "  soundingUrl: " << _soundingSpdbUrl << endl;
  }
  if (_soundingLocationName.size() > 0) {
    out << "  soundingLocationName: " << _soundingLocationName << endl;
  }
  out << "  freezingLevel? " << _freezingLevel << endl;
  if (_useWetBulbTemp) {
    out << "  using wet bulb temp" << endl;
  }
  if (_heightCorrectionKm != 0) {
    out << "  heightCorrectionKm: " << _heightCorrectionKm << endl;
  }

  int nLevels = (int) _profile.size();
  int nPrint = 50;
  int printInterval = nLevels / nPrint;
  if (nLevels < nPrint) {
    printInterval = 1;
  }
  for (size_t ii = 0; ii < _profile.size(); ii++) {
    bool doPrint = false;
    if (ii % printInterval == 0) {
      doPrint = true;
    }
    if (ii < _profile.size() - 1) {
      if (_profile[ii].getTmpC() * _profile[ii].getTmpC() <= 0) {
        // always print freezing level
        doPrint = true;
      }
    }
    if (ii > 0) {
      if (_profile[ii-1].getTmpC() * _profile[ii].getTmpC() <= 0) {
        doPrint = true;
      }
    }
    if (doPrint) {
      out << "  ilevel, press(Hpa), alt(km), temp(C), RH(%): " << ii << ", "
          << _profile[ii].getPressHpa() << ", "
          << _profile[ii].getHtKm() << ", "
          << _profile[ii].getTmpC() << ", "
          << _profile[ii].getRhPercent() << endl;
    }
  }
  out << "=====================================" << endl;
}

