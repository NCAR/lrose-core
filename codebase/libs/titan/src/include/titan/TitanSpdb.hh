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
// TitanSpdb.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2007
//
///////////////////////////////////////////////////////////////
//
// Routines for handling TITAN spdb data
//
////////////////////////////////////////////////////////////////

#ifndef TitanSpdb_HH
#define TitanSpdb_HH

#include <string>
#include <rapformats/tstorm_spdb.h>
#include <titan/TitanStormFile.hh>
#include <titan/TitanTrackFile.hh>
using namespace std;

////////////////////////
// This class

class TitanSpdb {
  
public:

  static void loadHeader(tstorm_spdb_header_t &header,
			 const storm_file_params_t &sparams,
			 const titan_grid_t &grid,
			 time_t dtime,
			 int n_entries);
  
  static void loadEntry(const tstorm_spdb_header_t &header,
			const track_file_entry_t &file_entry,
			const storm_file_global_props_t &gprops,
			const track_file_forecast_props_t &fprops,
			const titan_grid_comps_t &comps,
			tstorm_spdb_entry_t &entry);
     
};

#endif

