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
//////////////////////////////////////////////////////////
// Symprod_add.cc
//
// Add routines for symbolic product class
//
// Mike Dixon, from Nancy Rehak
// RAP, NCAR, Boulder, CO, 80307, USA
//
// Dec 1999
//
//////////////////////////////////////////////////////////

#include <math.h>
#include <Spdb/Symprod.hh>
#include <Spdb/SymprodObj.hh>
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <toolsa/pjg.h>
#include <toolsa/toolsa_macros.h>
using namespace std;

///////////////////////////////////////////////////////
// Add a generic object

void Symprod::addObject(SymprodObj *obj,
			int obj_type,
			bbox_t &bbox)
{
  _objs.push_back(obj);
  _objTypes.push_back(obj_type);
  _prodProps.num_objs++;
  updateBbox(_prodProps.bounding_box, bbox);
}

///////////////////////////////////////////////////////
// Add a SYMPROD text object.

void Symprod::addText(const char *text_string,
		      double lat,
		      double lon,
		      const char *color,
		      const char *background_color,
		      int offset_x /* = 0*/,
		      int offset_y /* = 0*/,
		      vert_align_t vert_alignment /* = VERT_ALIGN_CENTER*/,
		      horiz_align_t horiz_alignment /* = HORIZ_ALIGN_CENTER*/,
		      int font_size /* = 0*/,
		      font_style_t font_style /* = TEXT_NORM*/,
		      const char *fontname /* = NULL*/,
		      int object_id /* = 0*/,
		      int detail_level /* = 0*/ )
  
{

  // compute bounding box & centroid

  bbox_t bbox;
  initBbox(bbox);
  updateBbox(bbox, lat, lon);
  
  wpt_t centroid;
  centroid.lat = lat;
  centroid.lon = lon;

  // load hdr

  obj_hdr_t hdr;
  MEM_zero(hdr);
  hdr.object_type = OBJ_TEXT;
  hdr.object_id = object_id;
  hdr.num_bytes = sizeof(text_props_t) + strlen(text_string) + 1;
  hdr.detail_level = detail_level;
  STRncopy(hdr.color, color, SYMPROD_COLOR_LEN);
  STRncopy(hdr.background_color, background_color, SYMPROD_COLOR_LEN);
  hdr.centroid = centroid;

  // load props

  text_props_t props;
  MEM_zero(props);
  props.origin = centroid;
  props.offset.x = offset_x;
  props.offset.y = offset_y;
  props.vert_alignment = vert_alignment;
  props.horiz_alignment = horiz_alignment;
  props.font_size = font_size;
  props.font_style = font_style;
  props.length = strlen(text_string);
  if (fontname == NULL) {
    STRncopy(props.fontname, "None", SYMPROD_FONT_NAME_LEN);
  } else {
    STRncopy(props.fontname, fontname, SYMPROD_FONT_NAME_LEN);
  }

  // create a text object

  SymprodText *obj = new SymprodText(hdr, props, text_string);

  // add it, updating the product bounding box

  addObject(obj, hdr.object_type, bbox);

}

//////////////////////////////////////////////////////////
// Add a polyline object.
//

void Symprod::addPolyline(int npoints,
			  const wpt_t *pts,
			  const char *color,
			  linetype_t linetype /* = LINETYPE_SOLID*/,
			  int linewidth /* = 1*/,
			  capstyle_t capstyle /* = CAPSTYLE_BUTT*/,
			  joinstyle_t joinstyle /* = JOINSTYLE_BEVEL*/,
			  bool close_flag /* = false*/,
			  fill_t fill /* = FILL_NONE*/,
			  int object_id /* = 0*/,
			  int detail_level /* = 0*/ )

{
  
  // compute bounding box & centroid

  bbox_t bbox;
  initBbox(bbox);
  const wpt_t *pt = pts;
  for (int ipt = 0; ipt < npoints; ipt++, pt++) {
    if (pt->lat != WPT_PENUP &&
	pt->lon != WPT_PENUP)
      updateBbox(bbox, pt->lat, pt->lon);
  }
  
  wpt_t centroid;
  centroid.lat = (bbox.min_lat + bbox.max_lat) / 2.0;
  centroid.lon = (bbox.min_lon + bbox.max_lon) / 2.0;
  
  // load hdr
  
  obj_hdr_t hdr;
  MEM_zero(hdr);
  hdr.object_type = OBJ_POLYLINE;
  hdr.object_id = object_id;
  hdr.num_bytes = sizeof(polyline_props_t) + npoints * sizeof(wpt_t);
  hdr.detail_level = detail_level;
  STRncopy(hdr.color, color, SYMPROD_COLOR_LEN);
  hdr.centroid = centroid;

  // load props

  polyline_props_t props;
  MEM_zero(props);
  props.close_flag = close_flag;
  props.fill = fill;
  props.linetype = linetype;
  props.linewidth = linewidth;
  props.capstyle = capstyle;
  props.joinstyle = joinstyle;
  props.line_interp = LINE_INTERP_STRAIGHT;
  props.num_points = npoints;

  // create a polyline object

  SymprodPolyline *obj = new SymprodPolyline(hdr, props, pts);

  // add it, updating the product bounding box

  addObject(obj, hdr.object_type, bbox);

}

