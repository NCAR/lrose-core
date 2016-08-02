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
// OutputFile.hh
//
// OutputFile class - handles the output to MDV files
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 1998
//
///////////////////////////////////////////////////////////////

#ifndef OutputFile_HH
#define OutputFile_HH

#include "Params.hh"
#include <mdv/mdv_handle.h>
using namespace std;

class OutputFile {
  
public:
  
  // constructor
  
  OutputFile(char *prog_name, Params *params);
  
  // destructor
  
  virtual ~OutputFile();
  
  // clear data from volume ready for new merge
  void clearVol();

  // return pointer to the field planes
  ui08 ***OutputFile::fieldPlanes() {
    return (ui08 ***) (_handle.field_plane);
  }

  // write out merged volume
  int writeVol(time_t merge_time, time_t start_time, time_t end_time);

  // get handle
  MDV_handle_t *handle() { return (&_handle); }

  // set scale and bias
  void setScaleAndBias(char *field_name, double scale, double bias);

protected:
  
private:
  
  char *_progName;
  Params *_params;
  
  MDV_handle_t _handle;

  int _npointsPlane;

  void _initHeaders();
  
  void _setFieldName(MDV_field_header_t *fhdr,
		     char *name,
		     char *name_long,
		     char *units,
		     char *transform,
		     int field_code,
		     double scale,
		     double bias);

};

#endif
