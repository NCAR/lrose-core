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

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/04 02:22:16 $
 *   $Id: UnisysFile.hh,v 1.3 2016/03/04 02:22:16 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * UnisysFile: Class for operating on a Unisys mosaic file.
 *
 * RAP, NCAR, Boulder CO
 *
 * September 2003
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef UnisysFile_HH
#define UnisysFile_HH

#include <string>

#include <dataport/port_types.h>
#include <euclid/Pjg.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/udatetime.h>

using namespace std;


class UnisysFile
{
 public:

  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  // Constructor

  UnisysFile();
  
  // Destructor

  ~UnisysFile(void);
  
  // Initialization method

  bool init(const double input_parallel_1,
	    const double input_parallel_2,
	    const int valid_source_id,
	    const string &data_field_name_long,
	    const string &data_field_name,
	    const string &data_units,
	    const double data_scale,
	    const double data_bias,
	    const double missing_data_value,
	    const Pjg &output_projection,
	    const bool debug_flag = false);
  

  //////////////////////////
  // Input/output methods //
  //////////////////////////

  /*********************************************************************
   * readFile() - Reads the given Unisys mosiac file into the internal
   *              buffer.
   *
   * Returns true on success, false on failure.
   */

  bool readFile(const string &filename);
  

  /*********************************************************************
   * printHeader() - Prints the Unisys mosiac header to stderr.
   */

  void printHeader();
  

  /*********************************************************************
   * createMdvxField() - Create an MdvxField object from the file information.
   *
   * Returns a pointer to the new MdvxField object if successful,
   * 0 otherwise.
   */

  MdvxField *createMdvxField(const DateTime &data_time);
  

  ////////////////////
  // Access methods //
  ////////////////////

  /*********************************************************************
   * getDataTime() - Retrieves the data time from the file.
   */

  DateTime getDataTime()
  {
    return DateTime((_unisysHeader.DaysSince1970 - 1) * 86400 +
		    _unisysHeader.SecsSinceMidnight);
  }
  

  /*********************************************************************
   * getCenterLongitude() - Retrieves the center longitude from the file.
   */

  double getCenterLongitude()
  {
    return _unisysHeader.CenterLongitude;
  }
  

  /*********************************************************************
   * getCenterLatitude() - Retrieves the center latitude from the file.
   */

  double getCenterLatitude()
  {
    return _unisysHeader.CenterLatitude;
  }
  


 private:

  ///////////////////
  // Private types //
  ///////////////////

