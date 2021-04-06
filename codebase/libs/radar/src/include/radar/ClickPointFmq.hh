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
// ClickPointFmq.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2021
//
///////////////////////////////////////////////////////////////
//
// Reads/writes FMQ with click point for displays
//
////////////////////////////////////////////////////////////////

#ifndef ClickPointFmq_H
#define ClickPointFmq_H

#include <string>
#include <vector>
#include <Fmq/DsFmq.hh>
#include <toolsa/DateTime.hh>

using namespace std;

class ClickPointFmq {
  
public:

  // constructor
  
  ClickPointFmq(const string &url);
  
  // destructor
  
  ~ClickPointFmq();

  // write click point data, in XML format, to FMQ
  
  int write(time_t timeSecs,
            int nanoSecs,
            double azimuth,
            double elevation,
            double rangeKm,
            int gateNum);

  // read a new click point

  int read(bool &gotNew);

  // get methods for private members after read

  const string &getXml() const {
    return _xml; 
  }

  time_t getTimeSecs() const {
    return _timeSecs; 
  }
  
  int getNanoSecs() const {
    return _nanoSecs; 
  }
  
  const DateTime &getDateTime() const {
    return _time; 
  }
  
  double getElevation() const {
    return _elevation; 
  }

  double getAzimuth() const {
    return _azimuth; 
  }

  double getRangeKm() const {
    return _rangeKm; 
  }

  int getGateNum() const {
    return _gateNum; 
  }

  /// print

  void print(ostream &out) const;

  // debugging

  void setDebug() { _debug = true; }
  void setVerbose() { _verbose = true; }

  // missing value

  static const double missingValue;

protected:
private:

  bool _debug;
  bool _verbose;

  // fmq

  string _url;
  DsFmq _fmq;

  // data for click point

  string _xml;
  time_t _timeSecs;
  int _nanoSecs;
  DateTime _time;
  double _elevation;
  double _azimuth;
  double _rangeKm;
  int _gateNum;

  // methods

  int _checkFmqIsOpen();

};


#endif
