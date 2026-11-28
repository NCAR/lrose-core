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
 * @file hiResRadialFile.hh
 * @brief Decodes WSR88D Level-III NEXRAD Information 
 *        Dissemination Service (NIDS) data files and
 *        converts to Mdv files
 * 
 * 
 * @class hiResRadialFile.hh
 * @brief Decodes Level III NEXRAD Information 
 *        Dissemination Service (NIDS) data files and
 *        converts to Mdv files
 * 
 * The document used to create this code is 
 * Build 12.1 for document number 2620001, code id 0WY55 
 * (RPG to Class 1 User ICD) which can be found at:
 * http://www.roc.noaa.gov/WSR88D/DualPol/documentation.aspx.
 * Some information like gate spacing is found from internet
 * sources including papers:
 * 25th AMS IIPS -- WSR-88D Dual Polarization Initial Operational Capabilities 
 * Paper 
 * Deserializing code needed for decoding select products was 
 * imported from WSR-88D Common Operations and Development 
 * Environment which can be found at 
 * http://www.nws.noaa.gov/code88d
 */

#ifndef HI_RES_RADIAL_FILE_HH
#define HI_RES_RADIAL_FILE_HH

#include "Params.hh"
#include <dataport/port_types.h>
#include <vector>
#include <map>
#include <string>
#include "GraphicProductMsg.hh"
#include "Product.hh"
#include "Params.hh"

using namespace std;

class hiResRadialFile {
  
public:

  /**
   * Constructor 
   * Initialize data members, set up archive or real-time triggering based on
   * user defined parameters. Log the start time of the application. Exit if
   * driver does not initialize properly.
   * @param[in] filePath  Full path of radar file
   * @param[in] parameters  User defined parameters
   * @param[in] debug  Flag to indicate verbose debug messaging  
   */
  hiResRadialFile (const char *filePath, bool byteSwap, 
		   bool debug = false);

  /**
   * Destructor
   * Frees allocated memory 
   */ 
   ~hiResRadialFile ();

  /**
   * Decode NIDS file. This method assumes the file format described in section
   * 3.3 of the ICD and shown in Fig 3-6 of the ICD.
   * The message stucture is: 
   * Header Block
   * Product Description Block
   * Symbology Block (Symbology header, Radial data header OR generic format
   *                  header)
   * Graphic Alphanumeric Block
   * Tabular Alphanumeric Block
   * 
   * The decode method mangages the sequential decoding tasks. It reads the 
   * Header Block and gets the product code (See ICD Table III). Based on 
   * product code, a unique Product object is instantiated and populated with
   * information from the  Product Description Block. The Product object 
   * contains product specific meta data and decoding information for radial 
   * byte data. After the Product Description Block is read, the remainder 
   * of the file is read into memory and decompressed if necessary. The next 
   * message  component, the Symbology Block, contains the radial data. 
   * The radial data for products may be stored in different formats and is 
   * passed to an appropriate decoding methods for the format.
   * 
   * Note: The product's data packet code (16, 28, AF1F) indicates the storage 
   * format (radial, digital radial, generic).  Some product format 
   * information can be found in ICD Table 3 (radial or generic). 
   * If data has a radial image format, the packet code can be found in the 
   * radial header following the symbology block header. If the format is 
   * listed as generic in Table 3 then the packet code is 28 (or 29). 
   * @return 0 for success or 1 for failure
   */
  int decode();

  /**
   * Remap polar data to cartestian grid
   * @param[in] delta Distance between grid points in kilometers. If delta is 
   *                  negative then the polar gate spacing is used.
   * @param[in] dist Distance of the range in kilometers. If dist is negative 
   *                 then the maximum range of the radar is used,  
   *                 dist =  firstGateDist + nGates * gateSpacing
   */
  void remapToCart(double delta = -1.0, double dist = -1.0);

  /**
   * @return value assigned to missing data
   */
  fl32 getMissingFl32()  {return  _missingFl32;}

  /**
   * @return latitude of radar 
   */
  fl32 getLat() {return _lat;}

  /**
   * @return longitude of radar
   */
  fl32 getLon() {return _lon;}
  
  /**
   * @return altitude of radar
   */
  fl32 getAlt() { return _alt;}

  /**
   * @return data time 
   */
  time_t getTime() { return _time;}

  /**
   * @return elevation angle of radar sweep
   */
  fl32 getElevAngle() { return _elevAngle;}

  /**
   * @return integer number of radials er sweep
   */
  int getNradials() {return _nRadials;}
  
  /**
   * @return  number of gates in a radial.
   */
  int getNgates() {return _nGates;}

  /**
   * @return gate spacing in kilometers
   */
  fl32 getGateSpacing() {return _gateSpacing; }

