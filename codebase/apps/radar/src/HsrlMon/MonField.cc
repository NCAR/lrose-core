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

MonField::MonField(const Params &params,
                   const string &name,
                   const string &qualifier,
                   double minValidValue,
                   double maxValidValue,
                   const string &note) :
        _params(params),
        _name(name),
        _qualifier(qualifier),
        _note(note),
        _minValidValue(minValidValue),
        _maxValidValue(maxValidValue)
{
  _longName = name;
  _units = "notset";
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

  if (_nn == 0) {
    _min = NAN;
    _max = NAN;
  }

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

  // in normal mode, do not print nan data

  if (!_params.debug) {
    if (std::isnan(_min)) {
      return;
    }
  }

  string longName(_longName);
  if (_name.find("tcsaft") != string::npos) {
    longName += " aft";
  }
  if (_name.find("tcsfore") != string::npos) {
    longName += " fore";
  }
  int combinedLen = longName.size() + _units.size();
  int ntrim = combinedLen - 42;
  longName = longName.substr(0, longName.size() - ntrim);

  char label[1024];
  if (_units.size() == 0 || _units.find("unknown") == 0) {
    sprintf(label, "%s", longName.c_str());
  } else {
    sprintf(label, "%s (%s)", longName.c_str(), _units.c_str());
  }
  label[45] = '\0';
  double range = _max - _min;
  if (fabs(_max) > 9999) {
    fprintf(out, "%45s: %10.0f %10.0f %10.0f %10.0f",
            label, _min, _max, _mean, range);
  } else {
    fprintf(out, "%45s: %10.3f %10.3f %10.3f %10.3f",
            label, _min, _max, _mean, range);
  }

  if (_note.size() > 0) {
    fprintf(out, "  %s", _note.c_str());
  }

  if (_params.debug) {
    fprintf(out, "  :%s %s\n", _name.c_str(), _qualifier.c_str());
  } else {
    fprintf(out, "\n");
  }
          
}

void MonField::printStatsDebug(FILE *out)

{
  
  fprintf(out, "Stats field name: %s\n", _name.c_str());
  if (_qualifier.size() > 0) {
    fprintf(out, "  qualitfier: %s\n", _qualifier.c_str());
  }
  fprintf(out, "  longName: %s\n", _longName.c_str());
  fprintf(out, "  units: %s\n", _units.c_str());
  fprintf(out, "  mean: %g\n", _mean);
  fprintf(out, "  sdev: %g\n", _sdev);
  fprintf(out, "  min: %g\n", _min);
  fprintf(out, "  max: %g\n", _max);
  // fprintf(out, "  nn: %g\n", _nn);
  // fprintf(out, "  sum: %g\n", _sum);
  // fprintf(out, "  sumSq: %g\n", _sumSq);

}


