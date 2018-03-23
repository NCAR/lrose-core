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
 * ConvRegionHazardExt.cc: Identical to the ConvRegionHazard Class except
 *                         it adds Polygon numbers based on Titan primary
 *                         and secondary numbers and brings forward
 *                         Titan's storm Centroid for anchoring motion
 *                         vectors
 *
 * RAP, NCAR, Boulder CO
 *
 * Feb 2015
 *
 * Gary Blackburn 
 *
 *********************************************************************/

#include <cstdio>
#include <unistd.h>
#include <vector>

#include <dataport/bigend.h>
#include <dataport/port_types.h>
#include <euclid/WorldPoint2D.hh>
#include <euclid/WorldPolygon2D.hh>
#include <Spdb/ConvRegionHazardExt.hh>
#include <Spdb/WxHazard.hh>
using namespace std;


// Global variables

  
/*********************************************************************
 * Constructor - Creates a ConvRegionHazardExt from the given information.
 */

ConvRegionHazardExt::ConvRegionHazardExt(double top,
				   double speed, double direction,
                                   double longitude, double latitude,
                                   int simple_track_num, int complex_track_num,
                                   int seconds,
				   bool debug_flag) :
  WxHazard(WxHazard::CONVECTIVE_REGION_HAZARD_EXTENDED, debug_flag)
{

  _debugFlag = debug_flag;

  // Create the polygon object

  _polygon = new WorldPolygon2D();

  // Initialize the region information

  _top = top;
  _speed = speed;
  _direction = direction;
  _centroid_lon = longitude;
  _centroid_lat = latitude;
  _simple_track_num = simple_track_num;  
  _complex_track_num = complex_track_num;
  _forecast_seconds = seconds;

  
}


/*********************************************************************
 * Constructor - Creates a ConvRegionHazardExt from the given SPDB buffer.
 */

ConvRegionHazardExt::ConvRegionHazardExt(void *buffer,
				   bool debug_flag) :
  WxHazard(WxHazard::CONVECTIVE_REGION_HAZARD_EXTENDED, debug_flag)
{
  ui08 *buffer_ptr = (ui08 *)buffer;
  
  // Create the polygon object

  _polygon = new WorldPolygon2D();

  // Skip the hazard header
  
  buffer_ptr += sizeof(spdb_hazard_header_t);
  
  // Get the needed information from the conv hazard header

  spdb_header_t *header = (spdb_header_t *)buffer_ptr;
  _spdbHeaderToNative(header);
  
  _top = header->top;
  _speed = header->speed;
  _direction = header->direction;
  _centroid_lon = header->centroid_lon;;
  _centroid_lat = header->centroid_lat;;
  _simple_track_num = header->simple_track_num;
  _complex_track_num = header->complex_track_num;
  _forecast_seconds = header->forecast_seconds;
  
  buffer_ptr += sizeof(spdb_header_t);
  
  // Get the polygon points from the buffer
  
  for (int i = 0; i < header->num_polygon_pts; i++)
  {
    point_t *point = (point_t *)buffer_ptr;
    
    _spdbPointToNative(point);
    
    WorldPoint2D *world_point = new WorldPoint2D(point->lat, point->lon);
    _polygon->addPoint(world_point);
    
    buffer_ptr += sizeof(point_t);
  }
  
}


/*********************************************************************
 * Destructor
 */

ConvRegionHazardExt::~ConvRegionHazardExt()
{
  // Reclaim the space for the polygon

  delete _polygon;
}


/*********************************************************************
 * addPoint() - Add the given point to the end of the region polygon.
 *
 * Note that the pointer to this point is saved for the polygon so you
 * must not change the point value or delete the point object after
 * calling this routine.
 */

void ConvRegionHazardExt::addPoint(WorldPoint2D *point)
{
  _polygon->addPoint(point);
}


/*********************************************************************
 * getSpdbNumBytes() - Retrieve the number of bytes occupied by this
 *                     hazard when stored in an SPDB database.
 */

int ConvRegionHazardExt::getSpdbNumBytes(void) const
{
  int num_bytes = sizeof(spdb_hazard_header_t) + sizeof(spdb_header_t);
  
  if (_polygon == (WorldPolygon2D *)NULL)
    return num_bytes;
  
  num_bytes += _polygon->getNumPoints() * sizeof(point_t);
  
  return num_bytes;
}


