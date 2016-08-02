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

#include <radar/TempProfile.hh>
#include <toolsa/DateTime.hh>
#include <Spdb/DsSpdb.hh>
#include <Spdb/SoundingGet.hh>
#include <rapformats/Sndg.hh>
#include <physics/thermo.h>
using namespace std;

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
  _freezingLevel = -9999.0;

  _checkPressureMonotonicallyDecreasing = false;
  _useWetBulbTemp = false;

  _tmpProfile.clear();

}

// destructor

TempProfile::~TempProfile()

{

}

////////////////////////////////////////////////////////////////////////
// Get a valid temperature profile
// returns 0 on success, -1 on failure
// on failure, tmpProfile will be empty

int TempProfile::getTempProfile(const string &url,
                                time_t dataTime,
                                time_t &soundingTime,
                                vector<NcarParticleId::TmpPoint> &tmpProfile)

{

  _tmpProfile.clear();
  _soundingSpdbUrl = url;
  time_t earliestTime = dataTime - _soundingSearchTimeMarginSecs;
  time_t searchTime = dataTime;

  if (_debug) {
    cerr << "Searching for sounding, dataTime: "
	 << DateTime::strm(dataTime) << endl;
  }

  while (searchTime >= earliestTime) {

    // get a temperature profile
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
    tmpProfile = _tmpProfile;

    if (_debug) {
      cerr << "TempProfile::getTempProfile, url: " << url << endl;
      cerr << "  Got profile at time: "
           << DateTime::strm(_soundingTime) << endl;
    }
    
    _computeFreezingLevel();

    return 0;
  
  } // while
  
  _tmpProfile.clear();
  tmpProfile = _tmpProfile;
  return -1;
  
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
  
  if (_debug) {
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
    cerr << "  Bad sounding" << endl;
    return -1;
  }

  _tmpProfile.clear();
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
          NcarParticleId::TmpPoint tmpPt(pressHpa, htKm, tWet);
          _tmpProfile.push_back(tmpPt);
        } else {
          NcarParticleId::TmpPoint tmpPt(pressHpa, htKm, tempC);
          _tmpProfile.push_back(tmpPt);
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
          NcarParticleId::TmpPoint tmpPt(pressHpa, htKm, tWet);
          _tmpProfile.push_back(tmpPt);
        } else {
          NcarParticleId::TmpPoint tmpPt(pressHpa, htKm, tempC);
          _tmpProfile.push_back(tmpPt);
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

  // should have at least 20 points

  if (_tmpProfile.size() < 20) {
    return -1;
  }

  // check for monotonically decreasing pressure

  if (_checkPressureMonotonicallyDecreasing) {
    
    double prevPressure = _tmpProfile[0].pressHpa;
    size_t nSteps = 20;
    size_t intv = _tmpProfile.size() / nSteps;
    for (size_t ii = 1; ii < nSteps; ii++) {
      size_t jj = ii * intv; 
      if (jj > _tmpProfile.size() - 1) {
        jj = _tmpProfile.size() - 1;
      }
      double pressure = _tmpProfile[jj].pressHpa;
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
      if (jj == _tmpProfile.size() - 1) {
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

  for (size_t ii = 1; ii < _tmpProfile.size(); ii++) {

    double press = _tmpProfile[ii].pressHpa;
    if (press < minPress) minPress = press;
    if (press > maxPress) maxPress = press;
      
    double htM = _tmpProfile[ii].htKm * 1000.0;
    if (htM < minHt) minHt = htM;
    if (htM > maxHt) maxHt = htM;
    
    double tempC = _tmpProfile[ii].tmpC;
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
	cerr << "  Max pressure: " << minPress << endl;
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
	cerr << "  Max height: " << minHt << endl;
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
	cerr << "  Max temp: " << minHt << endl;
	cerr << "  Should be above: "
	     << _soundingRequiredMaxTempC << endl;
      }
      return -1;
    }
  } // if (_checkTempRange)

  return 0;
  
}

////////////////////////////////////////////////////////////////////////
// compute the freezing level

void TempProfile::_computeFreezingLevel()

{

  _freezingLevel = -9999.0;

  for (size_t ii = 1; ii < _tmpProfile.size(); ii++) {

    double tmp1 = _tmpProfile[ii-1].tmpC;
    double tmp2 = _tmpProfile[ii].tmpC;
    double ht1 = _tmpProfile[ii-1].htKm;
    double ht2 = _tmpProfile[ii].htKm;

    if (tmp1 * tmp2 <= 0) {
      
      // change in sign, so straddles freezing level
      
      double fraction = tmp1 / (tmp1 - tmp2);
      double dht = fraction * (ht2 - ht1);
      _freezingLevel = ht1 + dht;

      return;

    }

  } // ii

}

