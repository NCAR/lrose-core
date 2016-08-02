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
// GridAdvect.hh
//
// GridAdvect class
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2001
//
///////////////////////////////////////////////////////////////

#ifndef GridAdvect_H
#define GridAdvect_H

#include <string>

#include <advect/Advector.hh>
#include <euclid/GridTemplate.hh>
#include <euclid/Pjg.hh>
#include <dataport/port_types.h>

class GridAdvect
{
  
public:

  // constructor

  GridAdvect(const double image_val_min = 0.0,
	     const double image_val_max = 0.0,
	     const bool debug_flag = false);
  
  // destructor
  
  virtual ~GridAdvect();

  // compute the forecast grid
  //
  // Returns true on success, false on failure

  bool compute(Advector &advector,
	       const Pjg &projection,
	       const fl32 *image_data,
	       const fl32 missing_data_value);

  // Retrieve the forecast data

  const fl32 *getForecastData() const
  {
    return _forecastData;
  }
  
protected:
  
private:

  bool _debugFlag;
  
  double _imageValMin;
  double _imageValMax;
  bool   _checkImageValues;
  
  Pjg _forecastProj;
  fl32 *_forecastData;
  
};

#endif

