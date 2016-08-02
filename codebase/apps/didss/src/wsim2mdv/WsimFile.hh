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
/////////////////////////////////////////////////////////////
// WsimFile.h: WsimFile object
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// January 1998
//
//////////////////////////////////////////////////////////

#ifndef WsimFile_h
#define WsimFile_h

#include <sys/time.h>

#include <toolsa/toolsa_macros.h>
#include <toolsa/pjg.h>
using namespace std;

typedef enum
{
  WSIM_FILE_OKAY,

  WSIM_FILE_ERROR
} WsimFileStatus_t;


typedef enum
{
  WSIM_FILE_PROJ_LATLON,
  WSIM_FILE_PROJ_FLAT
} WsimFileProjType_t;


typedef enum
{
  WSIM_FILE_FILTER_MAX,
  WSIM_FILE_FILTER_MEAN_DBZ,
  WSIM_FILE_FILTER_MEAN_Z
} WsimFileFilterType_t;


class WsimFile
{
  
public:

  // Constructor
       
  WsimFile(double *value_table,
	   const int num_values,     // MUST be 15
	   const double data_scale,
	   const double data_bias,
	   const int    resample_grid = FALSE,
	   const WsimFileProjType_t output_projection = WSIM_FILE_PROJ_LATLON,
	   const double output_x_min = 0.0,
	   const double output_y_min = 0.0,
	   const double output_x_delta = 0.0,
	   const double output_y_delta = 0.0,
	   const int    output_nx = 0,
	   const int    output_ny = 0,
	   const double flat_origin_lat = 0.0,
	   const double flat_origin_lon = 0.0,
	   const WsimFileFilterType_t filter_type = WSIM_FILE_FILTER_MAX,
	   const double coverage_threshold = 0.0,
	   const int debug_flag = FALSE);
  
  // Destructor
  
  ~WsimFile();

  // Print the internal WSI header information

  void printHeader(FILE *stream);
  
  // Read in the indicated file

  WsimFileStatus_t read(char *wsi_file_name);
  
  WsimFileStatus_t read(FILE *wsi_file,
			size_t wsi_file_len);
  
  // Access member functions

  ui08 *getData(void)
  {
    return(_outputData);
  }
  
  int getDataSize(void)
  {
    return(_outputNX * _outputNY);
  }
  
  time_t getDataTime(void)
  {
   return(_wsiHeader.data_time);
  }
  
  double getDx(void)
  {
    return(_outputDeltaX);
  }
  
  double getDy(void)
  {
    return(_outputDeltaY);
  }
  
  double getMinX(void)
  {
    return(_outputMinX);
  }
  
  double getMinY(void)
  {
    return(_outputMinY);
  }
  
  int getNx(void)
  {
    return(_outputNX);
  }
  
  int getNy(void)
  {
    return(_outputNY);
  }
  
  double getOriginLat(void)
  {
    if (_resampleGrid &&
	_outputProjection == WSIM_FILE_PROJ_FLAT)
      return(_flatOriginLat);
    
    if (_wsiHeader.nav_code == 'C')
      return(_wsiHeader.top_lat);
    else if (_wsiHeader.nav_code == 'L')
      return(_wsiHeader.proj_center_lat);
    else
      return(-1.0);
  }
  
  double getOriginLon(void)
  {
    if (_resampleGrid &&
	_outputProjection == WSIM_FILE_PROJ_FLAT)
      return(_flatOriginLon);
    
    if (_wsiHeader.nav_code == 'C')
      return(_wsiHeader.center_lon);
    else if (_wsiHeader.nav_code == 'L')
      return(_wsiHeader.proj_center_lon);
    else
      return(-1.0);
  }
  
protected:
  
private:

  /*** WSI DEFINES ******/

  static const ui08 WSI_FLAG1 = 0x00;
  static const ui08 WSI_FLAG2 = 0xF0;

