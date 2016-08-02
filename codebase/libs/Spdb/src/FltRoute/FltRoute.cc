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
/*********************************************************************
 * FltRoute.cc: Class representing a flight route as stored in an SPDB
 *              database.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cstdio>
#include <unistd.h>

#include <dataport/bigend.h>
#include <toolsa/str.h>
#include <Spdb/Product_defines.hh>
#include <Spdb/DsSpdb.hh>
#include <Spdb/FltRoute.hh>
#include <Spdb/WayPoint.hh>
using namespace std;


// Global variables

  
/*********************************************************************
 * Constructors
 */

FltRoute::FltRoute(const string& id,
		   const bool debug_flag) :
  _debugFlag(debug_flag),
  _id(id)
{
  // Do nothing
}


FltRoute::FltRoute(const char *id,
		   const bool debug_flag) :
  _debugFlag(debug_flag),
  _id(id)
{
  // Do nothing
}


FltRoute::FltRoute(const void *spdb_buffer,
		   const bool debug_flag) :
  _debugFlag(debug_flag)
{
  // Create the flight route object from the SPDB buffer

  spdb_flt_route_t flt_route = *(spdb_flt_route_t *)spdb_buffer;
  spdbToNative(&flt_route);
  
  _id = flt_route.id;
  
  char *buffer_ptr = (char *)spdb_buffer + sizeof(spdb_flt_route_t);
  
  for (int i = 0; i < flt_route.num_way_pts; i++)
  {
    WayPoint way_pt(buffer_ptr, debug_flag);
    
    _wayPoints.push_back(way_pt);
    
    buffer_ptr += way_pt.getSpdbNumBytes();
  }
  
}


/*********************************************************************
 * Destructor
 */

FltRoute::~FltRoute()
{
  // Do nothing
}


/*********************************************************************
 * addWayPoint() - Add the given way point to the route.
 */

void FltRoute::addWayPoint(const WayPoint& way_point)
{
  _wayPoints.push_back(way_point);
}


/*********************************************************************
 * print() - Print the route to the given stream.
 */

void FltRoute::print(FILE *stream) const
{

  vector< WayPoint >::const_iterator way_pt_iter;
  
  fprintf(stream, "Flight Route:\n");
  fprintf(stream, "   id = <%s>\n", _id.c_str());
  fprintf(stream, "   num way pts = %lld\n", (long long) _wayPoints.size());
  fprintf(stream, "\n");
  for (way_pt_iter = _wayPoints.begin();
       way_pt_iter != _wayPoints.end();
       way_pt_iter++)
  {
    way_pt_iter->print(stream);
    fprintf(stream, "\n");
  }

}


/*********************************************************************
 * writeSpdb() - Write the route information to the given byte buffer
 *               in SPDB format. Note that the data will be written to
 *               the buffer in big-endian format.  Note also that it is
 *               assumed that the given buffer is big enough for the
 *               data.
 */

void FltRoute::writeSpdb(void *buffer) const
{
  // Put the header information in the buffer

  spdb_flt_route_t *flt_route = (spdb_flt_route_t *)buffer;
  
  STRcopy(flt_route->id, _id.c_str(), MAX_FLT_ID_LEN);
  flt_route->num_way_pts = _wayPoints.size();
  
  spdbToBigend(flt_route);
  
  // Put each of the way points in the buffer

  char *buffer_ptr = (char *)buffer + sizeof(spdb_flt_route_t);

  vector< WayPoint >::const_iterator way_pt_iter;
  
  for (way_pt_iter = _wayPoints.begin();
       way_pt_iter != _wayPoints.end();
       way_pt_iter++)
  {
    way_pt_iter->writeSpdb((void *)buffer_ptr);
    buffer_ptr += way_pt_iter->getSpdbNumBytes();
  }
  
}


/*********************************************************************
 * writeToDatabase() - Writes the flight route information to the
 *                     indicated database.
 */

void FltRoute::writeToDatabase(const char *database_url,
			       const time_t valid_time,
			       const time_t expire_time,
			       const int data_type) const
{
  const char *routine_name = "writeToDatabase()";
  
  // Allocate space for the SPDB buffer

  int buffer_size = getSpdbNumBytes();
  ui08 *spdb_buffer = new ui08[buffer_size];

  // Put the route information into the buffer

  writeSpdb((void *)spdb_buffer);
  
  // Write the route information to the database

  DsSpdb database;
  
  if (database.put(database_url,
		   SPDB_FLT_ROUTE_ID,
		   SPDB_FLT_ROUTE_LABEL,
		   data_type,
		   valid_time,
		   expire_time,
		   buffer_size,
		   (void *)spdb_buffer) != 0)
  {
    fprintf(stderr, "ERROR: %s::%s\n", _className(), routine_name);
    fprintf(stderr, "Error writing flight route to URL <%s>\n",
	    database_url);
    fprintf(stderr, "%s\n", database.getErrStr().c_str());
  }
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * spdbToBigend() - Swaps the spdb_flt_route_t structure from native
 *                  format to big-endian format.
 */

void FltRoute::spdbToBigend(spdb_flt_route_t *flt_route)
{
  BE_from_array_32((char *)flt_route + MAX_FLT_ID_LEN,
		   sizeof(spdb_flt_route_t) - MAX_FLT_ID_LEN);
}


/*********************************************************************
 * spdbToNative() - Swaps the spdb_flt_route_t structure from big-endian
 *                  format to native format.
 */

void FltRoute::spdbToNative(spdb_flt_route_t *flt_route)
{
  BE_to_array_32((char *)flt_route + MAX_FLT_ID_LEN,
		 sizeof(spdb_flt_route_t) - MAX_FLT_ID_LEN);
}
