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
// FlightLevel.h
//
// This class provides the pressure at any given flight level
// between -10 and 500.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
/////////////////////////////////////////////////////////////

#ifndef FlightLevel_H
#define FlightLevel_H

#define MIN_FL_LEVEL -10
#define MAX_FL_LEVEL 500
#define N_FL_LEVELS 52

#include "MM52Mdv.hh"
using namespace std;

typedef struct {
  int valid;
  int upperIsigma;
  int lowerIsigma;
  double wtUpper;
  double wtLower;
} FlightLevel_interp_t;

class FlightLevel {
  
public:

  // constructor
  
  FlightLevel (char *prog_name, Params *params);
  
  // destructor
  
  ~FlightLevel ();
  
  // number of levels

  int nLevels;

  // array of selected flight levels

  int *levels;
  
  // array of pressures for the selected flight levels

  double *Mb;
  
  // array of interpolated values

  double *interpField;

  // array of flags to indicate whether a given sigma level is
  // required for interpolation

  int *sigmaNeeded;

  // OK flag

  int OK;
  
  // prepareInterp
  //
  // Prepares interpolation info for a sigma pressure profile for
  // a given point.
  
  void prepareInterp(int n_sigma, double *p_sigma);
  
  // doInterp
  //
  // Interpolate a field from sigma to flight levels.
  //
  // Passed in is the array for the field column in sigma levels.
  //
  // Returned is a pointer to the array containing the interpolated values.
  //
  
  double *doInterp(double *field_sigma);
  
  // pres2level()
  //
  // Return the flight level for a given pressure.
  //
  
  double FlightLevel::pres2level(double pressure);
  
protected:
  
private:

  char *_progName;
  Params *_params;

  // array of interpolation indices
  
  FlightLevel_interp_t *_interpIndex;

  // table of pressure in MB for each flight level
  // between -10 and 500

  double _mbTable[N_FL_LEVELS];

  int _pressure(int flevel, double *mb_p);

  // load up table

  void _loadTable();

};

#endif






