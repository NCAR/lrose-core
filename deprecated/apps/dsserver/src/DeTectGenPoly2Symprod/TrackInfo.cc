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
/**
 *
 * @file TrackInfo.cc
 *
 * @class TrackInfo
 *
 * DeTect track information for rendering.
 *  
 * @date 9/20/2011
 *
 */

#include <iostream>

#include "TrackInfo.hh"


// Global variables


/*********************************************************************
 * Constructor
 */

TrackInfo::TrackInfo(const DeTectHorizontalSpdb &record) :
  _spdbInfo(record)
{
  // Save the track infomation

  // If this track is coasted, then there will be only one point and we will
  // need to set the coasted flag

  if (record.isCoasted())
  {
    GenPoly::vertex_t vertex = record.getVertex(0);
    
    point_t point;
    point.lat = vertex.lat;
    point.lon = vertex.lon;
    point.coasted_flag = true;
    
    _track.push_back(point);
    
    return;
  }
  
  // If we get here, we need to add all of the vertices to the track

  int num_vertices = record.getNumVertices();

  for (int i = 0; i < num_vertices; ++i)
  {
    GenPoly::vertex_t vertex = record.getVertex(0);
    
    point_t point;
    point.lat = vertex.lat;
    point.lon = vertex.lon;
    point.coasted_flag = false;
    
    _track.push_back(point);
  }
}


/*********************************************************************
 * Destructor
 */

TrackInfo::~TrackInfo()
{
}


/*********************************************************************
 * addRecord()
 */

void TrackInfo::addRecord(const DeTectHorizontalSpdb &record)
{
//  // If the current information we have is just for a "coasted" record, then
//  // we need to save this record information.  We need to do this because
//  // coasted records don't store any of the target characteristics and it is
//  // nice to have the latest characteristic information for our rendering of
//  // the track.
//
//  if (_spdbInfo.isCoasted())
//    _spdbInfo = record;
  
  // Add this track to the accumulated track.

  // We are assured that the records are being added in reverse chronological
  // order so we know this track is added at the end.  If it is a "coasted"
  // record, then we know there is only one location and.  Otherwise, we need
  // to look for overlapping locations and only add the new ones.
  
  if (record.isCoasted())
  {
    GenPoly::vertex_t vertex = record.getVertex(0);

    point_t point;
    point.lat = vertex.lat;
    point.lon = vertex.lon;
    point.coasted_flag = true;

    _track.push_back(point);
    
    return;
  }
  
  // If the current track is empty, add the new vertices to the track without
  // checking anything

  if (_track.size() == 0)
  {
    int num_vertices = record.getNumVertices();

    for (int i = 0; i < num_vertices; ++i)
    {
      GenPoly::vertex_t vertex = record.getVertex(0);
    
      point_t point;
      point.lat = vertex.lat;
      point.lon = vertex.lon;
      point.coasted_flag = false;
    
      _track.push_back(point);
    }

    return;
  }
  
  // If we get here, we have to look through the vertices in the record and
  // add any that aren't already in the track.

  GenPoly::vertex_t first_vertex = record.getVertex(0);
  int track_start = -1;
  
  for (int i = _track.size() - 1; i >= 0; --i)
  {
    if (_track[i].lat == first_vertex.lat &&
	_track[i].lon == first_vertex.lon)
    {
      track_start = i;
      break;
    }
  }

  // If the first point in this polyline wasn't found in the track, then we can
  // just add all of the vertices in the polyline to the track.

  int num_vertices = record.getNumVertices();

  if (track_start < 0)
  {
    for (int i = 0; i < num_vertices; ++i)
    {
      GenPoly::vertex_t vertex = record.getVertex(0);
    
      point_t point;
      point.lat = vertex.lat;
      point.lon = vertex.lon;
      point.coasted_flag = false;
    
      _track.push_back(point);
    }

    return;
  }
  
  // If the vertex was in the track, then we need to find the first vertex that
  // isn't in the track and add that one and all the following ones to the 
  // track.

  int i, j;
  
  for (i = track_start + 1, j = 1; i < (int)_track.size() && j < num_vertices;
       ++i, ++j)
  {
    GenPoly::vertex_t vertex = record.getVertex(j);
    
    point_t point = _track[i];
    
    if (vertex.lat != point.lat || vertex.lon != point.lon)
    {
      cerr << "===> Found not equal points" << endl;
      cerr << "     i = " << i << ", num_vertices = " << num_vertices << endl;
      cerr << "     j = " << j << ", track size = " << _track.size() << endl;
      
      break;
    }
    
  }
  
  for (; j < (int)_track.size(); ++j)
  {
    GenPoly::vertex_t vertex = record.getVertex(j);
    
    point_t point;
    point.lat = vertex.lat;
    point.lon = vertex.lon;
    point.coasted_flag = false;
    
    _track.push_back(point);
  }
  
}


