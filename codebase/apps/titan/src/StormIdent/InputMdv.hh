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
// November 1998
//
///////////////////////////////////////////////////////////////

#ifndef InputMdv_HH
#define InputMdv_HH

#include <mdv/mdv_handle.h>
#include <toolsa/umisc.h>
using namespace std;

class Params;
class Tops;

////////////////////////////////
// InputMdv

class InputMdv {
  
public:

  // constructor

  InputMdv(char *prog_name, Params *params);

  // destructor
  
  virtual ~InputMdv();

  int OK;

  // file handle

  MDV_handle_t handle;

  // public data members

  mdv_grid_t grid;

  int nFields;
  int minValidLayer;
  int maxValidLayer;
  int nPlanes;
  int lowDbzByte;
  int dualDbzByte;
  int highDbzByte;
  int dbzInterval[N_BYTE_DATA_VALS];

  double dbzScale;
  double dbzBias;

  ui08 *compDbz;
  ui08 *dbzVol;
  ui08 **dbzPlanes;
  ui08 **velPlanes;

  // read file for given time
  int read(time_t data_time);

protected:
  
private:

  char *_progName;
  Params *_params;
  Tops *_tops;

  double _prevDbzScale;
  double _prevDbzBias;

  ui08 *_compDbz;
  int _ncompDbzAlloc;

  ui08 **_dbzPlanes;
  ui08 **_velPlanes;
  int _nPlanesAlloc;
  int _nBytesPlaneAlloc;

  void _compDbzAlloc();
  void _compDbzFill();
  void _planesAlloc();
  void _planesFill();

};

#endif


