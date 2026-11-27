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
 * @file SandstormDisplayItem.hh
 *
 * @class SandstormDisplayItem
 *
 * Class representing a sand storm display item.
 *  
 * @date 10/21/2009
 *
 */

#ifndef SandstormDisplayItem_HH
#define SandstormDisplayItem_HH

#include <string>

#include <Spdb/Symprod.hh>

#include "BoundingBox.hh"
#include "DisplayItem.hh"

using namespace std;

/** 
 * @class SandstormDisplayItem
 */

class SandstormDisplayItem : public DisplayItem
{
 public:

  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /**
   * @brief Constructor
   *
   * @param[in] lat Sand storm latitude.
   * @param[in] lon Sand storm longitude.
   * @param[in] debug_level Debug level.
   */

  SandstormDisplayItem(const double lat, const double lon,
		       const int debug_level);
  

  /**
   * @brief Destructor
   */

  virtual ~SandstormDisplayItem(void);
  

  virtual void draw(Symprod &symprod,
		    const BoundingBox &bbox,
		    const int icon_size) const;
  

protected:

};


#endif
