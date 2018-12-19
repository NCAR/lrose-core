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
///////////////////////////////////////////////////////////////
// Server.cc
//
// File Server object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2000
//
///////////////////////////////////////////////////////////////

#include <toolsa/toolsa_macros.h>
#include <rapformats/fos.h>
#include <toolsa/pjg.h>
#include <euclid/WorldPoint2D.hh>
#include <euclid/WorldPolygon2D.hh>
#include "Server.hh"
using namespace std;

//////////////////////////////////////////////////////////////////////
// Constructor
//
// Inherits from DsSymprodServer
///

Server::Server(const string &prog_name,
	       Params *initialParams)
  : DsSymprodServer(prog_name,
		 initialParams->instance,
		 (void*)(initialParams),
		 initialParams->port,
		 initialParams->qmax,
		 initialParams->max_clients,
		 initialParams->no_threads,
		 initialParams->debug >= Params::DEBUG_NORM,
		 initialParams->debug >= Params::DEBUG_VERBOSE)
{
}

//////////////////////////////////////////////////////////////////////
// load local params if they are to be overridden.

int
Server::loadLocalParams( const string &paramFile, void **serverParams)

{
   Params  *localParams;
   char   **tdrpOverrideList = NULL;
   bool     expandEnvVars = true;

   const char *routine_name = "_allocLocalParams";

   if (_isDebug) {
     cerr << "Loading new params from file: " << paramFile << endl;
   }

   localParams = new Params( *((Params*)_initialParams) );
   if ( localParams->load( (char*)paramFile.c_str(),
                           tdrpOverrideList,
                           expandEnvVars,
                           _isVerbose ) != 0 ) {
      cerr << "ERROR - " << _executableName << "::" << routine_name << endl
           << "Cannot load parameter file: " << paramFile << endl;
      delete localParams;
      return( -1 );
   }

   if (_isVerbose) {
     localParams->print(stderr, PRINT_SHORT);
   }

   *serverParams = (void*)localParams;
   return( 0 );
}

//////////////////////////////////////////////////////////////////////
// convertToSymprod() - Convert the given data chunk from the SPDB
//                      database to symprod format.
//
// Returns 0 on success, -1 on failure

int Server::convertToSymprod(const void *params,
			     const string &dir_path,
			     const int prod_id,
			     const string &prod_label,
			     const Spdb::chunk_ref_t &chunk_ref,
                             const Spdb::aux_ref_t &aux_ref,
			     const void *spdb_data,
			     const int spdb_len,
			     MemBuf &symprod_buf)
  
{
  
  // check prod_id

  if (prod_id != SPDB_WX_HAZARDS_ID) {
    cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
    cerr << "  Incorrect prod_id: " << prod_id << endl;
    cerr << "  Should be SPDB_WX_HAZARDS_ID: " << SPDB_WX_HAZARDS_ID << endl;
    return -1;
  }

  Params *serverParams = (Params*) params;

  // Copy the SPDB data to local buffer
  
  MemBuf inBuf;
  inBuf.add(spdb_data, spdb_len);

  // Convert the SPDB data to a weather hazards buffer

  Spdb::chunk_ref_t hdr = chunk_ref;
  WxHazardBuffer
    hazard_buffer(&hdr, inBuf.getPtr(),
		  serverParams->debug >= Params::DEBUG_VERBOSE);

  // create Symprod object
  
  time_t now = time(NULL);
  Symprod prod(now, now,
	       chunk_ref.valid_time,
	       chunk_ref.expire_time,
	       chunk_ref.data_type,
	       chunk_ref.data_type2,
	       "WxHAzards");
  
  // Add the convective regions

  for (WxHazard *hazard = hazard_buffer.getFirstHazard();
       hazard != 0;
       hazard = hazard_buffer.getNextHazard()) {

    switch(hazard->getHazardType()) {
      case WxHazard::CONVECTIVE_REGION_HAZARD : {
        _addConvRegion(prod, serverParams,
                       (ConvRegionHazard *) hazard);
        break;
      }
      case WxHazard::CONVECTIVE_REGION_HAZARD_EXTENDED : {
        break;
      }
    }
    
  }

  // set return buffer
  
  if (_isVerbose) {
    prod.print(cerr);
  }
  prod.serialize(symprod_buf);

  return(0);
  
}

//////////////////////////////////////////////////////////////////////////
// Add the given convective region to the Symprod object

void Server::_addConvRegion(Symprod &prod,
			    Params *serverParams,
			    ConvRegionHazard *hazard)

{

  MemBuf pointBuf;

  // Load the polyline data into the point buffer

  WorldPolygon2D *polygon = hazard->getPolygon();
  
  for (WorldPoint2D *point = polygon->getFirstPoint();
       point != (WorldPoint2D *)NULL;
       point = polygon->getNextPoint()) {
    Symprod::wpt_t wpt;
    wpt.lat = point->lat;
    wpt.lon = point->lon;
    pointBuf.add(&wpt, sizeof(wpt));
  }

  int npts = pointBuf.getLen() / sizeof(Symprod::wpt_t);
  prod.addPolyline(npts,
		   (Symprod::wpt_t *) pointBuf.getPtr(),
		   serverParams->conv_region_color,
		   Symprod::LINETYPE_SOLID,
		   serverParams->conv_region_line_width,
		   Symprod::CAPSTYLE_BUTT,
		   Symprod::JOINSTYLE_BEVEL,
		   TRUE,
		   Symprod::FILL_NONE);

}

