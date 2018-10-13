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
//   $Id: MapSimpleLabel.cc,v 1.5 2018/10/13 23:22:11 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * MapSimpleLabel: class representing a simple label in a map.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 2006
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <iostream>
#include <cstdlib>

#include <rapformats/Map.hh>
#include <rapformats/MapSimpleLabel.hh>

using namespace std;

/**********************************************************************
 * Constructors
 */

MapSimpleLabel::MapSimpleLabel(void) :
  MapObject()
{
  // Do nothing
}


MapSimpleLabel::MapSimpleLabel(const MapPoint icon_location,
			       const string &label) :
  MapObject(),
  _location(icon_location),
  _label(label)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

MapSimpleLabel::~MapSimpleLabel(void)
{
  // Do nothing
}


//////////////////////////
// Input/Output methods //
//////////////////////////

/**********************************************************************
 * read() - Read the icon from the given file stream
 */

bool MapSimpleLabel::read(const char *header_line, FILE *stream)
{
  static const string method_name = "MapSimpleLabel::read()";
  
  cerr << "WARNING: " << method_name << endl;
  cerr << "This method not yet implemented" << endl;
  
  return false;
}

  
/**********************************************************************
 * write() - Write the icon to the given file stream.
 */

void MapSimpleLabel::write(FILE *stream) const
{
  static const string method_name = "MapSimpleLabel::write()";
  if (_label.empty())
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "Cannot write empty label" << endl;
  }
  else
  {
    fprintf(stream, "%s %f %f %s\n",
	    Map::SIMPLE_LABEL_STRING.c_str(),
	    _location.lat, _location.lon,
	    _label.c_str());
  }
  
}

  
/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
