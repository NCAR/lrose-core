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
// FlightLevel.cc
//
// This class provides the pressure at any given flight level
// between -10 and 500.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
//////////////////////////////////////////////////////////

#include "FlightLevel.hh"
#include <cstdio>
#include <toolsa/str.h>
#include <toolsa/mem.h>
using namespace std;


//////////////
// Constructor

FlightLevel::FlightLevel (char *prog_name, Params *params)

{
  
  OK = TRUE;
  _progName = STRdup(prog_name);
  _params = params;

  // load up table of pressure information

  _loadTable();

  // alloc arrays

  sigmaNeeded = NULL;
  levels = NULL;
  Mb = NULL;
  interpField = NULL;
  _interpIndex = NULL;

  nLevels = _params->flight_levels_n;
  levels = (int *) umalloc (nLevels * sizeof(int));
  Mb = (double *) umalloc (nLevels * sizeof(double));
  interpField = (double *)  umalloc(nLevels * sizeof(double));
  _interpIndex = (FlightLevel_interp_t *)
    umalloc(nLevels * sizeof(FlightLevel_interp_t));
  
  // load up levels and pressure array
  
  for (int i = 0; i < nLevels; i++) {
    levels[i] = _params->_flight_levels[i];
    if (_pressure(levels[i], &Mb[i])) {
      OK = FALSE;
    }
  } // i

}

//////////////
// Destructor

FlightLevel::~FlightLevel ()

{
  if (levels) {
    ufree(levels);
  }
  if (Mb) {
    ufree(Mb);
  }
  if (interpField) {
    ufree(interpField);
  }
  if (_interpIndex) {
    ufree(_interpIndex);
  }
  if (sigmaNeeded) {
    ufree(sigmaNeeded);
  }
}

////////////////
// prepareInterp
//
// Loads interpolation info for a sigma pressure profile for
// a given point.
//

void FlightLevel::prepareInterp(int n_sigma, double *p_sigma)

{

  // load up inter info

  int jstart = 0;
  
  for (int i = 0; i < nLevels; i++) {

    FlightLevel_interp_t *intp = _interpIndex + i;
    intp->valid = FALSE;
    double p_fl = Mb[i];
    
    for (int j = jstart; j < n_sigma-1; j++) {
      if (p_sigma[j] >= p_fl && p_sigma[j+1] <= p_fl) {
	intp->lowerIsigma = j;
	intp->upperIsigma = j + 1;
	double dp = p_sigma[j] - p_sigma[j+1];
	intp->wtLower = (p_fl - p_sigma[j+1]) / dp;
	intp->wtUpper = (p_sigma[j] - p_fl) / dp;
	intp->valid = TRUE;
	jstart = j;
	break;
      }
    } // j

  } // i

  // load up sigma_needed array

  if (sigmaNeeded == NULL) {
    sigmaNeeded = (int *) umalloc(n_sigma * sizeof(int));
  }

  for (int i = 0; i < nLevels; i++) {
    FlightLevel_interp_t *intp = _interpIndex + i;
//     fprintf(stderr, "Level %d: %d %d %d %g %g\n",
// 	    i, intp->valid,
// 	    intp->lowerIsigma, intp->upperIsigma,
// 	    intp->wtLower, intp->wtUpper);
    if (intp->valid) {
      sigmaNeeded[intp->lowerIsigma] = TRUE;
      sigmaNeeded[intp->upperIsigma] = TRUE;
    }
  }

  return;

}

///////////
// doInterp
//
// Interpolate a field from sigma to flight levels.
//
// Passed in is the array for the field column in sigma levels.
//
// Returned is a pointer to the array containing the interpolated values.
//

double *FlightLevel::doInterp(double *field_sigma)
  
{
  
  for (int i = 0; i < nLevels; i++) {
    
    FlightLevel_interp_t *intp = _interpIndex + i;

    if (intp->valid) {
      interpField[i] =
	(field_sigma[intp->lowerIsigma] * intp->wtLower +
	 field_sigma[intp->upperIsigma] * intp->wtUpper);
    } else {
      interpField[i] = MISSING_DOUBLE;
    }

  }

  return (interpField);

}

///////////////
// pres2level()
//
// Return the flight level for a given pressure.
//

double FlightLevel::pres2level(double pressure)
  
