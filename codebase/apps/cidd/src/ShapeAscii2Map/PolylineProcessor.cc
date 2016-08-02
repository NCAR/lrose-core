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
//   $Date: 2016/03/07 18:28:25 $
//   $Id: PolylineProcessor.cc,v 1.3 2016/03/07 18:28:25 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * PolylineProcessor : Class for reading map information from an ASCII
 *                     shape file that just contains simple polylines.
 *
 * RAP, NCAR, Boulder CO
 *
 * Sept 2006
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <toolsa/str.h>

#include "PolylineProcessor.hh"

using namespace std;

/*********************************************************************
 * Constructor
 */

PolylineProcessor::PolylineProcessor(const int header_lines,
				     const bool debug_flag) :
  InputProcessor(header_lines, debug_flag),
  _state(BEGINNING_OF_POLYLINE),
  _polyline(0)
{
}


/*********************************************************************
 * Destructor
 */

PolylineProcessor::~PolylineProcessor()
{
}


/*********************************************************************
 * _readMap() - Read the input file and return the associated map.
 *
 * Returns 0 if there was an error generating the Map object.
 */

Map *PolylineProcessor::_readMap(FILE *input_file)
{
  static const string method_name = "PolylineProcessor::_readMap()";
  
  Map *map = new Map();
  
  while (fgets(_inputLine, BUFSIZ, input_file)!= 0)
  {
    switch (_state)
    {
    case BEGINNING_OF_POLYLINE :
      // Skip this line and start reading the polyline
      _polyline = new MapPolyline();
      _state = READING_POLYLINE;
      break;
      
    case READING_POLYLINE :
      if (STRequal(_inputLine, "END"))
      {
	map->addObject(_polyline);
	_state = BEGINNING_OF_POLYLINE;
      }
      else
      {
	float lat, lon;
	if (sscanf(_inputLine, "%f %f", &lon, &lat) != 2)
	{
	  cerr << "ERROR : " << method_name << endl;
	  cerr << "Error reading vertex location from line: <"
	       << _inputLine << ">" << endl;
	  cerr << "*** Skipping line ***" << endl;
	}
	else
	{
	  MapPoint point(lat, lon);
	  _polyline->addVertex(point);
	}
      }
      break;
      
    } /* endswitch - _state */
    
  }
  
  return map;
}
