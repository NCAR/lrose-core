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
// OutputFmq.hh
//
// OutputFmq object
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2017
//
///////////////////////////////////////////////////////////////

#ifndef OutputFmq_H
#define OutputFmq_H

#include <string>
#include <vector>
#include <Fmq/DsRadarQueue.hh>
#include "Params.hh"
class RadxRay;
using namespace std;

////////////////////////
// This class

class OutputFmq {
  
public:

  // constructor
  
  OutputFmq(const string &prog_name,
	    const Params &params);
  
  // destructor
  
  ~OutputFmq();

  // write the params
  
  int writeParams(const RadxRay *ray);
  
  // write the status XML
  
  int writeStatusXml(const RadxRay *ray);
  
  // write the ray data
  
  int writeRay(const RadxRay *ray);
  
  // constructor status

  bool constructorOK;

protected:
  
private:

  static const double _missingDbl;
  
  string _progName;
  const Params &_params;

  // radar queue

  DsRadarQueue *_rQueue;
  DsRadarMsg _msg;
  int _nFields;

  // status
  
  int _nRaysWritten;
  
  // functions

  int _openFmq();

};

#endif

