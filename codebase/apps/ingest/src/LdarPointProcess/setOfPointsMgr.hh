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


#ifndef setOfPointsMgr_H
#define setOfPointsMgr_H

#include "Params.hh"
#include "setOfPoints.hh"
#include <Spdb/DsSpdb.hh>
#include <vector>

using namespace std;

class setOfPointsMgr {

public:
  //
  // Constructor. Reads the stations and pushes
  // them back into the correct vector.
  //
  setOfPointsMgr(Params *TDRP_params);
  //
  //
  // Method to add a point. Determines if we are done and if
  // we should save.
  //
  void addPoint(double lat,
		double lon,
		double alt,
		double t);
  //
  // Method to write a dummy point so that the program generates
  // output even when nothing is happening.
  //
  void writeDummy();

  bool writeChunks();

  //
  // Destructor. Cleans out the vector.
  //
  ~setOfPointsMgr();

  private :
  
  vector <setOfPoints *> _setsOfPoints;
  Params *_params;
  double _lastPrintT;
  time_t _lastWriteT;
  DsSpdb Out;
  DsSpdb OutLtg;

};

#endif

