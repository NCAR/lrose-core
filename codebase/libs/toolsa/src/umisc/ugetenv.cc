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
///////////////////////////////////////////////////////////////
// ugetenv - utilities for getting values from the environment
//
// Mike Dixon RAP NCAR Boulder CO USA
//
// November 2013
//
///////////////////////////////////////////////////////////////

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <toolsa/ugetenv.hh>

///////////////////////////////////////////////////////////////
// get an integer from an environment variable
// returns 0 on success, -1 on failure

int ugetenv_int(const char *env, int &val)
  
{
  
  const char *valstr = getenv(env);
  if (valstr == NULL) {
    // not set
    return -1;
  }

  int ival;
  if (sscanf(valstr, "%d", &ival) != 1) {
    return -1;
  }

  val = ival;
  return 0;

}

///////////////////////////////////////////////////////////////
// get a double from an environment variable
// returns 0 on success, -1 on failure

int ugetenv_double(const char *env, double &val)
  
{
  
  const char *valstr = getenv(env);
  if (valstr == NULL) {
    // not set
    return -1;
  }

  double dval;
  if (sscanf(valstr, "%lg", &dval) != 1) {
    return -1;
  }

  val = dval;
  return 0;

}

///////////////////////////////////////////////////////////////
// get a string value from an environment variable
// returns 0 on success, -1 on failure

int ugetenv_string(const char *env, string &val)
  
{
  
  const char *valstr = getenv(env);
  if (valstr == NULL) {
    // not set
    return -1;
  }

  val = valstr;
  return 0;

}

