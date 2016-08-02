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
// GridLoc.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2007
//
///////////////////////////////////////////////////////////////
//
// Data for a grid location
//
////////////////////////////////////////////////////////////////

#ifndef GridLoc_HH
#define GridLoc_HH

#include <vector>
#include "Params.hh"
#include "Beam.hh"
using namespace std;

////////////////////////
// This class

class GridLoc {
  
public:

  // constructor

  GridLoc(const Params &params,
	  double el,
	  double az);

  // destructor
  
  ~GridLoc();

  // clear beams

  void clearBeams();

  // add a beam

  void addBeam(const Beam &beam);

  // interpolate for this grid location

  void interpolate(const vector<double> &minima);

  // get methods

  double getEl() const { return _elMid; }
  double getAz() const { return _azMid; }
  const vector<double> &getInterp() const { return _interp; }
  
protected:
  
private:
  
  const Params &_params;

  // grid limits

  double _elMid;
  double _azMid;

  double _elMax;
  double _azMax;

  double _elMin;
  double _azMin;

  // vector of beams for interpolation
  // this will include all beams within the limits

  vector<Beam> _beams;

  // interpolated field values

  vector<double> _interp;

};

#endif
