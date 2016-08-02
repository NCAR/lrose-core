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
/////////////////////////////////////////////////////////////
// AtmosAtten.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2012
//
///////////////////////////////////////////////////////////////
//
//
// Provide estimate of 2-way atmospheric attenuation for
// elev / range
//
////////////////////////////////////////////////////////////////

#ifndef AtmosAtten_H
#define AtmosAtten_H

#include <string>
#include <radar/NcarParticleId.hh>

using namespace std;

class AtmosAtten {
  
public:

  // constructor
  
  AtmosAtten();
  
  // destructor
  
  ~AtmosAtten();
  
  // set the atmospheric attenuation method
  // Reference: The CRPL method is detailed in Doviak and Zrnic,
  // Page 44.
  
  typedef enum {
    METHOD_NONE, METHOD_CONSTANT, METHOD_CRPL
  } atten_method_t;

  // set zero constant atmospheric attenuation
  // this sets the method to METHOD_NONE
  
  void setAttenNone();
  
  // set constant atmospheric attenuation in db/km
  // this sets the method to METHOD_CONSTANT
  
  void setAttenConstant(double val);
  
  // set method to CRPL - Central Radio Propagation Laboratory
  // See Doviak and Zrnic page 44
  
  void setAttenCrpl(double wavelengthCm);

  // get 2-way attenuation at an elevation and range
  
  double getAtten(double elevationDeg, double rangeKm) const;
  
  // print attenuation table
  
  void printTable(ostream &out) const;

protected:
private:

  static const int _tableNRange;
  static const int _tableNElev;
  static const double _tableDeltaRange;
  static const double _tableDeltaElev;

  double _tableMaxElev;
  double _tableMaxRange;

  atten_method_t _method;
  
  double _attenDbPerKm;  // db/km constant

  double _wavelengthCm;
  double _wavelengthCorrection; // relative to 10 cm

  double **_table;


  // methods


};


#endif
