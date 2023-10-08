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
#include "Era5Data.hh"
#include <Mdv/DsMdvx.hh>

#include <string>
using namespace std;

class OutputFile {
  
public:
  
  // constructor
  
  OutputFile(const string &prog_name, const Params &params,
	     time_t model_time, time_t forecast_time, int forecast_delta,
	     Era5Data &inData /*, Params::afield_name_map_t *field_name_map*/);
  
  // destructor
  
  virtual ~OutputFile();
  
  // return reference to DsMdvx object
  DsMdvx &getDsMdvx() { return (_mdvx); }

  // write out merged volume
  int writeVol();

protected:

  
private:



  const string &_progName;
  const Params &_params;
  // const Params::afield_name_map_t *_field_name_map; //owned by Wrf2Mdv

  DsMdvx _mdvx;

  int _npointsPlane;

  void _initMdvx(time_t model_time, time_t forecast_time,
		 int forecast_delta, Era5Data &inData);
  
  void _setFieldName(Mdvx::field_header_t &fhdr,
		     const char *name,
		     const char *name_long,
		     const char *units,
		     const char *transform,
		     const int field_code);



  void _setFieldName(Mdvx::field_header_t &fhdr,
                     // const Params::output_field_name_t &field_name_enum,
                     const char *units,
                     const char *transform,
		     const int field_code);



};

#endif
