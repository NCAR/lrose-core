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

/************************************************************************
 * PjgTypes.hh: class defining types used by the Pjg classes.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2002
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef PjgTypes_hh
#define PjgTypes_hh

#include <string>

using namespace std;

class PjgTypes
{

public:

  //////////////////
  // Public types //
  //////////////////

  typedef enum
  {
    PROJ_INVALID = -1,
    PROJ_FLAT = 0,
    PROJ_POLAR_RADAR,
    PROJ_LC1,
    PROJ_LC2,
    PROJ_LATLON,
    PROJ_STEREOGRAPHIC,
    PROJ_MERCATOR,
    PROJ_POLAR_STEREO,
    PROJ_POLAR_ST_ELLIP,
    PROJ_CYL_EQUIDIST,
    PROJ_RADIAL,
    PROJ_OBLIQUE_STEREO,
    PROJ_TRANS_MERCATOR,
    PROJ_AZIM_EQUIDIST,
    PROJ_ALBERS,
    PROJ_LAMBERT_CONF,
    PROJ_LAMBERT_AZIM,
    PROJ_VERT_PERSP,
    PROJ_UNKNOWN = 99
  } proj_type_t;
  
  ////////////////////////////////////////////////
  // differentiates between polar stereographic
  // with tangent point at North and South poles
  //
  typedef enum {
    POLE_NORTH = 0,
    POLE_SOUTH = 1
  } pole_type_t;

  
  ///////////////////////////
  // Public static methods //
  ///////////////////////////

  static const string proj2string(const proj_type_t proj_type)
  {
    switch(proj_type)
    {
    case PROJ_INVALID :
      return "PROJ_INVALID";
      
    case PROJ_FLAT :
      return "PROJ_FLAT";
      
    case PROJ_POLAR_RADAR :
      return "PROJ_POLAR_RADAR";
      
    case PROJ_LC1 :
      return "PROJ_LC1";
      
    case PROJ_LC2 :
      return "PROJ_LC2";
      
    case PROJ_LATLON :
      return "PROJ_LATLON (Cyl. equidist.)";
      
    case PROJ_STEREOGRAPHIC :
      return "PROJ_STEREOGRAPHIC";
      
    case PROJ_MERCATOR :
      return "PROJ_MERCATOR";
      
    case PROJ_POLAR_STEREO :
      return "PROJ_POLAR_STEREO";
      
    case PROJ_POLAR_ST_ELLIP :
      return "PROJ_POLAR_ST_ELLIP";
      
    case PROJ_CYL_EQUIDIST :
      return "PROJ_CYL_EQUIDIST (LatLon)";
      
    case PROJ_RADIAL :
      return "PROJ_RADIAL";
      
    case PROJ_OBLIQUE_STEREO :
      return "PROJ_OBLIQUE_STEREO";
      
    case PROJ_TRANS_MERCATOR :
      return "PROJ_TRANS_MERCATOR";
      
    case PROJ_AZIM_EQUIDIST :
      return "PROJ_AZIM_EQUIDIST (Flat)";
      
    case PROJ_ALBERS :
      return "PROJ_ALBERS";
      
    case PROJ_LAMBERT_CONF :
      return "PROJ_LAMBERT_CONF";
      
    case PROJ_LAMBERT_AZIM :
      return "PROJ_LAMBERT_AZIM";
      
    case PROJ_VERT_PERSP :
      return "PROJ_VERT_PERSP (satellite)";
      
    case PROJ_UNKNOWN :
      return "PROJ_UNKNOWN";
    } /* endswitch - proj_type */

    return "PROJ_INVALID";
  }
  
};

#endif
