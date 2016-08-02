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

  if (prod_id != SPDB_TREC_PT_FORECAST_ID) {
    cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
    cerr << "  Incorrect prod_id: " << prod_id << endl;
    cerr << "  Should be SPDB_TREC_PT_FORECAST_ID: "
	 << SPDB_TREC_PT_FORECAST_ID << endl;
    return -1;
  }

  Params *serverParams = (Params*) params;

  // Copy the SPDB data to the local buffer, and byte-swap
  
  MemBuf inBuf;
  inBuf.add(spdb_data, spdb_len);

  trec_gauge_handle_t tgauge;
  trec_gauge_init(&tgauge);
  trec_gauge_load_from_chunk(&tgauge, inBuf.getPtr(), inBuf.getLen());

  // create Symprod object
  
  time_t now = time(NULL);
  Symprod prod(now, now,
	       chunk_ref.valid_time,
	       chunk_ref.expire_time,
	       chunk_ref.data_type,
	       chunk_ref.data_type2,
	       "TrecGauge");
  
  // add arrow for vector
  
  if (serverParams->plot_vectors) {
    _addVector(prod, serverParams, &tgauge);
  }
  
  // add text
  
  if (serverParams->plot_dbz_text) {
    _addDbzText(prod, serverParams, &tgauge);
  }
  
  // free up

  trec_gauge_free(&tgauge);

  // set return buffer
  
  if (_isVerbose) {
    prod.print(cerr);
  }
  prod.serialize(symprod_buf);

  return(0);
  
}

/////////////////////////////////////////////////////////////////////////
// add_vector()
//
//  Add an arrow to show movement vector

void Server::_addVector(Symprod &prod,
			Params *serverParams,
			trec_gauge_handle_t *tgauge)
  
{

  double lead_time =
    (tgauge->hdr->n_forecasts - 1) * tgauge->hdr->forecast_delta_time;
  
  double dx = (tgauge->hdr->u * lead_time / 1000.0);
  double dy = (tgauge->hdr->v * lead_time / 1000.0);

  double dist = sqrt(dx * dx + dy * dy);
  double dirn = atan2(dx, dy) * RAD_TO_DEG;
  
  prod.addArrowEndPt(serverParams->vector_color,
		     _convertLineTypeParam(serverParams->suggested_line_type),
		     serverParams->suggested_line_width,
		     _convertCapstyleParam(serverParams->suggested_capstyle),
		     _convertJoinstyleParam(serverParams->suggested_joinstyle),
		     tgauge->hdr->lat,
		     tgauge->hdr->lon,
		     dist, dirn,
		     serverParams->arrow_head_len,
		     serverParams->arrow_head_half_angle);

}

/////////////////////////////////////////////////////////////////////////
// add_dbz_text()
//
//  Add a text object for each dbz value

void Server::_addDbzText(Symprod &prod,
			 Params *serverParams,
			 trec_gauge_handle_t *tgauge)
  
{

  for (int i = 0; i < tgauge->hdr->n_forecasts; i++) {

    double lead_time = i * tgauge->hdr->forecast_delta_time;
    double dx = -(tgauge->hdr->u * lead_time / 1000.0);
    double dy = -(tgauge->hdr->v * lead_time / 1000.0);
  
    double lat, lon;

    PJGLatLonPlusDxDy(tgauge->hdr->lat, tgauge->hdr->lon,
		      dx, dy, &lat, &lon);

    char dbz_text[64];

    sprintf(dbz_text, "%.1f", tgauge->dbz[i]);

    prod.addText(dbz_text,
		 lat, lon,
		 serverParams->text_color, "",
		 0, 0,
		 Symprod::VERT_ALIGN_CENTER,
		 Symprod::HORIZ_ALIGN_CENTER,
		 0, Symprod::TEXT_NORM,
		 serverParams->text_font);
    
  }

}


//////////////////////////////////////////////////////////////////////
// _convertCapstyleParam() - Convert the TDRP capstyle parameter to
//                           the matching symprod value.

Symprod::capstyle_t Server::_convertCapstyleParam(int capstyle)
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
// _convertJoinstyleParam() - Convert the TDRP joinstyle parameter to
//                            the matching symprod value.

Symprod::joinstyle_t Server::_convertJoinstyleParam(int joinstyle)
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
// _convertLineTypeParam() - Convert the TDRP line type parameter to
//                           the matching symprod value.

Symprod::linetype_t Server::_convertLineTypeParam(int line_type)
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



