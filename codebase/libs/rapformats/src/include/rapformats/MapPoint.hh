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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/03 19:23:53 $
 *   $Id: MapPoint.hh,v 1.7 2016/03/03 19:23:53 dixon Exp $
 *   $Revision: 1.7 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * MapPoint.hh: class representing a point on a map.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 1999
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef MapPoint_HH
#define MapPoint_HH

#include <cstdio>

using namespace std;

class MapPoint
{
  friend class MapPolyline;
  
 private:

  // Value representing a penup.  For penup locations, both the lat and
  // lon for the point are set to this value.

  static const double PEN_UP;
  
 public:

  // Constructor

  MapPoint(const double lat = PEN_UP, const double lon = PEN_UP);
  
  // Destructor

  ~MapPoint(void);
  
  // The point's location

  double lat;
  double lon;
  

  //////////////////////////
  // Input/Output methods //
  //////////////////////////

  // Print the object to the given stream.

  void print(FILE *stream) const
  {
    fprintf(stream, "%s: lat = %f, lon = %f\n",
	    _className(), lat, lon);
  }
  
  ////////////////////
  // Access methods //
  ////////////////////

  // Determines whether the point contains a pen-up value.

  inline bool isPenup() const
  {
    return lat == PEN_UP && lon == PEN_UP;
  }
  
 protected:

 private:

  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("MapPoint");
  }
  
};


#endif