void Symprod::addPolyline(int npoints,
			  const wpt_t *pts,
                          double centroid_lat,
                          double centroid_lon,
			  const char *color,
			  linetype_t linetype /* = LINETYPE_SOLID*/,
			  int linewidth /* = 1*/,
			  capstyle_t capstyle /* = CAPSTYLE_BUTT*/,
			  joinstyle_t joinstyle /* = JOINSTYLE_BEVEL*/,
			  bool close_flag /* = false*/,
			  fill_t fill /* = FILL_NONE*/,
			  int object_id /* = 0*/,
			  int detail_level /* = 0*/ )

{

  // set centroid

  wpt_t centroid;
  centroid.lat = centroid_lat;
  centroid.lon = centroid_lon;
  
  // compute bounding box

  bbox_t bbox;
  initBbox(bbox);
  const wpt_t *pt = pts;
  for (int ipt = 0; ipt < npoints; ipt++, pt++) {
    if (pt->lat != WPT_PENUP &&
	pt->lon != WPT_PENUP)
      updateBbox(bbox, pt->lat, pt->lon);
  }
  
  // load hdr
  
  obj_hdr_t hdr;
  MEM_zero(hdr);
  hdr.object_type = OBJ_POLYLINE;
  hdr.object_id = object_id;
  hdr.num_bytes = sizeof(polyline_props_t) + npoints * sizeof(wpt_t);
  hdr.detail_level = detail_level;
  STRncopy(hdr.color, color, SYMPROD_COLOR_LEN);
  hdr.centroid = centroid;

  // load props

  polyline_props_t props;
  MEM_zero(props);
  props.close_flag = close_flag;
  props.fill = fill;
  props.linetype = linetype;
  props.linewidth = linewidth;
  props.capstyle = capstyle;
  props.joinstyle = joinstyle;
  props.line_interp = LINE_INTERP_STRAIGHT;
  props.num_points = npoints;

  // create a polyline object

  SymprodPolyline *obj = new SymprodPolyline(hdr, props, pts);

  // add it, updating the product bounding box

  addObject(obj, hdr.object_type, bbox);

}

//////////////////////////////////////////////////////////
// Add a iconline object.
//

void Symprod::addIconline(double origin_lat,
			  double origin_lon,
			  int npoints,
			  const ppt_t *pts,
			  const char *color,
			  linetype_t linetype /* = LINETYPE_SOLID*/,
			  int linewidth /* = 1*/,
			  capstyle_t capstyle /* = CAPSTYLE_BUTT*/,
			  joinstyle_t joinstyle /* = JOINSTYLE_BEVEL*/,
			  bool close_flag /* = false*/,
			  fill_t fill /* = FILL_NONE*/,
			  int object_id /* = 0*/,
			  int detail_level /* = 0*/ )

{
  
  // bounding box - icon line probably does not span more that
  // 1 degree each way
  
  bbox_t bbox;
  initBbox(bbox);
  updateBbox(bbox, origin_lat - 1.0, origin_lon - 1.0);
  updateBbox(bbox, origin_lat - 1.0, origin_lon + 1.0);
  updateBbox(bbox, origin_lat + 1.0, origin_lon - 1.0);
  updateBbox(bbox, origin_lat + 1.0, origin_lon + 1.0);
  
  wpt_t centroid;
  centroid.lat = origin_lat;
  centroid.lon = origin_lon;
  
  // load hdr
  
  obj_hdr_t hdr;
  MEM_zero(hdr);
  hdr.object_type = OBJ_ICONLINE;
  hdr.object_id = object_id;
  hdr.num_bytes = sizeof(iconline_props_t) + npoints * sizeof(ppt_t);
  hdr.detail_level = detail_level;
  STRncopy(hdr.color, color, SYMPROD_COLOR_LEN);
  hdr.centroid = centroid;

  // load props

  iconline_props_t props;
  MEM_zero(props);
  props.origin.lat = origin_lat;
  props.origin.lon = origin_lon;
  props.close_flag = close_flag;
  props.fill = fill;
  props.linetype = linetype;
  props.linewidth = linewidth;
  props.capstyle = capstyle;
  props.joinstyle = joinstyle;
  props.line_interp = LINE_INTERP_STRAIGHT;
  props.num_points = npoints;

  // create a iconline object

  SymprodIconline *obj = new SymprodIconline(hdr, props, pts);

  // add it, updating the product bounding box

  addObject(obj, hdr.object_type, bbox);

}

/////////////////////////////////////////////////////////////////
// Add stroked icons.