{
  
  if (pressure > _mbTable[0]) {
    return ((double) MIN_FL_LEVEL);
  }
  
  if (pressure < _mbTable[N_FL_LEVELS - 1]) {
    return ((double) MAX_FL_LEVEL);
  }

  for (int i = 0; i < N_FL_LEVELS - 1; i++) {
    
    double p1 = _mbTable[i];
    double p2 = _mbTable[i+1];

    if (pressure <= p1 && pressure >= p2) {
      double fraction = (p1 - pressure) / (p1 - p2);
      double level = ((double) i + fraction) * 10.0 + (double) MIN_FL_LEVEL;
      return (level);
    }
    
  }

  return ((double) MAX_FL_LEVEL);

}

///////////////
// _loadTable()
//
// Loads table of pressures for each level
// first level is MIN_FL_LEVEL
// last level is MAX_FL_LEVEL

void FlightLevel::_loadTable()

{

  _mbTable[0] = 1050.2;  // level -10
  _mbTable[1] = 1013.0;  // level 00
  _mbTable[2] = 976.9;  // level 10
  _mbTable[3] = 941.9;  // level 20
  _mbTable[4] = 907.9;  // level 30
  _mbTable[5] = 874.9;  // level 40
  _mbTable[6] = 842.8;  // level 50
  _mbTable[7] = 811.8;  // level 60
  _mbTable[8] = 781.6;  // level 70
  _mbTable[9] = 752.5;  // level 80
  _mbTable[10] = 724.1;  // level 90
  _mbTable[11] = 696.6;  // level 100
  _mbTable[12] = 670.0;  // level 110
  _mbTable[13] = 644.3;  // level 120
  _mbTable[14] = 619.2;  // level 130
  _mbTable[15] = 595.1;  // level 140
  _mbTable[16] = 571.6;  // level 150
  _mbTable[17] = 549.0;  // level 160
  _mbTable[18] = 527.1;  // level 170
  _mbTable[19] = 505.9;  // level 180
  _mbTable[20] = 485.3;  // level 190
  _mbTable[21] = 465.5;  // level 200
  _mbTable[22] = 446.3;  // level 210
  _mbTable[23] = 427.8;  // level 220
  _mbTable[24] = 409.9;  // level 230
  _mbTable[25] = 392.6;  // level 240
  _mbTable[26] = 375.9;  // level 250
  _mbTable[27] = 359.8;  // level 260
  _mbTable[28] = 344.2;  // level 270
  _mbTable[29] = 329.2;  // level 280
  _mbTable[30] = 314.7;  // level 290
  _mbTable[31] = 300.9;  // level 300
  _mbTable[32] = 287.4;  // level 310
  _mbTable[33] = 274.4;  // level 320
  _mbTable[34] = 262.0;  // level 330
  _mbTable[35] = 249.9;  // level 340
  _mbTable[36] = 238.4;  // level 350
  _mbTable[37] = 227.2;  // level 360
  _mbTable[38] = 216.6;  // level 360
  _mbTable[39] = 206.4;  // level 380
  _mbTable[40] = 196.7;  // level 390
  _mbTable[41] = 187.5;  // level 400
  _mbTable[42] = 178.7;  // level 410
  _mbTable[43] = 170.3;  // level 420 
  _mbTable[44] = 162.3;  // level 430
  _mbTable[45] = 154.7;  // level 440
  _mbTable[46] = 147.4;  // level 450
  _mbTable[47] = 140.5;  // level 460
  _mbTable[48] = 133.9;  // level 470
  _mbTable[49] = 127.6;  // level 480
  _mbTable[50] = 121.7;  // level 490
  _mbTable[51] = 116.0;  // level 500

}

/////////////
// _pressure
//
// Loads pressure in mb at given flight level
// for levels between -10 and 500.
// returns 0 on success, -1 on failure

int FlightLevel::_pressure(int flevel, double *mb_p)

{
  if (flevel < MIN_FL_LEVEL || flevel > MAX_FL_LEVEL) {
    fprintf(stderr, "ERROR - FlightLevel::_pressure\n");
    fprintf(stderr, "Flight level %d not supported\n", flevel);
    return (-1);
  }
  int array_pos = flevel / 10 + 1;
  *mb_p = _mbTable[array_pos];
  return (0);
}







