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
#include <rapformats/GenPt.hh>
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

  if (prod_id != SPDB_GENERIC_POINT_ID) {
    cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
    cerr << "  Incorrect prod_id: " << prod_id << endl;
    cerr << "  Should be SPDB_GENERIC_POINT_ID: "
	 << SPDB_GENERIC_POINT_ID << endl;
    return -1;
  }
  
  Params *serverParams = (Params*) params;
  
  double TimeLat = 0.0;
  double TimeLon = 0.0;
  double Radius = serverParams->radius;
  double SegLat[NSEGMENTS+1];
  double SegLon[NSEGMENTS+1];
  
  // Copy the SPDB data to the local buffer, and byte-swap
  GenPt genPt;
  genPt.disassemble(spdb_data,spdb_len);

  // create Symprod object

  time_t now = time(NULL);
  Symprod prod(now, now,
	       chunk_ref.valid_time,
	       chunk_ref.expire_time,
	       chunk_ref.data_type,
	       chunk_ref.data_type2,
	       "GenPtCircle");

  // Convert the SPDB data to symprod format
  string color("");

  bool plotCircle = false;
  int lineWidth;

  for (int j = 0; j < serverParams->stations_n; j++)
    {
      cerr << "station: " << serverParams->_stations[j] << endl;
      cerr <<"genPt.getText(): " << genPt.getText() << endl << endl;
      if ( string(serverParams->_stations[j]) == genPt.getText())
	{
	  for (int i = 0; i <  serverParams->circles_n; i++)
	    {
	      if ( genPt.get1DVal(1) >= serverParams->_circles[i].threshLowerBound &&
		   genPt.get1DVal(1) < serverParams->_circles[i].threshUpperBound)
		{
		  plotCircle = true;
		  color = serverParams->_circles[i].color;
		  lineWidth = serverParams->_circles[i].line_width;
		  i = serverParams->circles_n;
		}
	    }
	}
    }
       
  // compute circle geometry
  if (plotCircle)
  {
    compute_geometry(genPt.getLat(), genPt.getLon(),
		     Radius,
		     SegLat, SegLon,
		     TimeLat, TimeLon);
    
    // Add the cirlce
   
    add_circle(serverParams,
	       prod, 0, NSEGMENTS - 1, (char*)color.c_str(), lineWidth,
	       SegLat, SegLon); 

    if (serverParams->plot_text)
    {
      char text[64];
      // assumption is that fields have units of meters 
      if (serverParams->convert_meters_to_inches)
         sprintf(text, "%s:%.1f", genPt.getFieldName(1).c_str(), genPt.get1DVal(1) * 39.3701);
      else
         sprintf(text, "%s:%.1f", genPt.getFieldName(1).c_str(), genPt.get1DVal(1));

      prod.addText(text,
		   genPt.getLat(),genPt.getLon(),
		   serverParams->text_color,
		   serverParams->text_background_color,
		   0, 0,
		   Symprod::VERT_ALIGN_CENTER,
		   Symprod::HORIZ_ALIGN_CENTER,
		   0, Symprod::TEXT_NORM,
		   serverParams->text_font);
    }
   
    if (serverParams->plot_time)
    {
      char text[64];
      sprintf(text, "\ntime: %s", utimstr(genPt.getTime()));

      prod.addText(text,
		   TimeLat, TimeLon,
		   serverParams->time_color,
		   serverParams->text_background_color,
		   0,0,
		   Symprod::VERT_ALIGN_BOTTOM,
		   Symprod::HORIZ_ALIGN_CENTER,
		   0, Symprod::TEXT_NORM,
		   serverParams->text_font);
    }
    
    // set return buffer
    
    if (_isVerbose)
    {
      prod.print(cerr);
    }
  
    prod.serialize(symprod_buf);
  }

  return(0);
  
}

//////////////////////////////////
// compute circle segment geometry

void Server::compute_geometry(double centerLat,
			      double centerLon,
			      double radius,
			      double *segLat, double *segLon,
			      double &timeLat, double &timeLon)

{

  double dtheta = 360.0 / NSEGMENTS;
  
  for (int i = 0; i < NSEGMENTS + 1; i++)
  {
    double theta = i * dtheta;
    
    PJGLatLonPlusRTheta(centerLat, centerLon,
			radius, theta,
			&segLat[i], &segLon[i]);
  }

  PJGLatLonPlusRTheta(centerLat, centerLon,
                      radius, 180.0,
                      &timeLat, &timeLon);
}
		     


/////////////
// add_circle
void Server::add_circle(Params *serverParams,
			Symprod &prod,
			int start_index,
			int end_index,
			char *color, int lineWidth,
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
		   lineWidth,
		   convert_capstyle_param(serverParams->suggested_capstyle),
		   convert_joinstyle_param(serverParams->suggested_joinstyle));
}


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