void Symprod::addStrokedIcons(const char *color,
			      int num_icon_pts,
			      const ppt_t *icon_pts,
			      int num_icons,
			      const wpt_t *icon_origins,
			      int object_id /* = 0*/,
			      int detail_level /* = 0*/,
			      int linewidth /* = 1*/ )
{
  
  // compute bounding box & centroid

  bbox_t bbox;
  initBbox(bbox);
  for (int ipt = 0; ipt < num_icons; ipt++) {
    updateBbox(bbox, icon_origins[ipt].lat, icon_origins[ipt].lon);
  }
  
  wpt_t centroid;
  centroid.lat = (bbox.min_lat + bbox.max_lat) / 2.0;
  centroid.lon = (bbox.min_lon + bbox.max_lon) / 2.0;
  
  // load hdr
  
  obj_hdr_t hdr;
  MEM_zero(hdr);
  hdr.object_type = OBJ_STROKED_ICON;
  hdr.object_id = object_id;
  hdr.num_bytes = sizeof(stroked_icon_props_t) +
    num_icon_pts * sizeof(ppt_t) + num_icons * sizeof(wpt_t);
  hdr.detail_level = detail_level;
  STRncopy(hdr.color, color, SYMPROD_COLOR_LEN);
  hdr.centroid = centroid;

  // load props

  stroked_icon_props_t props;
  MEM_zero(props);
  props.num_icon_pts = num_icon_pts;
  props.num_icons = num_icons;
  props.linewidth = linewidth;

  // create a stroked icon object

  SymprodStrokedIcon *obj =
    new SymprodStrokedIcon(hdr, props, icon_pts, icon_origins);

  // add it, updating the product bounding box

  addObject(obj, hdr.object_type, bbox);

}

/////////////////////////////////////////////////////////////////
// Add named icons.

void Symprod::addNamedIcons(const char *name,
			    const char *color,
			    int num_icons,
			    const wpt_t *icon_origins,
			    int object_id /* = 0*/,
			    int detail_level /* = 0*/ )
{
  
  // compute bounding box & centroid

  bbox_t bbox;
  initBbox(bbox);
  for (int ipt = 0; ipt < num_icons; ipt++) {
    updateBbox(bbox, icon_origins[ipt].lat, icon_origins[ipt].lon);
  }
  
  wpt_t centroid;
  centroid.lat = (bbox.min_lat + bbox.max_lat) / 2.0;
  centroid.lon = (bbox.min_lon + bbox.max_lon) / 2.0;
  
  // load hdr
  
  obj_hdr_t hdr;
  MEM_zero(hdr);
  hdr.object_type = OBJ_NAMED_ICON;
  hdr.object_id = object_id;
  hdr.num_bytes = sizeof(named_icon_props_t) + num_icons * sizeof(wpt_t);
  hdr.detail_level = detail_level;
  STRncopy(hdr.color, color, SYMPROD_COLOR_LEN);
  hdr.centroid = centroid;

  // load props

  named_icon_props_t props;
  MEM_zero(props);
  STRncopy(props.name, name, SYMPROD_ICON_NAME_LEN);
  props.num_icons = num_icons;

  // create a named icon object
  
  SymprodNamedIcon *obj =
    new SymprodNamedIcon(hdr, props, icon_origins);

  // add it, updating the product bounding box

  addObject(obj, hdr.object_type, bbox);
  
}

/////////////////////////////////////////////////////////////////
// Add bitmap icons.

void Symprod::addBitmapIcons(const char *color,
			     int num_icons,
			     const wpt_t *icon_origins,
			     int bitmap_x_dim,
			     int bitmap_y_dim,
			     const ui08 *bitmap,
			     int object_id /* = 0*/,
			     int detail_level /* = 0*/ )
{
  
  // compute bounding box & centroid

  bbox_t bbox;
  initBbox(bbox);
  for (int ipt = 0; ipt < num_icons; ipt++) {
    updateBbox(bbox, icon_origins[ipt].lat, icon_origins[ipt].lon);
  }
  
  wpt_t centroid;
  centroid.lat = (bbox.min_lat + bbox.max_lat) / 2.0;
  centroid.lon = (bbox.min_lon + bbox.max_lon) / 2.0;
  
  // load hdr
  
  obj_hdr_t hdr;
  MEM_zero(hdr);
  hdr.object_type = OBJ_BITMAP_ICON;
  hdr.object_id = object_id;
  hdr.num_bytes = (sizeof(bitmap_icon_props_t) +
		   (num_icons * sizeof(wpt_t)) +
		   (bitmap_x_dim * bitmap_y_dim * sizeof(ui08)));
  hdr.detail_level = detail_level;
  STRncopy(hdr.color, color, SYMPROD_COLOR_LEN);
  hdr.centroid = centroid;
  
  // load props
  
  bitmap_icon_props_t props;
  MEM_zero(props);
  props.bitmap_x_dim = bitmap_x_dim;
  props.bitmap_y_dim = bitmap_y_dim;
  props.num_icons = num_icons;
  
  // create a bitmap icon object
  
  SymprodBitmapIcon *obj =
    new SymprodBitmapIcon(hdr, props, icon_origins, bitmap);

  // add it, updating the product bounding box

  addObject(obj, hdr.object_type, bbox);
  
}

