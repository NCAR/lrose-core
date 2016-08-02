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
// MdvOutput.hh
//
// MdvOutput class - handles the output to MDV files
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 1998
//
///////////////////////////////////////////////////////////////

#ifndef MdvOutput_HH
#define MdvOutput_HH

#include "Params.hh"
#include "Lookup.hh"
#include <Mdv/mdv/mdv_handle.h>
#include <Mdv/mdv/mdv_radar.h>
#include <rapformats/DsRadarMsg.hh>
#include <rapformats/radar_scan_table.h>
using namespace std;

class MdvOutput {
  
public:
  
  // constructor
  
  MdvOutput(char *prog_name,
	    Dsr2Mdv_tdrp_struct *params,
	    Lookup *lookup);
  
  // destructor
  
  virtual ~MdvOutput();
  
  char *_progName;
  Dsr2Mdv_tdrp_struct *_params;
  int OK;
  
  void setVolHdrs(DsRadarMsg &radarMsg,
		  MDV_radar_grid_t *grid,
		  int n_fields,
		  int *field_pos,
		  int missing_data_val,
		  radar_scan_table_t *scan_table);

  int writeCompleteVol(time_t start_time,
		       time_t end_time,
                       time_t reference_time,
		       ui08 **vol_data,
		       int npoints_plane);

  int writeIntermediateVol(time_t latest_time,
			   int vol_duration,
			   ui08 **vol_data,
			   int npoints_plane);

protected:
  
private:

  MDV_handle_t _handle;
  DsRadarParams_t _radarParamsStruct;
  DsRadarElev_t _elevs;
  Lookup *_lookup;

};

#endif
