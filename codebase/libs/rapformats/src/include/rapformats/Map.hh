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
 *   $Date: 2018/10/13 23:22:11 $
 *   $Id: Map.hh,v 1.8 2018/10/13 23:22:11 dixon Exp $
 *   $Revision: 1.8 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * Map.hh: class representing a map.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 1999
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef Map_HH
#define Map_HH

#include <string>
#include <vector>

#include <cstdio>

#include <rapformats/MapIconDef.hh>
#include <rapformats/MapObject.hh>

using namespace std;


class Map
{
 public:

  // Strings used for map objects in map files.

  static const string MAP_NAME_STRING;
  static const string TRANSFORM_STRING;
  static const string PROJECTION_STRING;
  static const string ICONDEF_STRING;
  static const string ICON_STRING;
  static const string POLYLINE_STRING;
  static const string LABEL_STRING;
  static const string SIMPLE_LABEL_STRING;

  // Constructor

  Map(void);
  
  // Destructor

  ~Map(void);
  

  //////////////////////////
  // Input/Output methods //
  //////////////////////////

  // Read the map information from the given file stream.

  bool read(FILE *stream);
  

  // Write the map information to the given file stream.

  void write(FILE *stream) const;
  

  ////////////////////
  // Access methods //
  ////////////////////

  // Add the indicated object to the map.  Note that the pointer to the
  // MapObject is what is saved in the map, so the memory used by that
  // pointer becomes the responsibility of the Map object after this call.

  inline void addObject(MapObject *object)
  {
    _objectList.push_back(object);
  }
  
  // Get the number of objects in the map

  inline size_t getNObjects() const
  {
    return _objectList.size();
  }
  
  // Get a pointer to the indicated object

  inline MapObject *getObject(const size_t index)
  {
    if (index >= _objectList.size())
      return 0;
    
    return _objectList[index];
  }
  
 protected:

  // The name of the map.

  string _mapName;
  
  // The list of objects in the map.

  vector< MapObject* > _objectList;
  
  // The list of defined icons for the map.

  vector< MapIconDef > _iconList;
  
  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("Map");
  }
  
  // Clear out the current map information

  void _clear();
  
};


#endif
