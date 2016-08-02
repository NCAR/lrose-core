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

////////////////////////////////////////////////////////////////
// Spdb/Symprod.hh
//
// Symprod class
//
// This class handles the Symprod symbolic products operations.
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 1999
//
////////////////////////////////////////////////////////////////
//
// This header file describes the buffers and messages used by the
// symbolic products library.  The idea of this library is to use
// symbolic product information in a common format so that displays
// and other processes can operate on and render a single format.
// The product information is generally stored in an SPDB database
// in some native format.  Then, a server is written using the
// DsSpdbServer class and an output transformation is included to
// convert the native format to SYMPROD format for sending to the
// display or other process.
//
////////////////////////////////////////////////////////////////

#ifndef Symprod_HH
#define Symprod_HH

#define _in_Symprod_hh

#include <ctime>
#include <cstdio>
#include <vector>
#include <string>
#include <dataport/port_types.h>
#include <toolsa/MemBuf.hh>
using namespace std;

class SymprodObj;
class TTest;

///////////////////////////////////////////////////////////////
// class definition

class Symprod

{

  friend class TTest;

public:

  // typedefs and constants

#include <Spdb/Symprod_typedefs.hh>

  // default constructor
  
  Symprod(time_t generate_time = 0,
	  time_t received_time = 0,
	  time_t start_time = 0,
	  time_t expire_time = 0,
	  const int data_type = 0,
	  const int data_type2 = 0,
	  const char *label = NULL);
  
  // destructor
  
  virtual ~Symprod();
  
  // clear all products
  
  virtual void clear();

  ////////////////////////
  // Serialize into buffer
  // Convert internal format to product buffer.
  // Byte swapping is performed as necessary.
  
  void serialize(MemBuf &out_buf);
  
  // Convert a product buffer to internal format.  The buffer is assumed
  // to be in native format (any necessary byte - swapping has already
  // been done).
  // Checks on buffer overrun if buf_len is provided.
  // returns 0 on success, -1 on failure.
  
  int deserialize(void *in_buf, int buf_len = -1);

  // set  times
  
  void setTimes(const time_t generate_time = 0,
		const time_t received_time = 0,
		const time_t start_time = 0,
		const time_t expire_time = 0);

  void setExpireTime(const time_t expire_time = 0);

  // set data types

  void setDataTypes(const int data_type = 0,
		    const int data_type2 = 0);

  // set label

  void setLabel(const char *label = NULL);

  ////////////////////////////////////////////////////////////////
  // bounding box

  // Initialize bounding box
  
  void initBbox(bbox_t &bb);

  // Update a bounding box given a point
  
  void updateBbox(bbox_t &bb, double lat, double lon);

  // Update one bounding box with another

  void updateBbox(bbox_t &bb, const bbox_t &template_bb);

  // access to members

  const prod_hdr_props_t &getProps() { return (_prodProps); }
  int getNObjects() { return (_objs.size()); }
  const vector<SymprodObj *> &getObjs() { return _objs; }
  
  ////////////////////////////////////////////////
  // error string - set if function returns error
  
  const string &getErrStr() { return (_errStr); }
  const string &getErrorStr() { return (_errStr); }
  const string &getErrorString() { return (_errStr); }

  ///////////////////////////////////////////////////////
  // Add a generic object
  
  void addObject(SymprodObj *obj, int obj_type, bbox_t &bbox);

  //////////////////////////////////////////////////////////////////
  // add basic objects

  // Adds the given object to the internal product structure.
  // Memory is allocated for the object info. This memory is freed
  // in clear()

  void addText(const char *text_string,
	       double lat,
	       double lon,
	       const char *color,
	       const char *background_color,
	       int offset_x = 0,
	       int offset_y = 0,
	       vert_align_t vert_alignment = VERT_ALIGN_CENTER,
	       horiz_align_t horiz_alignment = HORIZ_ALIGN_CENTER,
	       int font_size = 0,
	       font_style_t font_style = TEXT_NORM,
	       const char *fontname = NULL,
	       int object_id = 0,
	       int detail_level = 0);
  