//////////////////////////////////////////////////////////
// Add an arc object.
//

void Symprod::addArc(double origin_lat,
		     double origin_lon,
		     double radius_x,
		     double radius_y,
		     const char *color,
		     bool radii_in_pixels /* = false = same units as origin */,
		     double angle1 /* = 0.0*/,
		     double angle2 /* = 360.0*/,
		     double axis_rotation /* = 0.0*/,
		     int nsegments /* = 90*/,
		     linetype_t linetype /* = LINETYPE_SOLID*/,
		     int linewidth /* = 1*/,
		     capstyle_t capstyle /* = CAPSTYLE_BUTT*/,
		     joinstyle_t joinstyle /* = JOINSTYLE_BEVEL*/,
		     fill_t fill /* = FILL_NONE*/,
		     int object_id /* = 0*/,
		     int detail_level /* = 0*/ )
  
{
  
  // compute bounding box & centroid

  double maxr = MAX(radius_x, radius_y);
  double dlat = maxr * DEG_PER_KM_AT_EQ;
  double dlon = dlat * cos(origin_lat * RAD_PER_DEG);
  
  bbox_t bbox;
  bbox.min_lat = origin_lat - dlat;
  bbox.max_lat = origin_lat + dlat;
  bbox.min_lon = origin_lon - dlon;
  bbox.max_lon = origin_lon + dlon;

  wpt_t origin;
  origin.lat = origin_lat;
  origin.lon = origin_lon;
  
  // load hdr
  
  obj_hdr_t hdr;
  MEM_zero(hdr);
  hdr.object_type = OBJ_ARC;
  hdr.object_id = object_id;
  hdr.num_bytes = sizeof(arc_props_t);
  hdr.detail_level = detail_level;
  STRncopy(hdr.color, color, SYMPROD_COLOR_LEN);
  hdr.centroid = origin;

  // load props

  arc_props_t props;
  MEM_zero(props);
  props.origin = origin;
  props.radius_x = radius_x;
  props.radius_y = radius_y;
  props.angle1 = angle1;
  props.angle2 = angle2;
  props.axis_rotation = axis_rotation;
  props.linetype = linetype;
  props.linewidth = linewidth;
  props.fill = fill;
  props.capstyle = capstyle;
  props.joinstyle = joinstyle;
  props.nsegments = nsegments;
  props.radii_in_pixels = (int) radii_in_pixels;

  // create a arc object
  
  SymprodArc *obj = new SymprodArc(hdr, props);

  // add it, updating the product bounding box
  
  addObject(obj, hdr.object_type, bbox);

}

//////////////////////////////////////////////////////////
// Add a rectangle object.
//

void Symprod::addRectangle(double origin_lat,
			   double origin_lon,
			   double height,
			   double width,
			   const char *color,
			   linetype_t linetype /* = LINETYPE_SOLID*/,
			   int linewidth /* = 1*/,
			   capstyle_t capstyle /* = CAPSTYLE_BUTT*/,
			   joinstyle_t joinstyle /* = JOINSTYLE_BEVEL*/,
			   fill_t fill /* = FILL_NONE*/,
			   int object_id /* = 0*/,
			   int detail_level /* = 0*/ )
  
{
  
  // compute bounding box & centroid

  double dlat = height * DEG_PER_KM_AT_EQ;
  double dlon = width  * DEG_PER_KM_AT_EQ * cos(origin_lat * RAD_PER_DEG);
  
  bbox_t bbox;
  bbox.min_lat = origin_lat;
  bbox.max_lat = origin_lat + dlat;
  bbox.min_lon = origin_lon;
  bbox.max_lon = origin_lon + dlon;

  wpt_t origin;
  origin.lat = origin_lat;
  origin.lon = origin_lon;
  
  // load hdr
  
  obj_hdr_t hdr;
  MEM_zero(hdr);
  hdr.object_type = OBJ_RECTANGLE;
  hdr.object_id = object_id;
  hdr.num_bytes = sizeof(rectangle_props_t);
  hdr.detail_level = detail_level;
  STRncopy(hdr.color, color, SYMPROD_COLOR_LEN);
  hdr.centroid = origin;

  // load props

  rectangle_props_t props;
  MEM_zero(props);
  props.origin = origin;
  props.height = height;
  props.width = width;
  props.linetype = linetype;
  props.linewidth = linewidth;
  props.fill = fill;
  props.capstyle = capstyle;
  props.joinstyle = joinstyle;

  // create a rectangle object
  
  SymprodRectangle *obj = new SymprodRectangle(hdr, props);

  // add it, updating the product bounding box
  
  addObject(obj, hdr.object_type, bbox);

}

