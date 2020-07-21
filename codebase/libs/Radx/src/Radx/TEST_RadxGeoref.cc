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
/*
 * Name: TEST_RadxGeoref.cc
 *
 * Purpose:
 *
 *      To test the angle computations for RadxGeoref
 *
 * Usage:
 *
 *       % TEST_RadxGeoref
 *
 * Inputs: 
 *
 *       None
 *
 *
 * Author: Mike Dixon, July 2020
 *
 */

/*
 * include files
 */

#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iomanip>
#include <Radx/RadxGeoref.hh>

/* ======================================================================== */

/*
 * main program
 */

int main(int argc, char *argv[])
{

  // Compute the angles, and write them out

  fprintf(stderr, "# %8s %8s %8s %8s %8s %8s %8s %8s\n",
          "pitch", "roll", "el", "az", "rot", "tilt", "el2", "az2");

  double pitchMin = -2.0;
  double pitchMax = 4.1;
  double pitchDelta = 1.0;

  double rollMin = -30.0;
  double rollMax = 30.1;
  double rollDelta = 10.0;

  double hdg = 45.0;
  double el = 90.1;
  double az = hdg + 90;

  int iret = 0;
  
  for (double pitch = pitchMin; pitch < pitchMax; pitch += pitchDelta) {
    
    for (double roll = rollMin; roll < rollMax; roll += rollDelta) {
      
      double rot, tilt;
      RadxGeoref::computeRotTiltYPrime(pitch, roll, hdg, el, az, rot, tilt);
      double el2, az2;
      RadxGeoref::computeAzElYPrime(pitch, roll, hdg, rot, tilt, el2, az2);
      
      fprintf(stderr, "  %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f\n",
              pitch, roll, el, az, rot, tilt, el2, az2);

      if (fabs(el - el2) > 1.0e-6 || fabs(az - az2) > 1.0e-6) {
        iret = -1;
      }
      
    } // roll

  } // pitch

  if (iret == 0) {
    fprintf(stderr, "SUCCESS - TEST_RadxGeoref passed\n");
  } else {
    fprintf(stderr, "FAILURE - TEST_RadxGeoref failed\n");
  }

  return iret;

}

