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
//   $Id: MapPolyline.cc,v 1.10 2016/03/03 18:45:39 dixon Exp $
//   $Revision: 1.10 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * MapPolyline.cc: class representing a polyline in a map.
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
#include <iostream>
#include <string.h>

#include <rapformats/Map.hh>
#include <rapformats/MapPolyline.hh>
#include <rapformats/MapObject.hh>
#include <toolsa/str.h>


/**********************************************************************
 * Constructor
 */

MapPolyline::MapPolyline(void) :
  MapObject(),
  _polylineName("unnamed")
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

MapPolyline::~MapPolyline(void)
{
  // Do nothing
}


//////////////////////////
// Input/Output methods //
//////////////////////////

/**********************************************************************
 * read() - Read the polyline from the given file stream
 */

bool MapPolyline::read(const char *header_line, FILE *stream)
{
  static const char *method_name = "MapPolyline::read()";
  
  // Allocate the tokens needed for parsing the input lines

  _allocateTokens();
  
  // Extract the information from the header line

  if (STRparse(header_line, _tokens, strlen(header_line),
	       MAX_TOKENS, MAX_TOKEN_LEN) < 3)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error parsing " << Map::POLYLINE_STRING << " header line: "
	 << header_line << endl;
    cerr << "Cannot read polyline" << endl;
    
    return false;
  }
  
  if ((string)_tokens[0] != Map::POLYLINE_STRING)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error parsing " << Map::POLYLINE_STRING << " header line: "
	 << header_line << endl;
    cerr << "Line should start with \"" << Map::POLYLINE_STRING << "\"" << endl;
    cerr << "Cannot read polyline" << endl;
    
    return false;
  }
  
  _polylineName = _tokens[1];
  
  int num_vertices = atoi(_tokens[2]);
  if (num_vertices < 2)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error parsing " << Map::POLYLINE_STRING << " header line: "
	 << header_line << endl;
    cerr << "Invalid number of vertices indicated: " << num_vertices << endl;
    cerr << "Cannot read polyline" << endl;
    
    return false;
  }
  
  ssize_t bytes_read;
  char *line = 0;
  size_t line_len = 0;
  
  while ((bytes_read = getline(&line, &line_len, stream)) >= 0)
  {
    // Remove any comments from the line

    for (size_t i = 0; i < line_len; ++i)
    {
      if (line[i] == '\0')
	break;
      if (line[i] == '#')
      {
	line[i] = '\0';
	break;
      }
    }
    
    // Tokenize the line.  If there aren't any tokens then this is
    // a blank line or a comment line so skip it.

    int num_tokens_read;
    
    if ((num_tokens_read = STRparse(line, _tokens, strlen(line),
				    MAX_TOKENS, MAX_TOKEN_LEN)) != 2)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing " << Map::POLYLINE_STRING
	   << " vertex line: " << line << endl;
      cerr << "Expected 2 tokens, found " << num_tokens_read << endl;
      cerr << "Cannot read polyline" << endl;
      
      return false;
    }
    
    MapPoint vertex(atof(_tokens[0]), atof(_tokens[1]));
    
    _vertexList.push_back(vertex);
    
    if ((int)_vertexList.size() == num_vertices)
      break;
  }
  
  // Make sure there were enough vertices in the file

  if ((int)_vertexList.size() != num_vertices)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading " << Map::POLYLINE_STRING << endl;
    cerr << "Expected " << num_vertices << " vertices, found "
	 << _vertexList.size() << " vertices" << endl;
    cerr << "Cannot read polyline" << endl;
    
    return false;
  }
  
  return true;
}

  
/**********************************************************************
 * write() - Write the polyline to the given file stream.
 */

void MapPolyline::write(FILE *stream) const
{
  fprintf(stream, "%s %s %d\n",
	  Map::POLYLINE_STRING.c_str(), _polylineName.c_str(),
	  (int)(_vertexList.size()));
  
  vector< MapPoint >::const_iterator vertex_iterator;

  for (vertex_iterator = _vertexList.begin();
       vertex_iterator != _vertexList.end();
       vertex_iterator++)
    {
      fprintf(stream, "%f %f\n", vertex_iterator->lat, vertex_iterator->lon);
    }
  
}

  
/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
