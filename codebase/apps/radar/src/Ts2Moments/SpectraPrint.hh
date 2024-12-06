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
///////////////////////////////////////////////////////////
// SpectraPrint
// 
// Singleton - print spectra to a file
//
// EOL, NCAR, Boulder CO
//
// May 2007
//
// Mike Dixon
//
///////////////////////////////////////////////////////////

#ifndef SpectraPrint_HH
#define SpectraPrint_HH

#include "Params.hh"
#include <radar/RadarComplex.hh>
#include <cstdio>
using namespace std;

class SpectraPrint
{

public:

  // Destructor

  ~SpectraPrint(void);
  
  // Retrieve the singleton instance of this class.
  
  static SpectraPrint *Inst(const Params &params);
  static SpectraPrint *Inst();

  // add a spectrum to the file

  void addSpectrumToFile(time_t beamTime,
                         double el, double az, int gateNum,
                         double snr, double vel, double width,
                         int nSamples, const RadarComplex_t *iq);

private:

  // Singleton instance pointer

  static SpectraPrint *_instance;
  
  // Constructor -- private because this is a singleton object

  SpectraPrint(const Params &params);

  // data

  const Params &_params;
  string _filePath;
  FILE *_out;
  int _count;
  
};


#endif
