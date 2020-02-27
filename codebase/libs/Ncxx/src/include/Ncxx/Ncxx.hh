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
//////////////////////////////////////////////////////////////////////
//  Ncxx C++ classes for NetCDF4
//
//  Copied from code by:
//
//    Lynton Appel, of the Culham Centre for Fusion Energy (CCFE)
//    in Oxfordshire, UK.
//    The netCDF-4 C++ API was developed for use in managing
//    fusion research data from CCFE's innovative MAST
//    (Mega Amp Spherical Tokamak) experiment.
// 
//  Offical NetCDF codebase is at:
//
//    https://github.com/Unidata/netcdf-cxx4
//
//  Modification for LROSE made by:
//
//    Mike Dixon, EOL, NCAR
//    P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//  The base code makes extensive use of exceptions.
//
//  December 2016
//
//////////////////////////////////////////////////////////////////////

#ifndef Ncxx_HH
#define Ncxx_HH

// global include file for the Ncxx C++ API

#include <string>

#if __cplusplus >= 201103L
#include <cstdint>
#else
#include <sys/types.h>
#endif

using namespace std;

extern "C" {
#include <netcdf.h>
}

#include <Ncxx/NcxxAtt.hh>
#include <Ncxx/NcxxByte.hh>
#include <Ncxx/NcxxChar.hh>
#include <Ncxx/NcxxCheck.hh>
#include <Ncxx/NcxxCompoundType.hh>
#include <Ncxx/NcxxDim.hh>
#include <Ncxx/NcxxDouble.hh>
#include <Ncxx/NcxxEnumType.hh>
#include <Ncxx/NcxxException.hh>
#include <Ncxx/NcxxFile.hh>
#include <Ncxx/NcxxFloat.hh>
#include <Ncxx/NcxxGroup.hh>
#include <Ncxx/NcxxGroupAtt.hh>
#include <Ncxx/NcxxInt.hh>
#include <Ncxx/NcxxInt64.hh>
#include <Ncxx/NcxxOpaqueType.hh>
#include <Ncxx/NcxxShort.hh>
#include <Ncxx/NcxxString.hh>
#include <Ncxx/NcxxType.hh>
#include <Ncxx/NcxxUbyte.hh>
#include <Ncxx/NcxxUint.hh>
#include <Ncxx/NcxxUint64.hh>
#include <Ncxx/NcxxUshort.hh>
#include <Ncxx/NcxxVar.hh>
#include <Ncxx/NcxxVarAtt.hh>
#include <Ncxx/NcxxVlenType.hh>

class Ncxx {

public:

  // missing values for metadata:

  static double missingMetaDouble;
  static float missingMetaFloat;
  static int missingMetaInt;
  static char missingMetaUchar;
  static char missingMetaChar;

  // missing values

  static double missingDouble;
  static float missingFloat;
  static int missingInt;
  static unsigned char missingUchar;
  static char missingChar;
  
  // portable data types
  
#if __cplusplus >= 201103L
  typedef char si08;
  typedef unsigned char ui08;
  typedef int16_t si16;
  typedef uint16_t ui16;
  typedef int32_t si32;
  typedef uint32_t ui32;
  typedef int64_t si64;
  typedef uint64_t ui64;
  typedef float fl32;
  typedef double fl64;
#else
  typedef char si08;
  typedef unsigned char ui08;
  typedef int16_t si16;
  typedef u_int16_t ui16;
  typedef int32_t si32;
  typedef u_int32_t ui32;
  typedef int64_t si64;
  typedef u_int64_t ui64;
  typedef float fl32;
  typedef double fl64;
#endif

  // missing field values by portable type

  static fl64 missingFl64;
  static fl32 missingFl32;
  static si32 missingSi32;
  static si16 missingSi16;
  static si08 missingSi08;
  
  /// data encoding type

  typedef enum {

    SI08, ///< signed 8-bit int
    SI16, ///< signed 16-bit int
    SI32, ///< signed 32-bit int
    UI08, ///< unsigned 8-bit int
    UI16, ///< unsigned 16-bit int
    UI32, ///< unsigned 32-bit int
    FL32, ///< 32-bit IEEE float
    FL64  ///< 64-bit IEEE float
    
  } PortType_t;

  /// set missing values

  static void setMissingMetaDouble(double val) { missingMetaDouble = val; }
  static void setMissingMetaFloat(float val) { missingMetaFloat = val; }
  static void setMissingMetaInt(int val) { missingMetaInt = val; }
  static void setMissingMetaChar(char val) { missingMetaChar = val; }
  static void setMissingMetaUchar(unsigned char val) { missingMetaUchar = val; }

  static void setMissingDouble(double val) { missingDouble = val; }
  static void setMissingFloat(float val) { missingFloat = val; }
  static void setMissingInt(int val) { missingInt = val; }
  static void setMissingChar(char val) { missingChar = val; }
  static void setMissingUchar(unsigned char val) { missingUchar = val; }

  static void setMissingFl64(fl64 val) { missingFl64 = val; }
  static void setMissingFl32(fl32 val) { missingFl32 = val; }
  static void setMissingSi32(si32 val) { missingSi32 = val; }
  static void setMissingSi16(si16 val) { missingSi16 = val; }
  static void setMissingSi08(si08 val) { missingSi08 = val; }

  // get missing values

  static double getMissingMetaDouble() { return missingMetaDouble; }
  static float getMissingMetaFloat() { return missingMetaFloat; }
  static int getMissingMetaInt() { return missingMetaInt; }
  static char getMissingMetaChar() { return missingMetaChar; }
  static unsigned char getMissingMetaUchar() { return missingMetaUchar; }

  static double getMissingDouble() { return missingDouble; }
  static float getMissingFloat() { return missingFloat; }
  static int getMissingInt() { return missingInt; }
  static char getMissingChar() { return missingChar; }
  static unsigned char getMissingUchar() { return missingUchar; }

  static fl64 getMissingFl64() { return missingFl64; }
  static fl32 getMissingFl32() { return missingFl32; }
  static si32 getMissingSi32() { return missingSi32; }
  static si16 getMissingSi16() { return missingSi16; }
  static si08 getMissingSi08() { return missingSi08; }

  // get byte width of data type
  
  static int getByteWidth(nc_type nctype);

  // get byte width of data type

  static int getByteWidth(PortType_t ptype);

  // convert type enum to string
  
  static string ncTypeToStr(nc_type nctype);
  static string ncxxTypeToStr(NcxxType nctype);
  static string portTypeToStr(PortType_t ptype);
  static string ncErrToStr(int errtype);
  
  // strip redundant null from string
  
  static string stripNulls(const string &val);

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

};

#endif
  
