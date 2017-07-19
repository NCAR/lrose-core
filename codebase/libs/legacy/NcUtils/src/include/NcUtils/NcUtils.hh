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
// NcUtils.hh
//
// Definitions for NcUtils
//
// NetCDF utility library
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2016
//
///////////////////////////////////////////////////////////////

#ifndef NcUtils_HH
#define NcUtils_HH

#include <string>
#include <sys/types.h>
using namespace std;

///////////////////////////////////////////////////////////////
/// CLASS NcUtils
///
/// Utility class, type definitions.

class NcUtils {

public:

  ///////////////
  // enumerations

  /// data encoding type

  typedef enum {

    SI08, ///< signed 8-bit int
    SI16, ///< signed 16-bit int
    SI32, ///< signed 32-bit int
    UI08, ///< unsigned 8-bit int
    UI16, ///< unsigned 16-bit int
    UI32, ///< unsigned 32-bit int
    FL32, ///< 32-bit IEEE float
    FL64, ///< 64-bit IEEE float
    ASIS  ///< leave it as it is
    
  } DataType_t;

  // portable data types
  
  typedef char si08; ///< portable unsigned 8-bit integer
  typedef unsigned char ui08; ///< portable unsigned 8-bit integer
  typedef int16_t si16; ///< portable signed 16-bit integer
  typedef u_int16_t ui16; ///< portable unsigned 16-bit integer
  typedef int32_t si32; ///< portable signed 32-bit integer
  typedef u_int32_t ui32; ///< portable unsigned 32-bit integer
  typedef int64_t si64; ///< portable signed 32-bit integer
  typedef u_int64_t ui64; ///< portable unsigned 32-bit integer
  typedef float fl32; ///< portable 32-bit IEEE float
  typedef double fl64; ///< portable 64-bit IEEE float

  /// \name Missing values for metadata:
  //@{

  static double missingMetaDouble;
  static float missingMetaFloat;
  static int missingMetaInt;
  static char missingMetaChar;

  //@}

  /// \name Missing values for field data, by type:
  //@{

  static fl64 missingFl64;
  static fl32 missingFl32;
  static si32 missingSi32;
  static si16 missingSi16;
  static si08 missingSi08;
  
  //@}

  /// speed of light

  static const double LIGHT_SPEED;

  /// angle conversion

  static const double DegToRad;
  static const double RadToDeg;

  //@}

  /// Get byte width for given data type.

  static int getByteWidth(DataType_t dtype);

  /// get string representation of data type

  static string dataTypeToStr(DataType_t dtype);

  /// Add integer to error string.

  static void addErrInt(string &errStr,
                        string label, int iarg,
                        bool cr = true);

  /// Add double to error string.

  static void addErrDbl(string &errStr,
                        string label, double darg,
                        string format, bool cr = true);

  /// Add string to error string.

  static void addErrStr(string &errStr,
                        string label, string strarg = "",
                        bool cr = true);

  /// Safely make string from char text.
  /// Ensure null termination.
  
  static string makeString(const char *text, int len);
  
  /// Safely print char text.
  /// Ensure null termination.
  
  static void printString(const string &label, const char *text,
                          int len, ostream &out);

  /// replace spaces in a string with underscores

  static void replaceSpacesWithUnderscores(string &str);

  /// compute sin and cos together

  static void sincos(double radians, double &sinVal, double &cosVal);

  /// \name set missing values
  //@{

  static void setMissingMetaDouble(double val) { missingMetaDouble = val; }
  static void setMissingMetaFloat(float val) { missingMetaFloat = val; }
  static void setMissingMetaInt(int val) { missingMetaInt = val; }
  static void setMissingMetaChar(char val) { missingMetaChar = val; }
  static void setMissingFl64(fl64 val) { missingFl64 = val; }
  static void setMissingFl32(fl32 val) { missingFl32 = val; }
  static void setMissingSi32(si32 val) { missingSi32 = val; }
  static void setMissingSi16(si16 val) { missingSi16 = val; }
  static void setMissingSi08(si08 val) { missingSi08 = val; }

  //@}

  /// \name get missing values
  //@{

  static double getMissingMetaDouble() { return missingMetaDouble; }
  static float getMissingMetaFloat() { return missingMetaFloat; }
  static int getMissingMetaInt() { return missingMetaInt; }
  static char getMissingMetaChar() { return missingMetaChar; }
  static fl64 getMissingFl64() { return missingFl64; }
  static fl32 getMissingFl32() { return missingFl32; }
  static si32 getMissingSi32() { return missingSi32; }
  static si16 getMissingSi16() { return missingSi16; }
  static si08 getMissingSi08() { return missingSi08; }

  //@}

};

#endif
