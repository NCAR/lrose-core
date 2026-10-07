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
// RadxFm301Str.hh
//
// Base class containing
// strings for FM301-compliant radar data
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2026
//
///////////////////////////////////////////////////////////////
//
// Fm301 File classes will inherit both RadxFile and this class
//
///////////////////////////////////////////////////////////////

#ifndef RadxFm301Str_HH
#define RadxFm301Str_HH

#include <Radx/RadxNcfStr.hh>
#include <string>
using namespace std;

class RadxFm301Str : public RadxNcfStr

{
  
public:

  /// Constructor
  
  RadxFm301Str();
  
  /// Destructor
  
  virtual ~RadxFm301Str();

  // string constants
  
  string WmoCfProfile;

  // char * constants

  static constexpr const char* WMO__CF_PROFILE = "wmo__cf_profile";
  static constexpr const char* WMO__DATA_CATEGORY = "wmo__data_category";
  static constexpr const char* WMO__DATA_POLICY = "wmo__data_policy";
  static constexpr const char* WMO__ID = "wmo__id";
  static constexpr const char* WMO__ORIGINATING_CENTRE = "wmo__originating_centre";
  static constexpr const char* WMO__ORIGINATING_SUB_CENTRE = "wmo__originating_sub_centre";
  static constexpr const char* WMO__PARAMETER_NAME = "wmo__parameter_name";
  static constexpr const char* WMO__PARAMETER_URI = "wmo__parameter_uri";
  static constexpr const char* WMO__UPDATE_SEQUENCE_NUMBER = "wmo__update_sequence_number";
  static constexpr const char* WMO__WSI = "wmo__wsi";

private:

};

#endif
