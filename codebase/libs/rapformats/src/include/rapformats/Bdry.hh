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
// Bdry.hh
//
// C++ wrapper for boundary data.
//
// Boundary data is stored in the SPDB buffer in the following order:
//
//   Bdry::product_t: one of these
//   Repeat the following product_t::num_polylines times:
//     Bdry::polyline_t: one of these
//     Bdry::point_t: polyline_t::num_pts of these
//
// Nancy Rehak, RAP, NCAR
// PO Box 3000, Boulder, CO, USA
//
// March 2005
//////////////////////////////////////////////////////////////

#ifndef _Bdry_hh
#define _Bdry_hh

#include <string>
#include <cstdio>
#include <iostream>
#include <vector>

#include <dataport/port_types.h>
#include <rapformats/bdry_typedefs.h>
#include <rapformats/BdryPolyline.hh>
#include <rapformats/ascii_shapeio.h>
#include <toolsa/DateTime.hh>
#include <toolsa/MemBuf.hh>

using namespace std;

class Bdry
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /************************************************************************
   * Constructors
   */

  Bdry();

  Bdry(const time_t &data_time, const int extrap_seconds,
       const time_t &expire_time,
       const std::string &typeString, const std::string subtypeString,
       const int sequence, const int linetype, const int id,
       const std::string &description,
       const double motion_direction=0.0, const double motion_speed=0.0,
       const double quality=0.0, const double qual_thresh=0.0);


  /************************************************************************
   * Destructor
   */

  virtual ~Bdry();
  
  /************************************************************************
   * init(): Initializes the Boundary basd on the SIO shape information read
   *         in from an ASCII file.
   *
   * Returns true on success, false on failure.
   *
   * NOTE: This method needs to be tested because we're not currently sure
   * whether shapeio speed values are stored in km/hr or m/s.
   */

  void init(const SIO_shape_data_t &shape);


  ///////////////////////////////////////////////////////////////
  // set methods
  
  void clear();


  //////////////////////
  // Database methods //
  //////////////////////

  /************************************************************************
   * disassemble() - Disassembles a buffer, sets the object values. Handles
   *                 byte swapping.
   *
   * Returns true on success, false on failure.
   */

  bool disassemble(const void *buf, int len);

  /************************************************************************
   * assemble() - Load up the buffer from the object. Handles byte swapping.
   *
   * Returns true on success, false on failure.
   */
  
  bool assemble();

  /************************************************************************
   * Get the assembled buffer pointer
   */

  void *getBufPtr() const { return _memBuf.getPtr(); }
  int getBufLen() const { return _memBuf.getLen(); }


  ////////////////////
  // Access methods //
  ////////////////////

  /************************************************************************
   * addPolyline(): Add the given polyline to the boundary.
   */

  inline void addPolyline(const BdryPolyline &polyline)
  {
    _polylines.push_back(polyline);
  }
  

  void addSpareFloat(const fl32 v);


  /************************************************************************
   * getPolylines(): Get the list of polylines for the boundary.
   */

  inline const vector< BdryPolyline > &getPolylines() const
  {
    return _polylines;
  }
  

  /************************************************************************
   * getPolylinesEditable(): Get the list of polylines for the boundary, in
   *                         a way that the polylines can be editted.
   */

  inline vector< BdryPolyline > &getPolylinesEditable()
  {
    return _polylines;
  }
  

  /************************************************************************
   * getDataTime(): Get the data time of the boundary.
   */

  inline DateTime getDataTime() const
  {
    return _dataTime;
  }
  

  /************************************************************************
   * getExpireTime(): Get the expire time of the boundary.
   */

  inline DateTime getExpireTime() const
  {
    return _expireTime;
  }
  

  /************************************************************************
   * getType(): Get the type of the boundary.
   */

  inline int getType() const
  {
    return _type;
  }
  

  /************************************************************************
   * getBdryId(): Get the ID of the boundary.
   */

  inline int getBdryId() const
  {
    return _bdryId;
  }
  

  inline double getQualityValue() const
  {
    return _lineQualityValue;
  }
  
  inline bool getSpareFloat(const int i, fl32 &s)
  {
    if (i < 0 || i >= _numSpareFloat)
    {
      // give a warning here
      return false;
    }

    s = _spare_float[i];
    return true;
  }


  /************************************************************************
   * getDescription(): Get the description of the boundary.
   */

  inline string getDescription() const
  {
    return _description;
  }
  

  /************************************************************************
   * getDirectionPjg(): Get the direction of the boundary in PJG coordinates.
   */

  inline double getDirectionPjg() const
  {
    return spdb2PjgDirection(_motionDirection);
  }
  

  /************************************************************************
   * getSpeed(): Get the speed of the boundary in m/s.
   */

  inline double getSpeed() const
  {
    return _motionSpeed;
  }
  

  /************************************************************************
   * getLineType(): Get the type of line in the boundary.
   */

  inline int getLineType() const
  {
    return _lineType;
  }
  

  /************************************************************************
   * print(): Print the boundary to the given stream.
   */

  void print(FILE *stream, const bool print_points) const;
  void print(ostream &stream, const bool print_points) const;
  

  /************************************************************************
   * toSIOShape(): Converts the boundary into the internal SIO shape structure.
   *
   * Returns a pointer to memory allocated by this routine that should be
   * freed by the calling routine.  Returns NULL if there is an error.
   */

  SIO_shape_data_t *toSIOShape();


  /************************************************************************
   * spdb2PjgDirection(): Converts the SPDB direction value to the
   *                      value used by PJG routines.
   *
   * Colide stores the direction using cartesian coordinates where
   * 0 degrees is along the X axis and degrees increase counter-
   * clockwise.  PJG uses map coordinates, where 0 degrees is
   * north and degrees increase clockwise.  Both give the angle we are
   * moving TO.
   */

  static double spdb2PjgDirection(const double spdb_direction);


  /************************************************************************
   * getSpdbDataType():  Sets data type so that first 8 bits (from lowest
   *                     order bit to highest order bit) are the 
   *                     boundary type, the next 8 bits are the subtype 
   *                     and the last 8 bits are the forecast period.
   *                     A constant value is subtracted from the subtype
   *                     to ensure the value will fit in the 8 bits given
   *                     to it.
   */

  si32 getSpdbDataType() const;


  /************************************************************************
   * parseDataType(): Parses the data type field into type, subtype
   *                  and forecast period, assuming that the data type
   *                  was created such that the first 8 bits (from 
   *                  lowest order bit to highest order bit) are the
   *                  boundary type, the next 8 bits are the subtype
   *                  and the last 8 bits are the forecast period.
   *                  A constant value is added back to the subtype
   *                  since it should have been subtracted off in
   *                  the creation of the data type to ensure that the
   *                  subtype value would fit into the 8 bits assigned
   *                  to it.
   */

  void parseDataType(const si32 data_type);


