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

#include "TempProfile.hh"
#include <toolsa/DateTime.hh>
#include <Spdb/DsSpdb.hh>
#include <Spdb/SoundingGet.hh>
#include <rapformats/Sndg.hh>
using namespace std;

// Constructor

TempProfile::TempProfile(const string progName,
                         const Params &params) :
        _progName(progName),
        _params(params)
  
{
  
}

// destructor

TempProfile::~TempProfile()

{

}

////////////////////////////////////////////////////////////////////////
// Get a valid temperature profile
// returns 0 on success, -1 on failure

int TempProfile::getTempProfile(time_t dataTime,
                                time_t &soundingTime,
                                vector<NcarParticleId::TmpPoint> &tmpProfile)

{

  int marginSecs = _params.sounding_search_time_margin_secs;
  time_t earliestTime = dataTime - marginSecs;
  time_t searchTime = dataTime;
  
  while (searchTime >= earliestTime) {

    // get a temperature profile
    if (_getTempProfile(searchTime, marginSecs)) {
      // failed - move back in time and try again
      searchTime -= 3600;
      continue;
    }
    
    // check temp profile for QC

    if (_checkTempProfile()) {
      // failed - move back in time and try again
      searchTime -= 3600;
      continue;
    }

    // accept the current profile

    soundingTime = _soundingTime;
    tmpProfile = _tmpProfile;
    
    return 0;
  
  } // while
  
  return -1;
  
}

////////////////////////////////////////////////////////////////////////
// get temp profile from first sounding before given time.
// returns 0 on success, -1 on failure

int TempProfile::_getTempProfile(time_t searchTime,
                                 int marginSecs)

{

  // read in sounding
  
  DsSpdb spdb;
  if (spdb.getFirstBefore(_params.sounding_spdb_url,
                          searchTime, marginSecs,
                          0, 0)) {
    cerr << "ERROR - getTempProfile()" << endl;
    cerr << "  Calling getFirstBefore for url: "
         << _params.sounding_spdb_url << endl;
    cerr << "  Search time: " << DateTime::strm(searchTime) << endl;
    cerr << "  margin (secs): " << marginSecs << endl;
    cerr << spdb.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "=======>> Got spdb sounding" << endl;
    cerr << "url: " << _params.sounding_spdb_url << endl;
    cerr << "Prod label: " << spdb.getProdLabel() << endl;
    cerr << "Prod id:    " << spdb.getProdId() << endl;
    cerr << "N Chunks:   " << spdb.getNChunks() << endl;
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
    if (_params.debug >= Params::DEBUG_NORM) {
      cerr << "INFO - overriding temp profile with sounding:" << endl;
      cerr << "  url: " << _params.sounding_spdb_url << endl;
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
        double htKm = dataPtr->altitude / 1000.0;
        double tempC = dataPtr->temp;
        NcarParticleId::TmpPoint tmpPt(pressHpa, htKm, tempC);
        _tmpProfile.push_back(tmpPt);
      }
      dataPtr++;
    }
    
  } else if (prodId == SPDB_SNDG_PLUS_ID) {
    
    Sndg sounding;
    if (sounding.disassemble(chunks[0].data, chunks[0].len)) {
      cerr << "ERROR - getTempProfile" << endl;
      cerr << "  Cannot disassemble chunk" << endl;
      cerr << "url: " << _params.sounding_spdb_url << endl;
      return -1;
    }
    
    if (_params.debug >= Params::DEBUG_NORM) {
      const Sndg::header_t &hdr = sounding.getHeader();
      cerr << "INFO - overriding temp profile with sounding:" << endl;
      cerr << "  url: " << _params.sounding_spdb_url << endl;
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
        NcarParticleId::TmpPoint tmpPt(pressHpa, htKm, tempC);
        _tmpProfile.push_back(tmpPt);
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

  if (_params.sounding_check_pressure_monotonically_decreasing) {
    
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
        cerr << "ERROR - checkTempProfile()" << endl;
        cerr << "Pressure not monotonically decreasing" << endl;
        cerr << "  index: " << jj << endl;
        cerr << "  prev pressure: " << prevPressure << endl;
        cerr << "  this pressure: " << pressure << endl;
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

  for (size_t ii = 1; ii < _tmpProfile.size(); ii++) {

    double press = _tmpProfile[ii].pressHpa;
    if (press < minPress) minPress = press;
    if (press > maxPress) maxPress = press;
      
    double htM = _tmpProfile[ii].htKm * 1000.0;
    if (htM < minHt) minHt = htM;
    if (htM > maxHt) maxHt = htM;
    
  }

  if (minPress > _params.sounding_required_pressure_range_hpa.min_val) {
    cerr << "ERROR - checkTempProfile()" << endl;
    cerr << "  Min pressure not low enough" << endl;
    cerr << "  Min pressure: " << minPress << endl;
    cerr << "  Should be below: "
         << _params.sounding_required_pressure_range_hpa.min_val << endl;
    return -1;
  }

  if (maxPress < _params.sounding_required_pressure_range_hpa.max_val) {
    cerr << "ERROR - checkTempProfile()" << endl;
    cerr << "  Max pressure not high enough" << endl;
    cerr << "  Max pressure: " << minPress << endl;
    cerr << "  Should be above: "
         << _params.sounding_required_pressure_range_hpa.max_val << endl;
    return -1;
  }

  if (minHt > _params.sounding_required_height_range_m.min_val) {
    cerr << "ERROR - checkTempProfile()" << endl;
    cerr << "  Min height not low enough" << endl;
    cerr << "  Min height: " << minHt << endl;
    cerr << "  Should be below: "
         << _params.sounding_required_height_range_m.min_val << endl;
    return -1;
  }

  if (maxHt < _params.sounding_required_height_range_m.max_val) {
    cerr << "ERROR - checkTempProfile()" << endl;
    cerr << "  Max height not high enough" << endl;
    cerr << "  Max height: " << minHt << endl;
    cerr << "  Should be above: "
         << _params.sounding_required_height_range_m.max_val << endl;
    return -1;
  }

  return 0;
  
}

