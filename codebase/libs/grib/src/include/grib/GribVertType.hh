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
/////////////////////////////////////////////
// GribVertType - Main class for manipulating GRIB files.
//
////////////////////////////////////////////

#ifndef GribVertType_HH
#define GribVertType_HH

#include <cstdio>
#include <string>

#include <dataport/port_types.h>

using namespace std;

class GribVertType
{

public:

  // Vertical level types -- See WMO Code Table 3

  typedef enum
  {
    SURFACE = 1,
    CLOUD_BASE = 2,
    CLOUD_TOPS = 3,
    ZERO_ISOTHERM = 4,
    ADIABATIC_CONDENSATION = 5,
    MAX_WIND = 6,
    TROPOPAUSE = 7,
    NOMINAL_ATM_TOP = 8,
    SEA_BOTTOM = 9,

    ISOTHERMAL = 20,

    ISOBARIC = 100,
    BETWEEN_ISOBARIC = 101,
    MEAN_SEA_LEVEL = 102,
    ALTITUDE_ABOVE_MSL = 103,
    BETWEEN_ALT_ABOVE_MSL = 104,
    HEIGHT_ABOVE_GROUND = 105,
    BETWEEN_HT_ABOVE_GROUND = 106,
    SIGMA = 107,
    BETWEEN_SIGMA = 108,
    HYBRID = 109,
    BETWEEN_HYBRID = 110,
    DEPTH_BELOW_LAND_SURFACE = 111,
    BETWEEN_DEPTH = 112,
    ISENTROPIC = 113,
    BETWEEN_ISENTROPIC = 114,
    PRESSURE_DIFF_GROUND_TO_LEVEL = 115,
    BETWEEN_PRS_DIFF = 116,
    POTENTIAL_VORTICITY = 117,
    
    ETA = 119,
    BETWEEN_ETA = 120,
    BETWEEN_ISOBARIC_HP = 121,
  
    HEIGHT_ABOVE_GROUND_HP = 125,

    BETWEEN_SIGMA_HP = 128,

    BETWEEN_ISOBARIC_MP = 141,

    DEPTH_BELOW_SEA_LEVEL = 160,

    ENTIRE_ATMOSPHERE = 200,
    ENTIRE_OCEAN = 201,

    // Not in GRIB documentation -- NCEP specific???

    UNKNOWN_10 = 10,

    FREEZING_LEVEL = 204,
    GCBL = 206,			// grid scale cloud bottom level
    GCTL = 207,			// grid scale cloud top level
    UNKNOWN_211 = 211,
    UNKNOWN_212 = 212,
    UNKNOWN_213 = 213,
    UNKNOWN_214 = 214,
    UNKNOWN_222 = 222,
    UNKNOWN_223 = 223,
    UNKNOWN_224 = 224,
    UNKNOWN_232 = 232,
    UNKNOWN_233 = 233,
    UNKNOWN_234 = 234,
    UNKNOWN_242 = 242,
    CONV_CLD_TOP = 243,
    UNKNOWN_244 = 244,
    LLTW = 245,                 // lowest level of the wet bulb zero
    MAX_EQ_THETA_PRESSURE = 246,
    EQUILIBRIUM_LEVEL_HEIGHT = 247,
    SCGL = 248,			// shallow convective cloud bottom level
    SCTL = 249,			// shallow convective cloud top level
    DCBL = 251,			// deep convective cloud bottom level
    DCTL = 252			// deep convective cloud top level
  } vert_type_t;

  // Constructors/Destructors

  GribVertType(const vert_type_t vert_type = SURFACE,
	       const int level_value = 0,
	       const int level_value_bottom = 0);
  virtual ~GribVertType();
  

  // Access methods

  inline vert_type_t getLevelType() const { return _vertType; }
  inline bool isSingleLevelValue() const { return _singleLevelValue; }
  inline int getLevelValue() const { return _levelValue; }
  inline int getLevelValueTop() const { return _levelValue; }
  inline int getLevelValueBottom() const { return _levelValueBottom; }
  inline void getLevelValues(int &level_value_top,
			     int &level_value_bottom) const
  {
    level_value_top = _levelValue;
    level_value_bottom = _levelValueBottom;
  }

  void set(const vert_type_t vert_type = SURFACE,
	   const int level_value = 0, const int level_value_bottom = 0);

  void set(const ui08 octet9, const ui08 octet10,
	   const ui08 octet11, const int octet10_11);
  
  // Print methods

  string vertType2String() const;
  static string vertType2String(vert_type_t vert_type);
  
  void print(FILE *stream) const;
  void print(ostream &stream) const;
  static void print(FILE *stream, vert_type_t vert_type,
		    const bool is_single_level_value, const int level_val,
		    const int level_val_top, const int level_val_bottom);
  static void print(ostream &stream, vert_type_t vert_type,
		    const bool is_single_level_value, const int level_val,
		    const int level_val_top, const int level_val_bottom);

  
private:
  
  vert_type_t _vertType;
  bool _singleLevelValue;
  int _levelValue;
  int _levelValueBottom;
  
};

#endif
