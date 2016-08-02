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
////////////////////////////////////////////////
//
// Symprod_typedefs.hh
//
// Typedefs for Symprod class
//
////////////////////////////////////////////////

// This header file can only be included from within Symprod.hh

#ifdef _in_Symprod_hh

#define SYMPROD_LABEL_LEN        80
#define SYMPROD_COLOR_LEN        32
#define SYMPROD_ICON_NAME_LEN    32
#define SYMPROD_FONT_NAME_LEN    80

///////////////////////////////////////////////////
// enums
//

typedef enum
{
  SUCCESS,
  UNKNOWN_ERROR,
  WRITE_ERROR,
  READ_ERROR,
  FILE_OPEN_ERROR
} error_t;

typedef enum {
  OBJ_TEXT                      =  1,
  OBJ_POLYLINE                  =  2,
  OBJ_STROKED_ICON              =  3,
  OBJ_NAMED_ICON                =  4,
  OBJ_BITMAP_ICON               =  5,
  OBJ_ARC                       =  6,
  OBJ_RECTANGLE                 =  7,
  OBJ_CHUNK                     =  8,
  OBJ_ICONLINE                  =  9
} obj_code_t;

typedef enum {
  LINETYPE_SOLID                =  1,
  LINETYPE_DASH                 =  2,
  LINETYPE_DOT_DASH             =  3
} linetype_t;

typedef enum {
  CAPSTYLE_BUTT                 =  1,
  CAPSTYLE_NOT_LAST             =  2,
  CAPSTYLE_PROJECTING           =  3,
  CAPSTYLE_ROUND                =  4
} capstyle_t;

typedef enum {
  JOINSTYLE_BEVEL               =  1,
  JOINSTYLE_MITER               =  2,
  JOINSTYLE_ROUND               =  3
} joinstyle_t;

typedef enum {
  LINE_INTERP_STRAIGHT          =  1,
  LINE_INTERP_BEZIER            =  2,
  LINE_INTERP_CUBIC_SPLINE      =  3,
  LINE_INTERP_GREAT_CIRCLE      =  4
} line_interp_t;

typedef enum {
  FILL_NONE                     =  1,
  FILL_STIPPLE10                =  2,
  FILL_STIPPLE20                =  3,
  FILL_STIPPLE30                =  4,
  FILL_STIPPLE40                =  5,
  FILL_STIPPLE50                =  6,
  FILL_STIPPLE60                =  7,
  FILL_STIPPLE70                =  8,
  FILL_STIPPLE80                =  9,
  FILL_STIPPLE90                = 10,
  FILL_SOLID                    = 11,
  FILL_ALPHA10                  = 12,
  FILL_ALPHA20                  = 13,
  FILL_ALPHA30                  = 14,
  FILL_ALPHA40                  = 15,
  FILL_ALPHA50                  = 16,
  FILL_ALPHA60                  = 17,
  FILL_ALPHA70                  = 18,
  FILL_ALPHA80                  = 19,
  FILL_ALPHA90                  = 20
} fill_t;

typedef enum {
  VERT_ALIGN_TOP           =  1,
  VERT_ALIGN_CENTER        =  2,
  VERT_ALIGN_BOTTOM        =  3
} vert_align_t;

typedef enum {
  HORIZ_ALIGN_LEFT         =  1,
  HORIZ_ALIGN_CENTER       =  2,
  HORIZ_ALIGN_RIGHT        =  3
} horiz_align_t;

typedef enum {
  TEXT_NORM              = 0,
  TEXT_BOLD              = 1,
  TEXT_ITALICS           = 2,
  TEXT_SUBSCRIPT         = 3,
  TEXT_SUPERSCRIPT       = 4,
  TEXT_UNDERLINE         = 5,
  TEXT_STRIKETHROUGH     = 6
} font_style_t;

// detail_level_t is a bit-wise or'd value, which give hints to the
// renderer on how to deal with the object to be shown

typedef enum {
  DETAIL_LEVEL_NONE             = 0,  // No Hints - Can be used to clear hints
  DETAIL_LEVEL_DO_NOT_SCALE     = 1,  // Leave Icons, Text fixed size.
  DETAIL_LEVEL_IGNORE_TRESHOLDS = 2,  // Displayu regardless of Detail thresholds
  DETAIL_LEVEL_USUALLY_HIDDEN   = 4,  // Object Normally is hidden, Show when picked
  DETAIL_LEVEL_SHOWAS_POPOVER   = 8,  // Object Should Blank the area behind 
  DETAIL_LEVEL_PLAYAS_AUDIO     = 16, // Object Should be heard
  DETAIL_LEVEL_REQ_USER_ACK     = 32  // Request User Acknowledgement 
} detail_level_t;

