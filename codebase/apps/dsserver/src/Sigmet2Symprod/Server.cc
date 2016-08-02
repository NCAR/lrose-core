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

  if (prod_id != SPDB_SIGMET_ID) {
    cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
    cerr << "  Incorrect prod_id: " << prod_id << endl;
    cerr << "  Should be SPDB_SIGMET_ID: " << SPDB_SIGMET_ID << endl;
    return -1;
  }

  Params *serverParams = (Params*) params;

  // Copy the SPDB data to the local buffer, and byte-swap
  
  MemBuf inBuf;
  inBuf.add(spdb_data, spdb_len);
  SIGMET_spdb_t *sigmet = (SIGMET_spdb_t *) inBuf.getPtr();
  SIGMET_spdb_from_BE(sigmet);

  if (serverParams->debug >= Params::DEBUG_VERBOSE) {
    Spdb::printChunkRef(&chunk_ref, cout);
    SIGMET_print_spdb(stdout, sigmet);
  }
  
  // create Symprod object
  
  time_t now = time(NULL);
  Symprod prod(now, now,
	       chunk_ref.valid_time,
	       chunk_ref.expire_time,
	       chunk_ref.data_type,
	       chunk_ref.data_type2,
	       "Sigmet");
  
  // add polyline

  MemBuf wptBuf;
  for (int pt = 0; pt < sigmet->num_vertices; pt++) {
    Symprod::wpt_t wpt;
    wpt.lat = sigmet->vertices[pt].lat;
    wpt.lon = sigmet->vertices[pt].lon;
    wptBuf.add(&wpt, sizeof(wpt));
  }
  
  prod.addPolyline(sigmet->num_vertices,
		   (Symprod::wpt_t *) wptBuf.getPtr(),
		   serverParams->polygon_color,
		   _convertLineType(serverParams->display_linetype),
		   serverParams->display_line_width,
		   _convertCapstyle(serverParams->display_capstyle),
		   _convertJoinstyle(serverParams->display_joinstyle),
		   TRUE,
		   _convertFill(serverParams->polygon_fill));

  // set return buffer
  
  if (_isVerbose) {
    prod.print(cerr);
  }
  prod.serialize(symprod_buf);

  return(0);
  
}

//////////////////////////////////////////////////////////////////////
// Convert the TDRP capstyle parameter to the matching symprod value.

Symprod::capstyle_t Server::_convertCapstyle(int capstyle)
{
  switch(capstyle)
  {
  case Params::CAPSTYLE_BUTT :
    return(Symprod::CAPSTYLE_BUTT);
    
  case Params::CAPSTYLE_NOT_LAST :
    return(Symprod::CAPSTYLE_NOT_LAST);
    
  case Params::CAPSTYLE_PROJECTING :
    return(Symprod::CAPSTYLE_PROJECTING);

  case Params::CAPSTYLE_ROUND :
    return(Symprod::CAPSTYLE_ROUND);
  }
  
  return(Symprod::CAPSTYLE_BUTT);
}


//////////////////////////////////////////////////////////////////////
// Convert the TDRP joinstyle parameter to the matching symprod value.

Symprod::joinstyle_t Server::_convertJoinstyle(int joinstyle)
{
  switch(joinstyle)
  {
  case Params::JOINSTYLE_BEVEL :
    return(Symprod::JOINSTYLE_BEVEL);
    
  case Params::JOINSTYLE_MITER :
    return(Symprod::JOINSTYLE_MITER);
    
  case Params::JOINSTYLE_ROUND :
    return(Symprod::JOINSTYLE_ROUND);
  }
  
  return(Symprod::JOINSTYLE_BEVEL);
}


//////////////////////////////////////////////////////////////////////
// Convert the TDRP line type parameter to the matching symprod value.

Symprod::linetype_t Server::_convertLineType(int line_type)
{
  switch(line_type)
  {
  case Params::LINETYPE_SOLID :
    return(Symprod::LINETYPE_SOLID);
    
  case Params::LINETYPE_DASH :
    return(Symprod::LINETYPE_DASH);
    
  case Params::LINETYPE_DOT_DASH :
    return(Symprod::LINETYPE_DOT_DASH);
  }
  
  return(Symprod::LINETYPE_SOLID);
}

//////////////////////////////////////////////////////////////////////
// Convert the TDRP fill parameter to the matching symprod value.


Symprod::fill_t Server::_convertFill(int fill)
{

  switch(fill)  {

  case Params::FILL_NONE :
    return(Symprod::FILL_NONE);
    
  case Params::FILL_STIPPLE10 :
    return(Symprod::FILL_STIPPLE10);
    
  case Params::FILL_STIPPLE20 :
    return(Symprod::FILL_STIPPLE20);
    
  case Params::FILL_STIPPLE30 :
    return(Symprod::FILL_STIPPLE30);
    
  case Params::FILL_STIPPLE40 :
    return(Symprod::FILL_STIPPLE40);
    
  case Params::FILL_STIPPLE50 :
    return(Symprod::FILL_STIPPLE50);
    
  case Params::FILL_STIPPLE60 :
    return(Symprod::FILL_STIPPLE60);
    
  case Params::FILL_STIPPLE70 :
    return(Symprod::FILL_STIPPLE70);
    
  case Params::FILL_STIPPLE80 :
    return(Symprod::FILL_STIPPLE80);
    
  case Params::FILL_STIPPLE90 :
    return(Symprod::FILL_STIPPLE90);
    
  case Params::FILL_SOLID :
    return(Symprod::FILL_SOLID);
  }
  
  return(Symprod::FILL_NONE);

}


