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
// GridForecast.hh
//
// GridForecast object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1998
//
///////////////////////////////////////////////////////////////

#ifndef GridForecast_H
#define GridForecast_H

#include <dsdata/DsTrigger.hh>
#include <advect/VectorsAdvector.hh>
#include <Mdv/DsMdvxInput.hh>
#include <Mdv/MdvxField.hh>
#include <tdrp/tdrp.h>
#include <titan/radar.h>
#include <toolsa/umisc.h>

#include "Args.hh"
#include "Params.hh"

#define VGRID_MISSING -9999.999

class GridForecast {
  
public:

  // constructor

  GridForecast (int argc, char **argv);

  // destructor
  
  ~GridForecast();

  // run 

  bool Run();

  // data members

  int OK;
  int Done;
  char *_progName;
  Args *_args;
  Params *_params;
  DsMdvxInput _input;
  
  DsMdvx _motionFile;
  MdvxField *_motionUField;
  MdvxField *_motionVField;
  
  DsMdvx _imageFile;
  MdvxField *_imageField;

  // Processing trigger

  DsTrigger *_dataTrigger;

protected:

private:
 
  //
  // _initTrigger() - Initialize the data trigger.
  //

  bool _readImage(const time_t trigger_time);

  bool _readVectors(const time_t trigger_time);

  bool _writeForecast(const string &image_file_url,
		      const string &output_url,
		      int lead_time_secs,
		      const Mdvx::master_header_t &image_master_hdr,
		      const fl32 *forecast_data);
  
  bool _writeMotion(const VectorsAdvector &vectors,
		    const string &image_file_url,
		    const string &output_motion_url,
		    const int lead_time_secs,
		    const Mdvx::master_header_t &image_master_hdr,
		    const MdvxField &image_field);
  
};

#endif