/*********************************************************************
 * getFieldInfo()
 */

bool TrackInfo::getFieldInfo(const Params::field_id_t field_id,
			     string &field_name, string &field_units,
			     double &field_value) const
{
  // Convert the ID value to the field name

  switch (field_id)
  {
  case Params::AREA_FIELD :
    field_name = DeTectHorizontalSpdb::AREA_LABEL;
    break;
  case Params::AREA_KM_FIELD :
    field_name = DeTectHorizontalSpdb::AREA_KM_LABEL;
    break;
  case Params::MAX_SEGMENT_FIELD :
    field_name = DeTectHorizontalSpdb::MAX_SEGMENT_LABEL;
    break;
  case Params::MAX_SEGMENT_KM_FIELD :
    field_name = DeTectHorizontalSpdb::MAX_SEGMENT_KM_LABEL;
    break;
  case Params::PERIMETER_FIELD :
    field_name = DeTectHorizontalSpdb::PERIMETER_LABEL;
    break;
  case Params::PERIMETER_KM_FIELD :
    field_name = DeTectHorizontalSpdb::PERIMETER_KM_LABEL;
    break;
  case Params::ORIENTATION_FIELD :
    field_name = DeTectHorizontalSpdb::ORIENTATION_LABEL;
    break;
  case Params::ELLIPSE_MAJOR_FIELD :
    field_name = DeTectHorizontalSpdb::ELLIPSE_MAJOR_LABEL;
    break;
  case Params::ELLIPSE_MAJOR_KM_FIELD :
    field_name = DeTectHorizontalSpdb::ELLIPSE_MAJOR_KM_LABEL;
    break;
  case Params::ELLIPSE_MINOR_FIELD :
    field_name = DeTectHorizontalSpdb::ELLIPSE_MINOR_LABEL;
    break;
  case Params::ELLIPSE_MINOR_KM_FIELD :
    field_name = DeTectHorizontalSpdb::ELLIPSE_MINOR_KM_LABEL;
    break;
  case Params::ELLIPSE_RATIO_FIELD :
    field_name = DeTectHorizontalSpdb::ELLIPSE_RATIO_LABEL;
    break;
  case Params::TARGET_WIDTH_FIELD :
    field_name = DeTectHorizontalSpdb::TARGET_WIDTH_LABEL;
    break;
  case Params::TARGET_WIDTH_KM_FIELD :
    field_name = DeTectHorizontalSpdb::TARGET_WIDTH_KM_LABEL;
    break;
  case Params::TARGET_HEIGHT_FIELD :
    field_name = DeTectHorizontalSpdb::TARGET_HEIGHT_LABEL;
    break;
  case Params::TARGET_HEIGHT_KM_FIELD :
    field_name = DeTectHorizontalSpdb::TARGET_HEIGHT_KM_LABEL;
    break;
  case Params::AV_REFLECTIVITY_FIELD :
    field_name = DeTectHorizontalSpdb::AV_REFLECTIVITY_LABEL;
    break;
  case Params::MAX_REFLECTIVITY_FIELD :
    field_name = DeTectHorizontalSpdb::MAX_REFLECTIVITY_LABEL;
    break;
  case Params::MIN_REFLECTIVITY_FIELD :
    field_name = DeTectHorizontalSpdb::MIN_REFLECTIVITY_LABEL;
    break;
  case Params::STD_DEV_REFLECTIVITY_FIELD :
    field_name = DeTectHorizontalSpdb::STD_DEV_REFLECTIVITY_LABEL;
    break;
  case Params::RANGE_REFLECTIVITY_FIELD :
    field_name = DeTectHorizontalSpdb::RANGE_REFLECTIVITY_LABEL;
    break;
  case Params::RANGE_FIELD :
    field_name = DeTectHorizontalSpdb::RANGE_LABEL;
    break;
  case Params::BEARING_FIELD :
    field_name = DeTectHorizontalSpdb::BEARING_LABEL;
    break;
  case Params::TRACK_DISTANCE_FIELD :
    field_name = DeTectHorizontalSpdb::TRACK_DISTANCE_LABEL;
    break;
  case Params::HEADING_FIELD :
    field_name = DeTectHorizontalSpdb::HEADING_LABEL;
    break;
  case Params::SPEED_FIELD :
    field_name = DeTectHorizontalSpdb::SPEED_LABEL;
    break;
  case Params::P_HEADING_FIELD :
    field_name = DeTectHorizontalSpdb::P_HEADING_LABEL;
    break;
  }
  
  // Find the field in the database information

  int field_num;

  if ((field_num = _spdbInfo.getFieldNum(field_name)) < 0)
    return false;
  
  // Save the field information

  field_units = _spdbInfo.getFieldUnits(field_num);
  field_value = _spdbInfo.get1DVal(field_num);
  
  return true;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
