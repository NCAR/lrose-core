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

#include <cstdio>  
#include <rapformats/Windshear.hh>
#include <rapformats/WindshearArena.hh>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pjg.h>
#include <dataport/bigend.h>
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
  
  // check prod_id and allow either the SPDB_MAD_ID, or SPDB_XML_ID.
  // SPDB_XML_ID assumes a Windshear class that reads/writes XML,
  // SPDB_MAD_ID assumes an older product specfication

  if (prod_id != SPDB_MAD_ID && prod_id != SPDB_XML_ID)
  {
    cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
    cerr << "  Incorrect prod_id: " << prod_id << endl;
    cerr << "  Should be SPDB_MAD_ID: " << SPDB_MAD_ID << endl;
    cerr << "  Or should be SPDB_XML_ID: " << SPDB_XML_ID << endl;
    return -1;
  }
  else
  {
    // printf("Got product id %d\n",  prod_id);
  }

  Params *serverParams = (Params*) params;

  // create Symprod object

  time_t now = time(NULL);
  Symprod prod(now, now,
	       chunk_ref.valid_time,
	       chunk_ref.expire_time,
	       chunk_ref.data_type,
	       chunk_ref.data_type2,
	       "Mad2Symprod"); 

  MemBuf InBuf;
  InBuf.add(spdb_data, spdb_len);

  if (prod_id == SPDB_MAD_ID)
  {

    BE_to_array_32(InBuf.getPtr(),InBuf.getLen());

    rshape_polygon_t *polygon = (rshape_polygon_t *)InBuf.getPtr();
    
    //////////// ADD CODE TO CHECK MAGNITUDE HERE ///////////////////////


    // Convert the SPDB data to symprod format

    double centroid_lat, centroid_lon;

    add_polygon_to_buffer(prod, polygon, &centroid_lat, &centroid_lon,
			  serverParams);
  
    if (serverParams->render_label)
      add_label_to_buffer(prod, polygon, centroid_lat, centroid_lon,
			  serverParams);

  }
  else if (prod_id == SPDB_XML_ID)
  {
    string s = (const char *)InBuf.getPtr();

    // try out both windshear and arena and used whichever works.
    // the arena one is more picky so use that first.

    WindshearArena a(s, false);
    if (a.isWellFormed())
    {
      _convertArena(a, prod, serverParams);
    }
    else
    {
      Windshear Ws(s);
      if (Ws.isWellFormed())
      {
	bool ignore;
	_convertWindshear(Ws, prod, serverParams, ignore);
	if (ignore)
	{
	  return 0;
	}
      }
      else
      {
	return -1;
      }
    }
  }
  prod.serialize(symprod_buf);
  return(0);
}

void Server::_convertWindshear(const Windshear &Ws, Symprod &prod,
			       Params *params, bool &ignore)
{
  if (params->windshear_threshold >= 0)
  {
    // printf("Threshold=%lf\n", params->windshear_threshold);
    Windshear::Event_t e = Ws.getType();
    if (e == Windshear::WS_GAIN || e == Windshear::WS_LOSS)
    {
      if (Ws.getMagnitude() < params->windshear_threshold)
      {
	// ignore this one, which is ok.
	// printf("Ignoring mag=%lf\n", Ws.getMagnitude());
	ignore = true;
	return;
      }
    }
  }

  ignore = false;
  // Convert the data to symprod format

  double centroid_lat, centroid_lon;

  add_polygon_to_buffer(prod, Ws, &centroid_lat, &centroid_lon, params);
  
  if (params->render_label)
    add_label_to_buffer(prod, Ws, centroid_lat, centroid_lon, params);
  
}

