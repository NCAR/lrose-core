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
// Symprod_print.cc
//
// Printing for Symbolic product class
//
// Mike Dixon, from Nancy Rehak
// RAP, NCAR, Boulder, CO, 80307, USA
//
// Dec 1999
//
//////////////////////////////////////////////////////////

#include <Spdb/Symprod.hh>
#include <Spdb/SymprodObj.hh>
#include <toolsa/udatetime.h>
#include <iomanip>
using namespace std;

void Symprod::print(ostream &out)
  
{

  printProdHdrProps(out, _prodProps);
  // Symprod::printProdHdrOffsets(out, props->num_objs, objOffsets);
  // out << "   object types: " << endl;
  for (size_t i = 0; i < _objs.size(); i++) {
    out << "----------------------------------------------" << endl;
    out << "Object num: " << i << endl;
    _objs[i]->print(out);
    out << endl;
  }
  out << endl;
  
}

///////////////////////////////////////////////////////////////////////
// Print properties of a prod_hdr
//

void Symprod::printProdHdrProps(ostream &out,
				const prod_hdr_props_t &props)
  
{

  out << endl;
  out << "Product Header: " << endl;
  out << endl;
  
  out << "   generate time = " << utimstr(props.generate_time) << endl;
  out << "   received time = " << utimstr(props.received_time) << endl;
  out << "   start time    = " << utimstr(props.start_time) << endl;
  out << "   expire time   = " << utimstr(props.expire_time) << endl;

  printBoundingBox(out, props.bounding_box);

  out << "   label = " << props.label << endl;
  out << "   num objects = " << props.num_objs << endl;
  
  out << endl;
  
}

///////////////////////////////////////////////////////////////////////
// Print boundinx box
//

void Symprod::printBoundingBox(ostream &out, const bbox_t &box)

{
  out << "   bounding box: " << endl;
  out << "      min lat = " << box.min_lat << endl;
  out << "      max lat = " << box.max_lat << endl;
  out << "      min lon = " << box.min_lon << endl;
  out << "      max lon = " << box.max_lon << endl;
}

///////////////////////////////////////////////////////////////////////
// Print product header object offsets
//

void Symprod::printProdHdrOffsets(ostream &out,
				  int num_objs,
				  const si32 *offsets)

{  
  
  out << "   object offsets: " << endl;
  for (int i = 0; i < num_objs; i++) {
    out << "      " << offsets[i] << endl;
  }

}
  
///////////////////////////////////////////////////////////////////////
// Print object header
//

void Symprod::printObjHdr(ostream &out, const obj_hdr_t &hdr)
  
{
  
  out << endl;
  out << "Object Header information: " << endl;
  out << endl;
  
  out << "   object type = ";
  Symprod::printObjectType(out, hdr.object_type);
  out << endl;
  
  out << "   object id = " << hdr.object_id << endl;
  out << "   num bytes = " << hdr.num_bytes << endl;
  out << "   detail level = " << hdr.detail_level << endl;
  out << "   color = <" << hdr.color << ">" << endl;
  out << "   background color = <" << hdr.background_color <<  ">" << endl;
  out << "   centroid lat = " << hdr.centroid.lat << endl;
  out << "   centroid lon = " << hdr.centroid.lon << endl;
  
}

///////////////////////////////////////////////////////////////////////
// Print properties of a text object.
//

void Symprod::printTextProps(ostream &out, const text_props_t &props)
{

  out << endl;
  out << "Text Object properties: " << endl;
  out << endl;
  out << "   origin lat = " << props.origin.lat << endl;
  out << "   origin lon = " << props.origin.lon << endl;
  out << "   offset X = " << props.offset.x << endl;
  out << "   offset Y = " << props.offset.y << endl;
  
  out << "   vertical alignment = ";
  printVertAlign(out, props.vert_alignment);
  out << endl;
  
  out << "   horizontal alignment = ";
  printHorizAlign(out, props.horiz_alignment);
  out << endl;
  
  out << "   font_size = " << props.font_size << endl;
  
  out << "   font style = ";
  printFontStyle(out, props.font_style);
  out << endl;
  
  out << "   length = " << props.length << endl;
  out << "   font name = " << props.fontname << endl;
  
}


