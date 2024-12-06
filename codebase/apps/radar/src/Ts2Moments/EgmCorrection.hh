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

#ifndef EgmCorrection_HH
#define EgmCorrection_HH

#include <pthread.h>
#include "Params.hh"
#include <radar/Egm2008.hh>
using namespace std;

class EgmCorrection

{

public:

  ~EgmCorrection();
  
  ///////////////////////////////////////////////////////////
  // Inst() - Retrieve the singleton instance of this class.
  
  static EgmCorrection &inst();

  // set the parameters

  void setParams(const Params &params);

  // Load up EGM data from file, if the params indicate this.
  // Otherwise no action is taken.
  // Returns 0 on success, -1 on failure
  
  int loadEgmData();
  
  // get the egm correction in meters, based on lat/lon
  // will return 0 if egm data has not been loaded
  
  double getGeoidM(double lat, double lon) const;
  
private:

  // Singleton instance pointer

  static EgmCorrection *_instance;
  
  //////////////////////////////////////////////////////////////////
  // Constructor -- private because this is a singleton object

  EgmCorrection();

  // altitude correction

  Egm2008 _egm;

  // run-time parameters

  const Params *_params;

  // mutex for protecting _egm during reads

  static pthread_mutex_t _mutex;

};


#endif
