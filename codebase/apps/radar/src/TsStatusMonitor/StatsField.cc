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
// StatsField.cc
//
// Class for monitoring a given metadata field
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2017
//
/////////////////////////////////////////////////////////////

#include "StatsField.hh"
#include <cmath>
#include <Radx/RadxTime.hh>

//////////////////////////////////////////
// constructor

StatsField::StatsField(const Params &params,
                       Params::xml_entry_type_t entryType,
                       const string &xmlOuterTag,
                       const string &xmlInnerTag,
                       const string &units,
                       const string &comment,
                       bool okBoolean,
                       bool omitIfZero,
                       bool interpretAsTime) :
        _params(params),
        _entryType(entryType),
        _xmlOuterTag(xmlOuterTag),
        _xmlInnerTag(xmlInnerTag),
        _units(units),
        _comment(comment),
        _okBoolean(okBoolean),
        _omitIfZero(omitIfZero),
        _interpretAsTime(interpretAsTime)
{
  clear();
}

////////////////
// initialize

void StatsField::clear()
{

  _stringVal.clear();

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

void StatsField::addValue(double val)
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

void StatsField::addValue(const string &val)
{
  _stringVal = val;
}

////////////////////////////
// compute stats

void StatsField::computeStats()

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
  
void StatsField::printStats(FILE *out)

{

  if (_entryType == Params::XML_ENTRY_STRING) {
    _printStringVal(out);
    return;
  }

  // in normal mode, do not print nan data

  if (!_params.debug) {
    if (std::isnan(_min)) {
      return;
    }
  }

  // do not print all zeros

  if (_omitIfZero) {
    if (_min == 0.0 && _max == 0.0 && _mean == 0.0) {
      return;
    }
  }

  string name(_xmlInnerTag);
  int combinedLen = name.size() + _units.size();
  int ntrim = combinedLen - 30;
  name = name.substr(0, name.size() - ntrim);

  char label[1024];
  if (_units.size() == 0 || _units.find("unknown") == 0) {
    sprintf(label, "%s", name.c_str());
  } else {
    sprintf(label, "%s (%s)", name.c_str(), _units.c_str());
  }
  label[45] = '\0';
  double range = _max - _min;
  fprintf(out, "%30s %10s %10s %10.3f %10s",
          label,
          _formatVal(_min).c_str(),
          _formatVal(_max).c_str(),
          range,
          _formatVal(_mean).c_str());
  
  if (_comment.size() > 0) {
    fprintf(out, "  %s", _comment.c_str());
  } else {
    fprintf(out, "  %s", _xmlOuterTag.c_str());
  }
  
  fprintf(out, "\n");
          
}

////////////////////////////
// print string val
  
void StatsField::_printStringVal(FILE *out)

{

  if (_interpretAsTime) {
    RadxTime when(_stringVal);
    if (when.utime() == RadxTime::NEVER) {
      // not a valid time
      return;
    }
    if (when.getYear() < 1971) {
      // time not set
      return;
    }
  }

  string name(_xmlInnerTag);
  int combinedLen = name.size() + _units.size();
  int ntrim = combinedLen - 30;
  name = name.substr(0, name.size() - ntrim);

  char label[1024];
  if (_units.size() == 0 || _units.find("unknown") == 0) {
    sprintf(label, "%s", name.c_str());
  } else {
    sprintf(label, "%s (%s)", name.c_str(), _units.c_str());
  }
  label[30] = '\0';
  fprintf(out, "%30s %43s", label, _stringVal.c_str());
  
  if (_comment.size() > 0) {
    fprintf(out, "  %s", _comment.c_str());
  } else {
    fprintf(out, "  %s", _xmlOuterTag.c_str());
  }
  
  fprintf(out, "\n");
          
}

////////////////////////////////
// format value appropriately
  
string StatsField::_formatVal(double val)
  
{

  char text[1024];
  
  if (fabs(val) > 9999) {
    sprintf(text, "%10.0f", val);
    return text;
  }

  // boolean type

  if (_entryType == Params::XML_ENTRY_BOOLEAN) {
    if (val == 0.0) {
      sprintf(text, "%10s", "false");
      return text;
    }
    if (val == 1.0) {
      sprintf(text, "%10s", "true");
      return text;
    }
  }

  sprintf(text, "%10.3f", val);
  return text;
          
}

void StatsField::printStatsDebug(FILE *out)

{
  
  fprintf(out, "Stats field xmlOuterTag: %s\n", _xmlOuterTag.c_str());
  if (_xmlInnerTag.size() > 0) {
    fprintf(out, "  innerTag: %s\n", _xmlInnerTag.c_str());
  }
  fprintf(out, "  units: %s\n", _units.c_str());
  fprintf(out, "  comment: %s\n", _comment.c_str());
  fprintf(out, "  okBoolean: %s\n", (_okBoolean?"Y":"N"));
  fprintf(out, "  stringVal: %s\n", _stringVal.c_str());
  fprintf(out, "  mean: %g\n", _mean);
  fprintf(out, "  sdev: %g\n", _sdev);
  fprintf(out, "  min: %g\n", _min);
  fprintf(out, "  max: %g\n", _max);

}


