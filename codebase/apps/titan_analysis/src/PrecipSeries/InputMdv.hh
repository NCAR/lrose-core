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
// InputMdv.hh
//
// InputMdv class
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
///////////////////////////////////////////////////////////////

#ifndef InputMdv_HH
#define InputMdv_HH

#include <string>
#include <mdv/mdv_handle.h>
#include "Params.hh"
using namespace std;

////////////////////////////////
// InputMdv

class InputMdv {
  
public:

  // constructor

  InputMdv(string &prog_name, Params &params);
  
  // destructor
  
  virtual ~InputMdv();

  int isOK;

  // read file for given time

  int read(time_t data_time);

  // compute precip at a point

  double precipRateCompute(double lat, double lon);

protected:
  
private:

  string _progName;
  Params &_params;

  // file handle

  MDV_handle_t _handle;

  // public data members

  mdv_grid_t _grid;
  mdv_grid_comps_t _gridComps;

  int _nFields;
  int _minValidLayer;
  int _maxValidLayer;
  int _nPlanes;
  int _lowDbzByte;
  int _hailDbzByte;

  double _precipLookup[256];
  double _dbzScale;
  double _dbzBias;
  double _prevDbzScale;
  double _prevDbzBias;

  ui08 *_compDbz;
  ui08 **_dbzPlanes;
  int _ncompDbzAlloc;

  void _compDbzAlloc();
  void _compDbzFill();

};

#endif


