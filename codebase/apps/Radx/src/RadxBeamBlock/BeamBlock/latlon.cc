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

#include "latlon.h"

using namespace rainfields;

auto rainfields::operator<<(std::ostream& lhs, const latlon& rhs) -> std::ostream&
{
  int d, m;
  double s;
  char buf[64];

  rhs.lat.dms(d, m, s);
  snprintf(buf, sizeof(buf), "%03d%02d%02.lf%c", std::abs(d), m, s, d < 0 ? 'S' : 'N');
  lhs << buf;

  rhs.lon.dms(d, m, s);
  snprintf(buf, sizeof(buf), " %03d%02d%02.lf%c", std::abs(d), m, s, d < 0 ? 'W' : 'E');
  lhs << buf;

  return lhs;
}

auto rainfields::operator<<(std::ostream& lhs, const latlonalt& rhs) -> std::ostream&
{
  char buf[32];
  snprintf(buf, sizeof(buf), " %.0fm", rhs.alt);
  return lhs << static_cast<const latlon&>(rhs) << buf;
}

template <>
auto rainfields::from_string<latlon>(const char* str) -> latlon
{
  // TODO - support multiple formats here...
  double lat, lon;
  if (sscanf(str, "%lf %lf", &lat, &lon) != 2)
    throw string_conversion_error("latlon", str);
  return {lat * 1_deg, lon * 1_deg};
}

template <>
auto rainfields::from_string<latlonalt>(const char* str) -> latlonalt
{
  // TODO - support multiple formats here...
  double lat, lon, alt;
  if (sscanf(str, "%lf %lf %lf", &lat, &lon, &alt) != 3)
    throw string_conversion_error("latlonalt", str);
  return {lat * 1_deg, lon * 1_deg, alt};
}

