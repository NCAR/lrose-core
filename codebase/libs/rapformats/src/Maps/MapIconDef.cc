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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/03 18:45:39 $
//   $Id: MapIconDef.cc,v 1.10 2016/03/03 18:45:39 dixon Exp $
//   $Revision: 1.10 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * MapIconDef.cc: class representing an icon definition in a map.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <string>
#include <vector>

#include <cstdio>
#include <cstdlib>

#include <rapformats/Map.hh>
#include <rapformats/MapIconDef.hh>

const double MapIconDef::PEN_UP = -1000.0;

/**********************************************************************
 * Constructors
 */

MapIconDef::MapIconDef(void) :
  MapObject(),
  _iconName("")
{
  // Do nothing
}


MapIconDef::MapIconDef(string name,
		       vector< MapPointOffset > point_list) :
  _iconName(name)
{
  _pointList = point_list;
}


/**********************************************************************
 * Destructor
 */

MapIconDef::~MapIconDef(void)
{
  // Do nothing
}


//////////////////////////
// Input/Output Methods //
//////////////////////////

/**********************************************************************
 * read() - Read the icon definition information from the given file
 *          stream.  Clears any previous information in the object.
 */

bool MapIconDef::read(const char *header_line, FILE *stream)
{
  const char *routine_name = "read()";
  
  fprintf(stderr, "WARNING:  %s::%s\n",
	  _className(), routine_name);
  fprintf(stderr, "This method not yet implemented\n");
  
  return false;
}


/**********************************************************************
 * write() - Write the icon definition information to the given file
 *           stream.
 */

void MapIconDef::write(FILE *stream) const
{
  // Print out the header.  Note that we add 1 to the point list size
  // because we force a pen-up at the end of the icon def

  fprintf(stream, "%s %s %d\n",
	  Map::ICONDEF_STRING.c_str(), _iconName.c_str(),
	  (int)(_pointList.size() + 1));
  
  // Output the points in the icon def
  
  vector< MapPointOffset >::const_iterator point;

  for (point = _pointList.begin();
       point != _pointList.end();
       ++point)
    {
      fprintf(stream, "%d %d\n", point->x_offset, point->y_offset);
    } /* endfor - point */
  
  // Make sure we end with a pen-up

  fprintf(stream, "%d %d\n", MapPointOffset::PEN_UP, MapPointOffset::PEN_UP);
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
