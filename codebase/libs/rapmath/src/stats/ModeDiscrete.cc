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
// ModeDiscrete.cc
//
// Compute the mode of a discrete (integer-based) array.
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2021
//
///////////////////////////////////////////////////////////////

#include <rapmath/ModeDiscrete.hh>
#include <algorithm>
#include <climits>
#include <map>

///////////////////////////////////////////////////////////////////////
// Compute the mode of a discrete data set (integer-based values).
// The mode is defined as the value with the highest count in the array.
//
// Return val:
//   If there is more that one value with the highest count, the lowest
//   of those values will be returned.

int ModeDiscrete::compute(const int *vals, size_t nVals)
  
{

  if (nVals < 1) {
    return 0;
  }

  if (nVals == 1) {
    return vals[0];
  }

  // count up the frequency of each value
  
  map<int, size_t> counts;
  for (size_t ipt = 0; ipt < nVals; ipt++) {
    int val = vals[ipt];
    if (counts.count(val) == 0) {
      counts[val] = 1;
    } else {
      counts[val]++;
    }
  }

  // find the val with the max count

  size_t maxCount = 0;
  int valForMax = 0;
  for (auto ii = counts.begin(); ii != counts.end(); ii++) {
    if (ii->second > maxCount) {
      valForMax = ii->first;
      maxCount = ii->second;
    }
  }

  return valForMax;

}

