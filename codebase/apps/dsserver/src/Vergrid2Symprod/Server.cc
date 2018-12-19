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

#include "Server.hh"
#include <rapformats/VerGridRegion.hh>
#include <toolsa/pjg.h>

using namespace std;
#define NSEGMENTS 36

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

  if (prod_id != SPDB_VERGRID_REGION_ID) {
    cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
    cerr << "  Incorrect prod_id: " << prod_id << endl;
    cerr << "  Should be SPDB_VERGRID_REGION_ID: "
	 << SPDB_VERGRID_REGION_ID << endl;
    return -1;
  }

  double CenterLat;
  double CenterLon;
  double TimeLat;
  double TimeLon;
  double Radius;
  double SegLat[NSEGMENTS+1];
  double SegLon[NSEGMENTS+1];
  
  Params *serverParams = (Params*) params;

  // Copy the SPDB data to the local buffer, and byte-swap

  MemBuf inBuf;
  inBuf.add(spdb_data,spdb_len);
  VerGridRegion vgrid;
  vgrid.readChunk(inBuf.getPtr(), inBuf.getLen());

  // create Symprod object

  time_t now = time(NULL);
  Symprod prod(now, now,
	       chunk_ref.valid_time,
	       chunk_ref.expire_time,
	       chunk_ref.data_type,
	       chunk_ref.data_type2,
	       "Vergrid");

  // Convert the SPDB data to symprod format

  for (int i = 0; i < vgrid.nRegions; i++) {

    bool do_truth = (serverParams->plot_truth &&
		     (vgrid.data[i].percent_covered_truth >=
		      serverParams->truth_percentage_threshold));
    
    bool do_forecast = (serverParams->plot_forecast &&
			(vgrid.data[i].percent_covered_forecast >=
			 serverParams->forecast_percentage_threshold));
    
    if (!do_truth && !do_forecast) {
      continue;
    }
    
    // compute circle geometry
    
    compute_geometry(vgrid.data[i].latitude,
		     vgrid.data[i].longitude,
		     vgrid.data[i].radius,
		     CenterLat,
		     CenterLon,
		     Radius,
		     SegLat, SegLon,
		     TimeLat, TimeLon);

    // Add the truth products
    
    if (do_truth) {
      add_truth_circle(serverParams, prod, SegLat, SegLon);
      if (serverParams->plot_text) {
	add_truth_text(serverParams, prod,
		       vgrid.data[i].percent_covered_truth,
		       CenterLat, CenterLon);
      }
    }
    
    // Add the forecast products
    
    if (do_forecast) {
      add_forecast_circle(serverParams, prod, SegLat, SegLon);
      if (serverParams->plot_text) {
	add_forecast_text(serverParams, prod,
			  vgrid.data[i].percent_covered_forecast,
			  CenterLat, CenterLon);
      }
    }

    if (serverParams->plot_time) {
      add_time_text(serverParams, prod, vgrid.hdr->forecast_time,
		    TimeLat, TimeLon);
    }
    
  } // i


  // set return buffer
  
  if (_isVerbose) {
    prod.print(cerr);
  }
  prod.serialize(symprod_buf);

  return(0);
  
}



//////////////////////////////////
// compute circle segment geometry

void Server::compute_geometry(double center_lat,
			      double center_lon,
			      double radius,
			      double &CenterLat,
			      double &CenterLon,
			      double &Radius,
			      double *SegLat, double *SegLon,
			      double &TimeLat, double &TimeLon)

{

  CenterLat = center_lat;
  CenterLon = center_lon;
  Radius = radius;

  double dtheta = 360.0 / NSEGMENTS;
  
  for (int i = 0; i < NSEGMENTS + 1; i++) {

    double theta = i * dtheta;

    PJGLatLonPlusRTheta(CenterLat, CenterLon,
			Radius, theta,
			&SegLat[i], &SegLon[i]);

  }

  PJGLatLonPlusRTheta(CenterLat, CenterLon,
		      Radius, 180.0,
		      &TimeLat, &TimeLon);

}


//////////////////
// add_time_text()

