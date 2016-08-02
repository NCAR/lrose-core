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
 *   $Id: MapIcon.hh,v 1.7 2016/03/03 19:23:53 dixon Exp $
 *   $Revision: 1.7 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * MapIcon.hh: class representing an icon in a map.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 1999
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef MapIcon_HH
#define MapIcon_HH

#include <cstdio>

#include <rapformats/MapIconDef.hh>
#include <rapformats/MapPoint.hh>
#include <rapformats/MapPointOffset.hh>
#include <rapformats/MapObject.hh>

using namespace std;

class MapIcon : public MapObject
{
 public:

  // Constructors

  MapIcon(void);
  
  MapIcon(MapIconDef *icon_def,
	  const MapPoint icon_location,
	  const string &label = "",
	  const int x_offset = 0,
	  const int y_offset = 0);
  
  // Destructor

  ~MapIcon(void);
  

  //////////////////////////
  // Input/Output methods //
  //////////////////////////

  // Read the icon from the given file stream
  // NOTE: This method does not yet handle the icon label!

  bool read(const char *header_line, FILE *stream);
  
  // Write the icon to the given file stream.

  void write(FILE *stream) const;
  

  ////////////////////
  // Access methods //
  ////////////////////

  string getIconDefName() const
  {
    return _iconDefName;
  }
  
  MapIconDef *getIconDef() const
  {
    return _iconDef;
  }
  
  MapPoint getLocation() const
  {
    return _location;
  }
  
  string getLabel() const
  {
    return _label;
  }
  
  MapPointOffset getLabelOffset() const
  {
    return _labelOffset;
  }
  
 protected:

 private:

  // The icon definition.

  MapIconDef *_iconDef;
  string _iconDefName;
  
  // The icon location.

  MapPoint _location;
  
  // The label information

  string _label;
  MapPointOffset _labelOffset;
  
  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("MapIcon");
  }
  
};


#endif
