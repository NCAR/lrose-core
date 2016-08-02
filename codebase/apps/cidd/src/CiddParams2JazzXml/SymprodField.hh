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
 * @file SymprodField.hh
 *
 * @class SymprodField
 *
 * Class representing a symprod field in a CIDD parameter file.
 *  
 * @date 10/6/2010
 *
 */

#ifndef SymprodField_HH
#define SymprodField_HH

#include <string>

using namespace std;


/** 
 * @class SymprodField
 */

class SymprodField
{
 public:

  typedef enum
  {
    RENDER_ALL,
    RENDER_ALL_VALID,
    RENDER_VALID_IN_LAST_FRAME,
    RENDER_LATEST_IN_FRAME,
    RENDER_LATEST_IN_LOOP,
    RENDER_FIRST_BEFORE_FRAME_TIME,
    RENDER_FIRST_BEFORE_DATA_TIME,
    RENDER_FIRST_AFTER_DATA_TIME,
    RENDER_ALL_BEFORE_DATA_TIME,
    RENDER_ALL_AFTER_DATA_TIME,
    RENDER_GET_VALID,
    RENDER_GET_VALID_AT_FRAME_TIME
  } render_type_t;
  
    
  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor
   */

  SymprodField();
  
  /**
   * @brief Destructor
   */

  virtual ~SymprodField();
  

  ////////////////////
  // Public members //
  ////////////////////

  /**
   * @brief Menu label.
   */

  string menuLabel;
  
  /**
   * @brief URL.
   */

  string url;
  
  /**
   * @brief Data type.
   */

  int dataType;
  
  /**
   * @brief Render type.
   */

  render_type_t renderType;
  
  /**
   * @brief Flag indicating whether this product is on by default.
   */

  bool onByDefault;
  
  /**
   * @brief Time span for looking for product before the data time, in seconds.
   */

  int searchBefore;
  
  /**
   * @brief Time span for looking for product after the data time, in seconds.
   */

  int searchAfter;
  
  /**
   * @brief Text off threshold.
   */

  double textOffThreshold;
  
  /**
   * @brief Request data on zoom flag.
   */

  bool requestDataOnZoom;
  
  /**
   * @brief Request data on vertical level change flag.
   */

  bool requestDataOnVertChange;
  

};


#endif
