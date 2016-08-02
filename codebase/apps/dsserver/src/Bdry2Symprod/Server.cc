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
#include <ctime>

#include <euclid/Pjg.hh>
#include <rapformats/Bdry.hh>
#include <toolsa/toolsa_macros.h>

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

  if (prod_id != SPDB_BDRY_ID) {
    cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
    cerr << "  Incorrect prod_id: " << prod_id << endl;
    cerr << "  Should be SPDB_BDRY_ID: " << SPDB_BDRY_ID << endl;
    return -1;
  }

  Params *serverParams = (Params*) params;

  // create Symprod object
  
  time_t now = time(NULL);
  Symprod prod(now, now,
	       chunk_ref.valid_time,
               chunk_ref.expire_time,
	       chunk_ref.data_type,
	       chunk_ref.data_type2,
	       "Bdry2Symprod"); 
  
  // make local copy of in buffer, byte-swap

  Bdry bdry;
  bdry.disassemble(spdb_data, spdb_len);
  
  // Now put it in product format so it's a little easier to access the
  // pieces
  
  if (serverParams->debug >= Params::DEBUG_VERBOSE) {
    Spdb::printChunkRef(&chunk_ref, cerr);
    bdry.print(stderr, FALSE);
  }

  // If it is an empty boundary, don't display it

  if(bdry.getLineType() == BDRY_LINE_TYPE_EMPTY ) {
    if (_isDebug)
      cerr << "*** Boundary of type BDRY_LINE_TYPE_EMPTY, not processing" << endl;
    
     return( 0 );
  }

  // Get the polylines

  vector< BdryPolyline > &polylines = bdry.getPolylinesEditable();
  vector< BdryPolyline >::iterator polyline;
    
  if (_isDebug)
    cerr << "*** Database contains " << polylines.size()
	 << " boundaries for requested time" << endl;
  
  // Shift the location of the boundary vertices, if requested

  if (serverParams->latitude_shift != 0.0 ||
      serverParams->longitude_shift != 0.0)
  {
    if (_isDebug)
      cerr << "Shifting boundary location..." << endl;
    
    for (polyline = polylines.begin(); polyline != polylines.end(); ++polyline)
    {
      vector< BdryPoint > &points = polyline->getPointsEditable();
      vector< BdryPoint >::iterator point;
      
      for (point = points.begin(); point != points.end(); ++point) {
	point->setLat(point->getLat() + serverParams->latitude_shift);
	point->setLon(point->getLon() + serverParams->longitude_shift);
      } // point
    } // polyline
  } // endif - latitude_shift or longitude_shift
  
  // Convert the SPDB data to symprod format
  
  for (polyline = polylines.begin(); polyline != polylines.end(); ++polyline)
  {
    // do not display lines with no points

    if (polyline->getNumPoints() == 0)
      continue;

    double map_direction;
    int is_detection;
    
    // We never display the prediction lines.  These are just
    // used by colide.
    
    if (bdry.getType() == BDRY_TYPE_PREDICT_COLIDE)
      continue;
    
    // Convert the database direction to the direction needed by
    // the SPDB routines.
    
    map_direction = bdry.getDirectionPjg();
    
    MemBuf ptBuf;
    
    const vector< BdryPoint > &points = polyline->getPoints();
    vector< BdryPoint >::const_iterator point;
    
    for (point = points.begin(); point != points.end(); ++point)
    {
      Symprod::wpt_t wpt;
      wpt.lat = point->getLat();
      wpt.lon = point->getLon();
      ptBuf.add(&wpt, sizeof(wpt));
    }
    
    // Add the polyline to the product buffer

    char *color;
    int bdry_type = bdry.getType();
    
    if (bdry_type == BDRY_TYPE_BDRY_TRUTH ||
 	bdry_type == BDRY_TYPE_BDRY_MIGFA ||
 	bdry_type == BDRY_TYPE_BDRY_COLIDE ||
 	bdry_type == BDRY_TYPE_COMB_NC_ISSUE ||
 	bdry_type == BDRY_TYPE_COMB_NC_VALID ||
 	bdry_type == BDRY_TYPE_FIRST_GUESS_ISSUE ||
 	bdry_type == BDRY_TYPE_FIRST_GUESS_VALID ||
 	bdry_type == BDRY_TYPE_MINMAX_NC_ISSUE ||
 	bdry_type == BDRY_TYPE_MINMAX_NC_VALID) {

      color = serverParams->detection_color;
      is_detection = TRUE;

    } else {

      // If we are calculating extrapolations, we don't want to
      // display this boundary.

      if (serverParams->calc_extrapolations)
 	continue;
      
      color = serverParams->extrapolation_color;
      is_detection = FALSE;
      
    }

    //
    // Overwrite the color choice if take_color_from_id is
    // TRUE in the parameters and we find the ID in the color
    // look up table (LUT). Niles January 2003.
    //
    bool gotColor = false;
    if (serverParams->take_color_from_id){

      if (serverParams->debug >= Params::DEBUG_VERBOSE) {
	cerr << "Attempting to take color from ID for ID "
	     << bdry.getBdryId() << " ..." << endl;
      }

      for (int i=0; i < serverParams->color_lut_n; i++){
	if (bdry.getBdryId() == serverParams->_color_lut[i].bdryID){
	  color = serverParams->_color_lut[i].color;
	  gotColor = true;
	  break;
	}
      }
    }

    if (serverParams->debug >= Params::DEBUG_VERBOSE) {
      if (gotColor){
	cerr << "   ... succeeded, color is " << color << endl;
      } else {
	cerr << "   ... failed, accepted IDs are : " << endl;
	for (int i=0; i < serverParams->color_lut_n; i++){
	  cerr << "{ " << serverParams->_color_lut[i].bdryID;
	  cerr << ", " << serverParams->_color_lut[i].color << " }" << endl;
	}
      }
    }


    if ((is_detection && serverParams->display_detections) ||
 	(!is_detection && serverParams->display_extrapolations)) {
      
      if (_isDebug)
	cerr << "Adding polyline to product buffer..." << endl;
      
      prod.addPolyline(polyline->getNumPoints(),
		       (Symprod::wpt_t *) ptBuf.getPtr(),
		       color,
		       _convertLineType(serverParams->display_line_type),
		       serverParams->display_line_width,
		       _convertCapstyle(serverParams->display_capstyle),
		       _convertJoinstyle(serverParams->display_joinstyle));

      // Add the optional label to the product buffer

      if (serverParams->display_label) {
	if (_isDebug)
	  cerr << "Adding label to product buffer..." << endl;
	
	string label;
      
 	switch (serverParams->label_source) {
 	case Params::LABEL_DESCRIP :
 	  label = bdry.getDescription();
 	  break;
     	case Params::LABEL_POLYLINE :
 	  label = polyline->getLabel();
 	  break;
 	} // switch

	prod.addText(label.c_str(),
		     points[0].getLat(), points[0].getLon(),
		     color, "", 0,0,
		     Symprod::VERT_ALIGN_BOTTOM,
		     Symprod::HORIZ_ALIGN_CENTER,
		     0, Symprod::TEXT_NORM, 
		     serverParams->label_font);
		      
      } // if (serverParams->display_label)

    }  // if ((is_detection && serverParams->display_detections) ...

    // Add the optional motion vector to the product buffer

    if (serverParams->display_vector &&
 	is_detection &&
 	bdry.getSpeed() > 0.0) {

      if (_isDebug)
	cerr << "Adding motion vector to product buffer..." << endl;
      
      double vector_length =
 	(bdry.getSpeed() * serverParams->extrap_secs) / 3600.0;

      prod.addArrowStartPt( serverParams->vector_color,
			    _convertLineType(serverParams->display_line_type),
			    serverParams->display_line_width,
			    _convertCapstyle(serverParams->display_capstyle),
			    _convertJoinstyle(serverParams->display_joinstyle),
			    point->getLat(), point->getLon(),
			    vector_length,
			    map_direction,
			    serverParams->head_length,
			    serverParams->head_half_angle);

    } // if (serverParams->display_vector ...
    
    // Display the shear information, if requested

    if (serverParams->display_shear_info)
    {
      if (_isDebug)
	cerr << "Adding shear info to product buffer..." << endl;
      
      const vector< BdryPoint> &points = polyline->getPoints();
      bool shear_info_displayed = false;
      BdryPoint last_pt_disp;
      
      for (point = points.begin(); point != points.end(); ++point)
      {
	if (!point->hasShearInfo())
	  continue;
	
	double dist_km = 0.0;
	double theta = 0.0;
	
	if (shear_info_displayed)
	  Pjg::latlon2RTheta(point->getLat(), point->getLon(),
			     last_pt_disp.getLat(), last_pt_disp.getLon(),
			     dist_km, theta);
	
	if (shear_info_displayed &&
	    dist_km < serverParams->shear_min_distance_btwn_fields)
	  continue;
	
	shear_info_displayed = true;
	last_pt_disp = *point;
	
	BdryPointShearInfo shear_info = point->getShearInfo();
	
	double shear_value = 0.0;
	
	switch (serverParams->shear_field_to_display)
	{
	case Params::SHEAR_FIELD_NUM_PTS :
	  shear_value = shear_info.getNumPoints();
	  break;
	case Params::SHEAR_FIELD_ZBAR_CAPE :
	  shear_value = shear_info.getZbarCape();
	  break;
	case Params::SHEAR_FIELD_MAX_SHEAR :
	  shear_value = shear_info.getMaxShear();
	  break;
	case Params::SHEAR_FIELD_MEAN_SHEAR :
	  shear_value = shear_info.getMeanShear();
	  break;
	case Params::SHEAR_FIELD_KMIN :
	  shear_value = shear_info.getKMin();
	  break;
	case Params::SHEAR_FIELD_KMAX :
	  shear_value = shear_info.getKMax();
	  break;
	} /* endswitch - serverParams->shear_field_to_display */
	
	char shear_info_string[BUFSIZ];
	sprintf(shear_info_string, serverParams->shear_field_format_string,
		shear_value);
	
	prod.addText(shear_info_string,
		     point->getLat(), point->getLon(),
		     serverParams->shear_field_color, "", 0,0,
		     Symprod::VERT_ALIGN_BOTTOM,
		     Symprod::HORIZ_ALIGN_CENTER,
		     0, Symprod::TEXT_NORM, 
		     serverParams->label_font);
      } /* endfor - point */
    } /* endif - serverParams->display_shear_info */
    
    // Extrapolate this boundary, if requested.  Do this after the other
    // rendering since this changes the vertex locations of the underlying
    // boundaries.

    if (serverParams->display_extrapolations &&
 	serverParams->calc_extrapolations &&
 	bdry.getSpeed() > 0.0) {

      if (_isDebug)
	cerr << "Extrapolating boundary..." << endl;
      
      if (serverParams->point_extrapolations)
	polyline->extrapPointMotion(serverParams->extrap_secs);
      else
	polyline->extrapolate(serverParams->extrap_secs,
			      bdry.getSpeed(), bdry.getDirectionPjg());
      
      const vector< BdryPoint > &points = polyline->getPoints();
      
      MemBuf ptBuf;
      
      for (point = points.begin(); point != points.end(); ++point) {
	
	Symprod::wpt_t wpt;
	wpt.lat = point->getLat();
	wpt.lon = point->getLon();
	ptBuf.add(&wpt, sizeof(wpt));
      }
      
      if (serverParams->take_color_from_id){
	prod.addPolyline(polyline->getNumPoints(),
			 (Symprod::wpt_t *) ptBuf.getPtr(),
			 color,
			 _convertLineType(serverParams->display_line_type),
			 serverParams->display_line_width,
			 _convertCapstyle(serverParams->display_capstyle),
			 _convertJoinstyle(serverParams->display_joinstyle));
      } else {
	prod.addPolyline(polyline->getNumPoints(),
			 (Symprod::wpt_t *) ptBuf.getPtr(),
			 serverParams->extrapolation_color,
			 _convertLineType(serverParams->display_line_type),
			 serverParams->display_line_width,
			 _convertCapstyle(serverParams->display_capstyle),
			 _convertJoinstyle(serverParams->display_joinstyle));
      }

      // Label the extrapolation
      
      if (serverParams->display_label) {

	if (_isDebug)
	  cerr << "Adding extrapolation label to product buffer..." << endl;
	
 	char ext_label[BUFSIZ];
      
	switch (serverParams->label_source) {
	case Params::LABEL_DESCRIP :
	  sprintf(ext_label, "%d+%d",
		  bdry.getBdryId(),
		  (int)(serverParams->extrap_secs / 60));
	  break;
	case Params::LABEL_POLYLINE :
	  sprintf(ext_label, "Boundary_extrap");
	  break;
	} // switch
	
	prod.addText(ext_label,
		     points[0].getLat(), points[0].getLon(),
		     serverParams->extrapolation_color, "",
		     0, 0,
		     Symprod::VERT_ALIGN_BOTTOM,
		     Symprod::HORIZ_ALIGN_CENTER,
		     0, Symprod::TEXT_NORM,
		     serverParams->label_font);

      } // if (serverParams->display_label)
    
    } // if (serverParams->display_extrapolations ...

  } // obj
  

  // copy internal representation of product to output buffer

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

