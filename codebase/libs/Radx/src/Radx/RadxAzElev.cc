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
/**
 * @file RadxAzElev.cc
 */
#include <Radx/RadxAzElev.hh>
#include <cstdio>

//------------------------------------------------------------------
RadxAzElev::RadxAzElev(void) :
  _az(-1),
  _elev(-1),
  _ok(false)
{
}

//------------------------------------------------------------------
RadxAzElev::RadxAzElev(const double az, const double elev) :
  _az(az),
  _elev(elev),
  _ok(true)
{
}

//------------------------------------------------------------------
RadxAzElev::~RadxAzElev()
{
}

//------------------------------------------------------------------
bool RadxAzElev::operator==(const RadxAzElev &a1) const
{
  return _elev == a1._elev && _az == a1._az;
}

//------------------------------------------------------------------
bool RadxAzElev::operator<(const RadxAzElev &a1) const
{
  if (_elev < a1._elev)
  {
    return true;
  }
  else if (_elev == a1._elev)
  {
    if (_az < a1._az)
    {
      return true;
    }
    else if (_az == a1._az)
    {
      return false;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}


//------------------------------------------------------------------
std::string RadxAzElev::sprint(void) const
{
  char buf[1000];
  sprintf(buf, "(%lf,%lf)", _az, _elev);
  return buf;
}