void Server::_convertArena(const WindshearArena &a, Symprod &prod,
			   Params *params)
{
  switch (a.getType())
  {
  case WindshearArena::ARENA_NONE:
    break;
  case WindshearArena::ARENA_WS_GAIN:
    _polygon(prod, a.getDisplayPoints(), params->arena_ws_gain_render,
	     *params);
    break;
  case WindshearArena::ARENA_WS_LOSS:
    _polygon(prod, a.getDisplayPoints(), params->arena_ws_loss_render,
	     *params);
    break;
  case WindshearArena::ARENA_MICROBURST:
    _polygon(prod, a.getDisplayPoints(), params->arena_mb_render,
	     *params);
    break;
  case WindshearArena::ARENA_MODERATE_TURB:
    _polygon(prod, a.getDisplayPoints(), params->arena_turb_render,
	     *params);
    break;
  case WindshearArena::ARENA_SEVERE_TURB:
    _polygon(prod, a.getDisplayPoints(),
	     params->arena_severe_turb_render, *params);
    break;
  }

  if (a.isRunway())
  {
    // draw the polygon always
    _polygon(prod, a.getDisplayPoints(), params->line_render,
	     *params);
  }
  else
  {
    // draw the line with cross lines always
    _offRunway(prod, 
	       a.getCenter(), a.getCross(0), a.getCross(1),
	       params->line_render, *params);
  }

  // draw the box for what is known as the 'compute' or 'alert' box
  _polygon(prod, a.getAlertPoints(), params->compute_box_render,
	   *params);
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

///////////////////////////////////////////////////////
//
//  convert_fill_param() - Convert the TDRP fill parameter to the
//                        matching symprod value.
//

Symprod::fill_t Server::convert_fill_param(int fill)
{
  switch(fill)
  {
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



/////////////////////////////////////////////////////////////////////
//
// add_label_to_buffer() - Add the shape label as a SYMPROD text
//                         object to the indicated buffer.  The label
//                         is rendered at the given lat/lon position.


void Server::add_label_to_buffer( Symprod &prod,
				  rshape_polygon_t *polygon,
				  double lat, double lon,
				  Params *serverParams)
{
  char label[1024];

  sprintf(label, "%.1f", polygon->magnitude);

  string color = "";

  switch(polygon->prod_type)
  {
  case SPDB_MAD_MICROBURST_DATA_TYPE :

    color = serverParams->mb_render.label_color;
    break;

  case SPDB_MAD_CONVERGENCE_DATA_TYPE :

    color = serverParams->ws_gain_render.label_color;
    break;

  case SPDB_MAD_TURBULENCE_DATA_TYPE :
 
    color = serverParams->turb_render.label_color;
    break;

  default:

    color = serverParams->mb_render.label_color;
    break;

  }

  if (!color.empty())
  {
    prod.addText(label,
		 lat, lon,
		 color.c_str(), "",
		 0, 0,
		 Symprod::VERT_ALIGN_CENTER,
		 Symprod::HORIZ_ALIGN_CENTER,
		 strlen(label), // This strikes me as rather odd - Niles. 
		 Symprod::TEXT_NORM,
		 serverParams->label_font);
  }

  return;
}

/////////////////////////////////////////////////////////////////////
//
// add_label_to_buffer() - Add the shape label as a SYMPROD text
//                         object to the indicated buffer.  The label
//                         is rendered at the given lat/lon position.


void Server::add_label_to_buffer( Symprod &prod,
				  const Windshear &ws,
				  double lat, double lon,
				  Params *serverParams)
{
  char label[1024];

  string color = "";
  
  switch (ws.getType())
  {
  case Windshear::WS_GAIN:

    sprintf(label, "%.1lf", ws.getMagnitude());
    color = serverParams->ws_gain_render.label_color;
    break;

  case Windshear::WS_LOSS:

    sprintf(label, "%.1lf", ws.getMagnitude());
    color = serverParams->ws_loss_render.label_color;
    break;

  case Windshear::MODERATE_TURB:
    sprintf(label, "Mod");
    color = serverParams->turb_render.label_color;

  case Windshear::SEVERE_TURB:
 
    sprintf(label, "Svr");
    color = serverParams->severe_turb_render.label_color;

    break;

  case Windshear::MICROBURST:

    sprintf(label, "%.1lf", ws.getMagnitude());
    color = serverParams->mb_render.label_color;
    break;

  case Windshear::RED_X:

    sprintf(label, "%.1lf", ws.getMagnitude());
    color = serverParams->red_x_render.label_color;
    break;

  case Windshear::EMPTY_X:

    color = "";
    break;

  case Windshear::ARENA_WS_GAIN:

    sprintf(label, "%.1lf", ws.getMagnitude());
    color = serverParams->arena_ws_gain_render.label_color;
    break;

  case Windshear::ARENA_WS_LOSS:

    sprintf(label, "%.1lf", ws.getMagnitude());
    color = serverParams->arena_ws_loss_render.label_color;
    break;

  case Windshear::ARENA_MODERATE_TURB:
    sprintf(label, "Mod");
    color = serverParams->arena_turb_render.label_color;

  case Windshear::ARENA_SEVERE_TURB:
 
    sprintf(label, "Svr");
    color = serverParams->arena_severe_turb_render.label_color;

    break;

  case Windshear::ARENA_MICROBURST:

    sprintf(label, "%.1lf", ws.getMagnitude());
    color = serverParams->arena_mb_render.label_color;
    break;

  default:

    break;
  }

  if (!color.empty())
  {
    prod.addText(label,
		 lat, lon,
		 color.c_str(), "",
		 0, 0,
		 Symprod::VERT_ALIGN_CENTER,
		 Symprod::HORIZ_ALIGN_CENTER,
		 strlen(label), // This strikes me as rather odd - Niles. 
		 Symprod::TEXT_NORM,
		 serverParams->label_font);
  }

  return;
}

//////////////////////////////////////////////////////////////////////
//
// add_polygon_to_buffer() - Adds the MAD shape as a Symprod:: polyline
//                           object to the indicated buffer.  The
//                           centroid lat and lon are returned for use
//                           in rendering the label.


void Server::add_polygon_to_buffer(Symprod &prod,
				   rshape_polygon_t *polygon,
				   double *centroid_lat,
				   double *centroid_lon,
				   Params *serverParams)
{

  rshape_xy_t *pt_array;

  double min_lat = 90.0, max_lat = -90.0;
  double min_lon = 360.0, max_lon = -360.0;

  PJGflat_init(polygon->latitude, polygon->longitude, 0.0);
  
  pt_array = (rshape_xy_t *)((char *)polygon + sizeof(rshape_polygon_t));
  
  // load up pts buffer
  
  MemBuf ptBuf;
  
  for (int ipt = 0; ipt < (int)polygon->npt; ipt++) {

    double lat, lon;
    
    PJGflat_xy2latlon(pt_array[ipt].x / 1000.0,
		      pt_array[ipt].y / 1000.0,
		      &lat, &lon);
    
    Symprod::wpt_t pt;
    pt.lat = lat;
    pt.lon = lon;
    ptBuf.add(&pt, sizeof(pt));
    
    if (ipt == 0) {
      min_lat = max_lat = lat;
      min_lon = max_lon = lon;
    } else {
      if (lat < min_lat)
	min_lat = lat;
      if (lat > max_lat)
	max_lat = lat;
      if (lon < min_lon)
	min_lon = lon;
      if (lon > max_lon)
	max_lon = lon;
    }
  }
  
  // set pointer to pts array

  Symprod::wpt_t *pts = (Symprod::wpt_t *) ptBuf.getPtr();
  
  *centroid_lat = (min_lat + max_lat) / 2.0;
  *centroid_lon = (min_lon + max_lon) / 2.0;

  string color;
  Params::Linetype_t line_type;
  int line_width;
  Params::Fill_t fill_type;

  switch(polygon->prod_type)
  {
  case SPDB_MAD_MICROBURST_DATA_TYPE :
    color = serverParams->mb_render.color;
    line_type = serverParams->mb_render.line_type;
    line_width = serverParams->mb_render.line_width;
    fill_type = serverParams->mb_render.fill_style;
    break;

  case SPDB_MAD_CONVERGENCE_DATA_TYPE :
    color = serverParams->ws_gain_render.color;
    line_type = serverParams->ws_gain_render.line_type;
    line_width = serverParams->ws_gain_render.line_width;
    fill_type = serverParams->ws_gain_render.fill_style;
    break;

  case SPDB_MAD_TURBULENCE_DATA_TYPE :
    color = serverParams->turb_render.color;
    line_type = serverParams->turb_render.line_type;
    line_width = serverParams->turb_render.line_width;
    fill_type = serverParams->turb_render.fill_style;
    break;

  default:
    color = serverParams->mb_render.color;
    line_type = serverParams->mb_render.line_type;
    line_width = serverParams->mb_render.line_width;
    fill_type = serverParams->mb_render.fill_style;
    break;
  }
  prod.addPolyline(polygon->npt,
		   pts,
		   color.c_str(), 
		   convert_line_type_param(line_type),
		   line_width,
		   convert_capstyle_param(serverParams->display_capstyle),
		   convert_joinstyle_param(serverParams->display_joinstyle),
		   TRUE,
		   convert_fill_param(fill_type));
  return;
}



//////////////////////////////////////////////////////////////////////
//
// add_polygon_to_buffer() - Adds the MAD shape as a Symprod:: polyline
//                           object to the indicated buffer.  The
//                           centroid lat and lon are returned for use
//                           in rendering the label.


void Server::add_polygon_to_buffer(Symprod &prod,
				   const Windshear &ws,
				   double *centroid_lat,
				   double *centroid_lon,
				   Params *serverParams)
{

  double min_lat = 90.0, max_lat = -90.0;
  double min_lon = 360.0, max_lon = -360.0;

  MemBuf ptBuf;
  
  // printf("Adding polygon with %d points\n", ws.numPt());

  for (int ipt = 0; ipt < ws.numPt(); ipt++)
  {

    double lat, lon;
    ws.ithPt(ipt, lat, lon);
    // printf("\t%lf %lf\n", lat, lon);
    Symprod::wpt_t pt;
    pt.lat = lat;
    pt.lon = lon;
    ptBuf.add(&pt, sizeof(pt));
    
    if (ipt == 0) {
      min_lat = max_lat = lat;
      min_lon = max_lon = lon;
    } else {
      if (lat < min_lat)
	min_lat = lat;
      if (lat > max_lat)
	max_lat = lat;
      if (lon < min_lon)
	min_lon = lon;
      if (lon > max_lon)
	max_lon = lon;
    }
  }
  
  // set pointer to pts array

  Symprod::wpt_t *pts = (Symprod::wpt_t *) ptBuf.getPtr();
  
  *centroid_lat = (min_lat + max_lat) / 2.0;
  *centroid_lon = (min_lon + max_lon) / 2.0;

  string color;
  Params::Linetype_t line_type;
  int line_width;
  Params::Fill_t fill_type;
  bool doAdd = true;

  switch (ws.getType())
  {
  case Windshear::WS_GAIN:
    color = serverParams->ws_gain_render.color;
    line_type = serverParams->ws_gain_render.line_type;
    line_width = serverParams->ws_gain_render.line_width;
    fill_type = serverParams->ws_gain_render.fill_style;
    break;
  case Windshear::WS_LOSS:
    color = serverParams->ws_loss_render.color;
    line_type = serverParams->ws_loss_render.line_type;
    line_width = serverParams->ws_loss_render.line_width;
    fill_type = serverParams->ws_loss_render.fill_style;
    break;
  case Windshear::MODERATE_TURB:
    color = serverParams->turb_render.color;
    line_type = serverParams->turb_render.line_type;
    line_width = serverParams->turb_render.line_width;
    fill_type = serverParams->turb_render.fill_style;
    break;

  case Windshear::SEVERE_TURB:
    color = serverParams->severe_turb_render.color;
    line_type = serverParams->severe_turb_render.line_type;
    line_width = serverParams->severe_turb_render.line_width;
    fill_type = serverParams->severe_turb_render.fill_style;
    break;

  case Windshear::MICROBURST:
    color = serverParams->mb_render.color;
    line_type = serverParams->mb_render.line_type;
    line_width = serverParams->mb_render.line_width;
    fill_type = serverParams->mb_render.fill_style;
    break;

  case Windshear::RED_X:
    color = serverParams->red_x_render.color;
    line_type = serverParams->red_x_render.line_type;
    line_width = serverParams->red_x_render.line_width;
    fill_type = serverParams->red_x_render.fill_style;
    break;

  case Windshear::EMPTY_X:
    color = "white";
    line_type = Params::LINETYPE_SOLID;
    line_width = 0;
    fill_type = Params::FILL_NONE;
    break;

  case Windshear::ARENA_NONE:
    color = serverParams->arena_none_render.color;
    line_type = serverParams->arena_none_render.line_type;
    line_width = serverParams->arena_none_render.line_width;
    fill_type = serverParams->arena_none_render.fill_style;
    break;
  case Windshear::ARENA_WS_GAIN:
    color = serverParams->arena_ws_gain_render.color;
    line_type = serverParams->arena_ws_gain_render.line_type;
    line_width = serverParams->arena_ws_gain_render.line_width;
    fill_type = serverParams->arena_ws_gain_render.fill_style;
    break;
  case Windshear::ARENA_WS_LOSS:
    color = serverParams->arena_ws_loss_render.color;
    line_type = serverParams->arena_ws_loss_render.line_type;
    line_width = serverParams->arena_ws_loss_render.line_width;
    fill_type = serverParams->arena_ws_loss_render.fill_style;
    break;
  case Windshear::ARENA_MODERATE_TURB:
    color = serverParams->arena_turb_render.color;
    line_type = serverParams->arena_turb_render.line_type;
    line_width = serverParams->arena_turb_render.line_width;
    fill_type = serverParams->arena_turb_render.fill_style;
    break;

  case Windshear::ARENA_SEVERE_TURB:
    color = serverParams->arena_severe_turb_render.color;
    line_type = serverParams->arena_severe_turb_render.line_type;
    line_width = serverParams->arena_severe_turb_render.line_width;
    fill_type = serverParams->arena_severe_turb_render.fill_style;
    break;

  case Windshear::ARENA_MICROBURST:
    color = serverParams->arena_mb_render.color;
    line_type = serverParams->arena_mb_render.line_type;
    line_width = serverParams->arena_mb_render.line_width;
    fill_type = serverParams->arena_mb_render.fill_style;
    break;

  case Windshear::NONE:
  default:
    doAdd = false;
    break;
  }

  if (doAdd)
  {
    prod.addPolyline(ws.numPt(),
		     pts,
		     color.c_str(), 
		     convert_line_type_param(line_type),
		     line_width,
		     convert_capstyle_param(serverParams->display_capstyle),
		     convert_joinstyle_param(serverParams->display_joinstyle),
		     TRUE,
		     convert_fill_param(fill_type));
  }
  return;
}

void Server::_polygon(Symprod &prod,
		      const vector<pair<double,double> > &latlon,
		      Params::rendering_t p,
		      Params &params)
{

  MemBuf ptBuf;
  
  // printf("Adding filled polygon with %d points\n", (int)latlon.size());

  for (size_t ipt = 0; ipt < latlon.size(); ++ipt)
  {

    Symprod::wpt_t pt;
    pt.lat = latlon[ipt].first;
    pt.lon = latlon[ipt].second;
    ptBuf.add(&pt, sizeof(pt));
  }
  
  // set pointer to pts array

  Symprod::wpt_t *pts = (Symprod::wpt_t *) ptBuf.getPtr();
  prod.addPolyline(latlon.size(), 
		   pts,
		   p.color,
		   convert_line_type_param(p.line_type),
		   p.line_width,
		   convert_capstyle_param(params.display_capstyle),
		   convert_joinstyle_param(params.display_joinstyle),
		   TRUE,
		   convert_fill_param(p.fill_style));
  return;
}


void Server::_polygon(Symprod &prod,
		      const vector<pair<double,double> > &latlon,
		      Params::line_rendering_t p,
		      Params &params)
{

  MemBuf ptBuf;
  
  // printf("Adding non-filled polygon with %d points\n", (int)latlon.size());

  for (size_t ipt = 0; ipt < latlon.size(); ++ipt)
  {
    Symprod::wpt_t pt;
    pt.lat = latlon[ipt].first;
    pt.lon = latlon[ipt].second;
    ptBuf.add(&pt, sizeof(pt));
  }
  
  // set pointer to pts array

  Symprod::wpt_t *pts = (Symprod::wpt_t *) ptBuf.getPtr();
  prod.addPolyline(latlon.size(), 
		   pts,
		   p.color,
		   convert_line_type_param(p.line_type),
		   p.line_width,
		   convert_capstyle_param(params.display_capstyle),
		   convert_joinstyle_param(params.display_joinstyle),
		   TRUE,
		   convert_fill_param(Params::FILL_NONE));
  return;
}


void Server::_offRunway(Symprod &prod,
			const vector<pair<double,double> > &center,
			const vector<pair<double,double> > &cross0,
			const vector<pair<double,double> > &cross1,
			Params::line_rendering_t p,
			Params &params)
{
  
  // printf("Adding center lines\n");
  _polygon(prod, center, p, params);
  _polygon(prod, cross0, p, params);
  _polygon(prod, cross1, p, params);
}

