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
// LordOfTheMdv.hh
//
// LordOfTheMdv object
//
// Watches over MDV data in/out
//
// October 2002
//
///////////////////////////////////////////////////////////////


#ifndef LordOfTheMdv_H
#define LordOfTheMdv_H

#include "Params.hh"

#include <Mdv/DsMdvx.hh> 
using namespace std;

class LordOfTheMdv {
  
public:
  
  // constructor. Sets up DsMdvx object.
  LordOfTheMdv (Params *TDRP_params,
		time_t start,
		time_t end);

  // addGaussian. Adds Gaussian to the output grid.
  void addGaussian(double lat, double lon, 
		   double Duration, double Area);
  
  // destructor. Write Mdv data out.
  ~LordOfTheMdv();

  
protected:
  
private:

  Params *_params;
  DsMdvx *_Mdv;

  time_t _start, _end;

  void _startMdvObject();
  bool _readData();

  
};

#endif
