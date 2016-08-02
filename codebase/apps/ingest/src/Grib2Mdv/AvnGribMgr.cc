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
///////////////////////////////////////////////
// AvnGribMgr
//
//////////////////////////////////////////////


#include "AvnGribMgr.hh"

#include <toolsa/pmu.h>
using namespace std;


AvnGribMgr::AvnGribMgr()
:GribMgr()
{
}

AvnGribMgr::~AvnGribMgr() 
{
}


/// Most of the level types can simply use the level value here.
/// However, a few of the level types have a more complicated calculation
/// involving the level boundaries (top and bottom).
int AvnGribMgr::getLevel()
{
  if (_pds->getLevelId() == GribVertType::BETWEEN_DEPTH
    || _pds->getLevelId() == GribVertType::BETWEEN_PRS_DIFF)
  {
    // take the average
    // If this does not work (not unique?), we'll have to figure out something else.
    int averageLevel = (int)((_pds->getLevelValTop() + _pds->getLevelValBottom()) / 2.0 + 0.5);
    return averageLevel;
  }

  // the vast majority of types return the level value
  return (_pds->getLevelVal());
}