protected:

  typedef struct
  {
    char parse_string[80];
    int  parse_value;
  } parse_table_t;

  ///////////////////////
  // Protected members //
  ///////////////////////

  int _type;                    // Product type value as defined in
                                //   bdry_typedefs.h
  int _subtype;                 // Product subtype value as defined in
                                //   bdry_typedefs.h
  int _sequenceNum;             // Product counter
  int _groupId;                 // Group ID number
  DateTime _generateTime;       // Time of product generation
  DateTime _dataTime;           // Time of data used to create boundary
  DateTime _forecastTime;       // Time of forecast (extrapolation)
  DateTime _expireTime;         // Time product becomes invalid
  int _lineType;                // Line type value as defined in
                                //   bdry_typedefs.h (for COLIDE bdrys,
                                //   extraps)
  int _bdryId;                  // Boundary ID number
  double _motionDirection;      // Motion direction in degrees (all objects
                                //   move together). This value is given in
                                //   math coordinates (0 degrees along X axis,
                                //   increases in counter-clockwise direction).
  double _motionSpeed;          // Motion speed in m/s (all objects move
                                //   together)
  double _lineQualityValue;     // Quality (confidence) value (for COLIDE)
  double _lineQualityThreshold; // Quality threshold (for COLIDE)
  string _description;          // Label associated with the product.

  int _numSpareFloat;
  fl32 _spare_float[BDRY_SPARE_FLOAT_LEN];

  vector< BdryPolyline > _polylines;

  mutable MemBuf _memBuf;
  
  static const parse_table_t Type_table[];
  static const int Type_table_size;

  static const parse_table_t Subtype_table[];
  static const int Subtype_table_size;

  static const parse_table_t Line_type_table[];
  static const int Line_type_table_size;

  static const string UNKNOWN_VALUE_STRING;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /************************************************************************
   * _convertParseValue(): Convert a string value read in from the input
   *                       file into the corresponding integer value using
   *                       the given parse table.
   */

  static int _convertParseValue(const char *string,
				const parse_table_t *parse_table,
				const int parse_table_size);


  /************************************************************************
   * _convertStringValue(): Convert an integer value into the corresponding
   *                        string using the given parse table.
   */

  static string _convertStringValue(const int parse_value,
				    const parse_table_t *parse_table,
				    const int parse_table_size);
  

  /************************************************************************
   * _getExtrapolatedProdType(): Parse the product type line to see if
   *                             it's an extrapolated product.
   */

  static int _getExtrapolatedProdType(const char *type_string);


  /************************************************************************
   * _getExtrapolatedProdTypeString(): Convert an extrapolated product type
   *                                   to a string.
   */

  static string _getExtrapolatedProdTypeString(const int prod_type);
  

  /************************************************************************
   * _lineType2String(): Converts the line type value to a string.
   */

  static string _lineType2String(const int line_type);
  

  /************************************************************************
   * _lineTypeString2LineType(): Returns the integer line type
   *                             for a given line type string.
   */

  static int _lineTypeString2LineType(const char *line_type_string);
  static int _lineTypeString2LineType(const string &line_type_string);


  /************************************************************************
   * _spdbProductFromBE(): Convert a boundary product from big-endian
   *                       format to native format.
   */

  static void _spdbProductFromBE(BDRY_spdb_product_t &prod);
  

  /************************************************************************
   * _spdbProductToBE(): Convert a boundary product from native format
   *                     to big-endian format. 
   */

  void _spdbProductToBE(BDRY_spdb_product_t &prod);


  /************************************************************************
   * _subtype2String(): Convert the given subtype to a string.
   */

  static string _subtype2String(const int subtype);
  

  /************************************************************************
   * _subtypeString2Subtype(): Returns the integer subtype for a
   *                          given subtype string.
   */

  static int _subtypeString2Subtype(const char *subtype_string);


  /************************************************************************
   * _type2String(): Returns the string associated with a given boundary
   *                 type.
   */

  static string _type2String(const int type);
  

  /************************************************************************
   * _typeString2Type(): Returns the integer type for a given type
   *                    string.
   */

  static int _typeString2Type(const char *type_string);


};


#endif
