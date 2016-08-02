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

#ifndef RAINUTIL_LATLON_H
#define RAINUTIL_LATLON_H

#include "angle.h"
#include "string_utils.h"
#include <ostream>

namespace rainfields {

  struct latlon
  {
    angle lat, lon;

    latlon() noexcept  = default;
    latlon(angle lat, angle lon) noexcept : lat(lat), lon(lon) { }

    latlon(const latlon& rhs) noexcept = default;
    latlon(latlon&& rhs) noexcept = default;

    auto operator=(const latlon& rhs) noexcept -> latlon& = default;
    auto operator=(latlon&& rhs) noexcept -> latlon& = default;

    ~latlon() noexcept = default;
  };

  struct latlonalt : public latlon
  {
    double alt;

    latlonalt() noexcept  = default;
    latlonalt(angle lat, angle lon, double alt) noexcept : latlon{lat, lon}, alt{alt} { }
    latlonalt(latlon ll, double alt) noexcept : latlon{ll}, alt{alt} { }

    latlonalt(const latlonalt& rhs) noexcept = default;
    latlonalt(latlonalt&& rhs) noexcept = default;

    auto operator=(const latlonalt& rhs) noexcept -> latlonalt& = default;
    auto operator=(latlonalt&& rhs) noexcept -> latlonalt& = default;

    ~latlonalt() noexcept = default;
  };

  auto operator<<(std::ostream& lhs, const latlon& rhs) -> std::ostream&;
  auto operator<<(std::ostream& lhs, const latlonalt& rhs) -> std::ostream&;

  template <>
  auto from_string<latlon>(const char* str) -> latlon;
  template <>
  auto from_string<latlonalt>(const char* str) -> latlonalt;
}

#endif
