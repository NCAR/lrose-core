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
// EgmCorrection.hh
//
// Correct georef height for EGM
//
// See:
//  https://earth-info.nga.mil/GandG/wgs84/gravitymod/egm2008/egm08_wgs84.html
//
// EOL, NCAR, Boulder CO
//
// Nov 2019
//
// Mike Dixon
//
//////////////////////////////////////////////////////////////////////////
//
// This is implemented as a singleton.
//
//////////////////////////////////////////////////////////////////////////

#include "EgmCorrection.hh"

using namespace std;

pthread_mutex_t EgmCorrection::_mutex = PTHREAD_MUTEX_INITIALIZER;

// Global variables - instance

EgmCorrection *EgmCorrection::_instance = NULL;

//////////////////////////////////////////////////////////////////////////
// Constructor - private, called by inst()

EgmCorrection::EgmCorrection()
{
  _params = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Destructor

EgmCorrection::~EgmCorrection()
{

}

//////////////////////////////////////////////////////////////////////////
// Inst() - Retrieve the singleton instance of this class.

EgmCorrection &EgmCorrection::inst()
{

  if (_instance == (EgmCorrection *)NULL) {
    _instance = new EgmCorrection;
  }

  return *_instance;
}

//////////////////////////////////////////////////////////////////////////
// set the parameters

void EgmCorrection::setParams(const Params &params)
{
  _params = &params;

}

//////////////////////////////////////////////////////////////////////////
// Load up EGM data from file
//
// Returns 0 on success, -1 on failure

int EgmCorrection::loadEgmData()
{
  if (_params == NULL) {
    cerr << "ERROR - EgmCorrection::loadEgmData()" << endl;
    cerr << "  parameters not yet set" << endl;
    return -1;
  }
  if (_params->debug >= Params::DEBUG_VERBOSE) {
    _egm.setDebug();
  }
  if (_params->correct_altitude_for_egm) {
    if (_egm.readGeoid(_params->egm_2008_geoid_file)) {
      cerr << "ERROR - EgmCorrection::loadEgmData()" << endl;
      cerr << "  Altitude correction for geoid." << endl;
      cerr << "  Problem reading geoid file: " 
           << _params->egm_2008_geoid_file << endl;
      return -1;
    }
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////////
// get the geoid height in meters
//
// Returns 0 if geoid data has not been read

double EgmCorrection::getGeoidM(double lat, double lon) const
{
  pthread_mutex_lock(&_mutex);
  double geoidM = _egm.getInterpGeoidM(lat, lon);
  pthread_mutex_unlock(&_mutex);
  return geoidM;
}