/////////////////////////////////////////////////////////
// Add a chunk object.

void Symprod::addChunk(double min_lat,
		       double min_lon,
		       double max_lat,
		       double max_lon,
		       int chunk_type,
		       int nbytes_chunk,
		       const void *data,
		       const char *color,
		       const char *background_color,
		       int user_info /* = 0*/,
		       int object_id /* = 0*/,
		       int detail_level /* = 0*/ )

{

  // compute bounding box and centroid

  bbox_t bbox;
  bbox.min_lat = min_lat;
  bbox.max_lat = max_lat;
  bbox.min_lon = min_lon;
  bbox.max_lon = max_lon;

  wpt_t centroid;
  centroid.lat = (max_lat + min_lat) / 2.0;
  centroid.lon = (max_lon + min_lon) / 2.0;

  // load hdr
  
  obj_hdr_t hdr;
  MEM_zero(hdr);
  hdr.object_type = OBJ_CHUNK;
  hdr.object_id = object_id;
  hdr.num_bytes = sizeof(chunk_props_t) + nbytes_chunk;
  hdr.detail_level = detail_level;
  STRncopy(hdr.color, color, SYMPROD_COLOR_LEN);
  STRncopy(hdr.background_color, background_color, SYMPROD_COLOR_LEN);
  hdr.centroid = centroid;

  // load props

  chunk_props_t props;
  MEM_zero(props);
  props.chunk_type = chunk_type;
  props.user_info = user_info;
  props.nbytes_chunk = nbytes_chunk;

  // create a chunk object
  
  SymprodChunk *obj = new SymprodChunk(hdr, props, data);
  
  // add it, updating the product bounding box
  
  addObject(obj, hdr.object_type, bbox);

}

///////////////////////////////////////////////////////////////////////
// Add an arrow object given start and end points
// head_len_km: in km
// head_half_angle: in deg

void Symprod::addArrowBothPts(const char *color,
			      linetype_t linetype,
			      int linewidth,
			      capstyle_t capstyle,
			      joinstyle_t joinstyle,
			      double start_lat,
			      double start_lon,
			      double end_lat,
			      double end_lon,
			      double head_len_km,
			      double head_half_angle,
			      int object_id /* = 0*/,
			      int detail_level /* = 0*/ )
  
{
  
  double length, dirn;
  
  PJGLatLon2RTheta(start_lat, start_lon, end_lat, end_lon,
		   &length, &dirn);
  
  addArrow(color, linetype, linewidth, capstyle, joinstyle,
	   start_lat, start_lon, end_lat, end_lon, length, dirn,
	   head_len_km, head_half_angle, object_id, detail_level);

}

void Symprod::addArrowBothPts(const char *color,
			      linetype_t linetype,
			      int linewidth,
			      capstyle_t capstyle,
			      joinstyle_t joinstyle,
			      double start_lat,
			      double start_lon,
 			      double end_lat,
			      double end_lon,
			      int head_len_pixels,
			      double head_half_angle,
			      int object_id /* = 0*/,
			      int detail_level /* = 0*/ )
  
{
  
  double length, dirn;
  
  PJGLatLon2RTheta(start_lat, start_lon, end_lat, end_lon,
		   &length, &dirn);
  
  addArrow(color, linetype, linewidth, capstyle, joinstyle,
	   start_lat, start_lon, end_lat, end_lon, length, dirn,
	   head_len_pixels, head_half_angle, object_id, detail_level);

}

///////////////////////////////////////////////////////////////////////
// Add an arrow object given start pt, length, dirn
// head_len_km: in km
// head_half_angle: in deg

void Symprod::addArrowStartPt(const char *color,
			      linetype_t linetype,
			      int linewidth,
			      capstyle_t capstyle,
			      joinstyle_t joinstyle,
			      double start_lat,
			      double start_lon,
			      double length,
			      double dirn,
			      double head_len_km,
			      double head_half_angle,
			      int object_id /* = 0*/,
			      int detail_level /* = 0*/ )
  
{
  
  double end_lat, end_lon;
  
  PJGLatLonPlusRTheta(start_lat, start_lon, length, dirn,
		      &end_lat, &end_lon);
  
  addArrow(color, linetype, linewidth, capstyle, joinstyle,
	   start_lat, start_lon, end_lat, end_lon, length, dirn,
	   head_len_km, head_half_angle, object_id, detail_level);
  
}

void Symprod::addArrowStartPt(const char *color,
			      linetype_t linetype,
			      int linewidth,
			      capstyle_t capstyle,
			      joinstyle_t joinstyle,
			      double start_lat,
			      double start_lon,
			      double length,
			      double dirn,
			      int head_len_pixels,
			      double head_half_angle,
			      int object_id /* = 0*/,
			      int detail_level /* = 0*/ )
  
