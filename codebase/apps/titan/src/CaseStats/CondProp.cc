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
//////////////////////////////////////////////////////////
// CondProp.cc
//
// CondProp object - conditional property.
//
// Such a property is used to condition the analysis by only
// considering cases with prop values between the limits.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// December 1997
//
//////////////////////////////////////////////////////////

#include "CondProp.hh"
#include <toolsa/str.h>
using namespace std;

//////////////
// Constructor

CondProp::CondProp (char *prog_name, int debug, char *comb_name,
		    double min_val, double max_val) : Property()

{
  
  // make copy of name

  char *tmp_name = STRdup(comb_name);

  // set defaults - this assumes the property is not a
  // time_series

  char *name = comb_name;
  int from_tseries = FALSE;
  int delta_time = 0;

  // check to see if we have a time_series type
  // time_series props are of the form "name@secs"

  char *name_tok = strtok(tmp_name, "@");
  if (name_tok != NULL) {
    char *dt_tok = strtok(NULL, "");
    if (dt_tok != NULL) {
      int dt;
      if (sscanf(dt_tok, "%d", &dt) == 1) {
	name = name_tok;
	delta_time = dt;
	from_tseries = TRUE;
      }
    }
  }

  // initialize the object

  _init(prog_name, debug, name);
  _fromTseries = from_tseries;
  _deltaTime = delta_time;
  _minVal = min_val;
  _maxVal = max_val;

  // free up

  STRfree(tmp_name);

}

/////////////
// destructor

CondProp::~CondProp ()
  
{
}

///////////////////////////
// withinLimits()
//
// Check if val at given location is within conditional limits
//

int CondProp::withinLimits(int index_loc)
  
{
  double val = _vals[index_loc];

  if (val >= _minVal && val <= _maxVal) {
    return(TRUE);
  } else {
    return (FALSE);
  }

}
