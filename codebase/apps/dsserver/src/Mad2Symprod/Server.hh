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
// Server.hh
//
// File Server object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2000
//
///////////////////////////////////////////////////////////////


#ifndef _Server_HH
#define _Server_HH

#include <string>

#include <Spdb/Symprod.hh>
#include <Spdb/DsSymprodServer.hh>
#include <cstdio>
#include "Params.hh"
using namespace std;

// the following structs copied from <rsbuilt/rshape.h> to avoid
// dependency on that library

//#include <rsbutil/rshape.h>

class Windshear;
class WindshearArena;

typedef struct
{
    fl32 x;     /* x in meters relative to a fixed reference point */
    fl32 y;     /* y in meters relative to a fixed reference point */
} rshape_xy_t;

typedef struct
{
    si32 prod_type;  /* type of data in the polygon (SPDB_MAD_<>_DATA_TYPE
		      * value found in symprod/spdb_products.h) */
    fl32 magnitude;  /* units depend on type */
    fl32 latitude;   /* fixed reference point (degrees) */
    fl32 longitude;  /* fixed reference point (degrees) */
    ui32 npt;        /* number of rshape_xy_t values in shape */ 
    /* this is then followed by a list of npt rshape_xy_t structs */
} rshape_polygon_t;

class Server : public DsSymprodServer
{

public:
    
  // Constructor:                       
  //   o Registers with procmap
  //   o Opens socket on specified port              

  Server(const string &prog_name,
	 Params       *initialParams);

  // destructor

  virtual ~Server(){};
  
protected:

  // methods invoked from the base class for managing 
  // local override of parameter file

  int      loadLocalParams( const string &paramFile, void **params);
  void     freeLocalParams( void *params )
                          { delete (Params*)params; }

  // Convert the given data chunk from the SPDB database to symprod format.

  int convertToSymprod(const void *params,
		       const string &dir_path,
		       const int prod_id,
		       const string &prod_label,
		       const Spdb::chunk_ref_t &chunk_ref,
		       const Spdb::aux_ref_t &aux_ref,
		       const void *spdb_data,
		       const int spdb_len,
		       MemBuf &symprod_buf);

  Symprod::capstyle_t convert_capstyle_param(int capstyle);
  Symprod::joinstyle_t convert_joinstyle_param(int joinstyle);
  Symprod::linetype_t convert_line_type_param(int line_type);
  Symprod::fill_t convert_fill_param(int fill);
  
  void _convertWindshear(const Windshear &Ws, Symprod &prod,
			 Params *params, bool &ignore);
  void _convertArena(const WindshearArena &a, Symprod &prod,
		     Params *params);
  void add_label_to_buffer(Symprod &prod,
			   rshape_polygon_t *polygon,
			   double lat,
			   double lon,
			   Params *serverParams);
  void add_label_to_buffer(Symprod &prod,
			   const Windshear &ws,
			   double lat,
			   double lon,
			   Params *serverParams);
  
  void add_polygon_to_buffer(Symprod &prod,
			     rshape_polygon_t *polygon,
			     double *centroid_lat,
			     double *centroid_lon,
			     Params *serverParams);
  void add_polygon_to_buffer(Symprod &prod,
			     const Windshear &ws,
			     double *centroid_lat,
			     double *centroid_lon,
			     Params *serverParams);
  
  void _polygon(Symprod &prod,
		const vector<pair<double,double> > &latlon,
		Params::rendering_t p,
		Params &params);

  void _polygon(Symprod &prod,
		const vector<pair<double,double> > &latlon,
		Params::line_rendering_t p,
		Params &params);
  void _offRunway(Symprod &prod,
		  const vector<pair<double,double> > &center,
		  const vector<pair<double,double> > &cross0,
		  const vector<pair<double,double> > &cross1,
		  Params::line_rendering_t p,
		  Params &params);

private:

};

#endif