///////////////////////////////////////////////////////
// Typedefs used for the product buffer.
//
// The product buffer looks like this:
//
//            product header
//            object 1 header
//            object 1 information
//            object 2 header
//            object 2 information
//                   .
//                   .
//            object n header
//            object n information


typedef struct world_point {
  fl32        lat;
  fl32        lon;
} wpt_t;

typedef struct pixel_point {
  si32        x;
  si32        y;
} ppt_t;

typedef struct box {
  fl32        min_lat;
  fl32        max_lat;
  fl32        min_lon;
  fl32        max_lon;
} bbox_t;


typedef struct {
  
  si32        generate_time;          // Time product was generated.        
  si32        received_time;          // Time product was received.         
  si32        start_time;             // Time product starts being valid.   
  si32        expire_time;            // Time product expires.              
  si32        data_type;              // data_type
  si32        data_type2;             // data_type2
  bbox_t      bounding_box;           // Bounding box for displayable       
                                      //   objects in product.              
  si32        num_objs;               // Number of objects making up the    
                                      //   product.                         
  si32        spare;                  // Spare field for alignment purposes.
  char        label[SYMPROD_LABEL_LEN];
                                      // Optional product label to be       
                                      //   displayed in the select list for 
                                      //   this instance of the product.    
} prod_hdr_props_t;

typedef struct {
  
  si32        object_type;            // Type of object SYMPROD_OBJ_xxx     
  si32        object_id;              // Object ID.  This field gives more
                                      //   identifying info about the object
                                      //   that is specific to the creating 
                                      //   algorithm.                       
  si32        num_bytes;              // Number of bytes in object structure
                                      //   excluding object header.         
  si32        detail_level;           // Bitwise Usage Tips, including:
                                      // Detail level/threshold override,
                                      // scaling override,
                                      // Indicates a hidden/popup object,
                                      // Requests the user ACK
                                      // Note: Use enum detail_level_t

  char        color[SYMPROD_COLOR_LEN];
                                      // Suggested foreground color for     
                                      //   rendering object.                
  char        background_color[SYMPROD_COLOR_LEN];
                                      // Suggested background color for     
                                      //   rendering object.                
  wpt_t centroid;                     // Object centroid, used for picking  
                                      //   the object.                      
} obj_hdr_t;


////////////////////////////////////
// Typedefs for individual objects.


typedef struct {

  wpt_t                origin;           // World coordinate origin of text.
  ppt_t                offset;           // Pixel coordinate offset of text.
  si32                 vert_alignment;   // Text vertical alignment         
  si32                 horiz_alignment;  // Text horizontal alignment       
  si32                 font_size;        // An indication of the importance 
                                         //   of this text string to give   
                                         //   the display a hint about what 
                                         //   font to use in rendering it.  
  si32                 font_style;       // suggested style
  si32                 length;           // Text length, not including NULL.
  char                 fontname[SYMPROD_FONT_NAME_LEN];
                                         // The name of the font to be used 
                                         //   in rendering the text.        
} text_props_t;


typedef struct {

  si32                 close_flag;       // Flag indicating if the polyline 
                                         //   should be closed (i.e. should 
                                         //   be a polygon).                
  si32                 fill;             // Polygon fill information        
                                         //   FILL_xxx, only used if
                                         //   close_flag is TRUE.           
  si32                 linetype;         // Suggested line type for         
                                         //   rendering polyline            
                                         //   LINETYPE_xxx.         
  si32                 linewidth;        // Suggested line width for        
                                         //   rendering polyline.           
  si32                 capstyle;         // Suggested cap style for         
                                         //   rendering polyline            
                                         //   CAPSTYLE_xxx.         
  si32                 joinstyle;        // Suggested join style for        
                                         //   rendering polyline            
                                         //   JOINSTYLE_xxx.        
  si32                 line_interp;      // Suggested line interpolation    
                                         //   method for rendering polyline 
                                         //   LINE_INTERP_xxx.      
  si32                 num_points;       // Number of points in polyline.   

} polyline_props_t;


