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
///////////////////////////////////////////////////////////////////////////////
//
// RadarCalib.hh
//
// Radar calibration support
//
// Mike Dixon, EOL, NCAR, Bouler, CO, USA
//
// Oct 2018
//
///////////////////////////////////////////////////////////////////////////////
//
// There are 3 versions of the radar calibration object:
// 
//   libs/rapformats: DsRadarCalib
//   libs/Radx:       RadxRcalib
//   libs/radar:      IwrfCalib
//
// There are good reasons for this setup, but it does make it complicated
// to manage the various classes, and to copy the data from one class to
// another.
//
// This class provides the methods for copying contents from one
// class to another.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef RadarCalib_hh
#define RadarCalib_hh

#include <string>
class RadxRcalib;
class DsRadarCalib;
class IwrfCalib;
using namespace std;

class RadarCalib
{
public:
  
  RadarCalib();
  ~RadarCalib();
  
  // copy between DsRadarCalib and IwrfCalib

  static void copyDsRadarToIwrf(const DsRadarCalib &dsCalib, IwrfCalib &iwrfCalib);
  static void copyIwrfToDsRadar(const IwrfCalib &iwrfCalib, DsRadarCalib &dsCalib);

  // copy between RadxRcalib and IwrfCalib
  
  static void copyRadxToIwrf(const RadxRcalib &rCalib, IwrfCalib &iwrfCalib);
  static void copyIwrfToRadx(const IwrfCalib &iwrfCalib, RadxRcalib &rCalib);

  // copy between DsRadarCalib and RadxRcalib
  
  static void copyRadxToDsRadar(const RadxRcalib &rCalib, DsRadarCalib &dsCalib);
  static void copyDsRadarToRadx(const DsRadarCalib &dsCalib, RadxRcalib &rCalib);

protected:
private:
  
};

#endif