  typedef struct {
    ////////////////////////////////////////////////////////////////////////////////////
    // Variable names and descriptions are from:
    //
    // WeatherMAX(TM) - Site & Product Catalog
    // Appendix A
    // MOSAIC RASTER FORMAT PRODUCT FILE FORMAT
    // VER 3.00
    // Pages A-1 through A-6
    //
    // Each field is specified as a 16 BIT INTEGER
    // and some variables span multiple fields
    //
    // [Most of the header fields need byte swapping. -mp]
    //
    ////////////////////////////////////////////////////////////////////////////////////
    //
    //                           // FIELD COMMENTS
    //                           // ----- -------------------------------------------
    si16 ProductID;              // 01    Product ID
    si16 DaysSince1970;          // 02    Date (Days since 1/1/1970)
    si32 SecsSinceMidnight;      // 03,04 Time (Seconds since midnight)
    si32 TotalLengthBytes;       // 05,06 Total Length in bytes
    //                           //       (Field 05 = MSW, 06 = LSW)
    //                           //       [seems to be length of entire file -mp]
    si16 SourceID;               // 07    Source ID
    //                           //       (Site Dependent. e.g., National = 10000)
    si16 DestinationID;          // 08    Destination ID (unused, Constant = 0)
    si16 NumberOfBlocks;         // 09    Number of Blocks (Constant = 3)
    si16 BlockDivider1;          // 10    Block Divider (Constant = -1)
    si32 CenterLatitude;         // 11,12 Center Latitude (Degrees * 1000)
    //                           //       (e.g., 38000 indicates 38.0 north
    //                           //       (Field 11 = MSW, 12 = LSW)
    si32 CenterLongitude;        // 13,14 Center Longitude (Degrees * 1000)
    //                           //       (e.g., -98000 indicates 98.0 west
    //                           //       (Field 13 = MSW, 14 = LSW)
    si16 HeightOfRadar;          // 15    Height of Radar (unused, Constant = 0)
    si16 ProductID2;             // 16    Product ID (same as field #1)
    si16 OperationalMode;        // 17    Operational Mode (Constant = 2)
    si16 VolumeCoveragePattern;  // 18    Volume Coverage Pattern (Constant = 0)
    si16 SequenceNumber;         // 19    Sequence Number (unused, Constant = 0)
    si16 VolumeScanNumber;       // 20    Volume Scan Nummber (unused, Constant = 0)
    si16 Date_2;                 // 21    Date (Days since 1/1/1970)(same as field #2)
    si32 Time_2;                 // 22,23 Time (Seconds since midnight)
    //                           //       (same as fields #3 and #4)
    //                           //       (Field 22 = MSW, 23 = LSW)
    si16 Date_3;                 // 24    Date (Days since 1/1/1970)(same as field #2)
    si32 Time_3;                 // 25,26 Time (Seconds since midnight)
    //                           //       (same as fields #3 and #4)
    //                           //       (Field 25 = MSW, 26 = LSW)
    si16 ProductDependent1;      // 27    Product Dependent (unused, Constant = 0)
    si16 ProductDependent2;      // 28    Product Dependent (unused, Constant = 0)
    si16 ElevationNumber;        // 29    Elevation Number (unused, Constant = 0)
    si16 ProductDependent3;      // 30    Product Dependent (unused, Constant = 0)
    si16 DataLevel1;             // 31    Data Level 1 (same format as NEXRAD products)
    si16 DataLevel2;             // 32    Data Level 2
    si16 DataLevel3;             // 33    Data Level 3
    si16 DataLevel4;             // 34    Data Level 4
    si16 DataLevel5;             // 35    Data Level 5
    si16 DataLevel6;             // 36    Data Level 6
    si16 DataLevel7;             // 37    Data Level 7
    si16 DataLevel8;             // 38    Data Level 8
    si16 DataLevel9;             // 39    Data Level 9
    si16 DataLevel10;            // 40    Data Level 10
    si16 DataLevel11;            // 41    Data Level 11
    si16 DataLevel12;            // 42    Data Level 12
    si16 DataLevel13;            // 43    Data Level 13
    si16 DataLevel14;            // 44    Data Level 14
    si16 DataLevel15;            // 45    Data Level 15
    si16 DataLevel16;            // 46    Data Level 16
    si16 MaximumDataLevel;       // 47    Maximum Data Level
    si16 ProductDependent4;      // 48    Product Dependent (unused, Constant = 0)
    si16 ProductDependent5;      // 49    Product Dependent (unused, Constant = 0)
    si16 ProductDependent6;      // 50    Product Dependent (unused, Constant = 0)
    si16 ProductDependent7;      // 51    Product Dependent (unused, Constant = 0)
    si16 ProductDependent8;      // 52    Product Dependent (unused, Constant = 0)
    si16 NumberOfColumns;        // 53    Number of Columns
    //                           //       (Use this for raster display)
    si16 NumberOfMaps;           // 54    Number of Maps (unused, Constant = 0)
    si32 OffsetToSymbology;      // 55,56 Offset to Symbology (bytes, = 60)
    //                           //       (Field 55 = MSW, 56 = LSW)
    si32 OffsetToGraphic;        // 57,58 Offset to Graphic (bytes, = 0)
    //                           //       (Field 57 = MSW, 58 = LSW)
    si32 OffsetToTabular;        // 59,60 Offset to Tabular (bytes, = 0)
    //                           //       (Field 59 = MSW, 60 = LSW)
    //                           //
    // ************************ PRODUCT SYMBOLOGY BLOCK ******************************//
    si16 BlockDivider2;          // 61    Block Divider (Constant = -1)
    si16 BlockID;                // 62    Block ID (Constant = 1)
    si32 LengthOfBLock;          // 63,64 Length of Block (bytes)
    //                           //       (Field 63 = MSW, 64 = LSW)
    si16 NumberOfLayers;         // 65    Number of Layers (Value of 2 indcates that the
    //                           //       "Site Identifier Layer" is present)
    //                           //
    // ************************ THE "RASTER DATA LAYER" ******************************//
    si16 LayerDivider1;          // 66    Layer Divider (Constant = -1)
    si32 LengthOfRasterLayer;    // 67,68 Length of (Raster) Layer
    //                           //       (Field 67 = MSW, 68 = LSW)
    ui16 RasterOpCode;           // 69    Raster Opcode (Constant= HEX BA07=DEC 47623)
    ui16 RasterConstant1;        // 70    Raster Constant (Constant= HEX 8000=DEC 32768)
    ui16 RasterConstant2;        // 71    Raster Constant (Constant= HEX 00C0=DEC 192)
    si16 XOrigin;                // 72    X Origin (unused, Constant = 0)
    si16 YOrigin;                // 73    Y Origin (unused, Constant = 0)
    si16 XScaleInt;              // 74    X Scale Integer (unused, Constant = 1)
    si16 XScaleFrac;             // 75    X Scale Fractional (unused, Constant = 0)
    si16 YScaleInt;              // 76    Y Scale Integer (unused, Constant = 1)
    si16 YScaleFrac;             // 77    Y Scale Fractional (unused, Constant = 0)
    si16 NumberOfRows;           // 78    Number of Rows (use for raster display)
    si16 PackingDescriptor;      // 79    PackingDescriptor (Constant = 2)
    //
    // The raster data begin at this point in the file:
    //                           //.....................................................
    //                           // 80   Bytes in this row          (Row #1)
    //                           // 81   Run 0 Col 0  Run 1 Col 1   (Run length encoded
    //                           // 82   Run 2 Col 2  Run 3 Col 3   data - Same format
    //                           // 83   ...                        as NEXRAD)
    //                           //      ...                      
    //                           //      Run n Col n  0   0 0   0   [Run = run length as
    //                           //      |___| |___|  |___| |___|   number of colums,
    //                           //      HiNib LoNib  HiNib LoNib   [Col = color -mp]
    //                           //        
    //                           //      |_________|  |_________|   [Note that the last
    //                           //       High byte    Low byte     byte in the row
    //                           //                                 may be 0 -mp]
    //                           //      |______________________|
    //                           //           16 bit integer
    //                           //.....................................................
    //                           //      Bytes in this row          (Row #2)
    //                           //      Run 0 Col 0  Run 1 Col 1   (Run length encoded
    //                           //      Run 2 Col 2  Run 3 Col 3   data - Same format
    //                           //      ...                        as NEXRAD)
    //                           //      Run n Col n  0   0 0   0
    //                           //.....................................................
    //                           //      Bytes in this row          (Last Row
    //                           //                                 _unisysHeader.NumberOfRows)
    //                           //      Run 0 Col 0  Run 1 Col 1   (Run length encoded
    //                           //      Run 2 Col 2  Run 3 Col 3   data - Same format
    //                           //      ...                        as NEXRAD)
    //                           //      Run n Col n  0   0 0   0
    //                           //.....................................................
    //                           //
    // ************************ THE "SITE IDENTIFIER LAYER" **************************//
    //   [The site identifier layer is not being used in unisys2mdv at this time -mp]
    //
    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Note: [from documentation]
    //
    // 1.  National Mosaics are projected with a Lambert Conformal Conic Projection.
    //
    // 2.  Mosic product format is nearly identical to NEXRAD (e.g., Composite
    //     Reflectivity) product format. Some fields may not apply and are set
    //     to constant values.
    //
    //     The date and time formats are the same (in (UTC). The data levels
    //     are encoded as in NEXRAD products. The raster image is in the NEXRAD
    //     de facto standard run length encoded format.
    //
    // 3.  The 8 data lavel reflectivity mosaics use the NEXRAD 8 level
    //     Composite Reflectivity (Precipitation mode) levels:
    //     ND,5,18,30,41,46,50,57 dBZ.
    //
    // 4.  The 16 data lavel reflectivity mosaics use the NEXRAD 16 level
    //     Composite Reflectivity  (Precipitation mode) levels:
    //     ND,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75 dBZ.
    //
    // 5.  There can be an additional layer in the Product Symbology Block
    //     which identifies the included and excluded sites. This is present
    //     when the number of layers equals 2.
    //
    // 6.  The national mosaic products contain a site ID of 10000.
    //     The center is at 38.0 north latitude and 98.0 west longitude.
    //
    // 7.  The following relates spatial resolution to columns and rows
    //     for national mosaic products:
    //          2 km x  2 km    2560 columns x 1792 rows
    //          4 km x  4 km    1280 columns x  896 rows
    //          8 km x  8 km     640 columns x  448 rows
    //         16 km x 16 km     320 columns x  224 rows
    //         32 km x 32 km     160 columns x  112 rows
    //
    // 8.  The number of columns is assigned to location 53. The number of rows
    //     is assigned to  location 78.
    //
    // 9.  The Surface Rainfall Accumulation (e.g., 1-Hour Precipitation) msaic
    //     products use the NEXRAD 1-Hour Precipitation product data level values:
    //     ND, 0.0 ,0.1 ,0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0,
    //     6.0, 8.0, 10.0 inches.
    //
    // 10. The Echo Tops mosaic products use the NEXRAD Echo Tops product data
    //     level values: 
    //     ND,5,10,15,20,25,30,35,40,45,50,55,60,65,70 kft MSL.
    //
    // 11. The maximum data level value is assigned to field 47.
    //
    //     For Composite and Base Reflectivity mosaic products, the maximum data
    //     level is in dBZ (range -33 to 95, where -33 means no data).
    //
    //     For Surface Rainfall Accumulation (e.g., 1-Hour Precipitation) mosaic
    //     products the maximum data value is in tenths of inches
    //     (ramge: 0.0 to 198.0 inches).
    //
    //     For Echo Tops mosaic products, the maximum data level is in
    //     1000's of feet above mean sea lavel (MSL) (range: 0 to 70 kft MSL).
    //
    // 12. Regional mosaic products are based on the national mosaic projection
    //     Lambert Conic Conformal centered at
    //             38.0 degrees north and 98.0 degrees west).
    //     The regional mosaic center point latitude/longitude appears in
    //     fields 11 to 14. Each region has a unique site ID number
    //     (listed in this ICD) which appears in field 7.
    //
    //     The following relates spatial resolution to columns and rows:
    //         4 km x 4 km    640 columns x 448 rows
    //
    // 13. Custom mosaic products are based on the national mosaic projection
    //     Lambert Conic Conformal centered at
    //             38.0 degrees north and 98.0 degrees west).
    //     The custom mosaic center point latitude/longitude appears in
    //     fields 11 to 14. Each region has a unique site ID number
    //     (listed in this ICD) which appears in field 7.
    //
    //     The following relates spatial resolution to columns and rows:
    //         4 km x 4 km    232 columns x 232 rows   or
    //         4 km x 4 km    332 columns x 332 rows
    //
    ////////////////////////////////////////////////////////////////////////////////////
    //
  } uniHdr_t;


