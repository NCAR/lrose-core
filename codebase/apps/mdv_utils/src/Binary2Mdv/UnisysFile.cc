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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/04 02:22:09 $
//   $Id: UnisysFile.cc,v 1.2 2016/03/04 02:22:09 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * UnisysFile: Class for operating on a Unisys mosaic file.
 *
 * RAP, NCAR, Boulder CO
 *
 * September 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <dataport/bigend.h>
#include <toolsa/pjg.h>
#include <toolsa/str.h>

#include "UnisysFile.hh"

using namespace std;

// Global variables

const ui08 UnisysFile::UNISYS_COLOR_MASK = 0x0F;   // 4 bit 1s mask
const size_t UnisysFile::NUM_HEADER_BYTES = 158;


/*********************************************************************
 * Constructor
 */

UnisysFile::UnisysFile() :
  _imageData(0)
{
}


/*********************************************************************
 * Destructor
 */

UnisysFile::~UnisysFile()
{
  delete [] _imageData;
}


/*********************************************************************
 * init() - Initialize the object.  This must be called before the object
 *          is used.
 *
 * Returns true on success, false on failure.
 */

bool UnisysFile::init(const double input_parallel_1,
		      const double input_parallel_2,
		      const int valid_source_id,
		      const string &data_field_name_long,
		      const string &data_field_name,
		      const string &data_units,
		      const double data_scale,
		      const double data_bias,
		      const double missing_data_value,
		      const Pjg &output_projection,
		      const bool debug_flag)
{
  // Set the interal members

  _debug = debug_flag;
  
  _inputParallel1 = input_parallel_1;
  _inputParallel2 = input_parallel_2;
  
  _validSourceId = valid_source_id;

  _dataFieldNameLong = data_field_name_long;
  _dataFieldName = data_field_name;
  _dataUnits = data_units;
  
  _dataScale = data_scale;
  _dataBias = data_bias;
  _missingDataValue = missing_data_value;
  
  _outputProjection = output_projection;

  return true;
}


/*********************************************************************
 * createMdvxField() - Create an MdvxField object from the file information.
 *
 * Returns a pointer to the new MdvxField object if successful,
 * 0 otherwise.
 */

MdvxField *UnisysFile::createMdvxField(const DateTime &data_time)
{
  double unisysPixelRes;

  // The following relates spatial resolution to columns and rows
  // for national mosaic products:
  //          2 km x  2 km    2560 columns x 1792 rows
  //          4 km x  4 km    1280 columns x  896 rows
  //          8 km x  8 km     640 columns x  448 rows
  //         16 km x 16 km     320 columns x  224 rows
  //         32 km x 32 km     160 columns x  112 rows

  if (_unisysHeader.NumberOfColumns == 2560)
    unisysPixelRes = 2.0;
  else if (_unisysHeader.NumberOfColumns == 1280)
    unisysPixelRes = 4.0;
  else if (_unisysHeader.NumberOfColumns == 640)
    unisysPixelRes = 8.0;
  else if (_unisysHeader.NumberOfColumns == 320)
    unisysPixelRes = 16.0;
  else if (_unisysHeader.NumberOfColumns == 160)
    unisysPixelRes = 32.0;
  else
  {
    cerr << "Error - Unknown Unisys mosaic type for"
	 << _unisysHeader.NumberOfColumns << "data columns" << endl;
    
    return 0;
  }

  // Set up the field header

  Mdvx::field_header_t field_hdr;
  
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  STRcopy(field_hdr.field_name_long, _dataFieldNameLong.c_str(),
          MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, _dataFieldName.c_str(), MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, _dataUnits.c_str(), MDV_UNITS_LEN);

  field_hdr.field_code = 0;
  field_hdr.forecast_time = data_time.utime();

  field_hdr.nx = _unisysHeader.NumberOfColumns;
  field_hdr.ny = _unisysHeader.NumberOfRows;
  field_hdr.nz = 1;

  field_hdr.proj_type = Mdvx::PROJ_LAMBERT_CONF;
  field_hdr.encoding_type = Mdvx::ENCODING_INT8;
  field_hdr.data_element_nbytes = 1;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz *
    field_hdr.data_element_nbytes;

  field_hdr.proj_origin_lat = _unisysHeader.CenterLatitude / 1000.0;
  field_hdr.proj_origin_lon = _unisysHeader.CenterLongitude / 1000.0;
  field_hdr.proj_param[0] = _inputParallel1;
  field_hdr.proj_param[1] = _inputParallel2;
  
  field_hdr.grid_dx = unisysPixelRes;
  field_hdr.grid_dy = unisysPixelRes;
  field_hdr.grid_dz = 1.0;

  field_hdr.grid_minx = -((double)_unisysHeader.NumberOfColumns * unisysPixelRes / 2.0) +
    (0.5 * unisysPixelRes);
  field_hdr.grid_miny = -((double)_unisysHeader.NumberOfRows * unisysPixelRes / 2.0) +
    (0.5 * unisysPixelRes);
  field_hdr.grid_minz = 0.5;

  field_hdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
  field_hdr.vlevel_type = Mdvx::VERT_TYPE_Z;

  field_hdr.scale = _dataScale;
  field_hdr.bias = _dataBias;
  field_hdr.bad_data_value = _missingDataValue;
  field_hdr.missing_data_value = _missingDataValue;

  field_hdr.proj_rotation = 0.0;

  // Set up vlevel header

  Mdvx::vlevel_header_t vlevel_hdr;
  
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
  
  vlevel_hdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vlevel_hdr.level[0] = field_hdr.grid_minz;

  return new MdvxField(field_hdr, vlevel_hdr,
		       _imageData);
}