///////////////////////////////////////////////////////////////////////
// Print properties of a polyline object.
//

void Symprod::printPolylineProps(ostream &out, const polyline_props_t &props)
{
  
  out << endl;
  out << "Polyline Object properties: " << endl;
  out << endl;
  out << "   close flag = " << props.close_flag << endl;

  out << "   fill type = ";
  printFill(out, props.fill);
  out << endl;
  
  out << "   line type = ";
  printLineType(out, props.linetype);
  out << endl;
  
  out << "   line width = " << props.linewidth << endl;
  
  out << "   cap style = ";
  printCapstyle(out, props.capstyle);
  out << endl;
  
  out << "   join style = ";
  printJoinstyle(out, props.joinstyle);
  out << endl;
  
  out << "   line interpolation = ";
  printLineInterp(out, props.line_interp);
  out << endl;
  
  out << "   num points = " << props.num_points << endl;

}

///////////////////////////////////////////////////////////////////////
// Print polyline points array

void Symprod::printPolylinePtsArray(ostream &out,
				    int num_points,
				    const wpt_t *points)

{

  out << "   points: " << endl;
  for (int i = 0; i < num_points; i++) {
    if (points[i].lat == WPT_PENUP &&
	points[i].lon == WPT_PENUP) {
      out << "      ------> PENUP <------\n";
    } else {
      out << "      "
	  << setw(10) << points[i].lat << ", "
	  << setw(10) << points[i].lon << endl;
    }
  }

}

///////////////////////////////////////////////////////////////////////
// Print properties of a iconline object.
//

void Symprod::printIconlineProps(ostream &out, const iconline_props_t &props)
{
  
  out << endl;
  out << "Iconline Object properties: " << endl;
  out << endl;

  out << "   origin lat = " << props.origin.lat << endl;
  out << "   origin lon = " << props.origin.lon << endl;

  out << "   close flag = " << props.close_flag << endl;

  out << "   fill type = ";
  printFill(out, props.fill);
  out << endl;
  
  out << "   line type = ";
  printLineType(out, props.linetype);
  out << endl;
  
  out << "   line width = " << props.linewidth << endl;
  
  out << "   cap style = ";
  printCapstyle(out, props.capstyle);
  out << endl;
  
  out << "   join style = ";
  printJoinstyle(out, props.joinstyle);
  out << endl;
  
  out << "   line interpolation = ";
  printLineInterp(out, props.line_interp);
  out << endl;
  
  out << "   num points = " << props.num_points << endl;

}

///////////////////////////////////////////////////////////////////////
// Print iconline points array

void Symprod::printIconlinePtsArray(ostream &out,
				    int num_points,
				    const ppt_t *points)
  
{
  
  out << "   points: " << endl;
  for (int i = 0; i < num_points; i++) {
    if (points[i].x == PPT_PENUP &&
	points[i].y == PPT_PENUP) {
      out << "      ------> PENUP <------\n";
    } else {
      out << "      "
	  << setw(10) << points[i].x << ", "
	  << setw(10) << points[i].y << endl;
    }
  }

}

///////////////////////////////////////////////////////////////////////
// Print properties of a stroked icon object.
//

void Symprod::printStrokedIconProps(ostream &out,
				    const stroked_icon_props_t &props)
{
  
  out << endl;
  out << "Stroked Icon Object properties: " << endl;
  out << endl;
  out << "   num icon points = " << props.num_icon_pts << endl;
  out << "   num icons = " << props.num_icons << endl;

}


///////////////////////////////////////////////////////////////////////
// Print properties of a named icon object

void Symprod::printNamedIconProps(ostream &out, 
				  const named_icon_props_t &props)
{

  out << endl;
  out << "Named Icon Object properties: " << endl;
  out << endl;
  out << "   icon name = <" << props.name << ">" << endl;
  out << "   num icons = " << props.num_icons << endl;
  
}


///////////////////////////////////////////////////////////////////////
// Print properties of a bitmap icon object