  void addPolyline(int npoints,
		   const wpt_t *pts,
		   const char *color,
		   linetype_t linetype = LINETYPE_SOLID,
		   int linewidth = 1,
		   capstyle_t capstyle = CAPSTYLE_BUTT,
		   joinstyle_t joinstyle = JOINSTYLE_BEVEL,
		   bool close_flag = false,
		   fill_t fill = FILL_NONE,
		   int object_id = 0,
		   int detail_level = 0);

  void addPolyline(int npoints,
		   const wpt_t *pts,
                   double centroid_lat,
                   double centroid_lon,
		   const char *color,
		   linetype_t linetype = LINETYPE_SOLID,
		   int linewidth = 1,
		   capstyle_t capstyle = CAPSTYLE_BUTT,
		   joinstyle_t joinstyle = JOINSTYLE_BEVEL,
		   bool close_flag = false,
		   fill_t fill = FILL_NONE,
		   int object_id = 0,
		   int detail_level = 0);

  void addIconline(double origin_lat,
		   double origin_lon,
		   int npoints,
		   const ppt_t *pts,
		   const char *color,
		   linetype_t linetype = LINETYPE_SOLID,
		   int linewidth = 1,
		   capstyle_t capstyle = CAPSTYLE_BUTT,
		   joinstyle_t joinstyle = JOINSTYLE_BEVEL,
		   bool close_flag = false,
		   fill_t fill = FILL_NONE,
		   int object_id = 0,
		   int detail_level = 0);

  void addStrokedIcons(const char *color,
		       int num_icon_pts,
		       const ppt_t *icon_pts,
		       int num_icons,
		       const wpt_t *icon_origins,
		       int object_id = 0,
		       int detail_level = 0,
		       int linewidth = 1);
  
  void addNamedIcons(const char *name,
		     const char *color,
		     int num_icons,
		     const wpt_t *icon_origins,
		     int object_id = 0,
		     int detail_level = 0);

  void addBitmapIcons(const char *color,
		      int num_icons,
		      const wpt_t *icon_origins,
		      int bitmap_x_dim,
		      int bitmap_y_dim,
		      const ui08 *bitmap,
		      int object_id = 0,
		      int detail_level = 0);

  void addArc(double origin_lat,
	      double origin_lon,
	      double radius_x,
	      double radius_y,
	      const char *color,
	      bool radii_in_pixels = false,
	      double angle1 = 0.0,
	      double angle2 = 360.0,
	      double axis_rotation = 0.0,
	      int nsegments = 90,
	      linetype_t linetype = LINETYPE_SOLID,
	      int linewidth = 1,
	      capstyle_t capstyle = CAPSTYLE_BUTT,
	      joinstyle_t joinstyle = JOINSTYLE_BEVEL,
	      fill_t fill = FILL_NONE,
	      int object_id = 0,
	      int detail_level = 0);

  void addRectangle(double origin_lat,
		    double origin_lon,
		    double height,
		    double width,
		    const char *color,
		    linetype_t linetype = LINETYPE_SOLID,
		    int linewidth = 1,
		    capstyle_t capstyle = CAPSTYLE_BUTT,
		    joinstyle_t joinstyle = JOINSTYLE_BEVEL,
		    fill_t fill = FILL_NONE,
		    int object_id = 0,
		    int detail_level = 0);
  
  void addChunk(double min_lat,
		double min_lon,
		double max_lat,
		double max_lon,
		int chunk_type,
		int nbytes_chunk,
		const void *data,
		const char *color,
		const char *background_color,
		int user_info = 0,
		int object_id = 0,
		int detail_level = 0);

  ////////////////////////////////////////////////////
  // add arrows

  // Add an arrow object given start and end points
  // head_len_km:        in km
  // head_half_angle: in deg

  void addArrowBothPts(const char *color,
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
		       int object_id = 0,
		       int detail_level = 0);
  