/*********************************************************************
 * printHeader() - Prints the Unisys mosiac header to stderr.
 */

void UnisysFile::printHeader()
{
  // Print the header information

  cerr << endl;
  cerr << "Product ID:              " << _unisysHeader.ProductID << endl;
  cerr << "Days since 1970:         " << _unisysHeader.DaysSince1970 << endl;
  cerr << "Secs since midnight:     " << _unisysHeader.SecsSinceMidnight << endl;
  cerr << "Total length (bytes):    " << _unisysHeader.TotalLengthBytes << endl;
  cerr << "Source ID:               " << _unisysHeader.SourceID << endl;
  cerr << "Destination ID:          " << _unisysHeader.DestinationID << endl;
  cerr << "Number of blocks:        " << _unisysHeader.NumberOfBlocks << endl;
  cerr << "Block Divider:           " << _unisysHeader.BlockDivider1 << endl;
  cerr << "Center Latitude:         " << _unisysHeader.CenterLatitude << endl;
  cerr << "Center Longitude:        " << _unisysHeader.CenterLongitude << endl;
  cerr << "Height of Radar:         " << _unisysHeader.HeightOfRadar << endl;
  cerr << "Product ID #2:           " << _unisysHeader.ProductID2 << endl;
  cerr << "Operational mode:        " << _unisysHeader.OperationalMode << endl;
  cerr << "Volume Coverage Pattern: " << _unisysHeader.VolumeCoveragePattern << endl;
  cerr << "Sequence Number:         " << _unisysHeader.SequenceNumber << endl;
  cerr << "Volume Scan Number:      " << _unisysHeader.VolumeScanNumber << endl;
  cerr << "Date_2:                  " << _unisysHeader.Date_2 << endl;
  cerr << "Time_2:                  " << _unisysHeader.Time_2 << endl;
  cerr << "Date_3:                  " << _unisysHeader.Date_3 << endl;
  cerr << "Time_3:                  " << _unisysHeader.Time_3 << endl;
  cerr << "Product Dependent #1:    " << _unisysHeader.ProductDependent1 << endl;
  cerr << "Product Dependent #2:    " << _unisysHeader.ProductDependent2 << endl;
  cerr << "Elevation Number:        " << _unisysHeader.ElevationNumber << endl;
  cerr << "Product Dependent #3:    " << _unisysHeader.ProductDependent3 << endl;
  cerr << "Data Level  1:           " << _unisysHeader.DataLevel1 << endl;
  cerr << "Data Level  2:           " << _unisysHeader.DataLevel2 << endl;
  cerr << "Data Level  3:           " << _unisysHeader.DataLevel3 << endl;
  cerr << "Data Level  4:           " << _unisysHeader.DataLevel4 << endl;
  cerr << "Data Level  5:           " << _unisysHeader.DataLevel5 << endl;
  cerr << "Data Level  6:           " << _unisysHeader.DataLevel6 << endl;
  cerr << "Data Level  7:           " << _unisysHeader.DataLevel7 << endl;
  cerr << "Data Level  8:           " << _unisysHeader.DataLevel8 << endl;
  cerr << "Data Level  9:           " << _unisysHeader.DataLevel9 << endl;
  cerr << "Data Level 10:           " << _unisysHeader.DataLevel10 << endl;
  cerr << "Data Level 11:           " << _unisysHeader.DataLevel11 << endl;
  cerr << "Data Level 12:           " << _unisysHeader.DataLevel12 << endl;
  cerr << "Data Level 13:           " << _unisysHeader.DataLevel13 << endl;
  cerr << "Data Level 14:           " << _unisysHeader.DataLevel14 << endl;
  cerr << "Data Level 15:           " << _unisysHeader.DataLevel15 << endl;
  cerr << "Data Level 16:           " << _unisysHeader.DataLevel16 << endl;
  cerr << "Maximum Data Level:      " << _unisysHeader.MaximumDataLevel << endl;
  cerr << "Product Dependent #4:    " << _unisysHeader.ProductDependent4 << endl;
  cerr << "Product Dependent #5:    " << _unisysHeader.ProductDependent5 << endl;
  cerr << "Product Dependent #6:    " << _unisysHeader.ProductDependent6 << endl;
  cerr << "Product Dependent #7:    " << _unisysHeader.ProductDependent7 << endl;
  cerr << "Product Dependent #8:    " << _unisysHeader.ProductDependent8 << endl;
  cerr << "Number of Columns:       " << _unisysHeader.NumberOfColumns << endl;
  cerr << "Number of Maps:          " << _unisysHeader.NumberOfMaps << endl;
  cerr << "Offset to Symbology:     " << _unisysHeader.OffsetToSymbology << endl;
  cerr << "Offset to Graphic:       " << _unisysHeader.OffsetToGraphic << endl;
  cerr << "Offset to Tabular:       " << _unisysHeader.OffsetToTabular << endl;
  cerr << "Block Divider #2:        " << _unisysHeader.BlockDivider2 << endl;
  cerr << "BlockID:                 " << _unisysHeader.BlockID << endl;
  cerr << "Length of (Symbol)Block: " << _unisysHeader.LengthOfBLock <<endl;
  cerr << "Number of Layers:        " << _unisysHeader.NumberOfLayers << endl;
  cerr << "Layer Divider #1:        " << _unisysHeader.LayerDivider1 << endl;
  cerr << "Length of Raster Layer:  " << _unisysHeader.LengthOfRasterLayer << endl;
  cerr << "Raster Opcode:           " << _unisysHeader.RasterOpCode << endl;
  cerr << "Raster Constant #1:      " << _unisysHeader.RasterConstant1 << endl;
  cerr << "Raster Constant #2:      " << _unisysHeader.RasterConstant2 << endl;
  cerr << "X Origin:                " << _unisysHeader.XOrigin << endl;
  cerr << "Y Origin:                " << _unisysHeader.YOrigin << endl;
  cerr << "X Scale (integer):       " << _unisysHeader.XScaleInt << endl;
  cerr << "X Scale (fractional):    " << _unisysHeader.XScaleFrac << endl;
  cerr << "Y Scale (integer):       " << _unisysHeader.YScaleInt << endl;
  cerr << "Y Scale (fractional):    " << _unisysHeader.YScaleFrac << endl;
  cerr << "Number of Rows:          " << _unisysHeader.NumberOfRows << endl;
  cerr << "Packing Descriptor:      " << _unisysHeader.PackingDescriptor << endl;

  cerr << endl;
}


