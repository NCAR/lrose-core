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
////////////////////////////////////////////////////////////////////////////////
//
// Mike Dixon, EOL, NCAR, Boulder, CO, 80307, USA
// Jan 2013
//
// Based on IWRF message structs - will be merged into IWRF moments later
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _DS_PLATFORM_GEOREF_INC_
#define _DS_PLATFORM_GEOREF_INC_

#include <string>
#include <ctime>
#include <rapformats/ds_radar.h>
using namespace std;

class DsPlatformGeoref
{

public:
  
  DsPlatformGeoref();
  DsPlatformGeoref(const DsPlatformGeoref &rhs) { copy(rhs); }
  ~DsPlatformGeoref();
  
  DsPlatformGeoref& operator=(const DsPlatformGeoref &rhs);
  
  bool operator==(const DsPlatformGeoref &rhs) const;
  bool operator!=(const DsPlatformGeoref &rhs) const
  { return !operator==(rhs); }
  
  void copy(const DsPlatformGeoref &rhs);
  void clear();
  void print(FILE *out=stdout) const;
  void print(ostream &out) const;
  
  void toBe(ds_iwrf_platform_georef_t &val);
  void fromBe(ds_iwrf_platform_georef_t &val);

  void decode(const ds_iwrf_platform_georef_t &val);
  void encode(ds_iwrf_platform_georef_t &val);

  const ds_iwrf_platform_georef_t &getGeoref() const { return _georef; }
  void setGeoref(const ds_iwrf_platform_georef_t &val) { _georef = val; }
  
protected:
private:

  ds_iwrf_platform_georef_t _georef;

};

#endif