  void addArrowBothPts(const char *color,
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
		       int object_id = 0,
		       int detail_level = 0);
  
  // Add an arrow object given start pt, length, dirn
  // head_len_km:        in km
  // head_half_angle: in deg

  void addArrowStartPt(const char *color,
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
		       int object_id = 0,
		       int detail_level = 0);

  void addArrowStartPt(const char *color,
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
		       int object_id = 0,
		       int detail_level = 0);

  // Add an arrow object given end pt, length, dirn
  // head_len_km:        in km
  // head_half_angle: in deg
  
  void addArrowEndPt(const char *color,
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
		     int object_id = 0,
		     int detail_level = 0);

  void addArrowEndPt(const char *color,
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
		     int object_id = 0,
		     int detail_level = 0);

  // Add an arrow object given mid pt, length, dirn
  // head_len_km:        in km
  // head_half_angle: in deg
  
  void addArrowMidPt(const char *color,
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
		     int object_id = 0,
		     int detail_level = 0);
  
  void addArrowMidPt(const char *color,
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
		     int object_id = 0,
		     int detail_level = 0);
  
  // add generic arrow given start and end pt
  
  void addArrow(const char *color,
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
		int object_id = 0,
		int detail_level = 0);

  void addArrow(const char *color,
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
                int object_id = 0,
                int detail_level = 0);


  void addWindBarb(double lat, 
		   double lon, 
		   const char* color, 
		   double speed, //knots
		   double dir, //degrees
		   int station_posn_circle_radius, //pixels
		   int wind_barb_line_width, 	//pixels
		   double wind_ticks_angle_to_shaft, //degrees
		   double wind_barb_tick_len, //pixels
		   int wind_barb_shaft_len, //pixels
		   int detail_level);
  ///////////////////////////////////////////////////////////////
  // transformation routines
  
  // Calculate the distance between two pixel points.
  // The value returned is in pixels.

  double pptDistance(ppt_t point1, ppt_t point2);

  // Rotate an array of pixel points by the given angle. The array is
  // rotated around the point (0,0) and the angle is given in radians.
  
  void rotatePptArray(ppt_t *array, int n_points, double angle_rad);

  // Rotate an array of world points by the given angle.  The array is
  // rotated around the point (0,0) and the angle is given in radians.
  
  void rotateWptArray(wpt_t *array, int n_points, double angle_rad);
  
  // Scale an array of pixel points by the given factor.
  
  void scalePptArray(ppt_t *array, int n_points, double factor);

  // Scale an array of world points by the given factor.
  
  void scaleWptArray(wpt_t *array, int n_points, double factor);

  // Translate an array of pixel points by the given values.  The values
  // are assumed to be in pixels.
  
  void translatePptArray(ppt_t *array, int n_points,
			 int trans_x, int trans_y);

  // Translate an array of world points by the given values.
  // The values are assumed to be in degrees.
  
  void translateWptArray(wpt_t *array, int n_points,
			 double trans_lat, double trans_lon);

  // Calculate the distance between two world points.
  // The value returned is in degrees.
  
  double wptDistance(wpt_t point1, wpt_t point2);

  //////////////////////////////////////////////////////
  // byte swapping

  // productBufferToBE()
  // Checks for buffer overrun if buflen is set.
  // Returns 0 on success, -1 on failure

  int productBufferToBE();

  // productBufferFromBE()
  // Checks for buffer overrun if buflen is set.
  // Returns 0 on success, -1 on failure

  int productBufferFromBE(void *buffer, int buflen = -1);

  /////////////////////////////////////////////////////////////////
  // printing

  void print(ostream &out);