  /**
   * @return distance in kilometers to first gate
   */
  fl32 getFirstGateDist() {return _firstGateDist;}

  /**
   * @return angle difference between radials
   */
  fl32 getDelAz() { return _delAz;}

  /**
   * @return volume number
   */
  int getVolNum() { return _volNum;}

  /**
   * @return pointer to  decoded radial data.
   */
  fl32 *getAllRadialFl32s() { return _radialFl32s; }

  /**
   * @return pointer to cartesian data if requested
   */
  fl32 *getCartData() {  return _cartData; }

  /**
   * @return number of dimension(s) of cartesian grid
   */
  int getNxy(){return _nxy;}

  /**
   * @return distance in kilometers between cartesian grid points
   */
  fl32 getDxy()  {return _dxy;}

  int getContourSize( int contourNum) {return (int)_contours[contourNum].size();}

  void getContourPt (int contourNum, int ptIndex, si16 &i, si16 &j);

  bool isContourData() { return _linkedVectorPacket; }

protected:
  
private:

  /**
   * Constant assigned to missing or bad data
   */
  const static fl32  _missingFl32;

  /**
   * Path of file to be decoded
   */ 
  string _nidsFile;

  /**
   * Each Radial Image product has a Graphic Product Message 
   * Description block. See ICD For the RPG for Class 1 User, Fig 3-6, 
   * sheet 6. This class is modeled after this structure.
   * A Product object contains basic information about the radial image product
   * (latitude, longitude, altitude, VCP, etc) as well as product dependent 
   * information like the scale and bias used to decode the radial data. 
   * The type of Product object is instantiated based on the message code 
   * number (See Table III) and is used to decode and interpret the radial 
   * data.
   */
  Product *_product;

  /**
   * Latitude of radar. 
   */
  fl32 _lat;
  
  /**
   * Longitude of radar. 
   */
  fl32 _lon;

  /**
   * Altitude of radar. 
   */
  fl32 _alt;

  /**
   * Time  
   */
  time_t _time;

  /**
   * Elevation angle of radar tilt 
   */
  fl32 _elevAngle;

  /**
   * Number of radials in tilt. 
   */
  int _nRadials;

  /**
   * Number of gates per radial. 
   */
  int _nGates;

  /**
   * Distance between gates
   */
  fl32 _gateSpacing;

  /**
   * Distance to first gate
   */
  fl32  _firstGateDist;

  /**
   * Difference in azimuth between radials
   */ 
  fl32 _delAz;

  /**
   * Volume number
   */ 
  int _volNum;
  
  /**
   * Pointer to radial data 
   */
  fl32 *_radialFl32s;
 
  /**
   * Pointer to cartesian data. This is NULL unless user 
   * requests cartesion output. 
   */
  fl32 *_cartData;

  /**
   * Number of horizontal and vertical grid points in cartesian grid.
   * We assume the grid is square. Used only if user requests data
   * on cartesian grid.
   */
  int _nxy;

  /**
   * Distance in kilometers between cartesian grid points. Uniform spacing
   * is assumed. Used only if user requests data on cartesian grid 
   */
  fl32 _dxy;

  /** 
   * Flag to indicate radial data is Run Length Encoded. 
   * Radial data packet has code Hex "AF1F"
   */
  bool _runLengthEncoding;

  /** 
   * Flag to indicate radial data has Generic Product Format. See Appendix E 
   * of the  ICD. Data has packet code 28.
   */
  bool _genericRadialFormat;

  /** 
   * Flag to indicate data stored as Linked Vector Packet (packet code 6 or 
   * packet code 9) . See Figure 3-7 (sheets 1,2 and 3) of the  ICD. 
   */
  bool  _linkedVectorPacket;

  /** 
   * Flag to indicate debug messaging
   */ 
  bool _debug;

  /** 
   * Flag to indicate byte swapping the radar message components
   */ 
  bool _byteSwap;
  
  /** 
   * Set members from product description block
   * (radar latitude, radar longitude, altitude, data time )
   */
  void _setProductDescrMembers();

  /** 
   * The data after the header blocks are read into memory:
   * We stat the file to get its size, get the offset in the file to the 
   * radial data from the header block, do the math, and allocate the memory,  
   * and read the rest of the message ( the radial data) into a temporary
   * buffer.  The data are uncompressed if necessary stored in a (possibly 
   * larger) buffer.  
   * @params[in] decompressedBuf  A pointer to a buffer holding decompressed
   *                              radial data.
   * @params[in] fp  A FILE pointer to Graphic Product Message file.
   */
  int _getDecompressedData(unsigned char **decompressedBuf, FILE *fp);

