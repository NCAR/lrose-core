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
 * @file GridField.hh
 *
 * @class GridField
 *
 * Class representing a grid field in a CIDD parameter file.
 *  
 * @date 9/24/2010
 *
 */

#ifndef GridField_HH
#define GridField_HH

#include <string>

using namespace std;


/** 
 * @class GridField
 */

class GridField
{
 public:

  typedef enum
  {
    RENDER_POLYGONS,
    RENDER_FILLED_CONTOURS,
    RENDER_DYNAMIC_CONTOURS,
    RENDER_LINE_CONTOURS
  } render_method_t;
  
    
  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor
   */

  GridField();
  
  /**
   * @brief Destructor
   */

  virtual ~GridField();
  

  ////////////////////
  // Public members //
  ////////////////////

  /**
   * @brief Legend name for the field.
   */

  string legendName;
  
  /**
   * @brief Button name for the field.
   */

  string buttonName;
  
  /**
   * @brief URL for the field.
   */

  string url;
  
  /**
   * @brief Field name
   */

  string fieldName;
  
  /**
   * @brief Name of the colorscale file.
   */

  string colorFile;
  
  /**
   * @brief Units for the field.
   */

  string units;
  
  /**
   * @brief Contour low value.
   */

  double contourLow;
  
  /**
   * @brief Contour high value.
   */

  double contourHigh;
  
  /**
   * @brief Contour interval value.
   */

  double contourInterval;
  
  /**
   * @brief Render method.
   */

  render_method_t renderMethod;
  
  /**
   * @brief Composite mode flag.
   */

  bool compositeMode;
  
  /**
   * @brief Auto scale flag.
   */

  bool autoScale;
  
  /**
   * @brief Flag indicating whether the field should be displayed in the menu.
   */

  bool displayInMenu;
  
  /**
   * @brief Background render flag.
   */

  bool backgroundRender;
  

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Replace underscores with spaces in the appropriate strings.
   */

  void replaceUnderscores();
  

  /**
   * @brief Set the URL and field name from the given string taken from the
   *        CIDD parameter file.  The URL is separated from the field name
   *        by an '&' character.
   *
   * @param[in] token The token string from the CIDD parameter file.
   *
   * @return Returns true on success, false on failure.
   */

  bool setUrlAndFieldName(const string &token);
  

 private:

};


#endif
