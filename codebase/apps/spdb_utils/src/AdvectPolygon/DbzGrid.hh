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
// DbzGrid.hh
// Sue Dettling, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2006
//
///////////////////////////////////////////////////////////////
//
// DbzGrid object functions include reading Mdv dbz field from Mdv
// file, calculating rain rate within a given polygon.
// 
///////////////////////////////////////////////////////////////////////

#ifndef DBZGRID_HH
#define DBZGRID_HH

#include <string>
#include <math.h>
#include <Mdv/MdvxField.hh> 
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxPjg.hh>
#include <dataport/port_types.h>
#include <physics/physics.h>
#include <euclid/point.h>
#include <euclid/geometry.h>
#include <Spdb/DsSpdb.hh>
#include <rapformats/GenPoly.hh>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <toolsa/DateTime.hh>
#include "Params.hh"

using namespace std;

////////////////////////
// This class

class DbzGrid {
  
public:

  // constructor

  DbzGrid ();

  // destructor
  
  ~DbzGrid();
  
  int init(Params &params, time_t requestTime);
  
  float calculateRainRate( GenPoly *polygon);
  
  void clear();
 

protected:  

private:

  Params     _params;
  DsMdvx     _mdv;
  MdvxField *_dbzField;
  fl32     *_dbz;
  float      _dbzMissing;
  float      _dbzBad;
  MdvxPjg    _proj;
  time_t     _dataTime;

};

#endif
