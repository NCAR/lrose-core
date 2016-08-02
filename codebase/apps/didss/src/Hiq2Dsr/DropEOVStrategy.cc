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
//////////////////////////////////////////////////////////////////////
// $Id: DropEOVStrategy.cc,v 1.3 2016/03/06 23:53:40 dixon Exp $
//
/////////////////////////////////////////////////////////////////////

#include <iostream>
#include "DropEOVStrategy.hh"
using namespace std;


DropEOVStrategy::DropEOVStrategy(const double elevation_drop) :
  EOVStrategy(),
  _elevationDrop(elevation_drop),
  _maxElevInVol(0.0),
  _prevElev(-1.0),
  _inElevDrop(false)
{
}

DropEOVStrategy::~DropEOVStrategy() 
{
}

bool DropEOVStrategy::isEndOfVolume(double elevation) 
{
  // See if we are still in the elevation drop.  Update the previous
  // elevation now since we don't use it again beyond this point.

  if (_inElevDrop &&
      _prevElev >= 0.0 &&
      elevation >= _prevElev)
    _inElevDrop = false;
  
  _prevElev = elevation;
  
  // Don't return an end-of-volume indication if we are still in the
  // drop between volumes

  if (_inElevDrop)
    return false;
  
  // Check for a new maximum elevation

  if (elevation > _maxElevInVol)
     _maxElevInVol = elevation;
     
  // Finally, see if we are starting the elevation drop

  if (_maxElevInVol - elevation >= _elevationDrop)
  {
    _maxElevInVol = 0.0;
    _inElevDrop = true;

    return true;
  }
  
  return false;
}