{
  
  double end_lat, end_lon;
  
  PJGLatLonPlusRTheta(start_lat, start_lon, length, dirn,
		      &end_lat, &end_lon);
  
  addArrow(color, linetype, linewidth, capstyle, joinstyle,
	   start_lat, start_lon, end_lat, end_lon, length, dirn,
	   head_len_pixels, head_half_angle, object_id, detail_level);
  
}

///////////////////////////////////////////////////////////////////////
// Add an arrow object given end pt, length, dirn
// head_len_km: in km
// head_half_angle: in deg

void Symprod::addArrowEndPt(const char *color,
			    linetype_t linetype,
			    int linewidth,
			    capstyle_t capstyle,
			    joinstyle_t joinstyle,
			    double end_lat,
			    double end_lon,
			    double length,
			    double dirn,
			    double head_len_km,
			    double head_half_angle,
			    int object_id /* = 0*/,
			    int detail_level /* = 0*/ )
  
{

  double start_lat, start_lon;
  
  PJGLatLonPlusRTheta(end_lat, end_lon, length, dirn + 180.0,
		      &start_lat, &start_lon);
  
  addArrow(color, linetype, linewidth, capstyle, joinstyle,
	   start_lat, start_lon, end_lat, end_lon, length, dirn,
	   head_len_km, head_half_angle, object_id, detail_level);
  
}

void Symprod::addArrowEndPt(const char *color,
			    linetype_t linetype,
			    int linewidth,
			    capstyle_t capstyle,
			    joinstyle_t joinstyle,
			    double end_lat,
			    double end_lon,
			    double length,
			    double dirn,
			    int head_len_pixels,
			    double head_half_angle,
			    int object_id /* = 0*/,
			    int detail_level /* = 0*/ )
  
{

  double start_lat, start_lon;
  
  PJGLatLonPlusRTheta(end_lat, end_lon, length, dirn + 180.0,
		      &start_lat, &start_lon);
  
  addArrow(color, linetype, linewidth, capstyle, joinstyle,
	   start_lat, start_lon, end_lat, end_lon, length, dirn,
	   head_len_pixels, head_half_angle, object_id, detail_level);
  
}

///////////////////////////////////////////////////////////////////////
// Add an arrow object given mid pt, length, dirn
// head_len_km: in km
// head_half_angle: in deg

void Symprod::addArrowMidPt(const char *color,
			    linetype_t linetype,
			    int linewidth,
			    capstyle_t capstyle,
			    joinstyle_t joinstyle,
			    double mid_lat,
			    double mid_lon,
			    double length,
			    double dirn,
			    double head_len_km,
			    double head_half_angle,
			    int object_id /* = 0*/,
			    int detail_level /* = 0*/ )
  
{

  double start_lat, start_lon;
  double end_lat, end_lon;
  
  PJGLatLonPlusRTheta(mid_lat, mid_lon, length / 2.0, dirn,
		      &end_lat, &end_lon);
  
  PJGLatLonPlusRTheta(mid_lat, mid_lon, length / 2.0, dirn + 180.0,
		      &start_lat, &start_lon);
  
  addArrow(color, linetype, linewidth, capstyle, joinstyle,
	   start_lat, start_lon, end_lat, end_lon, length, dirn,
	   head_len_km, head_half_angle, object_id, detail_level);

}

void Symprod::addArrowMidPt(const char *color,
			    linetype_t linetype,
			    int linewidth,
			    capstyle_t capstyle,
			    joinstyle_t joinstyle,
			    double mid_lat,
			    double mid_lon,
			    double length,
			    double dirn,
			    int head_len_pixels,
			    double head_half_angle,
			    int object_id /* = 0*/,
			    int detail_level /* = 0*/ )
  
{

  double start_lat, start_lon;
  double end_lat, end_lon;
  
  PJGLatLonPlusRTheta(mid_lat, mid_lon, length / 2.0, dirn,
		      &end_lat, &end_lon);
  
  PJGLatLonPlusRTheta(mid_lat, mid_lon, length / 2.0, dirn + 180.0,
		      &start_lat, &start_lon);
  
  addArrow(color, linetype, linewidth, capstyle, joinstyle,
	   start_lat, start_lon, end_lat, end_lon, length, dirn,
	   head_len_pixels, head_half_angle, object_id, detail_level);

}

///////////////////////////////////////////////////////////////////////
// addArrow()
//
// Add an generic arrow object given the start and end lat, length and dirn.
//
// head_len_km: in km
// head_half_angle: in deg

void Symprod::addArrow(const char *color,
		       linetype_t linetype,
		       int linewidth,
		       capstyle_t capstyle,
		       joinstyle_t joinstyle,
		       double start_lat,
		       double start_lon,
		       double end_lat,
		       double end_lon,
		       double length,
		       double dirn,
		       double head_len_km,
		       double head_half_angle,
		       int object_id /* = 0*/,
		       int detail_level /* = 0*/ )
  
