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

#include <dataport/bigend.h>  
#include <rapformats/flt_path.h>

#include "Server.hh"
#include "Params.hh"
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

  if (prod_id != SPDB_FLT_PATH_ID) {
    cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
    cerr << "  Incorrect prod_id: " << prod_id << endl;
    cerr << "  Should be SPDB_FLT_PATH_ID: " << SPDB_FLT_PATH_ID << endl;
    return -1;
  }
  
  const int BITMAP_X_DIM = 16;
  const int BITMAP_Y_DIM = 16;
  const int BITMAP_SIZE = BITMAP_X_DIM * BITMAP_Y_DIM;
  
  int curr_pos_icon[] =
  { 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0 };
  
  
  Params *serverParams = (Params*) params;

  // create Symprod object

  time_t now = time(NULL);
  Symprod prod(now, now,
	       chunk_ref.valid_time,
               chunk_ref.expire_time,
               chunk_ref.data_type,
               chunk_ref.data_type2,
	       "Mad2Symprod"); 

  // make local copy of the data

  MemBuf InBuf;
  InBuf.add(spdb_data, spdb_len);

  // create flight path 'object'

  FLTPATH_path_t *flt_path_buffer = (FLTPATH_path_t *)InBuf.getPtr();
  FLTPATH_path_from_BE(flt_path_buffer);

  // Add the polyline

  MemBuf ptBuf;
  ptBuf.reserve(flt_path_buffer->num_pts * sizeof(Symprod::wpt_t));
  Symprod::wpt_t *pts = (Symprod::wpt_t *) ptBuf.getPtr();
  int num_pts;
  Symprod::wpt_t curr_pt;
  int curr_pt_found = FALSE;
  int begin_future_track = 0;
  
  if (serverParams->debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, "Path has %d points\n", flt_path_buffer->num_pts);
  }
  
  // Put together past track

  num_pts = 0;
  
  for (int i = 0; i < flt_path_buffer->num_pts; i++)
  {
    if (flt_path_buffer->pts[i].time < 0.0)
    {
      pts[i].lat = flt_path_buffer->pts[i].loc.y;
      pts[i].lon = flt_path_buffer->pts[i].loc.x;
      num_pts++;
    }
    else if (flt_path_buffer->pts[i].time == 0.0)
    {
      pts[i].lat = flt_path_buffer->pts[i].loc.y;
      pts[i].lon = flt_path_buffer->pts[i].loc.x;
      num_pts++;

      curr_pt.lat = pts[i].lat;
      curr_pt.lon = pts[i].lon;

      curr_pt_found = TRUE;
      begin_future_track = i;
    }
    else
    {
      begin_future_track = i - 1;
      break;
    }
    
    if (serverParams->debug >= Params::DEBUG_VERBOSE)
      fprintf(stderr,
	      "   Adding pt: lat = %f, lon = %f\n",
	      pts[i].lat, pts[i].lon);
  }

  prod.addPolyline(num_pts,pts,
		   serverParams->past_polyline_color,
		   convert_line_type_param(serverParams->polyline_line_type),
		   serverParams->polyline_line_width,
		   convert_capstyle_param(serverParams->polyline_capstyle),
		   convert_joinstyle_param(serverParams->polyline_join_style));

  // Put together future track

  num_pts = 0;
  
  for (int i = begin_future_track; i < flt_path_buffer->num_pts; i++)
  {
    pts[i-begin_future_track].lat = flt_path_buffer->pts[i].loc.y;
    pts[i-begin_future_track].lon = flt_path_buffer->pts[i].loc.x;

    num_pts++;
    
    if (serverParams->debug >= Params::DEBUG_NORM)
      fprintf(stderr,
	      "   Adding future pt: lat = %f, lon = %f\n",
	      pts[i].lat, pts[i].lon);
  }

  prod.addPolyline(num_pts,pts,
		   serverParams->future_polyline_color,
		   convert_line_type_param(serverParams->polyline_line_type),
		   serverParams->polyline_line_width,
		   convert_capstyle_param(serverParams->polyline_capstyle),
		   convert_joinstyle_param(serverParams->polyline_join_style));

  // Add the current position icon

  if (curr_pt_found)
  {
    ui08 bitmap[BITMAP_SIZE];
  
    //  for (int i = 0; i < BITMAP_SIZE; i++)
    //    bitmap[i] = serverParams->curr_pos_icon.val[i];
    for (int i = 0; i < BITMAP_SIZE; i++) {
      bitmap[i] = curr_pos_icon[i];
    }

    prod.addBitmapIcons(serverParams->curr_pos_icon_color,
			1,
			&curr_pt,
			BITMAP_X_DIM,
			BITMAP_Y_DIM,
			bitmap);
  }
  
  // Copy the internal product to a flat output buffer

  prod.serialize(symprod_buf);
   
  return(0);
  
}


////////////////////////////////////////////////////////
//
// Functions to convert parameter values into more useful types follow.
//

////////////////////////////////////////////////////////////////////
// convert_capstyle_param() - Convert the TDRP capstyle parameter to
//                            the matching symprod value.

Symprod::capstyle_t Server::convert_capstyle_param(int capstyle)
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


/////////////////////////////////////////////////////////////////////
// convert_joinstyle_param() - Convert the TDRP joinstyle parameter to
//                             the matching symprod value.


Symprod::joinstyle_t Server::convert_joinstyle_param(int joinstyle)
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
// convert_line_type_param() - Convert the TDRP line type parameter to
//                             the matching symprod value.


Symprod::linetype_t Server::convert_line_type_param(int line_type)
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























