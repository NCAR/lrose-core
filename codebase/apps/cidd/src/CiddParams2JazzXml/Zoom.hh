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
 * @file Zoom.hh
 *
 * @class Zoom
 *
 * Class representing a zoom in a CIDD parameter file.
 *  
 * @date 10/5/2010
 *
 */

#ifndef Zoom_HH
#define Zoom_HH

#include <string>

using namespace std;


/** 
 * @class Zoom
 */

class Zoom
{
 public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor
   */

  Zoom();
  
  /**
   * @brief Destructor
   */

  virtual ~Zoom();
  

  ////////////////////
  // Public members //
  ////////////////////

  /**
   * @brief The zoom name.
   */

  string name;
  
  /**
   * @brief Flag indicating whether this is the default zoom.
   */

  bool isDefault;
  
  /**
   * @brief The minimum X value for the zoom.  This value is given in the
   *        units of the underlying display projection.
   */

  double minX;
  
  /**
   * @brief The maximum X value for the zoom.  This value is given in the
   *        units of the underlying display projection.
   */

  double maxX;
  
  /**
   * @brief The minimum Y value for the zoom.  This value is given in the
   *        units of the underlying display projection.
   */

  double minY;
  
  /**
   * @brief The maximum Y value for the zoom.  This value is given in the
   *        units of the underlying display projection.
   */

  double maxY;
  
};


#endif