{

  // compute the arrow points

  wpt_t arrow[5];
  arrow[0].lat = start_lat;
  arrow[0].lon = start_lon;
  arrow[1].lat = end_lat;
  arrow[1].lon = end_lon;

  PJGLatLon2RTheta(start_lat, start_lon, end_lat, end_lon,
		   &length, &dirn);

  double angle = dirn + 180.0 + head_half_angle;
  double lat, lon;
  PJGLatLonPlusRTheta(end_lat, end_lon, head_len_km, angle,
		      &lat, &lon);

  arrow[2].lat = lat;
  arrow[2].lon = lon;

  arrow[3].lat = end_lat;
  arrow[3].lon = end_lon;

  angle = dirn + 180.0 - head_half_angle;

  PJGLatLonPlusRTheta(end_lat, end_lon, head_len_km, angle,
		      &lat, &lon);

  arrow[4].lat = lat;
  arrow[4].lon = lon;

  // add polyline object
  
  addPolyline(5, arrow, color, linetype, linewidth, capstyle, joinstyle,
	      FALSE, FILL_NONE, object_id, detail_level);

}

///////////////////////////////////////////////////////////////////////
// addArrow()
//
// Add an generic arrow object given the start and end lat, length and dirn
//
// head_len_pixels: in pixels (icon space)
// head_half_angle: in deg

void Symprod::addArrow(const char *color,
		       linetype_t linetype,
		       int linewidth,
		       capstyle_t capstyle,
		       joinstyle_t joinstyle,
		       double start_lat,
		       double start_lon,
		       double end_lat,
		       double end_lon,
		       double length,
		       double dirn,
		       int head_len_pixels,
		       double head_half_angle,
		       int object_id /* = 0*/,
		       int detail_level /* = 0*/ )
  
{

  // compute the arrow end points

  wpt_t shaft[2];
  shaft[0].lat = start_lat;
  shaft[0].lon = start_lon;
  shaft[1].lat = end_lat;
  shaft[1].lon = end_lon;

  // add polyline for shaft
  
  addPolyline(2, shaft, color, linetype, linewidth, capstyle, joinstyle,
	      FALSE, FILL_NONE, object_id, detail_level);

  // compute direction and head angles
  
  PJGLatLon2RTheta(start_lat, start_lon, end_lat, end_lon,
		   &length, &dirn);
  
  double angle1 = 270 - dirn - head_half_angle;
  double angle2 = 270 - dirn + head_half_angle;

  // compute head icon line

  ppt_t head[3];

  head[0].x = (int)
    ((double) head_len_pixels * cos(angle1 * DEG_TO_RAD) + 0.5);
  head[0].y = (int)
    ((double) head_len_pixels * sin(angle1 * DEG_TO_RAD) + 0.5);

  head[1].x = 0;
  head[1].y = 0;

  head[2].x = (int)
    ((double) head_len_pixels * cos(angle2 * DEG_TO_RAD) + 0.5);
  head[2].y = (int)
    ((double) head_len_pixels * sin(angle2 * DEG_TO_RAD) + 0.5);

  // add head icon line

  addIconline(end_lat, end_lon, 3, head, color,
	      linetype, linewidth, capstyle, joinstyle,
	      FALSE, FILL_NONE, object_id, detail_level);
}

