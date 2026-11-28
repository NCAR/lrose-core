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
// Reads MDV, makes beams.
//
// Niles Oien, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2005
//
/////////////////////////////////////////////////////////////

#ifndef MDV2BEAM_HH
#define MDV2BEAM_HH

#include <string>
#include <vector>
#include <ctime>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>

#include "Params.hh"

using namespace std;

class Mdv2Beam {
  
public:
  
  const static double badVal = -9999.0;

  //
  // Constructor - reads the MDV, calculates _numR
  //
  Mdv2Beam(time_t dataTime, Params *P);
  //
  // Get a beam at an azimuth, tilt. Caller to allocate space for beamData.
  //
  void getBeam(double az, int tiltNum, int numFields, int numGates,
	       fl32 *beamData);


  //
  // Get the number of gates for a certain tilt.
  //
  int getNumGates( int tiltNum );

  //
  // Desructor -does nothing.
  //
  ~Mdv2Beam();
  
protected:
  
private:

  int _OK;
  DsMdvx _mdvMgr;

  Params *_params;

};

#endif