/*********************************************************************
 * readFile() - Reads the given Unisys mosiac file into the internal
 *              buffer.
 *
 * Returns true on success, false on failure.
 */

bool UnisysFile::readFile(const string &filename)
{
  static const string method_name = "UnisysFile::readUnisys()";
  
  if (_debug)
    cerr << endl << "Processing input file: " << filename << endl;

  // Stat the input file to make sure it exists and to get the file
  // size for allocating the input buffer below

  struct stat file_stat;
  
  if (stat(filename.c_str(), &file_stat) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting system status (stat) for input file: " << filename << endl;
    
    return false;
  }
  
  // Open the input file

  FILE *input_file;
  if ((input_file = fopen(filename.c_str(), "r")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening input file: " << input_file << endl;
    
    return false;
  }
  
  // Read the header information

  if (!_readHeader(input_file))
    return false;
  
  printHeader();
  
  // Check for valid Source ID

  if (_unisysHeader.SourceID != _validSourceId)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Source_ID " << _unisysHeader.SourceID << " in file <" << filename << ">"
	 << "does not match valid_source_id of " << _validSourceId
	 << " in parameter file" << endl;

    return false;
  }

  // Create the data buffer

  int data_buffer_alloc = file_stat.st_size - NUM_HEADER_BYTES;
  ui08 *data_buffer = new ui08[data_buffer_alloc];

  // Read the data from the input file

  int bytes_read;

  if ((bytes_read = fread((void *)data_buffer, sizeof(ui08),
			  data_buffer_alloc, input_file)) != data_buffer_alloc)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading input file" << endl;
    cerr << "Expected " << data_buffer_alloc << "bytes, got "
	 << bytes_read << " bytes" << endl;
    
    return false;
  }

  if (_debug)
    cerr << "Number of bytes read: " << bytes_read << endl << endl;

  fclose(input_file);

  // Load up the Unisys valueTable (in real world data units)

  double unisys_value_table[16];
  
  unisys_value_table[0] = _missingDataValue;
  unisys_value_table[1] = _unisysHeader.DataLevel2;
  unisys_value_table[2] = _unisysHeader.DataLevel3;
  unisys_value_table[3] = _unisysHeader.DataLevel4;
  unisys_value_table[4] = _unisysHeader.DataLevel5;
  unisys_value_table[5] = _unisysHeader.DataLevel6;
  unisys_value_table[6] = _unisysHeader.DataLevel7;
  unisys_value_table[7] = _unisysHeader.DataLevel8;
  unisys_value_table[8] = _unisysHeader.DataLevel9;
  unisys_value_table[9] = _unisysHeader.DataLevel10;
  unisys_value_table[10] = _unisysHeader.DataLevel11;
  unisys_value_table[11] = _unisysHeader.DataLevel12;
  unisys_value_table[12] = _unisysHeader.DataLevel13;
  unisys_value_table[13] = _unisysHeader.DataLevel14;
  unisys_value_table[14] = _unisysHeader.DataLevel15;
  unisys_value_table[15] = _unisysHeader.DataLevel16;

  // Load up the MDV valueTable (in 0-255 range) using scale and bias

  for (int i = 0; i < 16; i ++)
  {
    _valueTableMDV[i] =
      (int)(((unisys_value_table[i] - _dataBias) / _dataScale) + 0.5);

//  if (_valueTableMDV[i] < 0)
//    _valueTableMDV[i] = 0;

    if (_valueTableMDV[i] > 255)
      _valueTableMDV[i] = 255;

    if (_debug)
      cerr << "_valueTableMDV[" << i << "] = " << (int)_valueTableMDV[i] << endl;
  }

  // Expand the data from the original RLE format

  if (!_expandRLE(data_buffer))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error expanding the Unisys RLE data" << endl;
    
    delete [] data_buffer;
    
    return false;
  }
  
  delete [] data_buffer;
  
  return true;
} 


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _expandRLE() - Expands the RLE data in the given buffer.
 *
 * Returns true on success, false on failure.
 */

