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
// Bprp2Dsr.hh
//
// Bprp2Dsr object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2002
//
///////////////////////////////////////////////////////////////
//
// Bprp2Dsr reads Bethlehem-format radar data and writes it to
// and FMQ in DsRadar format
//
////////////////////////////////////////////////////////////////

#ifndef Bprp2Dsr_HH
#define Bprp2Dsr_HH

#include <string>
#include "Args.hh"
#include "Params.hh"
#include <Fmq/DsRadarQueue.hh>
#include <rapformats/bprp.h>
using namespace std;

#define DBZ_SCALE (0.5)
#define DBZ_BIAS (-30.0)

////////////////////////
// This class

class Bprp2Dsr {
  
public:

  // constructor

  Bprp2Dsr (int argc, char **argv);

  // destructor
  
  ~Bprp2Dsr();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  DsRadarQueue _rQueue;
  bprp_response_t _response;

  int _rdasFd;
  int _beamCount;
  int _volNum;
  int _tiltNum;
  int _scanType;

  int _openRdas();
  void _closeRdas();
  int _readBeam();
  int _handleResponse();
  int _handleBeam();

  double _computeDbz(double range,
		     double atten_per_km,
		     double atten_at_100km,
		     double xmtTerm,
		     int count,
		     double slope,
		     double viplo,
		     double plo,
		     bool print);

  int _writeRadarAndFieldParams(int radarId,
				double start_range, double gate_spacing);

  int _writeBeam(time_t beamTime,
		 double azimuth,
		 double elevation,
		 void *gateData,
		 int dataNBytes,
		 int byteWidth);

};

#endif