typedef struct {

  wpt_t                origin;           // Origin for rendering line
  si32                 close_flag;       // Flag indicating if the iconline 
                                         //   should be closed (i.e. should 
                                         //   be a polygon).                
  si32                 fill;             // Polygon fill information        
                                         //   FILL_xxx, only used if
                                         //   close_flag is TRUE.           
  si32                 linetype;         // Suggested line type for         
                                         //   rendering iconline            
                                         //   LINETYPE_xxx.         
  si32                 linewidth;        // Suggested line width for        
                                         //   rendering iconline.           
  si32                 capstyle;         // Suggested cap style for         
                                         //   rendering iconline            
                                         //   CAPSTYLE_xxx.         
  si32                 joinstyle;        // Suggested join style for        
                                         //   rendering iconline            
                                         //   JOINSTYLE_xxx.        
  si32                 line_interp;      // Suggested line interpolation    
                                         //   method for rendering iconline 
                                         //   LINE_INTERP_xxx.      
  si32                 num_points;       // Number of points in iconline.   

} iconline_props_t;


typedef struct {

  si32             num_icon_pts;         // Number of points making up the  
                                         //   icon.                         
  si32             num_icons;            // Number of icons to display.     
                                         //   should be num_icons of these. 
  si32             linewidth;            // Suggested line width 

} stroked_icon_props_t;

typedef struct {

  char  name[SYMPROD_ICON_NAME_LEN]; // Name of icon to display.        
  si32  num_icons;                   // Number of icons to display.     
  si32  spare;                       // Spare field for alignment.      
  
} named_icon_props_t;


typedef struct {

  si32             bitmap_x_dim;         // Number of bits in the bitmap in 
                                         //   the X direction.              
  si32             bitmap_y_dim;         // Number of bits in the bitmap in 
                                         //   the Y direction.              
  si32             num_icons;            // Number of icons to display.     
  si32             spare;                // Spare field for alignment.      

} bitmap_icon_props_t;

typedef struct {

  wpt_t           origin;                // Center of rectangle containing  
                                         //   the arc.                      
  fl32            radius_x;              // Radius on x axis - km/pixels
  fl32            radius_y;              // Radius on y axis - km/pixels
  fl32            angle1;                // Start of arc in degrees
                                         // counterclockwise
                                         // relative to the x-axis
  fl32            angle2;                // End of arc in degrees
                                         // counterclockwise
                                         // relative to the x-axis
  fl32            axis_rotation;         // Whole ellipse may be rotated
                                         // counterclockwise by this amount
                                         // in degrees
  si32            linetype;              // Suggested line type for         
                                         //   rendering arc                 
                                         //   LINETYPE_xxx.         
  si32            linewidth;             // Suggested line width for        
                                         //   rendering arc.                
  si32            fill;                  // Arc fill information            
                                         //   FILL_xxx.             
  si32            capstyle;              // Suggested cap style for         
                                         //   rendering arc                 
                                         //   CAPSTYLE_xxx.         
  si32            joinstyle;             // Suggested join style for        
                                         //   rendering arc                 
                                         //   JOINSTYLE_xxx.
  si32            nsegments;             // hint to rendering routine on
                                         // how many segments to use to 
                                         // render the arc
  si32            radii_in_pixels;       // if 0, radii are in the same units
                                         //   as the wpt coordinates
                                         // if 1, radii are in pixels
} arc_props_t;

typedef struct {

  wpt_t           origin;                // Lower left hand corner of       
                                         // rectangle.                    

  fl32            height;                // Height of rectangle - km
  fl32            width;                 // Width of rectangle - km
             
  si32            linetype;              // Suggested line type for         
                                         //   rendering rectangle           
                                         //   LINETYPE_xxx.         
  si32            linewidth;             // Suggested line width for        
                                         //   rendering rectangle.          
  si32            fill;                  // Fill information for rectangle  
                                         //   FILL_xxx.             
  si32            capstyle;              // Suggested cap style for         
                                         //   rendering rectangle           
                                         //   CAPSTYLE_xxx.         
  si32            joinstyle;             // Suggested join style for        
                                         //   rendering rectangle           
                                         //   JOINSTYLE_xxx.        
  si32            spare;                 // Spare field for alignment.      

} rectangle_props_t;

typedef struct chunk_obj {

  si32            chunk_type;            // Chunk type identifier.  This can
                                         //   be used to identify the type  
                                         //   of data in the chunk.         
  si32            user_info;             // User-specified info that can be
                                         // used by the client.  
  si32            nbytes_chunk;          // number of bytes in chunk
  si32            spare;                 // for byte alignment

} chunk_props_t;

#endif