  ///////////////////////
  // Private constants //
  ///////////////////////

  // const si16 RASTER_OPCODE = 0xBA07;
  // const si16 RASTER_CONST1 = 0x8000;
  // const si16 RASTER_CONST2 = 0x00C0;

  static const ui08 UNISYS_COLOR_MASK;   // 4 bit 1s mask
  static const size_t NUM_HEADER_BYTES;
  

  /////////////////////
  // Private members //
  /////////////////////

  bool _debug;
  
  // Processing information

  double _inputParallel1;
  double _inputParallel2;
  
  int _validSourceId;

  string _dataFieldNameLong;
  string _dataFieldName;
  string _dataUnits;
  
  double _dataScale;
  double _dataBias;
  double _missingDataValue;

  Pjg _outputProjection;
  
  uniHdr_t _unisysHeader;  // Struct to hold Unisys file header info

  ui08 *_imageData;   // Array to hold expanded RLE image data

  ui08 _valueTableMDV[16];  // Array for MDV data levels


  /*********************************************************************
   * _expandRLE() - Expands the RLE data in the given buffer.
   *
   * Returns true on success, false on failure.
   */

  bool _expandRLE(ui08 *data_buffer);


  /*********************************************************************
   * _readHeader() - Read the header information from the given file and
   *                 update the local header members.
   *
   * Returns true on success, false on failure.
   */

  bool _readHeader(FILE *input_file);
  

  /*********************************************************************
   * _retrieve16bit() - Retrieve a 16 bit value from the given buffer
   *                    pointer and move the buffer pointer to the
   *                    position after the retrieved value.
   *
   * Returns the value read from the buffer, appropriately byte swapped.
   */

  si16 _retrieve16bit(ui08* &buffer_ptr);
  

  /*********************************************************************
   * _retrieve32bit() - Retrieve a 32 bit value from the given buffer
   *                    pointer and move the buffer pointer to the
   *                    position after the retrieved value.
   *
   * Returns the value read from the buffer, appropriately byte swapped.
   */

  si32 _retrieve32bit(ui08* &buffer_ptr);
  

};


#endif
