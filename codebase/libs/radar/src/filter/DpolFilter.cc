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
// DpolFilter.cc
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2009
//
/////////////////////////////////////////////////////////////
//
// Filter dual pol fields based on other variables
//
/////////////////////////////////////////////////////////////

#include <cmath>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <radar/DpolFilter.hh>
#include <radar/FilterUtils.hh>

using namespace std;

/////////////////////////////////////////////////////
// constrain ZDR for precip using DBZ
//
// Ref: Brandes et al, JTech, March 2004, pp 461-475
// Drop size distribution Retrieval with Polarimetric Radar,
// Model and Application

// filter a single gate

double DpolFilter::constrainZdrFromDbz(double wavelengthCm,
                                       double dbzIn,
                                       double zdrIn)
{
  
  double zdrLower = exp(3.12e-4 * dbzIn * dbzIn
			+ 6.48e-2 * dbzIn
			- 3.87);
  if (isfinite(zdrLower) && zdrIn < zdrLower) {
    return zdrLower;
  }

  
  double zdrUpper = exp(1.01e-4 * dbzIn * dbzIn * dbzIn
			- 7.09e-3 * dbzIn * dbzIn
			+ 2.38e-1 * dbzIn
			- 3.44);

  if (isfinite(zdrUpper) && zdrIn > zdrUpper) {
    return zdrUpper;
  }

  return zdrIn;

}

// filter all gates

void DpolFilter::constrainZdrFromDbz(double wavelengthCm,
                                     int nGates,
                                     const double *dbzIn,
                                     double *zdr)

{

  for (int ii = 0; ii < nGates; ii++) {
    zdr[ii] = constrainZdrFromDbz(wavelengthCm, dbzIn[ii], zdr[ii]);
  }

}


/////////////////////////////////////////////////////
// constrain KDP for precip using DBZ
//
// Ref: Vivekanandan et al, BAMS, Vol 80, No 3, March 1999, 381-388
// Cloud Microphysics Retrieval Using S-Band Dual-Polarization
// Radar Measurements
//
// Upper KDP vs DBZ envelope can be approximated by:
//
//  DBZ     KDP
//
//  -10.0   0.1
//   20.0   0.1
//   40.0   0.5
//   50.0   1.5
//   52.0   2.5
//   52.0   20.0

// filter a single gate

double DpolFilter::constrainKdpFromDbz(double wavelengthCm,
                                       double dbzIn,
                                       double kdp)
{

  // table is for 10 cm
  // get wavelength dependency
  
  double mult = (10.0 / wavelengthCm);
  kdp = kdp / mult;
  double kdpConstrained = kdp;

  // below 20 dBZ, kdp max is 0.1

  if (dbzIn < 20.0) {
    
    if (kdp > 0.1) {
      kdpConstrained =  0.1;
    } else {
      kdpConstrained =  kdp;
    }

  } else if (dbzIn <= 52.0) {

    // between 20 and 52, interpolate on piecewise linear
    
    double maxKdp = kdp;
    if (dbzIn < 40.0) {
      maxKdp = FilterUtils::linearInterp(20.0, 0.1, 40.0, 0.5, dbzIn);
    } else if (dbzIn < 50.0) {
      maxKdp = FilterUtils::linearInterp(40.0, 0.5, 50.0, 1.5, dbzIn);
    } else if (dbzIn <= 52.0) {
      maxKdp = FilterUtils::linearInterp(50.0, 1.5, 52.0, 2.5, dbzIn);
    }
    
    if (kdp > maxKdp) {
      kdpConstrained =  maxKdp;
    }

  } else {

    // above 52 dbz, any KDP is valid
    
    kdpConstrained =  kdp;

  }

  return kdpConstrained * mult;

}

// filter all gates

void DpolFilter::constrainKdpFromDbz(double wavelengthCm,
                                     int nGates,
                                     const double *dbzIn,
                                     double *kdp)

{

  for (int ii = 0; ii < nGates; ii++) {
    kdp[ii] = constrainKdpFromDbz(wavelengthCm, dbzIn[ii], kdp[ii]);
  }

}


