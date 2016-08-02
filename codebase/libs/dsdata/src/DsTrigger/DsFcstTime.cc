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
 * @file DsFcstTime.cc
 */
#include <cstdio>
#include <dsdata/DsFcstTime.hh>
#include <toolsa/DateTime.hh>

//----------------------------------------------------------------
DsFcstTime::DsFcstTime()
{  
  _gt = -1;
  _lt = -1;
}

//----------------------------------------------------------------
DsFcstTime::DsFcstTime(const time_t &t, const int lt)
{  
  _gt = t;
  _lt = lt;
}

//----------------------------------------------------------------
DsFcstTime::~DsFcstTime()
{
}

//----------------------------------------------------------------
void DsFcstTime::print(void) const
{
  printf("%s+%d\n", DateTime::strn(_gt).c_str(), _lt);
}

//----------------------------------------------------------------
string DsFcstTime::sprint(void) const
{
  char buf[100];
  sprintf(buf, "%s+%d", DateTime::strn(_gt).c_str(), _lt);
  string ret = buf;
  return ret;
}

//----------------------------------------------------------------
bool DsFcstTime::lessOrEqual(const DsFcstTime f0, const DsFcstTime f1)
{
  if (f0._gt < f1._gt)
  {
    return true;
  }
  if (f0._gt > f1._gt)
  {
    return false;
  }
  return (f0._lt <= f1._lt);
}
    
//----------------------------------------------------------------
bool DsFcstTime::operator!=(const DsFcstTime f) const
{
  return (_gt != f._gt || _lt != f._lt);
}
    
