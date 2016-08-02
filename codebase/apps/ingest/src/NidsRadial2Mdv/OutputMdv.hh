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
// OutputMdv.hh
//
// OutputMdv class - handles the output to MDV files
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
///////////////////////////////////////////////////////////////

#ifndef OutputMdv_HH
#define OutputMdv_HH

#include <string>
#include <Mdv/mdv/mdv_handle.h>
#include "Params.hh"
using namespace std;

class OutputMdv {
  
public:
  
  // constructor
  
  OutputMdv(const string &prog_name, const Params &params);
  
  // destructor
  
  virtual ~OutputMdv();
  
  // Set the radar position in the grid struct
  void setRadarPos(double radar_lat, double radar_lon,
		   double radar_ht, double elev_angle);

  // initialize the headers
  void initHeaders(const char *radar_name,
		   const char *field_name,
		   const char *field_units,
		   const char *field_transform,
		   const int field_code);
  
  // clear data from volume ready for new merge
  void clearVol();

  // load scale and bias for a given field
  void loadScaleAndBias(const double scale, const double bias);

  // write out merged volume
  int writeVol(const time_t scan_time, const char *output_dir);

  // get handle
  MDV_handle_t *handle() { return (&_handle); }

  // get field plane

  ui08 *getFieldPlane() { return ((ui08 *) _handle.field_plane[0][0]); }

protected:
  
private:
  
  const string &_progName;
  const Params &_params;

  MDV_handle_t _handle;
  mdv_grid_t _grid;

};

#endif
