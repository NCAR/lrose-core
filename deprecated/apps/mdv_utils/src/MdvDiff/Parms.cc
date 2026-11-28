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
#include <toolsa/copyright.h>

//   File: $RCSfile: Parms.cc,v $
//   Version: $Revision: 1.12 $  Dated: $Date: 2016/03/04 02:22:11 $

/**
 * @file Parms.cc
 */

/*----------------------------------------------------------------*/
#include <algorithm>
#include "Parms.hh"
#include "Params.hh"

/*----------------------------------------------------------------*/
Parms::Parms(void) : Params()
{
}

/*----------------------------------------------------------------*/
Parms::Parms(int argc, char **argv) : Params()
{
  if (loadFromArgs(argc, argv, NULL, NULL))
  {
    printf("ERROR loading params\n");
    exit(-1);
  }

  _all_fields = compare_fields_n == 0;
  for (int i=0; i<compare_fields_n; ++i)
  {
    _fields.push_back(_compare_fields[i]);
  }
}

/*----------------------------------------------------------------*/
Parms::~Parms()
{
}

/*----------------------------------------------------------------*/
bool Parms::wanted_field(const string &s) const
{
  if (_all_fields)
  {
    return true;
  }
  else
  {
    return find(_fields.begin(), _fields.end(), s) != _fields.end();
  }
}