  static const ui08 WSI_BIN_FLAG            = 0x00;
  static const ui08 WSI_TRANS_COMPLETE_FLAG = 0x02;
  static const ui08 WSI_LABEL_FLAG          = 0x03;
  static const ui08 WSI_STATUS_FLAG         = 0x06;
  static const ui08 WSI_HEADER_FLAG         = 0x09;
  static const ui08 WSI_IMAGE_SIZE_FLAG     = 0x0A;
  static const ui08 WSI_PROJ_NAV_FLAG       = 0x0B;
  static const ui08 WSI_IMAGE_LINE_FLAG     = 0x0C;

  static const ui08 WSI_MARK_MIN            = 0x80;
  static const ui08 WSI_MARK_MAX            = 0x82;
  
  static const ui08 WSI_RES_MED_4BIT        = 0x11;
  static const ui08 WSI_RES_HIGH_4BIT       = 0x12;
  static const ui08 WSI_RES_MED_8BIT        = 0x14;
  static const ui08 WSI_RES_HIGH_8BIT       = 0x15;
  
  static const ui08 WSI_3_BYTE_COUNT        = 0xD;
  static const ui08 WSI_2_BYTE_COUNT        = 0xE;
  static const ui08 WSI_HI_COLOR_FLAG       = 0xF;

  static const int WSI_SITE_UP_MIN           = 16;
  static const int WSI_SITE_UP_MAX           = 31;
  static const int WSI_SITE_DOWN_MIN         = 32;
  static const int WSI_SITE_DOWN_MAX         = 47;

  static const ui08 WSI_COLOR_MASK          = 0x0F;
  static const ui08 WSI_LENGTH_MASK         = 0xF0;
  static const int IMAGE_LABEL_LEN           = 80;

  typedef struct
  {
    time_t  data_time;
    
    char    nav_code;

    double  center_lon;
    double  top_lat;
    double  diff_lon;
    double  deg_per_line;
    double  deg_per_element;

    double  parallel_1;
    double  parallel_2;
    double  proj_center_lat;
    double  proj_center_lon;
    double  upper_left_x;
    double  upper_left_y;
    double  pixel_res_x;
    double  pixel_res_y;
    
    char    *image_header;
    char    mark;
    char    resolution_byte;
	
    int     image_lines;
    int     image_pixels;
	
    char    image_label[IMAGE_LABEL_LEN+1];

  } wsi_header_t;
    
  int _debugFlag;
  
  wsi_header_t _wsiHeader;
  ui08         *_imageData;
  double       _dataScale;
  double       _dataBias;
  int          _validImageSizeInfo;
  int          _validNavInfo;
  int          _lastLine;
  
  ui08         _valueTable[16];
  
  int          _resampleGrid;
  WsimFileFilterType_t _filterType;
  double       _coverageThreshold;
  WsimFileProjType_t _outputProjection;
  double       _outputMinX;
  double       _outputMinY;
  double       _outputDeltaX;
  double       _outputDeltaY;
  int          _outputNX;
  int          _outputNY;
  double       _flatOriginLat;
  double       _flatOriginLon;
  
  ui08         *_outputData;
  
  ui08 *_addLineToBuffer(ui08 *buffin,
			 int line_num);

  void _convertToVIP(ui08 *buffin);

  void _freeData(void);
  void _initData(int free_data_flag);
  
  ui08 *_loadCEGrid(void);
  ui08 *_loadCEGridFlat(void);
  ui08 *_loadCEGridResampled(void);
  ui08 *_loadLCGrid(void);
  
  ui08 *_readHeader(ui08 *header_ptr);
  ui08 *_readImageLine(ui08 *image_line_ptr);
  ui08 *_readImageSize(ui08 *image_size_ptr);
  ui08 *_readLabel(ui08 *label_ptr);
  ui08 *_readProjection(ui08 *projection_ptr);
  ui08 *_readStatus(ui08 *status_ptr);
  
  void _setImageValue(int line, int pixel, ui08 color);
  
  inline ui08 _calculateColor(ui08 high_nibble, ui08 color_byte)
  {
    return(high_nibble | (WSI_COLOR_MASK & color_byte));
  }
  
};

#endif