void Symprod::printBitmapIconProps(ostream &out,
				   const bitmap_icon_props_t &props)
{
  
  out << endl;
  out << "Bitmap Icon Object properties: " << endl;
  out << endl;
  out << "   bitmap X dimension = " << props.bitmap_x_dim << endl;
  out << "   bitmap Y dimension = " << props.bitmap_y_dim << endl;
  out << "   num icons = " << props.num_icons << endl;

}

///////////////////////////////////////////////////////////////////////
// Print icon origin array

void Symprod::printIconOrigins(ostream &out,
			       int num_icons,
			       const wpt_t *icon_origins)

{

  out << "   icon origins: " << endl;
  for (int i = 0; i < num_icons; i++) {
    out << "      "
	<< setw(10) << icon_origins[i].lat << ", "
	<< setw(10) << icon_origins[i].lon << endl;
  }
  
}

///////////////////////////////////////////////////////////////////////
// Print icon points array

void Symprod::printIconPoints(ostream &out,
			      int num_icon_pts,
			      const ppt_t *icon_pts)

{

  out << "   icon points: " << endl;
  for (int i = 0; i < num_icon_pts; i++) {
    out << "      "
	<< setw(4) << icon_pts[i].x
	<< setw(4) << icon_pts[i].y << endl;
  }

}
  
///////////////////////////////////////////////////////////////////////
// Print bitmap array

void Symprod::printBitmap(ostream &out,
			  int bitmap_x_dim, int bitmap_y_dim,
			  const ui08 *bitmap)

{

  out << "   bitmap: " << endl;
  ui08 *bit_val = (ui08 *) bitmap;
  for (int i = 0; i < bitmap_y_dim; i++) {
    out << "      ";
    for (int j = 0; j < bitmap_x_dim; j++) {
      out << (int) *bit_val << " ";
      bit_val++;
    } // endfor - j 
    out << endl;
  } // endfor - i 
  
}

///////////////////////////////////////////////////////////////////////
// Print properties of an arc object.

void Symprod::printArcProps(ostream &out,  const arc_props_t &props)
{
  out << endl;
  out << "Arc Object properties: " << endl;
  out << endl;
  out << "   origin lat = " << props.origin.lat << endl;
  out << "   origin lon = " << props.origin.lon << endl;
  out << "   radius_x = " << props.radius_x << endl;
  out << "   radius_y = " << props.radius_y << endl;
  out << "   angle1 = " << props.angle1 << endl;
  out << "   angle2 = " << props.angle2 << endl;
  out << "   axis_rotation = " << props.axis_rotation << endl;
  
  out << "   line type = ";
  printLineType(out, props.linetype);
  out << endl;
  
  out << "   line width = " << props.linewidth << endl;

  out << "   fill = ";
  printFill(out, props.fill);
  out << endl;
  
  out << "   cap style = ";
  printCapstyle(out, props.capstyle);
  out << endl;
  
  out << "   join style = ";
  printJoinstyle(out, props.joinstyle);
  out << endl;
  
  out << "   nsegments = " << props.nsegments << endl;

}


///////////////////////////////////////////////////////////////////////
// Print properties of a rectangle object
//

void Symprod::printRectangleProps(ostream &out,
				  const rectangle_props_t &props)
{
  out << endl;
  out << "Rectangle Object properties: " << endl;
  out << endl;
  out << "   origin lat = " << props.origin.lat << endl;
  out << "   origin lon = " << props.origin.lon << endl;
  out << "   height = " << props.height << endl;
  out << "   width = " << props.width << endl;
  
  out << "   line type = ";
  printLineType(out, props.linetype);
  out << endl;
  
  out << "   line width = " << props.linewidth << endl;
  
  out << "   fill type = ";
  printFill(out, props.fill);
  out << endl;
  
  out << "   cap style = ";
  printCapstyle(out, props.capstyle);
  out << endl;
  
  out << "   join style = ";
  printJoinstyle(out, props.joinstyle);
  out << endl;
  
}


///////////////////////////////////////////////////////////////////////
// Print properties of a chunk object.

void Symprod::printChunkProps(ostream &out, const chunk_props_t &props)
{
  out << endl;
  out << "Chunk Object properties: " << endl;
  out << endl;
  out << "   chunk_type = " << props.chunk_type << endl;
  out << "   user_info = " << props.user_info << endl;
  out << "   nbytes_chunk = " << props.nbytes_chunk << endl;
}

