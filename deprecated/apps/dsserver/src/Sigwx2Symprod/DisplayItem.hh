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
 * @file DisplayItem.hh
 *
 * @class DisplayItem
 *
 * Base class for display items.
 *  
 * @date 10/10/2009
 *
 */

#ifndef DisplayItem_HH
#define DisplayItem_HH

#include <string>

#include <Spdb/Symprod.hh>

#include "BoundingBox.hh"

using namespace std;

/** 
 * @class DisplayItem
 */

class DisplayItem
{
 public:

  //////////////////
  // Public types //
  //////////////////

  typedef enum
  {
    TYPE_CYCLONE,
    TYPE_VOLCANO,
    TYPE_SANDSTORM,
    TYPE_RADIATION
  } type_t;
  

  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /**
   * @brief Constructor
   */

  DisplayItem(const type_t itype,
	      const double lat, const double lon,
	      const int debug_level = 0);
  

  /**
   * @brief Destructor
   */

  virtual ~DisplayItem(void);
  

  virtual void draw(Symprod &symprod,
		    const BoundingBox &bbox,
		    const int icon_size) const = 0;
  

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Debug level.
   */

  int _debugLevel;
  

  /**
   * @brief Type of item to display.
   */

  type_t _itype;
  

  /**
   * @brief Item latitude in degrees.
   */

  double _lat;
  

  /**
   * @brief Item longitude in degrees.
   */

  double _lon;
  

};


#endif
