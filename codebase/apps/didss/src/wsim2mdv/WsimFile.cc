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
// WsimFile.cc : WSI mosaic file handling
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// January 1998
//
//////////////////////////////////////////////////////////

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include <toolsa/os_config.h>
#include <dataport/port_types.h>
#include <dataport/smallend.h>
#include <toolsa/udatetime.h>
#include <toolsa/file_io.h>
#include <toolsa/globals.h>
#include <toolsa/mem.h>
#include <toolsa/pjg.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/utim.h>

#include "WsimFile.hh"
using namespace std;


/************************************************************
 * Constructor
 */

WsimFile::WsimFile(double *value_table,
		   const int num_values,
		   const double data_scale,
		   const double data_bias,
		   const int    resample_grid,
		   const WsimFileProjType_t output_projection,
		   const double output_x_min,
		   const double output_y_min,
		   const double output_x_delta,
		   const double output_y_delta,
		   const int    output_nx,
		   const int    output_ny,
		   const double flat_origin_lat,
		   const double flat_origin_lon,
		   const WsimFileFilterType_t filter_type,
		   const double coverage_threshold,
		   const int debug_flag)
{
  _debugFlag = debug_flag;

  // Save the output grid inforation

  _dataScale = data_scale;
  _dataBias = data_bias;
  
  _resampleGrid = resample_grid;
  
  _filterType = filter_type;
  _coverageThreshold = coverage_threshold;
  
  _outputProjection = output_projection;
  
  _outputMinX = output_x_min;
  _outputMinY = output_y_min;
  _outputDeltaX = output_x_delta;
  _outputDeltaY = output_y_delta;
  _outputNX = output_nx;
  _outputNY = output_ny;
  _flatOriginLat = flat_origin_lat;
  _flatOriginLon = flat_origin_lon;
  
  _outputData = (ui08 *)NULL;
  
  // Initialize the value table

  if (num_values != 15)
  {
    fprintf(stderr, "WsimFile::Constructor\n");
    fprintf(stderr, "Invalid number of values in value table -- must be 15\n");
    
    exit(-1);
  }
  
  _valueTable[0] = 0;    // missing/bad data flag
  for (int i = 1; i < 16; i++)
  {

    int ival =
      (int)(((value_table[i-1] - _dataBias) / _dataScale) + 0.5);

    if (ival < 1)
      ival = 1;
    else if (ival > 255)
      ival = 255;
    
    _valueTable[i] = (ui08) ival;

    if (_debugFlag)
      fprintf(stderr, "_valueTable[%d] = %d\n", i, _valueTable[i]);
  }
  
  // Initialize the data stores

  _initData(FALSE);

}


/************************************************************
 * Destructor
 */

WsimFile::~WsimFile ()
{
  _freeData();
}


/*********************************************************************
 * printHeader()
 */

void WsimFile::printHeader(FILE *stream)
{
  fprintf(stream, "WSI header:\n");
  fprintf(stream, "   data_time = %s\n", utimstr(_wsiHeader.data_time));
  fprintf(stream, "\n");
  fprintf(stream, "   nav_code = %c\n", _wsiHeader.nav_code);
  if (_wsiHeader.nav_code == 'C')
  {
    fprintf(stream, "   center_lon = %f deg\n", _wsiHeader.center_lon);
    fprintf(stream, "   top_lat = %f deg\n", _wsiHeader.top_lat);
    fprintf(stream, "   diff_lon = %f deg\n", _wsiHeader.diff_lon);
    fprintf(stream, "   deg_per_line = %f deg\n", _wsiHeader.deg_per_line);
    fprintf(stream, "   deg_per_element = %f deg\n",
	    _wsiHeader.deg_per_element);
  }
  else if (_wsiHeader.nav_code == 'L')
  {
    fprintf(stream, "   parallel_1 = %f deg\n", _wsiHeader.parallel_1);
    fprintf(stream, "   parallel_2 = %f deg\n", _wsiHeader.parallel_2);
    fprintf(stream, "   proj_center_lat = %f deg\n",
	    _wsiHeader.proj_center_lat);
    fprintf(stream, "   proj_center_lon = %f deg\n",
	    _wsiHeader.proj_center_lon);
    fprintf(stream, "   upper_left_x = %f km\n",
	    _wsiHeader.upper_left_x);
    fprintf(stream, "   upper_left_y = %f km\n",
	    _wsiHeader.upper_left_y);
    fprintf(stream, "   pixel_res_x = %f km\n",
	    _wsiHeader.pixel_res_x);
    fprintf(stream, "   pixel_res_y = %f km\n",
	    _wsiHeader.pixel_res_y);
  }
  else
  {
    fprintf(stream, "   *** UNKNOWN NAV CODE ***\n");
  }

  fprintf(stream, " \n");
  fprintf(stream, "   image_header = %s\n", _wsiHeader.image_header);
  fprintf(stream, "   mark = %d\n", _wsiHeader.mark);
  fprintf(stream, "   resolution_byte = %d\n", _wsiHeader.resolution_byte);
  fprintf(stream, " \n");
  fprintf(stream, "   image_lines = %d\n", _wsiHeader.image_lines);
  fprintf(stream, "   image_pixels = %d\n", _wsiHeader.image_pixels);
  fprintf(stream, " \n");
  fprintf(stream, "   image_label = %s\n", _wsiHeader.image_label);
  fprintf(stream, "\n");
  
  fprintf(stream, "Output grid:\n");
  fprintf(stream, "   output_min_x = %f deg\n", _outputMinX);
  fprintf(stream, "   output_min_y = %f deg\n", _outputMinY);
  fprintf(stream, "   output_delta_x = %f deg\n", _outputDeltaX);
  fprintf(stream, "   output_delta_y = %f deg\n", _outputDeltaY);
  fprintf(stream, "   output_nx = %d\n", _outputNX);
  fprintf(stream, "   output_ny = %d\n", _outputNY);
  fprintf(stream, "\n");
  
  return;
}

  
/*********************************************************************
 * read() - Read the indicated WSI NOWrad mosaic file.
 */