bool UnisysFile::_expandRLE(ui08 *data_buffer)
{
  // Allocate space for image

  delete [] _imageData;
  _imageData = 0;

  int image_size = _unisysHeader.NumberOfColumns * _unisysHeader.NumberOfRows;

  _imageData = new ui08[image_size];
  memset(_imageData, 0, image_size);

  int bytesInRow;     // Number of bytes of RLE data to make current row
  int runLen;         // Length of run
  ui08 colVal;        // Color value from nibble
  ui08 bVal;          // Raster byte data

  si16 *rowPtr;       // Used for accessing each row of data
  ui08 *rlePtr;       // Used for accessing RLE data within rows

  int imgPos;         // Array index of current pixel

  // Start at first raster data element

  rowPtr = (si16 *)data_buffer;

  // Convert the Run Length Encoded data into _imageData array

  int imageByteCount = 0;
  for (int row = 0; row < _unisysHeader.NumberOfRows; row++)
  {

    // Get the number of bytes in this row

    bytesInRow = BE_to_si16(*rowPtr);

    //    if (_debug)
    //      cerr << "Bytes in row " << row << " is " << bytesInRow << endl;

    // Point to first RLE data for current row

    int rowByteCount = 0;

    // Get array position in _imageData array. Row #1 of Unisys data
    // is at bottom of image, image data works up from there

    imgPos = (_unisysHeader.NumberOfRows - row - 1) * _unisysHeader.NumberOfColumns;

    // Reading si16 numbers, so we need half as many si16's as bytes

    for (int rleCnt = 0; rleCnt < (bytesInRow/2); rleCnt++)
    {

      rowPtr++;
      rlePtr = (ui08 *)rowPtr;

      // Process 2 bytes of RLE data

      for (int byteCnt = 0; byteCnt < 2; byteCnt++)
      {
        bVal = *rlePtr;
        runLen = (bVal >> 4);
        colVal = (bVal & UNISYS_COLOR_MASK);

        rowByteCount += runLen;
        imageByteCount += runLen;

	//        if (_debug)
	//          cerr << "bVal " << rleCnt << " is " << bVal
	//	       << " (Count: " << runLen << " Color " << colVal << ")" << endl;

        // Expand RLE data into _imageData array,
        // Use lookup values in valueTable for data values

        for (int j = 0; j < runLen; j++)
        {
          if (colVal <= 15)
            _imageData[imgPos++] = _valueTableMDV[colVal];
          else
            _imageData[imgPos++] = _valueTableMDV[0];
        }

        rlePtr += 1;

      } // endfor - byteCount

    } // endfor - rleCnt

    //    if (_debug)
    //      cerr << " Bytes found for RLE expansion: " << rowByteCount
    //	   << " , difference " << (rowByteCount - _unisysHeader.NumberOfColumns) << endl;

    rowPtr += 1;

  } // endfor - row

  if (_debug)
  {
    cerr << "Total RLE data bytes found for expansion: " << imageByteCount << endl;
    cerr << "Anticipated total " << (_unisysHeader.NumberOfRows*_unisysHeader.NumberOfColumns)
	 << " for " << _unisysHeader.NumberOfColumns << " cols by "
	 << _unisysHeader.NumberOfRows << " rows" << endl;
    cerr << "Difference: "
	 << (imageByteCount - _unisysHeader.NumberOfRows*_unisysHeader.NumberOfColumns) << endl;
  } // endif - _debug

  return true;
}