/*********************************************************************
 * print() - Print the hazard information to the given stream.
 */

void ConvRegionHazardExt::print(FILE *stream) const
{
  fprintf(stream, "Convective Region Hazard:\n");
  fprintf(stream, "   top = %f feet\n", _top);
  fprintf(stream, "   speed = %f\n", _speed);
  fprintf(stream, "   direction = %f\n", _direction);
  fprintf(stream, "   centroid lat = %f lon = %f\n", _centroid_lat, _centroid_lon);
  fprintf(stream, "   polygon: sequence number = %d,%d\n", _simple_track_num, _complex_track_num);
  fprintf(stream, "            points:\n");
  
  for (WorldPoint2D *point = _polygon->getFirstPoint();
       point != (WorldPoint2D *)NULL;
       point = _polygon->getNextPoint())
    fprintf(stream, "      %f   %f\n",
	    point->lat, point->lon);
  
}


/*********************************************************************
 * writeSpdb() - Write the hazard information to the given buffer in
 *               SPDB format.
 *
 * Note that the calling routine must call getSpdbNumBytes() and allocate
 * a large enough buffer before calling this routine.
 */

void ConvRegionHazardExt::writeSpdb(ui08 *buffer) const
{
  // Write the header information
  
  ui08 *buffer_ptr = (ui08 *)buffer;
  
  spdb_hazard_header_t *hazard_header = (spdb_hazard_header_t *)buffer_ptr;
  
  hazard_header->hazard_type = CONVECTIVE_REGION_HAZARD_EXTENDED;
  hazard_header->spare = 0;
  
  spdbHazardHeaderToBigend(hazard_header);
  
  buffer_ptr += sizeof(spdb_hazard_header_t);

  spdb_header_t *header = (spdb_header_t *)buffer_ptr;
  
  header->top = _top;
  header->speed = _speed;
  header->direction = _direction;
  header->simple_track_num = _simple_track_num;
  header->complex_track_num = _complex_track_num;
  header->centroid_lat = _centroid_lat;
  header->centroid_lon = _centroid_lon;
  
  header->spare_si32 = 0;
  
  if (_polygon == (WorldPolygon2D *)NULL)
  {
    header->num_polygon_pts = 0;
    return;
  }
  
  header->num_polygon_pts = _polygon->getNumPoints();
  
  _spdbHeaderToBigend(header);
  
  buffer_ptr += sizeof(spdb_header_t);
  
  // Save the polygon points

  for (WorldPoint2D *point = _polygon->getFirstPoint();
       point != (WorldPoint2D *)NULL;
       point = _polygon->getNextPoint())
  {
    point_t *point_ptr = (point_t *)buffer_ptr;
    
    point_ptr->lat = point->lat;
    point_ptr->lon = point->lon;
    
    _spdbPointToBigend(point_ptr);
    
    buffer_ptr += sizeof(point_t);
  }
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _spdbHeaderToBigend() - Swaps the spdb_header_t structure from
 *                         native format to big-endian format.
 */

void ConvRegionHazardExt::_spdbHeaderToBigend(spdb_header_t *header)
{
  BE_from_array_32(header, sizeof(spdb_header_t));
}


/*********************************************************************
 * _spdbHeaderToNative() - Swaps the spdb_header_t structure from
 *                         big-endian format to native format.
 */

void ConvRegionHazardExt::_spdbHeaderToNative(spdb_header_t *header)
{
  BE_to_array_32(header, sizeof(spdb_header_t));
}


/*********************************************************************
 * _spdbPointToBigend() - Swaps the point_t structure from native
 *                        format to big-endian format.
 */

void ConvRegionHazardExt::_spdbPointToBigend(point_t *point)
{
  BE_from_array_32(point, sizeof(point_t));
}


/*********************************************************************
 * _spdbPointToNative() - Swaps the point_t structure from big-endian
 *                        format to native format.
 */

void ConvRegionHazardExt::_spdbPointToNative(point_t *point)
{
  BE_to_array_32(point, sizeof(point_t));
}
