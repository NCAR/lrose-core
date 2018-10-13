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
//   $Date: 2018/10/13 23:22:11 $
//   $Id: MapIcon.cc,v 1.8 2018/10/13 23:22:11 dixon Exp $
//   $Revision: 1.8 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Icon.cc: class representing an icon in a map.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <rapformats/Map.hh>
#include <rapformats/MapIcon.hh>
#include <rapformats/MapObject.hh>
#include <toolsa/str.h>

using namespace std;

/**********************************************************************
 * Constructors
 */

MapIcon::MapIcon(void) :
  MapObject()
{
  // Do nothing
}


MapIcon::MapIcon(MapIconDef *icon_def,
		 const MapPoint icon_location,
		 const string &label,
		 const int x_offset,
		 const int y_offset) :
  MapObject(),
  _iconDef(icon_def),
  _location(icon_location),
  _label(label),
  _labelOffset(x_offset, y_offset)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

MapIcon::~MapIcon(void)
{
  // Do nothing
}


//////////////////////////
// Input/Output methods //
//////////////////////////

/**********************************************************************
 * read() - Read the icon from the given file stream
 */

bool MapIcon::read(const char *line, FILE *stream)
{
  static const char *method_name = "MapIcon::read()";
  
  // Allocate the tokens needed for parsing the input lines

  _allocateTokens();
  
  // Extract the information from the line

  if (STRparse(line, _tokens, strlen(line),
	       MAX_TOKENS, MAX_TOKEN_LEN) != 6)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error parsing " << Map::ICON_STRING << " line: "
	 << line << endl;
    cerr << "Cannot read icon location" << endl;
    
    return false;
  }
  
  if ((string)_tokens[0] != Map::ICON_STRING)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error parsing " << Map::ICON_STRING << " line: "
	 << line << endl;
    cerr << "Line should start with \"" << Map::ICON_STRING << "\"" << endl;
    cerr << "Cannot read icon location" << endl;
    
    return false;
  }
  
  // Set the icon location values

  _iconDef = 0;
  _iconDefName = _tokens[1];
  _location.lat = atof(_tokens[2]);
  _location.lon = atof(_tokens[3]);
  _labelOffset.x_offset = atoi(_tokens[4]);
  _labelOffset.y_offset = atoi(_tokens[5]);
  
  return true;
}

  
/**********************************************************************
 * write() - Write the icon to the given file stream.
 */

void MapIcon::write(FILE *stream) const
{
  if (_label.empty())
    fprintf(stream, "%s %s %f %f %d %d\n",
	    Map::ICON_STRING.c_str(),
	    _iconDef->getName().c_str(),
	    _location.lat, _location.lon,
	    MapPointOffset::PEN_UP, MapPointOffset::PEN_UP);
  else
    fprintf(stream, "%s %s %f %f %d %d %s\n",
	    Map::ICON_STRING.c_str(),
	    _iconDef->getName().c_str(),
	    _location.lat, _location.lon,
	    _labelOffset.x_offset, _labelOffset.y_offset,
	    _label.c_str());
}

  
/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