///////////////////////////////////////////////////////////////////////
// addWindBarb()
//
// Add an wind barb object
//
//  	speed 				//knots
//    	dir 				//degrees
// 	station_posn_circle_radius 	//pixels
// 	wind_barb_line_width		//pixels
// 	wind_ticks_angle_to_shaft	//degrees
// 	wind_barb_tick_len		//pixels
// 	wind_barb_shaft_len		//pixels
void Symprod::addWindBarb(double lat, double lon, const char* color, double speed, double dir, int station_posn_circle_radius, int wind_barb_line_width, double wind_ticks_angle_to_shaft, double wind_barb_tick_len, int wind_barb_shaft_len, int detail_level)
{

  // abort if wind value is too high
  if (speed > 350.0) {
    return;
  }

    // add circle to show location
    if (station_posn_circle_radius > 0) {
      addArc(lat, lon,
                  station_posn_circle_radius,
                  station_posn_circle_radius,
                  color, true,
                  0, 360.0, 0.0, 60,		     
                  LINETYPE_SOLID, 1,
                  CAPSTYLE_BUTT,
                  JOINSTYLE_BEVEL,
                  FILL_NONE,
                  0, detail_level);
    }

  // return now if speed below 2.5
  if (speed < 2.5) {
    return;
  }

  // add shaft
  double dirrad = dir * DEG_TO_RAD;
  double cosdir = cos(dirrad);
  double sindir = sin(dirrad);
  
  ppt_t shaft[2];
  shaft[0].x = 0;
  shaft[0].y = 0;
  shaft[1].x = (int) floor(wind_barb_shaft_len * sindir + 0.5);
  shaft[1].y = (int) floor(wind_barb_shaft_len * cosdir + 0.5);
  
  addIconline(lat, lon,
		   2, shaft,
		   color,		     
		   LINETYPE_SOLID,
		   wind_barb_line_width,
		   CAPSTYLE_BUTT,
		   JOINSTYLE_BEVEL, false,
		   FILL_NONE,
		   0, detail_level);


 // add flags for each 50 kts
  double tick_rel_angle;
  if (lat >= 0.0) {
    tick_rel_angle = wind_ticks_angle_to_shaft;
  } else {
    tick_rel_angle = -wind_ticks_angle_to_shaft;
  }
  double tick_rad = dirrad + tick_rel_angle * DEG_TO_RAD;
  double tick_len = wind_barb_tick_len;
  double tick_point_offset_x = tick_len * sin(tick_rad);
  double tick_point_offset_y = tick_len * cos(tick_rad);
  
  double tick_spacing;
  if (speed < 150.0) {
    tick_spacing = wind_barb_shaft_len / 7.5;
  } else {
    tick_spacing = wind_barb_shaft_len / 10.0;
  }
  if (tick_spacing < 3.0) {
    tick_spacing = 3.0;
  }
  double tick_dx = tick_spacing * sindir;
  double tick_dy = tick_spacing * cosdir;
  
  // Add the speed markings

  double speed_left = speed;
  double shaft_left = wind_barb_shaft_len;

  double x_inner = shaft_left * sindir;
  double y_inner = shaft_left * cosdir;

  // flags for each 50 kts

  while (speed_left >= 47.5) {
    
    double x_outer = x_inner;
    double y_outer = y_inner;

    x_inner -= tick_dx;
    y_inner -= tick_dy;

    double x_point = x_outer + tick_point_offset_x;
    double y_point = y_outer + tick_point_offset_y;

    ppt_t flag[3];
    flag[0].x = (int) floor(x_outer + 0.5);
    flag[0].y = (int) floor(y_outer + 0.5);
    flag[1].x = (int) floor(x_point + 0.5);
    flag[1].y = (int) floor(y_point + 0.5);
    flag[2].x = (int) floor(x_inner + 0.5);
    flag[2].y = (int) floor(y_inner + 0.5);
    addIconline(lat, lon,
		     3, flag,
		     color,
		     LINETYPE_SOLID,
		     wind_barb_line_width,
		     CAPSTYLE_BUTT,
		     JOINSTYLE_BEVEL,
		     true,
		     FILL_SOLID,		     
		     0, detail_level);

    speed_left -= 50.0;
    shaft_left -= tick_spacing;

  } // while (speed_left >= 47.5)

 
  if (speed > 47.5) {
    x_inner -= tick_dx / 2.0;
    y_inner -= tick_dy / 2.0;
  }
  while (speed_left >= 7.5) {
    
    double x_outer = x_inner;
    double y_outer = y_inner;

    x_inner -= tick_dx;
    y_inner -= tick_dy;

    double x_point = x_outer + tick_point_offset_x;
    double y_point = y_outer + tick_point_offset_y;

    ppt_t full_tick[2];
    full_tick[0].x = (int) floor(x_outer + 0.5);
    full_tick[0].y = (int) floor(y_outer + 0.5);
    full_tick[1].x = (int) floor(x_point + 0.5);
    full_tick[1].y = (int) floor(y_point + 0.5);
    addIconline(lat, lon,
		     2, full_tick,
		     color,		     
		     LINETYPE_SOLID,
 		     wind_barb_line_width,
		     CAPSTYLE_BUTT,
		     JOINSTYLE_BEVEL, false,
		     FILL_NONE,
		     0, detail_level);
    
    speed_left -= 10.0;
    shaft_left -= tick_spacing;

  } // while (speed_left >= 7.5)

  // half ticks for each 5 kts

  // single 5-kt barb is rendered in from the end

  if (speed < 7.5) {
    x_inner -= tick_dx;
    y_inner -= tick_dy;
  }

  if (speed_left >= 2.5) {
    
    double x_point = x_inner + tick_point_offset_x / 2.0;
    double y_point = y_inner + tick_point_offset_y / 2.0;
    
    ppt_t half_tick[2];
    half_tick[0].x = (int) floor(x_inner + 0.5);
    half_tick[0].y = (int) floor(y_inner + 0.5);
    half_tick[1].x = (int) floor(x_point + 0.5);
    half_tick[1].y = (int) floor(y_point + 0.5);
    addIconline(lat, lon,
		     2, half_tick,
		     color,		     
		     LINETYPE_SOLID,
 		     wind_barb_line_width,
		     CAPSTYLE_BUTT,
		     JOINSTYLE_BEVEL, false,
		     FILL_NONE,
		     0, detail_level);

  } // if (speed_left >= 5)

}
