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
// IcaoStdAtmos.h
//
// Computations related to the ICAO standard atmosphere
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2002
//
///////////////////////////////////////////////////////////////
//
// This code was obtained in IDL form from:
//
// (c) 2001: Dominik Brunner
//     Institute for Atmospheric and Climate Science, ETH Zurich,
//     Switzerland  (brunner@atmos.umnw.ethz.ch)  
//
//  See http://www.pdas.com/coesa.htm for exact definition
//       
//  The 7 layers of the US standard atmosphere are:
//
//     h1   h2     dT/dh    h1,h2 geopotential alt in km
//      0   11     -6.5     dT/dh in K/km
//     11   20      0.0
//     20   32      1.0
//     32   47      2.8
//     47   51      0.0
//     51   71     -2.8   
//     71   84.852 -2.0
//
//////////////////////////////////////////////////////////////////       

#ifndef IcaoStdAtmos_H
#define IcaoStdAtmos_H

class IcaoStdAtmos {
  
public:

  // constructor

  IcaoStdAtmos();

  // destructor
  
  ~IcaoStdAtmos();

  // convert pressure(mb) to height(m)

  double pres2ht(double pres) const;

  // convert pressure(mb) to flight level (hft)

  double pres2flevel(double pres) const;

  // convert ht(m) to pressure(mb)
  
  double ht2pres(double ht) const;

  // convert flight level (hft) to pressure(mb)
  
  double flevel2pres(double flevel) const;

  // convert ht(m) to tmp (K)

  double ht2temp(double ht) const;

protected:
  
private:

  static const double G;
  static const double R;
  static const double GMR;

  static const double _limits[8];
  static const double _lrs[7];
  static const double _isZero[7];

  double _pb[8];
  double _tb[8];

};

#endif
