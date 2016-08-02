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


#include <string>
#include <toolsa/toolsa_macros.h>
using namespace std;

#ifndef DeltaTimeINCLUDED
# include <toolsa/DeltaTime.hh>
#endif

// Todo: Decide if we want to allow this.
// const long DeltaTime::NOT_CONSTANT_SIZE = -LARGE_LONG;

DeltaTime::DeltaTime() : _duration(0), _units(Seconds)
{
}

DeltaTime::DeltaTime(long seconds) : _duration(seconds), _units(Seconds)
{
}

DeltaTime::DeltaTime(const DeltaTime & orig)
          : _duration(orig._duration), _units(orig._units)
{
}

DeltaTime::DeltaTime(long duration, TimeUnit units)
          : _duration(duration), _units(units)
{
}

DeltaTime::~DeltaTime()
{
}

// Returns: Number of seconds represented by this DeltaTime if in fixed units.
//          NOT_CONSTANT_SIZE If this DeltaTime has Months or Years units.
//          0                 If this DeltaTime has unknown units.
long DeltaTime::getDurationInSeconds() const
{
    long multiplier = 0;
    switch (_units) {
      case Seconds: multiplier = (1L);          break;
      case Minutes: multiplier = (60L);         break;
      case Hours:   multiplier = (60*60L);      break;
      case Days:    multiplier = (24*60*60L);   break;
      case Weeks:   multiplier = (7*24*60*60L); break;
      default:      return 0;
    }

    return (_duration * multiplier);
}
    
bool DeltaTime::operator == (const DeltaTime &other) const
{
    return ((_units == other._units) && (_duration == other._duration));
}

bool DeltaTime::operator != (const DeltaTime &other) const
{
   return !(*this == other);
}

DeltaTime & DeltaTime::operator = (const DeltaTime &other)
{
    _units = other._units;
    _duration = other._duration;

    return *this;
}

// friend
ostream& operator<< (ostream &os, const DeltaTime &d)
{
  string units_str;
  
  switch (d._units)
  {
  case DeltaTime::Seconds :
    units_str = "Seconds";
    break;
    
  case DeltaTime::Minutes :
    units_str = "Minutes";
    break;
    
  case DeltaTime::Hours :
    units_str = "Hours";
    break;
    
  case DeltaTime::Days :
    units_str = "Days";
    break;
    
  case DeltaTime::Weeks :
    units_str = "Weeks";
    break;
    
  default:
    units_str = "UNKNOWN";
    break;
  }
  
  return os << d._duration << " " << units_str;
}

// friend
ostream& operator<< (ostream &os, const DeltaTime *d)
{
  string units_str;
  
  switch (d->_units)
  {
  case DeltaTime::Seconds :
    units_str = "Seconds";
    break;
    
  case DeltaTime::Minutes :
    units_str = "Minutes";
    break;
    
  case DeltaTime::Hours :
    units_str = "Hours";
    break;
    
  case DeltaTime::Days :
    units_str = "Days";
    break;
    
  case DeltaTime::Weeks :
    units_str = "Weeks";
    break;
    
  default:
    units_str = "UNKNOWN";
    break;
  }
  
  return os << d->_duration << " " << units_str;
}