/*********************************************************************
 * _readHeader() - Read the header information from the given file and
 *                 update the local header members.
 *
 * Returns true on success, false on failure.
 */

bool UnisysFile::_readHeader(FILE *input_file)
{
  static const string method_name = "UnisysFile::readHeader()";
  
  // Read the header information from the given file

  ui08 *header_buffer = new ui08[NUM_HEADER_BYTES];
  
  if (fread((void *)header_buffer, 1, NUM_HEADER_BYTES, input_file)
      != NUM_HEADER_BYTES)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading header information from input file" << endl;
    
    delete [] header_buffer;
    
    return false;
  }
  
  // Copy header information from buffer into struct

  ui08 *buffer_ptr = header_buffer;
  
  _unisysHeader.ProductID =             _retrieve16bit(buffer_ptr);
  _unisysHeader.DaysSince1970 =         _retrieve16bit(buffer_ptr);
  _unisysHeader.SecsSinceMidnight =     _retrieve32bit(buffer_ptr);
  _unisysHeader.TotalLengthBytes =      _retrieve32bit(buffer_ptr);
  _unisysHeader.SourceID =              _retrieve16bit(buffer_ptr);
  _unisysHeader.DestinationID =         _retrieve16bit(buffer_ptr);
  _unisysHeader.NumberOfBlocks =        _retrieve16bit(buffer_ptr);
  _unisysHeader.BlockDivider1 =         _retrieve16bit(buffer_ptr);
  _unisysHeader.CenterLatitude =        _retrieve32bit(buffer_ptr);
  _unisysHeader.CenterLongitude =       _retrieve32bit(buffer_ptr);
  _unisysHeader.HeightOfRadar =         _retrieve16bit(buffer_ptr);
  _unisysHeader.ProductID2 =            _retrieve16bit(buffer_ptr);
  _unisysHeader.OperationalMode =       _retrieve16bit(buffer_ptr);
  _unisysHeader.VolumeCoveragePattern = _retrieve16bit(buffer_ptr);
  _unisysHeader.SequenceNumber =        _retrieve16bit(buffer_ptr);
  _unisysHeader.VolumeScanNumber =      _retrieve16bit(buffer_ptr);
  _unisysHeader.Date_2 =                _retrieve16bit(buffer_ptr);
  _unisysHeader.Time_2 =                _retrieve32bit(buffer_ptr);
  _unisysHeader.Date_3 =                _retrieve16bit(buffer_ptr);
  _unisysHeader.Time_3 =                _retrieve32bit(buffer_ptr);
  _unisysHeader.ProductDependent1 =     _retrieve16bit(buffer_ptr);
  _unisysHeader.ProductDependent2 =     _retrieve16bit(buffer_ptr);
  _unisysHeader.ElevationNumber =       _retrieve16bit(buffer_ptr);
  _unisysHeader.ProductDependent3 =     _retrieve16bit(buffer_ptr);
  _unisysHeader.DataLevel1 =            _retrieve16bit(buffer_ptr);
  _unisysHeader.DataLevel2 =            _retrieve16bit(buffer_ptr);
  _unisysHeader.DataLevel3 =            _retrieve16bit(buffer_ptr);
  _unisysHeader.DataLevel4 =            _retrieve16bit(buffer_ptr);
  _unisysHeader.DataLevel5 =            _retrieve16bit(buffer_ptr);
  _unisysHeader.DataLevel6 =            _retrieve16bit(buffer_ptr);
  _unisysHeader.DataLevel7 =            _retrieve16bit(buffer_ptr);
  _unisysHeader.DataLevel8 =            _retrieve16bit(buffer_ptr);
  _unisysHeader.DataLevel9 =            _retrieve16bit(buffer_ptr);
  _unisysHeader.DataLevel10 =           _retrieve16bit(buffer_ptr);
  _unisysHeader.DataLevel11 =           _retrieve16bit(buffer_ptr);
  _unisysHeader.DataLevel12 =           _retrieve16bit(buffer_ptr);
  _unisysHeader.DataLevel13 =           _retrieve16bit(buffer_ptr);
  _unisysHeader.DataLevel14 =           _retrieve16bit(buffer_ptr);
  _unisysHeader.DataLevel15 =           _retrieve16bit(buffer_ptr);
  _unisysHeader.DataLevel16 =           _retrieve16bit(buffer_ptr);
  _unisysHeader.MaximumDataLevel =      _retrieve16bit(buffer_ptr);
  _unisysHeader.ProductDependent4 =     _retrieve16bit(buffer_ptr);
  _unisysHeader.ProductDependent5 =     _retrieve16bit(buffer_ptr);
  _unisysHeader.ProductDependent6 =     _retrieve16bit(buffer_ptr);
  _unisysHeader.ProductDependent7 =     _retrieve16bit(buffer_ptr);
  _unisysHeader.ProductDependent8 =     _retrieve16bit(buffer_ptr);
  _unisysHeader.NumberOfColumns =       _retrieve16bit(buffer_ptr);
  _unisysHeader.NumberOfMaps =          _retrieve16bit(buffer_ptr);
  _unisysHeader.OffsetToSymbology =     _retrieve32bit(buffer_ptr);
  _unisysHeader.OffsetToGraphic =       _retrieve32bit(buffer_ptr);
  _unisysHeader.OffsetToTabular =       _retrieve32bit(buffer_ptr);
  _unisysHeader.BlockDivider2 =         _retrieve16bit(buffer_ptr);
  _unisysHeader.BlockID =               _retrieve16bit(buffer_ptr);
  _unisysHeader.LengthOfBLock =         _retrieve32bit(buffer_ptr);
  _unisysHeader.NumberOfLayers =        _retrieve16bit(buffer_ptr);
  _unisysHeader.LayerDivider1 =         _retrieve16bit(buffer_ptr);
  _unisysHeader.LengthOfRasterLayer =   _retrieve32bit(buffer_ptr);
  _unisysHeader.RasterOpCode =          _retrieve16bit(buffer_ptr);
  _unisysHeader.RasterConstant1 =       _retrieve16bit(buffer_ptr);
  _unisysHeader.RasterConstant2 =       _retrieve16bit(buffer_ptr);
  _unisysHeader.XOrigin =               _retrieve16bit(buffer_ptr);
  _unisysHeader.YOrigin =               _retrieve16bit(buffer_ptr);
  _unisysHeader.XScaleInt =             _retrieve16bit(buffer_ptr);
  _unisysHeader.XScaleFrac =            _retrieve16bit(buffer_ptr);
  _unisysHeader.YScaleInt =             _retrieve16bit(buffer_ptr);
  _unisysHeader.YScaleFrac =            _retrieve16bit(buffer_ptr);
  _unisysHeader.NumberOfRows =          _retrieve16bit(buffer_ptr);
  _unisysHeader.PackingDescriptor =     _retrieve16bit(buffer_ptr);

  delete [] header_buffer;
  
  return true;
}


/*********************************************************************
 * _retrieve16bit() - Retrieve a 16 bit value from the given buffer
 *                    pointer and move the buffer pointer to the
 *                    position after the retrieved value.
 *
 * Returns the value read from the buffer, appropriately byte swapped.
 */

si16 UnisysFile::_retrieve16bit(ui08* &buffer_ptr)
{
  // Calculate the return value

  si16 return_value;
  memcpy(&return_value, buffer_ptr, sizeof(return_value));
  return_value = BE_to_si16(return_value);

  // Update the buffer pointer

  buffer_ptr += sizeof(return_value);
  
  return return_value;
}


/*********************************************************************
 * _retrieve32bit() - Retrieve a 32 bit value from the given buffer
 *                    pointer and move the buffer pointer to the
 *                    position after the retrieved value.
 *
 * Returns the value read from the buffer, appropriately byte swapped.
 */

si32 UnisysFile::_retrieve32bit(ui08* &buffer_ptr)
{
  // Calculate the return value

  si32 return_value;
  memcpy(&return_value, buffer_ptr, sizeof(return_value));
  return_value = BE_to_si32(return_value);

  // Update the buffer ptr

  buffer_ptr += sizeof(return_value);
  
  return return_value;
}


