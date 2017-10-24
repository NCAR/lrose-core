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
// MonField.cc
//
// Class for monitoring a given metadata field
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2017
//
/////////////////////////////////////////////////////////////

#include "MonField.hh"
#include <cmath>

//////////////////////////////////////////
// constructor

MonField::MonField(const string &name,
                   const string &qualifier,
                   double minValidValue,
                   double maxValidValue) :
        _name(name),
        _qualifier(qualifier),
        _minValidValue(minValidValue),
        _maxValidValue(maxValidValue)
{
  clear();
}

////////////////
// initialize

void MonField::clear()
{

  _sum = 0.0;
  _sumSq = 0.0;
  _nn = 0.0;
  
  _mean = NAN;
  _sdev = NAN ;
  _min = 1.0e99;
  _max = -1.0e99;

}

////////////////////////////
// add a value to the stats

void MonField::addValue(double val)

{
  _sum += val;
  _sumSq += val * val;
  _nn++;
  if (val < _min) {
    _min = val;
  }
  if (val > _max) {
    _max = val;
  }
}

////////////////////////////
// compute stats

void MonField::computeStats()

{
  if (_nn > 0) {
    _mean = _sum / _nn;
  }

  if (_nn > 1) {
    double xx = _nn * _sumSq - _sum * _sum;
    if (xx <= 0.0) {
      _sdev = 0.0;
    } else {
      _sdev = sqrt(xx) / (_nn - 1.0);
    }
  }
  
}

////////////////////////////
// print stats
  
void MonField::printStats(FILE *out)

{
  
  fprintf(out, "Stats field name: %s\n", _name.c_str());
  if (_qualifier.size() > 0) {
    fprintf(out, "  qualitfier: %s\n", _qualifier.c_str());
  }
  fprintf(out, "  mean: %g\n", _mean);
  fprintf(out, "  sdev: %g\n", _sdev);
  fprintf(out, "  min: %g\n", _min);
  fprintf(out, "  max: %g\n", _max);
  // fprintf(out, "  nn: %g\n", _nn);
  // fprintf(out, "  sum: %g\n", _sum);
  // fprintf(out, "  sumSq: %g\n", _sumSq);

}