///////////////////////////////////////////////////////////////////////
// printObjectType
//

void Symprod::printObjectType(ostream &out, int type)
{
  switch(type) {

  case OBJ_TEXT :
    out << "OBJ_TEXT";
    break;
    
  case OBJ_POLYLINE :
    out << "OBJ_POLYLINE";
    break;
    
  case OBJ_ICONLINE :
    out << "OBJ_ICONLINE";
    break;
    
  case OBJ_STROKED_ICON :
    out << "OBJ_STROKED_ICON";
    break;
    
  case OBJ_NAMED_ICON :
    out << "OBJ_NAMED_ICON";
    break;
    
  case OBJ_BITMAP_ICON :
    out << "OBJ_BITMAP_ICON";
    break;
    
  case OBJ_ARC :
    out << "OBJ_ARC";
    break;
    
  case OBJ_RECTANGLE :
    out << "OBJ_RECTANGLE";
    break;
    
  case OBJ_CHUNK :
    out << "OBJ_CHUNK";
    break;
    
  default:
    out << "UNKNOWN OBJECT TYPE: " << type;
    break;
  }
  
  return;
}


///////////////////////////////////////////////////////////////////////
// printVertAlign
//

void Symprod::printVertAlign(ostream &out, int align)
{
  switch(align) {
  case VERT_ALIGN_TOP :
    out << "VERT_ALIGN_TOP";
    break;
    
  case VERT_ALIGN_CENTER :
    out << "VERT_ALIGN_CENTER";
    break;
    
  case VERT_ALIGN_BOTTOM :
    out << "VERT_ALIGN_BOTTOM";
    break;
    
  default:
    out << "UNKNOWN VERTICAL ALIGNMENT: " << align;
    break;
  }
  
  return;
}


///////////////////////////////////////////////////////////////////////
// printHorizAlign
//

void Symprod::printHorizAlign(ostream &out, int align)
{
  switch(align) {
  case HORIZ_ALIGN_LEFT :
    out << "HORIZ_ALIGN_LEFT";
    break;
    
  case HORIZ_ALIGN_CENTER :
    out << "HORIZ_ALIGN_CENTER";
    break;
    
  case HORIZ_ALIGN_RIGHT :
    out << "HORIZ_ALIGN_RIGHT";
    break;
    
  default:
    out << "UNKNOWN HORIZONTAL ALIGNMENT: " << align;
    break;
  }
  
  return;
}


///////////////////////////////////////////////////////////////////////
// printFontStyle
//

void Symprod::printFontStyle(ostream &out, int style)
{
  switch(style) {

  case TEXT_NORM :
    out << "TEXT_NORM";
    break;
    
  case TEXT_BOLD :
    out << "TEXT_BOLD";
    break;
    
  case TEXT_ITALICS :
    out << "TEXT_ITALICS";
    break;
    
  case TEXT_SUBSCRIPT :
    out << "TEXT_SUBSCRIPT";
    break;
    
  case TEXT_SUPERSCRIPT :
    out << "TEXT_SUPERSCRIPT";
    break;
    
  case TEXT_UNDERLINE :
    out << "TEXT_UNDERLINE";
    break;
    
  case TEXT_STRIKETHROUGH :
    out << "TEXT_STRIKETHROUGH";
    break;
    
  default:
    out << "UNKNOWN FONT STYLE: " << style;
    break;
  }
  
  return;
}


///////////////////////////////////////////////////////////////////////
// printFill
//

