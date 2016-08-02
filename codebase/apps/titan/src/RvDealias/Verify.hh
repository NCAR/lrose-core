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
// Verify.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2003
//
///////////////////////////////////////////////////////////////

#ifndef Verify_hh
#define Verify_hh

#include <cstdio>
#include "Params.hh"
#include "ErrorMatrix.hh"

using namespace std;

////////////////////////
// This class

class Verify {
  
public:
  
  // constructor - initializes for given size
  
  Verify (const Params &params);
  
  // destructor
  
  ~Verify();

  bool isOK() { return _isOK; }
  
  // add to statistics
  
  void addToStats(double estDbm1, double truthDbm1,
		  double estVel1, double truthVel1,
		  double estWidth1, double truthWidth1,
		  double estDbm2, double truthDbm2,
		  double estVel2, double truthVel2,
		  double estWidth2, double truthWidth2,
		  double r1Ratio);

  // compute stats and write

  void computeStatsAndWrite();

  // write a data line to the table file
  
  void writeToTable(double estDbm1, double truthDbm1,
		    double estVel1, double truthVel1,
		    double estWidth1, double truthWidth1,
		    double estDbm2, double truthDbm2,
		    double estVel2, double truthVel2,
		    double estWidth2, double truthWidth2,
		    double r1Ratio);

protected:
  
private:

  const Params &_params;
  FILE *_tableFile;
  bool _isOK;

  ErrorMatrix _p1_175_225;
  ErrorMatrix _p1_275_325;
  ErrorMatrix _p1_375_425;

  ErrorMatrix _p2_175_225;
  ErrorMatrix _p2_275_325;
  ErrorMatrix _p2_375_425;

  ErrorMatrix _v1_175_225;
  ErrorMatrix _v1_275_325;
  ErrorMatrix _v1_375_425;

  ErrorMatrix _v2_175_225;
  ErrorMatrix _v2_275_325;
  ErrorMatrix _v2_375_425;

  ErrorMatrix _w1_175_225;
  ErrorMatrix _w1_275_325;
  ErrorMatrix _w1_375_425;

  ErrorMatrix _w2_175_225;
  ErrorMatrix _w2_275_325;
  ErrorMatrix _w2_375_425;

};

#endif

