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
// Lookup.hh
//
// Lookup class - handles the lookup tables
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 1998
//
///////////////////////////////////////////////////////////////

#ifndef Lookup_HH
#define Lookup_HH

#include "Params.hh"
#include <rapformats/polar2mdv_lookup.h>
#include <rapformats/clutter_table.h>
#include <rapformats/DsRadarMsg.hh>
using namespace std;

class Lookup {
  
public:
  
  // constructor
  
  Lookup(char *prog_name,
	 Dsr2Mdv_tdrp_struct *params);
  
  // destructor
  
  virtual ~Lookup();

  int OK;

  P2mdv_lookup_file_index_t handle;

  clut_table_file_index_t clut;

  char *lookupTablePath;
  char *clutTablePath;

  int update(int scan_type);
  int checkGeom(DsRadarMsg &radarMsg, int dbzFieldPos);
  int elev_handle (double elev);


protected:
  
private:

  char *_progName;
  Dsr2Mdv_tdrp_struct *_params;
  int _scanType;
  int _prevElev_handle;
  fl32 _prevElev;

  int _checkClutterGeom(DsRadarMsg &radarMsg, int dbzFieldPos);

};

#endif
