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
// DpolFilter.hh
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

#ifndef DpolFilter_HH
#define DpolFilter_HH

using namespace std;

////////////////////////
// This class

class DpolFilter {
  
public:

  // constrain ZDR for precip using DBZ
  //
  // Ref: Brandes et al, JTech, March 2004, pp 461-475
  // Drop size distribution Retrieval with Polarimetric Radar,
  // Model and Application
  
  // filter a single value

  static double constrainZdrFromDbz(double wavelengthCm,
                                    double dbz,
                                    double zdr);

  // filter all gates

  static void constrainZdrFromDbz(double wavelengthCm,
                                  int nGates,
                                  const double *dbzIn,
                                  double *zdr);

  // constrain KDP for precip using DBZ
  //
  // Ref: Vivekanandan et al, BAMS, Vol 80, No 3, March 1999, 381-388
  // Cloud Microphysics Retrieval Using S-Band Dual-Polarization
  // Radar Measurements
  
  // filter a single value

  static double constrainKdpFromDbz(double wavelengthCm,
                                    double dbz,
                                    double zdr);

  // filter all gates

  static void constrainKdpFromDbz(double wavelengthCm,
                                  int nGates,
                                  const double *dbzIn,
                                  double *kdp);

protected:
private:

};

#endif

