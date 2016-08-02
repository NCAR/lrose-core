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
 *   $Id: MapIconDef.hh,v 1.8 2016/03/03 19:23:53 dixon Exp $
 *   $Revision: 1.8 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * MapIconDef.hh: class representing an icon definition in a map.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 1999
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef MapIconDef_HH
#define MapIconDef_HH

#include <string>
#include <vector>

#include <cstdio>

#include <rapformats/MapObject.hh>
#include <rapformats/MapPointOffset.hh>

using namespace std;

class MapIconDef : public MapObject
{
 private:

  // Value representing a penup.  For penup locations, both the lat and
  // lon for the point are set to this value.

  static const double PEN_UP;
  
 public:

  // Constructors

  MapIconDef(void);
  MapIconDef(string name, vector< MapPointOffset > point_list);
  
  // Destructor

  ~MapIconDef(void);
  

  //////////////////////////
  // Input/Output methods //
  //////////////////////////

  // Read the icon definition information from the given file stream.
  // Clears any previous information in the object.

  bool read(const char *header_line, FILE *stream);
  
  // Write the icon definition information to the given file stream.

  void write(FILE *stream) const;
  

  ////////////////////
  // Access methods //
  ////////////////////

  // Get the icon name.

  inline string getName(void)
  {
    return _iconName;
  }
  
  // Get the point list.

  inline vector< MapPointOffset > getPointList(void)
  {
    return _pointList;
  }
  
 protected:

  // The name of the icon.

  string _iconName;
  
  // The list of points making up the icon.

  vector< MapPointOffset > _pointList;
  
 private:

  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("MapIconDef");
  }
  
};


#endif
