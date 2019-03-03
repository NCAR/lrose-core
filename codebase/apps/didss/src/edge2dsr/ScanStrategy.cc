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

#include "ScanStrategy.hh"
#include <math.h>
using namespace std;

ScanStrategy::ScanStrategy()
{
}

ScanStrategy::~ScanStrategy() 
{
}


/*************************************************************************
 * getTiltNum() - Returns the tilt number for the given elevation.  If the
 *                given elevation is outside of all of the tilts, returns
 *                -1.
 */

int ScanStrategy::getTiltNum(const double elevation) const
{
  double minDiff = 1000.0;
  double diff;

  int tilt_num;
  
  vector< double >::const_iterator elev_iter;
  int i;
  
  for (elev_iter = _elevations.begin(), i = 0;
       elev_iter != _elevations.end(); ++elev_iter, ++i)
  {
    diff = fabs(*elev_iter - elevation);
    
    if (diff < minDiff)
    {
      tilt_num = i;
      minDiff = diff;
    }
  } 

  return tilt_num;
}

