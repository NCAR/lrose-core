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
//////////////////////////////////////////////////////////////////////////
// Sounding.cc: helper objects for moments computations
//
// RAP, NCAR, Boulder CO
//
// April 2010
//
// Mike Dixon
//
//////////////////////////////////////////////////////////////////////////
//
// This is implemented as a singleton.
//
//////////////////////////////////////////////////////////////////////////


#include "Sounding.hh"
#include <cstdio>
#include <cassert>
#include <iostream>
#include <toolsa/DateTime.hh>

using namespace std;

// Global variables - instance

Sounding *Sounding::_instance = NULL;

//////////////////////////////////////////////////////////////////////////
// Constructor - private, called by inst()

Sounding::Sounding()
{
  _params = NULL;
  _tempProfile.clear();
}

//////////////////////////////////////////////////////////////////////////
// Destructor

Sounding::~Sounding()
{

}

//////////////////////////////////////////////////////////////////////////
// Inst() - Retrieve the singleton instance of this class.

Sounding &Sounding::inst()
{

  if (_instance == (Sounding *)NULL) {
    _instance = new Sounding();
  }

  return *_instance;

}

///////////////////////////////////////////////////
// retrieve temperature profile for specified time
// returns 0 on success, -1 on failure
  
int Sounding::retrieveTempProfile(time_t profileTime)
  
{

  if (_params->debug >= Params::DEBUG_VERBOSE) {
    cerr << "Getting temp profile for time: " 
         << DateTime::strm(profileTime) << endl;
  }

  assert(_params != NULL);
  
  time_t retrievedTime = profileTime;
  _tempProfile.clear();

  if (_params->sounding_mode == Params::READ_SOUNDING_FROM_SPDB) {
    
    _tempProfile.setSoundingLocationName
      (_params->sounding_location_name);
    _tempProfile.setSoundingSearchTimeMarginSecs
      (_params->sounding_search_time_margin_secs);
    
    _tempProfile.setCheckPressureRange
      (_params->sounding_check_pressure_range);
    _tempProfile.setSoundingRequiredMinPressureHpa
      (_params->sounding_required_pressure_range_hpa.min_val);
    _tempProfile.setSoundingRequiredMaxPressureHpa
      (_params->sounding_required_pressure_range_hpa.max_val);
    
    _tempProfile.setCheckHeightRange
      (_params->sounding_check_height_range);
    _tempProfile.setSoundingRequiredMinHeightM
      (_params->sounding_required_height_range_m.min_val);
    _tempProfile.setSoundingRequiredMaxHeightM
      (_params->sounding_required_height_range_m.max_val);
    
    _tempProfile.setCheckPressureMonotonicallyDecreasing
      (_params->sounding_check_pressure_monotonically_decreasing);
    
    if (_params->debug >= Params::DEBUG_EXTRA) {
      _tempProfile.setVerbose();
    }
    if (_params->debug >= Params::DEBUG_VERBOSE) {
      _tempProfile.setDebug();
    }
  
    if (_tempProfile.loadFromSpdb(_params->sounding_spdb_url,
                                  profileTime,
                                  retrievedTime)) {
      cerr << "ERROR - Sounding::retrieveTempProfile" << endl;
      cerr << "  Cannot retrive profile for time: "
           << DateTime::strm(profileTime) << endl;
      cerr << "  url: " << _params->sounding_spdb_url << endl;
      cerr << "  station name: " << _params->sounding_location_name << endl;
      cerr << "  time margin secs: "
           << _params->sounding_search_time_margin_secs << endl;
      return -1;
    }
    
    if (_params->debug) {
      cerr << "=====================================" << endl;
      cerr << "Got temp profile, URL: " << _params->sounding_spdb_url << endl;
      cerr << "Overriding temperature profile" << endl;
      cerr << "  vol time: " << DateTime::strm(profileTime) << endl;
      cerr << "  retrievedTime: " << DateTime::strm(retrievedTime) << endl;
      cerr << "  freezingLevel: " << _tempProfile.getFreezingLevel() << endl;
    }

  } else {
    
    // set profile from param file

    _tempProfile.clear();
    for (int ii = 0; ii < _params->specified_sounding_n; ii++) {
      const Params::sounding_entry_t &entry = 
        _params->_specified_sounding[ii];
      TempProfile::PointVal point;
      point.setHtKm(entry.height_m / 1000.0);
      point.setTmpC(entry.temp_c);
      if (entry.pressure_hpa >= 0) {
        point.setPressHpa(entry.pressure_hpa);
      }
      if (entry.rh_percent >= 0) {
        point.setRhPercent(entry.rh_percent);
      }
      _tempProfile.addPoint(point);
    } // ii
    _tempProfile.prepareForUse();

  }

  if (_params->debug >= Params::DEBUG_VERBOSE) {
    _tempProfile.print(cerr);
  }
  
  return 0;

}