  /** 
   * The method will allocate memory for and return a buffer holding 
   * decompressed data starting at the Symbology Block. If the data are 
   * compressed a bzip decompression lib method is called. If the data
   * are not compressed, the data are copied to the new decompressed 
   * data buffer.
   * @params[in] compressedBuf  Buffer holding data remaining after Product
   *                            Description block.
   * @params[out] decompressedBuf  Buffer holding decompressed data remaining 
   *                               after Product Description block.
   * @params[in] numBytes  Size of compressed data buffer
   */ 
  int _decompress( unsigned char *compressedBuf,  
		   unsigned char **decompressedBuf,
		   unsigned int numBytes);

  /** Decode linked vector data. Data packet code id is 9.
   * Read linked vector data header, allocate space for
   * decoded point data (I,J) pairs that connect to form a 
   * polygon. 
   */
  
  void _decodeLinkedVectors(unsigned char* decompressedBuf, 
			    unsigned long &offset);
  /**
   * Decode digital radial data. Data packet code id is 16.
   * Read radial data header, record relevant information (number of 
   * gates, radials etc), and allocate space for decoded radial data.
   * Loop through all radials and gates and record decoded data value.
   * Advance the data offset appropriately. 
   * Note that decoding byte data is product specific and handled
   * by the unique Product object which was instantiated based on the 
   * message code.
   * @params[in] decompressedBuf Buffer holding radial data to be decoded.
   * @params[in] offset  Offset to bytes being decoded.
   * 
   */
  void _decodeDigitalRadialMsg(unsigned char* decompressedBuffer, 
			       unsigned long &offset);
  /**
   * Decode radial data. Data packet code Hex "AF1F". Read radial data header, 
   * record relevant information (number of gates, radials etc), and allocate 
   * space for decoded radial data. Loop through all radials, decode Run Length
   * Encoded values and assign as necessary to proper number of gates.
   * Advance the data offset appropriately. 
   * Note that decoding byte data is product specific and handled
   * by the unique Product object which was instantiated based on the 
   * message code.
   * @params[in] decompressedBuf  Buffer holding radial data to be decoded.
   * @params[in] offset  Offset to bytes being decoded.
   */
  void _decodeRadialMsg(unsigned char* decompressedBuffer, 
			unsigned long &offset);

  /**
   * Decode radial data in Generic Radial Data format (packet code 28).
   * Read generic header (See appendix E of ICD). Deserialize the radial data. 
   * Read radial data header and record relevant information. Make space for
   * decoded radial data. Loop through all radials and all gates and record 
   * decoded data value.
   * 
   * Note: Data with packet 28 is serialized. Code to deserialize the 
   * data can be found in the  freely available CODE repository in 
   * orpg_product.h and  orpg_xdr.c. Also in orpg_product.h are structs that 
   * are helpful in reading the generic message ( RPGP_product_t,  
   * RPGP_radial_t,  RPGP_radial_data_t).
   * @params[in] decompressedBuf  Buffer holding radial data to be decoded.
   * @params[in] offset  Offset to bytes being decoded. 
   */
  void _decodeGenericMsg(unsigned char* decompressedBuffer, 
			 unsigned long &offset);

  /**
   * Decode byte data. This is product dependent
   * @return floating point value for byte data
   */ 
  double _decodeData(ui08 x);
 
  /**
   * Decode RLE byte. 
   * @param[in] x  RLE byte data
   * @param[out] run  Number of bins that have the same value
   * @param[out] val  Value to be assigned to bins
   */
  void _decodeData(ui08 x, ui08 &run, fl32 &val);

  /**
   * Allocate space for decoded radial float data. 
   * @param[in] _nGates  Number of gates per radial
   * @param[in] _nRadials  Number of radials per sweep
   * @param[out] radialData  Buffer for decoded byte data 
   */
  void _allocateRadialSpace( int _nGates, int _nRadials, fl32 **radialData ); 

  /**
   * Track maximum and minimum decoded values for debugging purposes 
   * @param[in] first Flag to indicate value is the first. Hence 
   *                  max and min get set to this value
   * @param[in/out] max  Maximum decoded data value 
   * @param[in/out] min  Minimum decoded data value
   * @param[out] val  Value to be compared to max and min. max and min will be
   *                 reset if necessary.
   */
  void _trackMaxMin(bool &first, float &max, float &min, float val);

  /**
   * Return index for a given azimuth.
   * @param[in] az  float representing degrees
   * @return integer radial number 
   */ 
  int _getAzIndex( fl32 az );

  /**
   * Get the maximum distance from radar for which we have valid data.
   * This is used in converting to cartesian format.
   * @return distance in kilometers
   */ 
  double _getMaxDist();

  /**
   * Color value for contour vector 
   */
  si16 _colorValue;

  vector < vector < pair< si16, si16 > > > _contours;
  
};

#endif