void Symprod::printFill(ostream &out, int fill)
{
  switch(fill) {
  case FILL_NONE :
    out << "FILL_NONE";
    break;
    
  case FILL_STIPPLE10 :
    out << "FILL_STIPPLE10";
    break;
    
  case FILL_STIPPLE20 :
    out << "FILL_STIPPLE20";
    break;
    
  case FILL_STIPPLE30 :
    out << "FILL_STIPPLE30";
    break;
    
  case FILL_STIPPLE40 :
    out << "FILL_STIPPLE40";
    break;
    
  case FILL_STIPPLE50 :
    out << "FILL_STIPPLE50";
    break;
    
  case FILL_STIPPLE60 :
    out << "FILL_STIPPLE60";
    break;
    
  case FILL_STIPPLE70 :
    out << "FILL_STIPPLE70";
    break;
    
  case FILL_STIPPLE80 :
    out << "FILL_STIPPLE80";
    break;
    
  case FILL_STIPPLE90 :
    out << "FILL_STIPPLE90";
    break;
    
  case FILL_SOLID :
    out << "FILL_SOLID";
    break;
    
  case FILL_ALPHA10 :
    out << "FILL_ALPHA10";
    break;
    
  case FILL_ALPHA20 :
    out << "FILL_ALPHA20";
    break;
    
  case FILL_ALPHA30 :
    out << "FILL_ALPHA30";
    break;
    
  case FILL_ALPHA40 :
    out << "FILL_ALPHA40";
    break;
    
  case FILL_ALPHA50 :
    out << "FILL_ALPHA50";
    break;
    
  case FILL_ALPHA60 :
    out << "FILL_ALPHA60";
    break;
    
  case FILL_ALPHA70 :
    out << "FILL_ALPHA70";
    break;
    
  case FILL_ALPHA80 :
    out << "FILL_ALPHA80";
    break;
    
  case FILL_ALPHA90 :
    out << "FILL_ALPHA90";
    break;
    
  default:
    out << "UNKNOWN FILL TYPE: " << fill;
    break;
  }
  
  return;
}


///////////////////////////////////////////////////////////////////////
// printLineType
//

void Symprod::printLineType(ostream &out, int linetype)
{
  switch(linetype) {
  case LINETYPE_SOLID :
    out << "LINETYPE_SOLID";
    break;
    
  case LINETYPE_DASH :
    out << "LINETYPE_DASH";
    break;
    
  case LINETYPE_DOT_DASH :
    out << "LINETYPE_DOT_DASH";
    break;
    
  default:
    out << "UNKNOWN LINETYPE: " << linetype;
    break;
  }
  
  return;
}


///////////////////////////////////////////////////////////////////////
// printCapstyle
//

void Symprod::printCapstyle(ostream &out, int capstyle)
{
  switch(capstyle) {
  case CAPSTYLE_BUTT :
    out << "CAPSTYLE_BUTT";
    break;
    
  case CAPSTYLE_NOT_LAST :
    out << "CAPSTYLE_NOT_LAST";
    break;
    
  case CAPSTYLE_PROJECTING :
    out << "CAPSTYLE_PROJECTING";
    break;
    
  case CAPSTYLE_ROUND :
    out << "CAPSTYLE_ROUND";
    break;
    
  default:
    out << "UNKNOWN CAPSTYLE: " << capstyle;
    break;
  }
  
  return;
}


///////////////////////////////////////////////////////////////////////
// printJoinstyle
//

void Symprod::printJoinstyle(ostream &out, int joinstyle)
{
  switch(joinstyle) {
  case JOINSTYLE_BEVEL :
    out << "JOINSTYLE_BEVEL";
    break;
    
  case JOINSTYLE_MITER :
    out << "JOINSTYLE_MITER";
    break;
    
  case JOINSTYLE_ROUND :
    out << "JOINSTYLE_ROUND";
    break;
    
  default:
    out << "UNKNOWN JOINSTYLE: " << joinstyle;
    break;
  }
  
  return;
}


///////////////////////////////////////////////////////////////////////
// printLineInterp
//

void Symprod::printLineInterp(ostream &out, int line_interp)

{

  switch(line_interp) {
  case LINE_INTERP_STRAIGHT :
    out << "LINE_INTERP_STRAIGHT";
    break;
    
  case LINE_INTERP_BEZIER :
    out << "LINE_INTERP_BEZIER";
    break;
    
  case LINE_INTERP_CUBIC_SPLINE :
    out << "LINE_INTERP_CUBIC_SPLINE";
    break;
    
  default:
    out << "UNKNOWN LINE INTERPOLATION: " << line_interp;
    break;
  }
  
  return;
}

