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
/////////////////////////////////////////////////////////////
// StatsField.hh
//
// Class for monitoring a given metadata field
// to send to catalog
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2017
//
/////////////////////////////////////////////////////////////

#ifndef _CAT_FIELD_HH
#define _CAT_FIELD_HH

#include "Params.hh"
#include <string>
#include <cstdio>
using namespace std;

// class for monitoring data

class StatsField {
  
public:

  // constructor
  
  StatsField(const Params &params,
             const string &xmlOuterTag,
             const string &xmlInnerTag,
             bool isBoolean,
             const string &comment);

  // initialize

  void clear();

  // add a value to the stats
  
  void addValue(double val);

  // compute stats
  
  void computeStats();

  // print stats
  
  void printStats(FILE *out);
  void printStatsDebug(FILE *out);

  // set methods
  
  void setLongName(const string &val) { _longName = val; }
  void setUnits(const string &val) { _units = val; }

  // get methods

  const string &getXmlOuterTag() const { return _xmlOuterTag; }
  const string &getXmlInnerTag() const { return _xmlInnerTag; }
  const string &getLongName() const { return _longName; }
  const string &getUnits() const { return _units; }
  const string &getComment() const { return _comment; }

  bool getIsBoolean() const { return _isBoolean; }

  double getMean() const { return _mean; }
  double getSdev() const { return _sdev; }
  double getMin() const { return _min; }
  double getMax() const { return _max; }

  double getSum() const { return _sum; }
  double getSumSq() const { return _sumSq; }
  double getNn() const { return _nn; }

private:

  const Params &_params;

  string _xmlOuterTag;
  string _xmlInnerTag;
  bool _isBoolean;

  string _longName;
  string _units;

  string _comment;

  double _sum;
  double _sumSq;
  double _nn;

  double _mean;
  double _sdev;
  double _min;
  double _max;


};

#endif