void Server::add_time_text(Params *serverParams,
			   Symprod &prod,
			   time_t forecast_time,
			   double &TimeLat, double &TimeLon)
{

  // form text string

  char text[64];
  sprintf(text, "Ftime: %s", utimstr(forecast_time));

  prod.addText(text,
	       TimeLat, TimeLon,
	       serverParams->time_color,
	       serverParams->text_background_color,
	       0,0,
	       Symprod::VERT_ALIGN_BOTTOM,
	       Symprod::HORIZ_ALIGN_CENTER,
	       0, Symprod::TEXT_NORM, serverParams->text_font);

}




////////////////////
// add_truth_circle

void Server::add_truth_circle(Params *serverParams, Symprod &prod,
			      double *SegLat, double *SegLon)

{

  add_circle(serverParams,
	     prod, 0, NSEGMENTS - 1, serverParams->truth_color,
	     SegLat, SegLon);
  
}

	     
//////////////////////
// add_forecast_circle

void Server::add_forecast_circle(Params *serverParams, Symprod &prod,
				 double *SegLat, double *SegLon)

{

  add_circle(serverParams,
	     prod, 1, NSEGMENTS, serverParams->forecast_color,
	     SegLat, SegLon);
  
}


	
		     
/////////////////
// add_truth_text

void Server::add_truth_text(Params *serverParams, Symprod &prod,
			    double percent_coverage,
			    double CenterLat, double CenterLon)

{

  add_text(serverParams, prod,
	   serverParams->truth_color,
	   Symprod::VERT_ALIGN_TOP,
	   (char *) "Now",
	   percent_coverage,
	   CenterLat, CenterLon);
  
}

     
////////////////////
// add_forecast_text

void Server::add_forecast_text(Params *serverParams,
			       Symprod &prod,
			       double percent_coverage,
			       double CenterLat, double CenterLon)

{

  add_text(serverParams,
	   prod,
	   serverParams->forecast_color,
	   Symprod::VERT_ALIGN_BOTTOM,
	   (char *) "Fcast",
	   percent_coverage,
	   CenterLat, CenterLon);
  
}
		     
	
/////////////
// add_circle

void Server::add_circle(Params *serverParams,
			Symprod &prod,
			int start_index,
			int end_index,
			char *color,
			double *SegLat, double *SegLon)
  
{

  MemBuf pointsBuf;
  
  // load polyline data into pointsBuf
  
  for (int i = start_index; i < end_index; i += 2) {
    
    Symprod::wpt_t point;
    
    point.lon = SegLon[i];
    point.lat = SegLat[i];
    pointsBuf.add(&point, sizeof(point));

    point.lon = SegLon[i+1];
    point.lat = SegLat[i+1];
    pointsBuf.add(&point, sizeof(point));

    if (serverParams->jazz_penup){
        point.lon = Symprod::PPT_PENUP;
        point.lat = Symprod::PPT_PENUP;
    }
    else {
        point.lon = Symprod::WPT_PENUP;
        point.lat = Symprod::WPT_PENUP;
    }
      pointsBuf.add(&point, sizeof(point));

  } // i

  // add the polyline
  
  int npoints = pointsBuf.getLen() / sizeof(Symprod::wpt_t);
  prod.addPolyline(npoints, (Symprod::wpt_t *) pointsBuf.getPtr(), color,
		   convert_line_type_param(serverParams->suggested_line_type),
		   serverParams->suggested_line_width,
		   convert_capstyle_param(serverParams->suggested_capstyle),
		   convert_joinstyle_param(serverParams->suggested_joinstyle));
}


/////////////
// add_text()

void Server::add_text(Params *serverParams,
		      Symprod &prod,
		      char *color,
		      int vert_align,
		      char *label,
		      double percent_coverage,
		      double CenterLat, double CenterLon)

{

  // form text string

  char text[64];
  sprintf(text, "%s:%.0f%%", label, percent_coverage);

  prod.addText(text,
	       CenterLat, CenterLon,
	       color,
	       serverParams->text_background_color,
	       0, 0,
	       (Symprod::vert_align_t) vert_align,
	       Symprod::HORIZ_ALIGN_CENTER,
	       0, Symprod::TEXT_NORM,
	       serverParams->text_font);

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






