  static void printProdHdrProps(ostream &out,
				const prod_hdr_props_t &props);
  static void printProdHdrOffsets(ostream &out, int num_objs,
				  const si32 *offsets);
  static void printObjHdr(ostream &out, const obj_hdr_t &hdr);
  static void printBoundingBox(ostream &out, const bbox_t &box);
  static void printTextProps(ostream &out, const text_props_t &props);
  static void printPolylineProps(ostream &out, const polyline_props_t &props);
  static void printPolylinePtsArray(ostream &out, int num_points,
				    const wpt_t *points);
  static void printIconlineProps(ostream &out, const iconline_props_t &props);
  static void printIconlinePtsArray(ostream &out, int num_points,
				    const ppt_t *points);
  static void printStrokedIconProps(ostream &out,
				    const stroked_icon_props_t &props);
  static void printNamedIconProps(ostream &out,
				  const named_icon_props_t &props);
  static void printBitmapIconProps(ostream &out,
				   const bitmap_icon_props_t &props);
  static void printIconOrigins(ostream &out, int num_icons,
			       const wpt_t *icon_origins);
  static void printIconPoints(ostream &out, int num_icon_pts,
			      const ppt_t *icon_pts);
  static void printBitmap(ostream &out, int bitmap_x_dim, int bitmap_y_dim,
			  const ui08 *bitmap);
  static void printArcProps(ostream &out,  const arc_props_t &props);
  static void printRectangleProps(ostream &out,
				  const rectangle_props_t &props);
  static void printChunkProps(ostream &out, const chunk_props_t &props);
  
  static void printObjectType(ostream &out, int type);
  static void printVertAlign(ostream &out, int align);
  static void printHorizAlign(ostream &out, int align);
  static void printFill(ostream &out, int fill);
  static void printLineType(ostream &out, int linetype);
  static void printCapstyle(ostream &out, int capstyle);
  static void printJoinstyle(ostream &out, int joinstyle);
  static void printLineInterp(ostream &out, int line_interp);
  static void printFontStyle(ostream &out, int style);

  /////////////////
  // byte swapping

  static void prodHdrToBE(prod_hdr_props_t *prod_hdr);
  static void prodHdrFromBE(prod_hdr_props_t *prod_hdr);
  static void objHdrToBE(obj_hdr_t *obj_hdr);
  static void objHdrFromBE(obj_hdr_t *obj_hdr);
  static void textToBE(text_props_t *props);
  static void textFromBE(text_props_t *props);
  static void polylineToBE(polyline_props_t *props);
  static void polylineFromBE(polyline_props_t *props);
  static void iconlineToBE(iconline_props_t *props);
  static void iconlineFromBE(iconline_props_t *props);
  static void strokedIconToBE(stroked_icon_props_t *props);
  static void strokedIconFromBE(stroked_icon_props_t *props);
  static void namedIconToBE(named_icon_props_t *props);
  static void namedIconFromBE(named_icon_props_t *props);
  static void bitmapIconToBE(bitmap_icon_props_t *props);
  static void bitmapIconFromBE(bitmap_icon_props_t *props);
  static void arcToBE(arc_props_t *props);
  static void arcFromBE(arc_props_t *props);
  static void rectangleToBE(rectangle_props_t *props);
  static void rectangleFromBE(rectangle_props_t *props);
  static void chunkToBE(chunk_props_t *props);
  static void chunkFromBE(chunk_props_t *props);
  static void bboxToBE(bbox_t *bbox);
  static void boxFromBE(bbox_t *bbox);
  static void wptToBE(wpt_t *wpt);
  static void wptFromBE(wpt_t *wpt);
  static void pptToBE(ppt_t *ppt);
  static void pptFromBE(ppt_t *ppt);

  ////////////
  // constants

  static const si32 PPT_PENUP;
  static const fl32 WPT_PENUP;

protected:

  prod_hdr_props_t _prodProps;
  vector<SymprodObj *> _objs;
  vector<int> _objTypes;
  string _errStr;

  // functions
  
  void _freeObjs();
  void _clearErrStr() { _errStr = ""; }
  void _addIntErr(const char *err_str, const int iarg);
  void _addStrErr(const char *err_str, const string &sarg);

private:

};

#undef _in_Symprod_hh

#endif


