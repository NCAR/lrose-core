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
 * @file MapsParams.cc
 *
 * @class MapsParams
 *
 * Class controlling access to the MAPS section of the CIDD parameter
 * file.
 *  
 * @date 9/28/2010
 *
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <toolsa/str.h>

#include "MapsParams.hh"

using namespace std;

// Globals

const size_t MapsParams::PARSE_LINE_LEN = 2048;
const int MapsParams::MAX_TOKENS = 32;
const int MapsParams::MAX_TOKEN_LEN = 1024;

/**********************************************************************
 * Constructor
 */

MapsParams::MapsParams () :
  ParamSection(),
  _lineBuffer(0),
  _tokens(0)
{
}


/**********************************************************************
 * Destructor
 */

MapsParams::~MapsParams(void)
{
  delete [] _lineBuffer;

  if (_tokens != 0)
  {
    for (int i = 0; i < MAX_TOKENS; ++i)
      delete [] _tokens[i];
    delete [] _tokens;
  }
  
}
  

/**********************************************************************
 * init()
 */

bool MapsParams::init(const MainParams &main_params,
		      const char *params_buf, const size_t buf_size)
{
  static const string method_name = "MapsParams::init()";
  
  // Allocate space for the parsing buffers

  _lineBuffer = new char[PARSE_LINE_LEN];
  
  _tokens = new char*[MAX_TOKENS];
  for (int i = 0; i < MAX_TOKENS; ++i)
    _tokens[i] = new char[MAX_TOKEN_LEN];
  
  // Pull out the MAPS section of the parameters buffer

  const char *param_text;
  long param_text_line_no = 0;
  long param_text_len = 0;
  
  if ((param_text = _findTagText(params_buf, "MAPS",
				 &param_text_len, &param_text_line_no)) == 0)
    return false;
  
  if (param_text == 0 || param_text_len <= 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Couldn't find MAPS section in CIDD parameter file" << endl;
    
    return false;
  }
  
  if (!_loadOverlayInfo(main_params,
			param_text, param_text_len, param_text_line_no))
    return false;
  
  return true;
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _loadOverlayInfo()
 */

bool MapsParams::_loadOverlayInfo(const MainParams &main_params,
				  const char *param_buf,
				  long param_buf_len, long line_no)
{
  static const string method_name = "MapsParams::_loadOverlayInfo()";
  
  // Read all the lines in the information file

  int total_len = 0;
  const char *start_ptr = param_buf;
  const char *end_ptr;

  while ((end_ptr = strchr(start_ptr,'\n')) != 0 &&
	 total_len < param_buf_len)
  {
    int len = (end_ptr - start_ptr) + 1;
    STRcopy(_lineBuffer, start_ptr, len);

    _parseLine(main_params, _lineBuffer, len);
    
    total_len += len + 1;
    start_ptr = end_ptr + 1; // Skip past the newline
    ++line_no;
  }

  return true;
}


/**********************************************************************
 * _parseLine()
 */

void MapsParams::_parseLine(const MainParams &main_params,
			     const char *parse_line, const long line_len)
{
  static const string method_name = "MapsParams::_parseLine()";
  
  // Skip over blank, short or commented lines

  if (line_len <= 20 || *parse_line == '#')
    return;
  
//    usubstitute_env(_lineBuffer, BUFSIZ);
         
  MapField map_field;
  
  // Parse the line into tokens

  int num_fields;

  if ((num_fields = STRparse(_lineBuffer, _tokens, line_len,
			     MAX_TOKENS, MAX_TOKEN_LEN)) < 7)
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "Cannot parse MAP line: " << _lineBuffer << endl;
    cerr << "Expected 7 tokens, found " << num_fields << " tokens" << endl;
    cerr << "--- Skipping line ---" << endl;
      
    return;
  }
    
  map_field.mapCode = _tokens[0];
  map_field.controlLabel = _tokens[1];
  map_field.filePath = main_params.getMapFileSubdir() + "/" + _tokens[2];

  int on_state_line_width = atoi(_tokens[3]);
  if (on_state_line_width <= 0)
  {
    map_field.onFlag = false;
    map_field.lineWidth = abs(on_state_line_width);
  }
  else
  {
    map_field.onFlag = true;
    map_field.lineWidth = on_state_line_width;
  }

  map_field.detailMin = atof(_tokens[4]);
  map_field.detailMax = atof(_tokens[5]);
	
  // Extract the color from the map line.  The color might have spaces in it
  // and must be in all lower case for Jazz to be able to use it.

  char color_buffer[80];
  
  strcpy(color_buffer, _tokens[6]);
  
  for (int i = 7; i < num_fields; ++i)
  {
    strcat(color_buffer, " ");
    strcat(color_buffer, _tokens[i]);
  }
	
  for (size_t i = 0; i < strlen(color_buffer); ++i)
    color_buffer[i] = tolower(color_buffer[i]);
  
  map_field.color = color_buffer;
  
  // Strip underscores out of control label

  for (size_t i = 0; i < map_field.controlLabel.length(); ++i)
  {
    if (map_field.controlLabel[i] == '_')
      map_field.controlLabel[i] = ' ';
  }
      
  _mapFields.push_back(map_field);
}
