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
 *
 * @file WindField.hh
 *
 * @class WindField
 *
 * Class representing a wind field in a CIDD parameter file.
 *  
 * @date 10/5/2010
 *
 */

#ifndef WindField_HH
#define WindField_HH

#include <string>

using namespace std;


/** 
 * @class WindField
 */

class WindField
{
 public:

  typedef enum
  {
    MARKER_ARROWS,
    MARKER_TUFT,
    MARKER_BARB,
    MARKER_VECTOR,
    MARKER_TICKVECTOR,
    MARKER_LABELEDBARB,
    MARKER_METBARB,
    MARKER_BARB_SH,
    MARKER_LABELEDBARB_SH
  } marker_type_t;
  
    
  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor
   */

  WindField();
  
  /**
   * @brief Destructor
   */

  virtual ~WindField();
  

  ////////////////////
  // Public members //
  ////////////////////

  /**
   * @brief Legend label for the field.
   */

  string legendLabel;
  
  /**
   * @brief URL for the field.
   */

  string url;
  
  /**
   * @brief U field name.
   */

  string uFieldName;
  
  /**
   * @brief V field name.
   */

  string vFieldName;
  
  /**
   * @brief W field name.  Set to "" if no W field.
   */

  string wFieldName;
  
  /**
   * @brief Units.
   */

  string units;
  
  /**
   * @brief Line width.
   */

  int lineWidth;
  
  /**
   * @brief Flag indicating whether this field is on by default.
   */

  bool isOn;
  
  /**
   * @brief Marker type used for rendering this field.
   */

  marker_type_t markerType;
  
  /**
   * @brief Color.
   */

  string color;
  

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Replace underscores with spaces in the appropriate strings.
   */

  void replaceUnderscores();
  

};


#endif
