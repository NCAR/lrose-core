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
// MdvFactory.hh
//
// MdvFactory class - handles the output to MDV files
//
// Frank Hage, RAP, NCAR, 
//
// After Mike Dixon
///////////////////////////////////////////////////////////////
using namespace std;

#ifndef MdvFactory_HH
#define MdvFactory_HH

#include <string>
#include <toolsa/str.h>
#include <dsserver/DsLdataInfo.hh>
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxField.hh>

class MdvFactory {
  
public:
  
  // Constructor  (Empty - Allocates space for local members)
  MdvFactory() {};
  
  // Destructor (Empty - Deallocates local members)
  ~MdvFactory() {};

  //
  // Access Functions

  // Access to Mdvx Object 
  Mdvx& GetMdvxObj() { return _Mdvx_obj; }

  //-----------------------------------------------------
  //  Principal FUNCTIONS:
  //-----------------------------------------------------

  // Build and populate the headers - from Params
  //-----------------------------------------------------
  void BuildHeaders(void);

  // Put a RGB raster into MDV_RGBAIMAGE format - !
  // This version uses the Application Classes's TDRP Params to
  // geo-refrence, set Alpha and set the field labels 
  //-----------------------------------------------------
  void PutData(double *data, uint32 height, uint32 width, uint32 depth);

  // write out merged volume
  //-----------------------------------------------------
  int WriteFile(const time_t data_time, const char *output_dir);


protected:
  
private:

  // Our Newly Constructed MDV Object.
  Mdvx _Mdvx_obj;

  // Our Master Header Workspace
  Mdvx::master_header_t _mhdr;

  // Our Field Header Workspace.
  Mdvx::field_header_t _fhdr;

  // Our Vlevel Header Workspace.
  Mdvx::vlevel_header_t _vhdr;
};

#endif