WsimFileStatus_t WsimFile::read(char *wsi_file_name)
{
  FILE *wsim_file;
  struct stat file_stat;
  
  if (stat(wsi_file_name, &file_stat) != 0)
  {
    fprintf(stderr,
	    "Error stating file <%s> -- skipping\n",
	    wsi_file_name);
    return(WSIM_FILE_ERROR);
  }
  
  if ((wsim_file = ta_fopen_uncompress(wsi_file_name, "r"))
      == (FILE *)NULL)
  {
    fprintf(stderr, "Error opening input file <%s> -- skipping\n",
	    wsi_file_name);
    return(WSIM_FILE_ERROR);
  }
  
  WsimFileStatus_t status = read(wsim_file,
				 file_stat.st_size);

  fclose(wsim_file);
  
  return(status);
}

WsimFileStatus_t WsimFile::read(FILE *wsi_file,
				size_t wsi_file_len)
{
  static ui08 *wsi_buffer;
  static size_t wsi_buffer_len = 0;

  int  reading_file = TRUE;
  ui08 *byte_in;

  // Set up timing of operations

  if (_debugFlag)
    UTIMtime_diff( 1);

  /* keep growing the buffer as needed */
  if (wsi_buffer_len == 0)
  {
    wsi_buffer = (ui08 *)umalloc(wsi_file_len);
    wsi_buffer_len = wsi_file_len;
  }
  else if (wsi_file_len > wsi_buffer_len)
  {
    wsi_buffer = (ui08 *)urealloc(wsi_buffer, wsi_file_len);
    wsi_buffer_len = wsi_file_len;
  }

  if (_debugFlag)
    fprintf(stderr, "WsimFile::read(): nbytes = %ld\n", wsi_file_len);

  if (wsi_file_len <= 0)
  {
    fprintf(stderr, "WSI file is empty.  Not processing...\n");
    return(WSIM_FILE_ERROR);
  }
  
  /****** read in the input file */

  if (fread(wsi_buffer, 1, wsi_file_len, wsi_file) != wsi_file_len)
  {
    fprintf(stderr, "Error reading in WSI input file\n");
    return(WSIM_FILE_ERROR);
  }
  
  /****** convert file to native structure */

  _initData(TRUE);
  
  byte_in = wsi_buffer;
	
  while (reading_file)
  {
    PMU_auto_register("Reading in file");
    
    if (*byte_in != WSI_FLAG1 || *(byte_in+1) != WSI_FLAG2)
    {
      fprintf(stderr,
	      "Invalid values in input file at byte %ld:  %x %x\n"
	      "    Expecting command flags: %x %x\n",
	      byte_in - wsi_buffer, *byte_in, *(byte_in+1),
	      WSI_FLAG1, WSI_FLAG2);
      return(WSIM_FILE_ERROR);
    }

    byte_in += 2;
      
    switch (*byte_in)
    {
    case WSI_TRANS_COMPLETE_FLAG :
      if (_debugFlag)
	fprintf(stderr, "WSI_TRANS_COMPLETE_FLAG found\n");
      
      reading_file = FALSE;
      break;
			
    case WSI_LABEL_FLAG :
      byte_in = _readLabel(++byte_in);
      break;
			
    case WSI_STATUS_FLAG :
      byte_in = _readStatus(++byte_in);
      break;
			
    case WSI_HEADER_FLAG :
      byte_in = _readHeader(++byte_in);
      break;
	
    case WSI_IMAGE_SIZE_FLAG :
      byte_in = _readImageSize(++byte_in);
      break;
			
    case WSI_PROJ_NAV_FLAG :
      byte_in = _readProjection(++byte_in);
      break;
			
    case WSI_IMAGE_LINE_FLAG :
      byte_in = _readImageLine(++byte_in);
      break;
	
    default:
      fprintf(stderr,
	      "Invalid WSI command flag: %x at byte %ld in input file -- skipping to next command flag\n",
	      *byte_in,
	      byte_in - wsi_buffer);

      // Skip to the next control byte in the file

      while (!(*byte_in == WSI_FLAG1 &&
	       *(byte_in+1) == WSI_FLAG2 &&
	       *(byte_in+2) != WSI_BIN_FLAG))
	byte_in++;

    } /* endswitch - *byte_in */
		
  }

  // Calculate the data time

  char *colon_pos = strchr(_wsiHeader.image_label, ':');
  
  if (colon_pos == (char *)NULL)
  {
    return(WSIM_FILE_ERROR);
  }
    
  date_time_t time_struct;
  char month_str[4];
  
  char cmon[][6]={"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

  sscanf(colon_pos + 3, "%d-%3s-%d",
	 &time_struct.day, month_str, &time_struct.year);
  
  if (time_struct.year < 50)
    time_struct.year += 2000;
  else
    time_struct.year += 1900;
  
  // Get integer month from month string

  for (int i = 0; i < 12; i++)
  {
    if (strstr(month_str, cmon[i]) != (char *)NULL)
    {
      time_struct.month = i + 1;
      break;
    }
  }  
 
  // Now get the time

  colon_pos = strchr(_wsiHeader.image_header, ':');
  
  if (colon_pos == (char *)NULL)
  {
    return(WSIM_FILE_ERROR);
  }
  
  sscanf(colon_pos - 2, "%d:%d",
	 &time_struct.hour, &time_struct.min);
  time_struct.sec = 0;
  
  _wsiHeader.data_time = uunix_time(&time_struct);
  
  // Transform the data into the Cylindrical Equidistant projection
  // we use for display

  if (_outputData != (ui08 *)NULL)
  {
    ufree(_outputData);
    _outputData = (ui08 *)NULL;
  }
  
  if (_wsiHeader.nav_code == 'C')
  {
    _outputData = _loadCEGrid();
  }
  else if (_wsiHeader.nav_code == 'L')
  {
    _outputData = _loadLCGrid();
  }
  
  /******* determine the time taken to process this file */
  if (_debugFlag)
  {
    fprintf(stderr, "==== took %ld millsecs\n", UTIMtime_diff(0));
    fprintf(stderr, "\n");

  }
  
  return(WSIM_FILE_OKAY);

}


/************************************************************
 * PRIVATE MEMBER FUNCTIONS
 ************************************************************/

/************************************************************
 * _addLineToBuffer() - Adds an image line from the input file
 *                      to the image buffer.
 *
 * From "EXPRESS NOWradPLUS SERVICE TECHNICAL DESCRIPTION",
 * Jan. 1992:
 *
 * Run Length Encoding:
 *
 * The format of the run length encoding allows runs of up to
 * 65,536 pixels of the color values 0 to 255 to be encoded.
 * Runs are encoded with a varying number of bytes depending
 * on the length of the run.  Color values are encoded as 2
 * separate nibbles (4 bits) with the high nibble being set in
 * a special flag byte and the low nibble being sent with each
 * run.
 *
 * Runs of 1 to 13 pixels are encoded in a single byte with the
 * lower 4 bits begin the low nibble of the color and the higher
 * 4 bits being the length of the run minus 1 (0x0 to 0xC hex).
 *
 * Runs of 14 to 256 pixels are encoded in 2 bytes.  The first
 * byte contains the flag 0xE or 14 in the high nibble, the low
 * nibble of the color in the lower 4 bits.  This byte is followed
 * by one byte with the length of the run minus 1 (0x0D to 0xFF hex).
 *
 * Runs of 257 to 65,536 pixels are encoded in 3 bytes.  The
 * first byte contains the flag 0xD or 13 in the high nibble, the
 * low nibble of the color in the lower 4 bits.  The byte is
 * followed by 2 bytes with the length of the run minus 1 (0x0100
 * to 0xFFFF hex). [NOTE:  This length byte is probably in little-
 * endian format since the line number at the beginning of the line
 * is in little-endian format, but this is not specified in the
 * technical description. -nr]
 *
 * At the beginning of a line segment, the high nibble of the color
 * is assumed to be zero.  This value is changed with a special
 * flag byte.  The high nibble of the flag byte contains the value
 * 0xF or 15.  The lower 4 bits of the byte contain the high nibble
 * of the color value.  The value of the high nibble should be
 * carried over to the next 1 or more runs.  The value is only
 * changed when a new flag byte for the high nibble color is
 * encountered or the end of the line.
 *
 * With this encoding, it is possible for the flag sequence
 * "0x00 0xF0" to appear as data.  To prevent a misinterpretation
 * of the data as a flag sequence, a control byte of 0x00 is placed
 * as the next byte after the sequence.  This indicates that the
 * preceding 2 bytes are data, and should not be intrepreted as a
 * start of a new segment.
 *
 * The following is an example of run length encoded bytes and their
 * interpretation:
 *
 *       0xD0 0x0105 0xF1 0x01 0x12 0xE3 0x20 0x00 0xF0 0x00 0xD0 0x0F00
 *
 *       0xD0 0x0105   run of 262 bytes of color 0
 *       0xF1          flag byte to change the high nibble of the
 *                       color to 0x1
 *       0x01          run of 1 byte of color 16  [color 17???]
 *       0x12          run of 2 bytes of color 17  [color 18???]
 *       0xE3 0x20     run of 32 bytes of color 18  [color 19???]
 *       0x00          run of 1 byte of color 16
 *       0xF0          flag byte to change the high nibble of the
 *                       color to 0x0
 *       0x00          stuff byte to indicated previous 2 bytes are data
 *       0xD0 0x0F00   run of 3840 bytes of color 0 (0x00)
 */

ui08 *WsimFile::_addLineToBuffer(ui08 *buffin,
				 int line_num)
{
  ui08 high_nibble = 0;         /* high nibble of color byte */
  ui08 color;                   /* color level of pixel */
  int column = 0;               /* column number of current pixel */
  ui16 length = 0;	        /* length of pixel run */

  int i;
  
  // Process the line until we reach the control data indicating
  // the next file segment

  while ((*buffin != WSI_FLAG1 &&
	  *(buffin+1) != WSI_FLAG2) ||
	 !(*buffin == WSI_FLAG1 &&
	   *(buffin+1) == WSI_FLAG2 &&
	   *(buffin+2) != WSI_BIN_FLAG))
  {
    // Put the contents of the input buffer into the image buffer
    // in the correct format.

    if (*buffin == WSI_FLAG1 &&
	*(buffin+1) == WSI_FLAG2 &&
	*(buffin+2) == WSI_BIN_FLAG)
    {
      // In this case, the first bin flag indicates to take the preceding
      // two bytes as data.  The first byte (0x00) indicates a run of 1 byte
      // of color 0 with the current high nibble.  The second byte (0xF0)
      // indicates changing the high nibble for the next runs to 0.
      
      color = _calculateColor(high_nibble, *buffin);

      length = 1;
      
//      if (color != 0)
//	fprintf(stderr,
//		"1-byte count, length = 1, row = %d, column = %d, "
//		"color = %d\n", 
//		line_num, column, color);

      int hn = (WSI_FLAG2 << 4);
      high_nibble = (ui08) hn;

      buffin += 3;
    }
    else if ((*buffin >> 4) == WSI_HI_COLOR_FLAG)
    {
      // In this case, we are resetting the high nibble for the
      // following runs

      high_nibble = (*buffin << 4);
      buffin++;

      length = 0;
    }
    else if ((*buffin >> 4) == WSI_2_BYTE_COUNT)
    {
      // In this case, we have a run contained in two bytes

      color = _calculateColor(high_nibble, *buffin);

      length = *(buffin+1) + 1;

//      if (color != 0)
//	fprintf(stderr,
//		"2-byte count, length = %d, row = %d, column = %d, "
//		"color = %d\n", 
//		length, line_num, column, color);

      buffin += 2;
    }
    else if((*buffin >> 4) == WSI_3_BYTE_COUNT)
    {
      // In this case, we have a run contained in 3 bytes

      color = _calculateColor(high_nibble, *buffin);

      memcpy(&length, buffin+1, 2);
      length = SE_to_ui16(length);
      length++;
      
//      if (color != 0)
//	fprintf(stderr,
//		"3-byte count, length = %d, row = %d, column = %d, "
//		"color = %d\n", 
//		length, line_num, column, color);
      
      buffin += 3;
    }
    else
    {
      // In this case, we have run information in a single byte

      color = _calculateColor(high_nibble, *buffin);

      length = ((unsigned) (WSI_LENGTH_MASK & *buffin) >> 4) + 1;

//      if (color != 0)
//	fprintf(stderr,
//		"1-byte count, length = %d, row = %d, "
//		"column = %d, color = %d\n", 
//		length, line_num, column, color);
			
      buffin++;
    }

    for (i = 0; i < length; i++)
      _setImageValue(line_num, column + i, color);
	
    column += length;

  }
	
  if (_debugFlag && column != _wsiHeader.image_pixels)
    fprintf(stderr, "=== Line %d had %d columns, should have %d columns\n",
	    line_num, column, _wsiHeader.image_pixels);
  
  return(buffin);
}


/************************************************************
 * _convertToVIP()
 */

void WsimFile::_convertToVIP(ui08 *buffin)
{
  ui08 color;		/* color level of pixel */

  color = *buffin & WSI_COLOR_MASK;

  switch (color)
  {
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
    *buffin = (*buffin & WSI_LENGTH_MASK) | 1;
    break;

  case 6:
  case 7:
    *buffin = (*buffin & WSI_LENGTH_MASK) | 2;
    break;

  case 8:
    *buffin = (*buffin & WSI_LENGTH_MASK) | 3;
    break;

  case 9:
    *buffin = (*buffin & WSI_LENGTH_MASK) | 4;
    break;

  case 10:
    *buffin = (*buffin & WSI_LENGTH_MASK) | 5;
    break;

  case 11:
  case 12:
  case 13:
  case 14:
  case 15:
    *buffin = (*buffin & WSI_LENGTH_MASK) | 6;
    break;
  }
}


/************************************************************
 * _freeData()
 */

void WsimFile::_freeData(void)
{
  if (_wsiHeader.image_header != (char *)NULL)
    ufree(_wsiHeader.image_header);
  _wsiHeader.image_header = (char *)NULL;
  
  if (_imageData != (ui08 *)NULL)
    ufree(_imageData);
  _imageData = (ui08 *)NULL;
  
  if (_outputData != (ui08 *)NULL)
    ufree(_outputData);
  _outputData = (ui08 *)NULL;
  
  return;
}


/************************************************************
 * _initData()
 */

void WsimFile::_initData(int free_data_flag)
{
  if (free_data_flag)
    _freeData();
  
  memset((void *)&_wsiHeader, 0, sizeof(wsi_header_t));
  
  _validImageSizeInfo = FALSE;
  _validNavInfo = FALSE;
  _lastLine = 0;

  _imageData = (ui08 *)NULL;
  _outputData = (ui08 *)NULL;
  
  return;
}


/************************************************************
 * _loadCEGrid()
 */

ui08 *WsimFile::_loadCEGrid(void)
{
  ui08 *return_buffer;
  
  if (_resampleGrid)
  {
    if (_outputProjection == WSIM_FILE_PROJ_FLAT)
      return_buffer = _loadCEGridFlat();
    else
      return_buffer = _loadCEGridResampled();
    
  }
  else
  {
    _outputMinX = _wsiHeader.center_lon - _wsiHeader.diff_lon +
      (_wsiHeader.deg_per_element / 2.0);
    _outputMinY = _wsiHeader.top_lat -
	      (_wsiHeader.image_lines * _wsiHeader.deg_per_line) +
      (_wsiHeader.deg_per_line / 2.0);
    
    _outputDeltaX = _wsiHeader.deg_per_element;
    _outputDeltaY = _wsiHeader.deg_per_line;
    
    _outputNX = _wsiHeader.image_pixels;
    _outputNY = _wsiHeader.image_lines;
    
    int image_size = _wsiHeader.image_lines * _wsiHeader.image_pixels;
    
    return_buffer = (ui08 *)umalloc(image_size);
    
    memcpy(return_buffer, _imageData, image_size);
  }
    
  return(return_buffer);
}


/************************************************************
 * _loadCEGridFlat()
 */

ui08 *WsimFile::_loadCEGridFlat(void)
{
  static int first_call = TRUE;
  static int *index_grid;
  static si32 npoints;

  ui08 *data_grid;
  si32 i, ix, iy;

  /*
   * allocate on first call
   */

  if (first_call)
  {
    /*
     * allocate
     */

    npoints = _outputNX * _outputNY;
    index_grid = (int *) umalloc ((ui32) (npoints * sizeof(int)));
    
    /*
     * load up index array
     */
    
    PJGflat_init(_flatOriginLat,
		 _flatOriginLon,
		 0.0);

    i = 0;
    for (iy = 0; iy < _outputNY; iy++)
    {
      for (ix = 0; ix < _outputNX; ix++)
      {
	double grid_centroid_x, grid_centroid_y;
	double grid_centroid_lat, grid_centroid_lon;
	int wsi_x, wsi_y;

	grid_centroid_x = _outputMinX + ix * _outputDeltaX;
	grid_centroid_y = _outputMinY + iy * _outputDeltaY;

	PJGflat_xy2latlon(grid_centroid_x, grid_centroid_y,
			  &grid_centroid_lat, &grid_centroid_lon);

	wsi_x = (int)((grid_centroid_lon -
		       (_wsiHeader.center_lon - _wsiHeader.diff_lon)) /
		      _wsiHeader.deg_per_element);
	wsi_y = (int)((grid_centroid_lat - 
		       (_wsiHeader.top_lat -
			(_wsiHeader.deg_per_line * _wsiHeader.image_lines))) /
		      _wsiHeader.deg_per_line);

	if (wsi_x < 0 || wsi_x >= _wsiHeader.image_pixels ||
	    wsi_y < 0 || wsi_y >= _wsiHeader.image_lines)
	  index_grid[i] = -1;
	else
	  index_grid[i] = (wsi_y * _wsiHeader.image_pixels) + wsi_x;

	i++;
      } /* endfor - ix */
    } /* endfor - iy */

    first_call = FALSE;
  }
  
  /*
   * initialize
   */
  
  data_grid = (ui08 *) umalloc ((ui32) (npoints * sizeof(ui08)));
  memset((void *) data_grid, 0, (int) (npoints * sizeof(ui08)));

  /*
   * load up data grid array
   */

  for (i = 0; i < npoints; i++)
  {
    if (index_grid[i] >= 0)
      data_grid[i] = _imageData[index_grid[i]];
    else
      data_grid[i] = 0;
  } /* endfor - i */

  return(data_grid);
}


/************************************************************
 * _loadCEGridResampled()
 */

ui08 *WsimFile::_loadCEGridResampled(void)
{
  static int first_call = TRUE;
  static ui08 *data_grid, *dg;
  static si32 npoints;
  static si32 *xindex;
  static double *nhits, *nh;
  static double *npossible, *np;
  static double *sum, *z_sum, *su;
  static double z_table[256];

  ui08 *line;
  si32 ilat, ilon;
  si32 i, ix, iy;
  si32 index, start_index;
  si32 mean_val;
  double lon, lat;
  double fraction_covered;

  /*
   * allocate on first call
   */

  if (first_call)
  {
    /*
     * allocate
     */

    npoints = _outputNX * _outputNY;
    data_grid = (ui08 *) umalloc ((ui32) (npoints * sizeof(ui08)));
    sum = (double *) umalloc ((ui32) (npoints * sizeof(double)));
    z_sum = (double *) umalloc ((ui32) (npoints * sizeof(double)));
    nhits = (double *) umalloc ((ui32) (npoints * sizeof(double)));
    npossible = (double *) umalloc ((ui32) (npoints * sizeof(double)));
    xindex = (si32 *) umalloc ((ui32) (_wsiHeader.image_pixels * sizeof(si32)));
    
    /*
     * load up x index array
     */
    
    lon = (_wsiHeader.center_lon - _wsiHeader.diff_lon) +
      _wsiHeader.deg_per_element * 0.5;
    for (ilon = 0; ilon < _wsiHeader.image_pixels;
	 ilon++, lon += _wsiHeader.deg_per_element)
    {
      ix = (si32) ((lon - _outputMinX) / _outputDeltaX + 0.5);
      if (ix >= 0 && ix < _outputNX)
      {
	xindex[ilon] = ix;
      }
      else
      {
	xindex[ilon] = -1;
      }

    } /* ilon */

    /*
     * Load up the Z table
     */

    for (i = 0; i < 255; i++)
    {
      z_table[i] = pow(10.0,
		       (((double)i * _dataScale) + _dataBias) / 10.0);
    } /* endfor - i */
    
    first_call = FALSE;
  }
  
  /*
   * initialize
   */
  
  memset((void *) data_grid, 0, (int) (npoints * sizeof(ui08)));
  memset((void *) sum, 0, (int) (npoints * sizeof(double)));
  memset((void *) z_sum, 0, (int) (npoints * sizeof(double)));
  memset((void *) nhits, 0, (int) (npoints * sizeof(double)));
  memset((void *) npossible, 0, (int) (npoints * sizeof(double)));

  /*
   * Unpack data lines, and accumulate stats.
   * Data starts at bottom of grid, and works up.
   */
  
  lat = _wsiHeader.top_lat + (_wsiHeader.deg_per_line / 2.0);
  
  for (ilat = 0; ilat < _wsiHeader.image_lines;
       ilat++, lat -= _wsiHeader.deg_per_line)
  {
    /*
     * compute y index
     */
    
    iy = (si32) ((lat - _outputMinY) / _outputDeltaY + 0.5);
    start_index = iy * _outputNX;
    
    /*
     * unpack line
     */

    line = _imageData +
      ((_wsiHeader.image_lines - ilat) * _wsiHeader.image_pixels);
    
    if (line == NULL)
    {
      fprintf(stderr, "ERROR - null line %d\n", i);
    }
    else if (iy >= 0 && iy < _outputNY)
    {
      /*
       * accumulate the grid data stats
       */

      for (ilon = 0; ilon < _wsiHeader.image_pixels; ilon++)
      {
	if (xindex[ilon] >= 0)
	{
	  index = start_index + xindex[ilon];
	  if (line[ilon] > 0)
	  {
	    switch (_filterType)
	    {
	    case WSIM_FILE_FILTER_MAX :
	      data_grid[index] = MAX(data_grid[index], line[ilon]);
	      break;

	    case WSIM_FILE_FILTER_MEAN_DBZ :
	      sum[index] += (double) line[ilon];
	      break;
	      
	    case WSIM_FILE_FILTER_MEAN_Z :
	      z_sum[index] += z_table[line[ilon]];
	      break;
	      
	    } /* endswitch - _filterType */
	    nhits[index] += 1.0;
	  }
	  npossible[index] += 1.0;
	}
      } /* ilon */
	   
    } /* if (iy >= 0 && iy < _outputNY) */
    
  } /* ilat */

  /*
   * load up data grid array
   */

  switch (_filterType)
  {
  case WSIM_FILE_FILTER_MAX :

    dg = data_grid;
    nh = nhits;
    np = npossible;
    
    for (i = 0; i < npoints; i++, dg++, nh++, np++)
    {
      fraction_covered = *nh / *np;
      if (fraction_covered < _coverageThreshold)
      {
	*dg = 0;
      }
    } /* i */

    break;

  case WSIM_FILE_FILTER_MEAN_DBZ :

    dg = data_grid;
    su = sum;
    nh = nhits;
    np = npossible;
    
    for (i = 0; i < npoints; i++, dg++, su++, nh++, np++)
    {
      fraction_covered = *nh / *np;
      
      if (fraction_covered >= _coverageThreshold)
      {
	mean_val = (si32) (*su / *nh + 0.5);
	if (mean_val < 0)
	{
	  mean_val = 0;
	}
	else if (mean_val > 255)
	{
	  mean_val = 255;
	}
	*dg = mean_val;
	
      } /* if (fraction_covered >= _coverageThreshold) */
      
    } /* i */

    break;

  case WSIM_FILE_FILTER_MEAN_Z :

    dg = data_grid;
    su = z_sum;
    nh = nhits;
    np = npossible;
    
    for (i = 0; i < npoints; i++, dg++, su++, nh++, np++)
    {
      fraction_covered = *nh / *np;
      
      if (fraction_covered >= _coverageThreshold)
      {
	mean_val =
	  (si32) (((10.0 * log10(*su / *nh) - _dataBias) / _dataScale) + 0.5);
	if (mean_val < 0)
	{
	  mean_val = 0;
	}
	else if (mean_val > 255)
	{
	  mean_val = 255;
	}
	*dg = mean_val;
	
      } /* if (fraction_covered >= _coverageThreshold) */
      
    } /* i */

    break;

  default:
    break;

  } /* switch */

  return(data_grid);
}


/************************************************************
 * _loadLCGrid()
 */

ui08 *WsimFile::_loadLCGrid(void)
{
  ui08 *return_buffer;
  
  int image_size = _outputNX * _outputNY;
    
  return_buffer = (ui08 *)umalloc(image_size);
  memset(return_buffer, 0, image_size);
    
  for (int y = 0; y < _outputNY; y++)
  {
    for (int x = 0; x < _outputNX; x++)
    {
      double lon = _outputMinX + (x * _outputDeltaX);
      double lat = _outputMinY + (y * _outputDeltaY);
	
      double lc_x, lc_y;
	
      PJGlc2_latlon2xy(lat, lon, &lc_x, &lc_y);
	
      int lc_x_index =
	(int)(((lc_x - _wsiHeader.upper_left_x) /
	       _wsiHeader.pixel_res_x) + 0.5);
      int lc_y_index = 
	(int)(((lc_y - (_wsiHeader.upper_left_y -
			((_wsiHeader.image_lines - 1) *
			 _wsiHeader.pixel_res_y))) /
	       _wsiHeader.pixel_res_y) + 0.5);
	
      if (lc_x_index >= 0 && lc_x_index < _wsiHeader.image_pixels &&
	  lc_y_index >= 0 && lc_y_index < _wsiHeader.image_lines)
      {
	int output_index = (y * _outputNX) + x;
	int image_index =
	  (lc_y_index * _wsiHeader.image_pixels) + lc_x_index;
	  
	return_buffer[output_index] =
	  _imageData[image_index];
      }
	
    }  // endfor - x
      
  }  // endfor - y
    
  return(return_buffer);
}


/************************************************************
 * _readHeader() - Read the WSI header information from the file.
 *
 * From "EXPRESS NOWradPLUS SERVICE TECHNICAL DESCRIPTION",
 * Jan. 1992:
 *
 * 0x09  Indicates that the data in the segment is the broadcast
 *       image header.  The segment begins with a binary count of
 *       the number of characters in the header (not including the
 *       flag sequence, the control byte or the count itself).
 *       This is followed by ASCII characters which make up fields.
 *       The fields are separated by 1 or more space characters.
 *       The first field is 1 to 4 characters long, indicating the
 *       coverage of the image, such as "NA" or "US".  The second
 *       field is the time of the image in the format "hh:mm" (where
 *       hh is the 2 digit hour, and mm is the 2 digit minute, all
 *       times are in GMT).  The third field is 1 to 8 characters
 *       long which contains the type of data in the image such as
 *       "NOWRADHD".  Note the specific values of the coverage and
 *       data type codes will be detailed in the section "Products".
 *       Some products contains a "mark" byte after the data type
 *       name.  This is a single byte of the values 128-130 (0x80 -
 *       0x82 hex).  For most applications this byte can be ignored.
 *
 * [Note that there is no mention in this part of the documentation
 * of the start of image byte.  This byte is shown in the example in
 * the document, but I never saw where it was mentioned in the
 * description.  I just assume that it always follows the header.
 * This could possibly be a problem. -nr]
 */

ui08 *WsimFile::_readHeader(ui08 *header_ptr)
{
  ui08 *byte_in = header_ptr;
  int  header_size;
  
  // Get the header size.  The header size is the number of bytes
  // in the header itself.  It doesn't include the size byte or the
  // following mark byte or start of image byte.

  header_size = *byte_in;
  byte_in++;

  if (_debugFlag)
    fprintf(stderr, "   header_size = %d\n", header_size);
  
  _wsiHeader.image_header = (char *)umalloc(header_size+1);
  memcpy((void *)_wsiHeader.image_header, (void *)byte_in, header_size);
  _wsiHeader.image_header[header_size] = '\0';
  byte_in += header_size;

  if (_debugFlag)
    fprintf(stderr, "header = <%s>\n", _wsiHeader.image_header);
  
  // Check for a mark byte.

  if (*byte_in >= WSI_MARK_MIN &&
      *byte_in <= WSI_MARK_MAX)
  {
    _wsiHeader.mark = *byte_in;
    byte_in++;

    if (_debugFlag)
      fprintf(stderr, "Mark byte found: %d\n", _wsiHeader.mark);
  }
  else
    _wsiHeader.mark = 0;
      
  // Get the start of image byte.  I can't really tell from the documentation
  // if this byte is always here, but I'm going to make that assumption
  // for now.  This byte also indicates the resolution of the data.  We
  // don't use this information yet, but save it just in case.

  _wsiHeader.resolution_byte = *byte_in;
  byte_in++;

  if (_debugFlag)
    fprintf(stderr, "Resolution byte: %d\n", _wsiHeader.resolution_byte);
  
  // Print a debug message

  if (_debugFlag)
    fprintf(stderr, "WSI_HEADER_FLAG found\n");
      
  return(byte_in);
}


/************************************************************
 * _readImageLine() - Read an image line from the file.
 *
 * From "EXPRESS NOWradPLUS SERVICE TECHNICAL DESCRIPTION",
 * Jan. 1992:
 *
 * 0x0C  Indicates that the data in the segment is an image line.
 *       It is followed by 2 bytes to indicate the line number
 *       (least significant first).  The rest of the line is run
 *       length encoded.  This is further described in the section
 *       "Run Length Encoding".  [This section of the documentation
 *       is included in the header for the _addLineToBuffer()
 *       member function. -nr]
 */

ui08 *WsimFile::_readImageLine(ui08 *image_line_ptr)
{
  ui08 *byte_in = image_line_ptr;
  ui16  line_num;
      
  // The first two bytes are the line number.  This is stored in
  // little-endian format.

  memcpy(&line_num, byte_in, 2);
  line_num = SE_to_ui16(line_num);
  byte_in += 2;

  // Make sure the line number is valid

  if (line_num < _lastLine + 1)
  {
    fprintf(stderr,
	    "Line number %d encountered after line %d, skipping line\n",
	    line_num, _lastLine);
    while (!(*byte_in == WSI_FLAG1 &&
	     *(byte_in+1) == WSI_FLAG2 &&
	     *(byte_in+2) != WSI_BIN_FLAG))
      byte_in++;
  }
  else
  {
    byte_in = _addLineToBuffer(byte_in, line_num);
    _lastLine = line_num;
  }

  // Print a debug message

//  if (_debugFlag)
//    fprintf(stderr, "WSI_IMAGE_LINE_FLAG found\n");
      
  return(byte_in);
}


/************************************************************
 * _readImageSize() - Read the image size information from the file.
 *
 * From "EXPRESS NOWradPLUS SERVICE TECHNICAL DESCRIPTION",
 * Jan. 1992:
 *
 * 0x0A  Indicates that the data in the segment is the image size.
 *       This is the number of lines and elements (pixels) per line
 *       in the images.  It is formatted with ASCII characters.
 *       The number of lines is first, followed by a space character,
 *       followed by the number of elements per line.
 */

ui08 *WsimFile::_readImageSize(ui08 *image_size_ptr)
{
  ui08 *byte_in = image_size_ptr;
  
  // Retrieve the number of lines in the image.  This is an
  // ASCII string followed by a space character.

  _wsiHeader.image_lines = atoi((char *)byte_in);

  while (*byte_in != ' ')
    byte_in++;

  // Retrieve the number of pixels per line in the image.  This
  // is also an ASCII string.

  _wsiHeader.image_pixels = atoi((char *)byte_in);

  // Find the next control information in the file.  This is the
  // only way to find the end of the number of pixels value.
  
  while (!(*byte_in == WSI_FLAG1 &&
	   *(byte_in+1) == WSI_FLAG2 &&
	   *(byte_in+2) != WSI_BIN_FLAG))
    byte_in++;

  // Make sure we have valid image size info

  if (_wsiHeader.image_lines > 0 &&
      _wsiHeader.image_pixels > 0)
    _validImageSizeInfo = TRUE;
  
  // Allocate space for the image

  if (_imageData != (ui08 *)NULL)
  {
    ufree(_imageData);
    _imageData = (ui08 *)NULL;
  }
  
  _imageData = (ui08 *)umalloc(_wsiHeader.image_lines *
			       _wsiHeader.image_pixels);
  memset(_imageData, 0, _wsiHeader.image_lines * _wsiHeader.image_pixels);
  
  // Print out a debug message

  if (_debugFlag)
    fprintf(stderr,
	    "WSI_IMAGE_SIZE_FLAG found: image_lines = %d, image_pixels = %d\n",
	    _wsiHeader.image_lines,
	    _wsiHeader.image_pixels);
      
  // Return the new file pointer

  return(byte_in);
}


/************************************************************
 * _readLabel() - Read the label information from the file.
 *
 * From "EXPRESS NOWradPLUS SERVICE TECHNICAL DESCRIPTION",
 * Jan. 1992:
 *
 * 0x03  Indicates that the data in the segment is an ASCII string
 *       of 80 [IMAGE_LABEL_LEN -nr] characters containing the
 *       label for the image product.
 */

ui08 *WsimFile::_readLabel(ui08 *label_ptr)
{
  ui08 *byte_in = label_ptr;
  
  // Copy the label into the header.  Note that the header contains
  // IMAGE_LABEL_LEN + 1 bytes for the label so there isn't an off-
  // by-one error below.

  memcpy((void *)_wsiHeader.image_label, 
	 (void *)byte_in, IMAGE_LABEL_LEN);
  _wsiHeader.image_label[IMAGE_LABEL_LEN] = '\0';
  byte_in += IMAGE_LABEL_LEN;
  
  // Print a debug message

  if (_debugFlag)
    fprintf(stderr, "WSI_LABEL_FLAG found: <%s>\n", _wsiHeader.image_label);

  return(byte_in);
}


/************************************************************
 * _readProjection() - Read the projection information from the file.
 *
 * From "EXPRESS NOWradPLUS SERVICE TECHNICAL DESCRIPTION",
 * Jan. 1992:
 *
 * 0x0B  Indicates that the data segment contains projections and
 *       navigational information.  This is described in detail in
 *       the section "Navigational Information". [given below -nr]
 *
 * Navigational Information
 *
 * This section describes the navigational information which appears
 * in the navigational segment of a graphics image.  All navigational
 * parameters are described in "radians".  Scientific notation is
 * used for floating point numbers.
 *
 * Master Sector
 * -------------
 * For this image there are 6 parameters in the header.
 *
 *     Navigational Code Letter: (C) Cylindrical Equidistant
 *     Center Longitude: (-1.658063e+000) This is the center longitude
 *         of the image.
 *     Top Latitude: (9.250565e-001) This is the northern most
 *         parallel of the image.
 *     Difference Longitude: (-6.107335e-001) This is the difference in
 *         the longitude from the center of the image to the edge
 *         meridians.
 *     Radians/Line: (3.135713e-004) This is the number of radians
 *         per line of the image.
 *     Radians/Element: (3.336956e-004) This is the number of radians
 *         per element of the image.
 *
 * Additionally the image size is important to the navigation.  For
 * this there are 2 parameters in the header.
 *
 *     Lines: (1837) This is the number of lines in the image.
 *     Elements: (3661) This is the number of elements per line.
 *
 * National Mosaic
 * ---------------
 * For this image there are 9 parameters in the header.
 *
 *     Navigational Code Letter: (L) Lambert Conformal
 *     Parallel 1: (2.000000e001) This is the 1st reference parallel.
 *     Parallel 2: (4.400000e001) This is the 2nd reference parallel.
 *     Projection Center: (4.4000000e001 -9.5000000e001) This is the
 *         center of the projection.  This is not the image center.
 *     Upper Left Coordinate: (2.31949e-001 -4.49378e-001) The exact
 *         meaning of this value is not known. [direct quote from
 *         document -nr]
 *     Pixel Resolution: (1.2459e-003 1.13793e-003) The exact meaning
 *         of this value is not known. [direct quote from document -nr]
 *
 * Additionally the image size is important to the navigation.  For
 * this there are 2 parameters in the header.
 *
 *     Lines: (480) This is the number of lines in the image.
 *     Elements: (768) This is the number of elements per line.
 *
 * Note: Navigational information will be sent with each image.
 * Since this information could change, the above values should not
 * be assumed.
 */

ui08 *WsimFile::_readProjection(ui08 *projection_ptr)
{
  ui08 *byte_in = projection_ptr;
      
  // Print out a debugging message

  if (_debugFlag)
    fprintf(stderr, "WSI_PROJ_NAV_FLAG found: <%s>\n",
	    (char *)(byte_in+1));
      
  // Get the navigation code from the header.  If this is a 'C',
  // the data is in a Cylindrical Equidistant projection.  If this
  // is an 'L', the data is in a Lambert Conformal projection.

  _wsiHeader.nav_code = *byte_in;
  byte_in++;

  if (_wsiHeader.nav_code == 'C')
  {
    // Read in the center longitude.  This is an ASCII string followed
    // by a space character.  The value is given in radians.  This is
    // the center longitude of the image.

    _wsiHeader.center_lon = atof((char *)byte_in) * RAD_TO_DEG;
    byte_in++;
    while (*byte_in != ' ')
      byte_in++;

    // Read in the top latitude.  This is an ASCII string followed
    // by a space character.  The value is given in radians.  This is
    // the northern most parallel of the image.

    _wsiHeader.top_lat = atof((char *)byte_in) * RAD_TO_DEG;
    byte_in++;
    while (*byte_in != ' ')
      byte_in++;

    // Read in the longitude difference value.  This is an ASCII string
    // followed by a space character.  The value is given in radians.
    // This is the difference in the longitude from the center of the
    // image to the edge meridans.

    _wsiHeader.diff_lon = atof((char *)byte_in) * RAD_TO_DEG;
    if (_wsiHeader.diff_lon < 0)
      _wsiHeader.diff_lon = -_wsiHeader.diff_lon;
    
    byte_in++;
    while (*byte_in != ' ')
      byte_in++;

    // Read in the radians per line.  This is an ASCII string followed
    // by a space character.  This is the number of radians per line
    // of the image.

    _wsiHeader.deg_per_line = atof((char *)byte_in) * RAD_TO_DEG;
    byte_in++;
    while (*byte_in != ' ')
      byte_in++;

    // Read in the radians per element.  This is a ASCII string.  Look
    // for the control information for the next field to end this field.
    // This is the number of radians per element of the image.

    _wsiHeader.deg_per_element = atof((char *)byte_in) * RAD_TO_DEG;
    while (!(*byte_in == WSI_FLAG1 &&
	     *(byte_in+1) == WSI_FLAG2 &&
	     *(byte_in+2) != WSI_BIN_FLAG))
      byte_in++;

    // Show the the current navigational information is valid

    _validNavInfo = TRUE;
  }
  else if (_wsiHeader.nav_code == 'L')
  {
    // Read in the first parallel.  This is an ASCII string followed
    // by a space character.  The value is given in radians.  This is
    // the first reference parallel for the projection.

    _wsiHeader.parallel_1 = atof((char *)byte_in) * RAD_TO_DEG;
    byte_in++;
    while (*byte_in != ' ')
      byte_in++;

    // Read in the second parallel.  This is an ASCII string followed
    // by a space character.  The value is given in radians.  This is
    // the second reference parallel for the projection.

    _wsiHeader.parallel_2 = atof((char *)byte_in) * RAD_TO_DEG;
    byte_in++;
    while (*byte_in != ' ')
      byte_in++;

    // Read in the projection center latitude.  This is an ASCII string
    // followed by a space character.  The value is given in radians.
    // This is the latitude of the center of the projection.  This is
    // not the image center.

    _wsiHeader.proj_center_lat = atof((char *)byte_in) * RAD_TO_DEG;
    byte_in++;
    while (*byte_in != ' ')
      byte_in++;

    // Read in the projection center longitude.  This is an ASCII string
    // followed by a space character.  The value is given in radians.
    // This is the longitude of the center of the projection.  This is
    // not the image center.

    _wsiHeader.proj_center_lon = atof((char *)byte_in) * RAD_TO_DEG;
    byte_in++;
    while (*byte_in != ' ')
      byte_in++;

    // Read in the upper left latitude.  This is an ASCII string
    // followed by a space character.
    // This is the ???

    _wsiHeader.upper_left_y = atof((char *)byte_in) * EARTH_RADIUS;
    
    byte_in++;
    while (*byte_in != ' ')
      byte_in++;

    // Read in the upper left longitude.  This is an ASCII string
    // followed by a space character.
    // This is the ???

    _wsiHeader.upper_left_x = atof((char *)byte_in) * EARTH_RADIUS;
    
    byte_in++;
    while (*byte_in != ' ')
      byte_in++;

    // Read in the pixel resolution latitude.  This is an ASCII string
    // followed by a space character.
    // This is the ???

    _wsiHeader.pixel_res_y = atof((char *)byte_in) * EARTH_RADIUS;
    
    byte_in++;
    while (*byte_in != ' ')
      byte_in++;

    // Read in the pixel resolution longitude.  This is an ASCII string.
    // Look for the control information for the next field to end this field.
    // This is the ???

    _wsiHeader.pixel_res_x = atof((char *)byte_in) * EARTH_RADIUS;

    while (!(*byte_in == WSI_FLAG1 &&
	     *(byte_in+1) == WSI_FLAG2 &&
	     *(byte_in+2) != WSI_BIN_FLAG))
      byte_in++;

    // Show the the current navigational information is valid

    _validNavInfo = TRUE;

    // Initialize the projection values

    if (!PJGlc2_init(_wsiHeader.proj_center_lat,
		     _wsiHeader.proj_center_lon,
		     _wsiHeader.parallel_1,
		     _wsiHeader.parallel_2))
    {
      fprintf(stderr, "PJGlc2_init() failed\n");
      exit(-1);
    }
    
  }
  else
  {
    fprintf(stderr, "Don't know how to process nav_code %c data -- skipping\n",
	    _wsiHeader.nav_code);
    _validNavInfo = FALSE;
  }
  
  if (_debugFlag)
    printHeader(stderr);
  
  return(byte_in);
}


/************************************************************
 * _readStatus() - Read the status information from the file.
 *
 * From "EXPRESS NOWradPLUS SERVICE TECHNICAL DESCRIPTION",
 * Jan. 1992:
 *
 * 0x06  Indicates that the data in the segment is an ASCII string
 *       containing a status or informational message about the
 *       broadcast service.
 */

ui08 *WsimFile::_readStatus(ui08 *status_ptr)
{
  ui08 *byte_in = status_ptr;
      
  // Skip the status message -- we don't use it.  Look for the flag
  // indicating the next control information to find the end of the
  // status field.

  while(!(*byte_in == WSI_FLAG1 &&
	  *(byte_in+1) == WSI_FLAG2 &&
	  *(byte_in+2) != WSI_BIN_FLAG))
    byte_in++;

  // Print a debugging message

  if (_debugFlag)
    fprintf(stderr, "Skipping broadcast status message\n");

  return(byte_in);
}


/************************************************************
 * _setImageValue()
 */

void WsimFile::_setImageValue(int line, int pixel, ui08 color)
{
  int index;
    
  if (!_validImageSizeInfo ||
      line < 0 || line >= _wsiHeader.image_lines ||
      pixel < 0 || pixel >= _wsiHeader.image_pixels)
    return;
    
  index = ((_wsiHeader.image_lines - 1 - line) *
	   _wsiHeader.image_pixels) + pixel;
    
  if (color < 16)
    _imageData[index] = _valueTable[color];
  else
    _imageData[index] = 0;
}
  
