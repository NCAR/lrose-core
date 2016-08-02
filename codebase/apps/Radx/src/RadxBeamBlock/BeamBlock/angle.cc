// %=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%
// ** Rainfields Utilities Library (rainutil)
// ** Copyright BOM (C) 2013
// ** Bureau of Meteorology, Commonwealth of Australia, 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from the BOM.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of the BOM nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// %=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%

#include "angle.h"

using namespace rainfields;

auto angle::dms(int& deg, int& min, double& sec) const -> void
{
  double decimal = std::abs(degrees());
  deg = decimal;
  decimal = (decimal - deg) * 60.0;
  min = decimal;
  decimal = (decimal - min) * 60.0;
  sec = decimal;

  // fix rare case of floating point stuff up
  if (sec >= 60.0)
  {
    min++;
    sec -= 60.0;
  }

  // ensure correct sign on degrees
  if (rads_ < 0.0)
    deg = -deg;
}

auto angle::set_dms(int d, int m, double s) -> void
{
  rads_ = (d + (m / 60.0) + (s / 3600.0)) * constants<double>::to_radians;
}

template<>
auto rainfields::from_string<angle>(const char* str) -> angle
{
  // for now assume it's just a number specified in degrees
  return from_string<double>(str) * 1._deg;
}

