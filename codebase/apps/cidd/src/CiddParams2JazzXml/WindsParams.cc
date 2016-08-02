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
 * @file WindsParams.cc
 *
 * @class WindsParams
 *
 * Class controlling access to the WINDS section of the CIDD parameter
 * file.
 *  
 * @date 10/5/2010
 *
 */

#include <stdlib.h>
#include <string.h>

#include <toolsa/str.h>

#include "WindsParams.hh"

using namespace std;

// Globals

const size_t WindsParams::PARSE_LINE_LEN = 2048;
const int WindsParams::MAX_TOKENS = 32;
const int WindsParams::MAX_TOKEN_LEN = 1024;

/**********************************************************************
 * Constructor
 */

WindsParams::WindsParams () :
  ParamSection(),
  _lineBuffer(0),
  _tokens(0)
{
}


/**********************************************************************
 * Destructor
 */

WindsParams::~WindsParams(void)
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

bool WindsParams::init(const MainParams &main_params,
		       const char *params_buf, const size_t buf_size)
{
  static const string method_name = "WindsParams::init()";
  
  // Allocate space for the parsing buffers

  _lineBuffer = new char[PARSE_LINE_LEN];
  
  _tokens = new char*[MAX_TOKENS];
  for (int i = 0; i < MAX_TOKENS; ++i)
    _tokens[i] = new char[MAX_TOKEN_LEN];
  
  // Pull out the GRIDS section of the parameters buffer

  const char *param_text;
  long param_text_line_no = 0;
  long param_text_len = 0;
  
  if ((param_text = _findTagText(params_buf, "WINDS",
				 &param_text_len, &param_text_line_no)) == 0 ||
      param_text == 0 || param_text_len <= 0)
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "Couldn't find WINDS section in CIDD parameter file" << endl;
    cerr << "Not processing WINDS parameters" << endl;
    
    return true;
  }
  
  if (!_initWindDataLinks(main_params,
			  param_text, param_text_len, param_text_line_no))
    return false;
  
  return true;
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _initWindDataLinks()
 */

bool WindsParams::_initWindDataLinks(const MainParams &main_params,
				     const char *param_buf,
				     long param_buf_len, long line_no)
{
  static const string method_name = "WindsParams::_initDataLinks()";
  
  // Set up the rendering preference

  WindField::marker_type_t default_marker_type = WindField::MARKER_ARROWS;
  
  string marker_string = main_params.getWindMarkerType();

  if (strncasecmp(marker_string.c_str(), "tuft", strlen("tuft")) == 0)
    default_marker_type = WindField::MARKER_TUFT;
  else if (strncasecmp(marker_string.c_str(), "barb", strlen("barb")) == 0)
    default_marker_type = WindField::MARKER_BARB;
  else if (strncasecmp(marker_string.c_str(), "vector", strlen("vector")) == 0)
    default_marker_type = WindField::MARKER_VECTOR;
  else if (strncasecmp(marker_string.c_str(), "tickvector",
		       strlen("tickvector")) == 0)
    default_marker_type = WindField::MARKER_TICKVECTOR; 
  else if (strncasecmp(marker_string.c_str(), "labeledbarb",
		       strlen("labeledbarb")) == 0)
    default_marker_type = WindField::MARKER_LABELEDBARB;
  else if (strncasecmp(marker_string.c_str(), "metbarb",
		       strlen("metbarb")) == 0)
    default_marker_type = WindField::MARKER_METBARB;
  else if(strncasecmp(marker_string.c_str(), "barb_sh",
		      strlen("barb_sh")) == 0)
    default_marker_type = WindField::MARKER_BARB_SH;
  else if(strncasecmp(marker_string.c_str(), "labeledbarb_sh",
		      strlen("labeledbarb_sh")) == 0)
    default_marker_type = WindField::MARKER_LABELEDBARB_SH;

  // Read all the lines in the data information buffer

  int total_len = 0;
  const char *start_ptr = param_buf;
  const char *end_ptr;

  while ((end_ptr = strchr(start_ptr,'\n')) != NULL &&
	 (total_len < param_buf_len))
  {
    // Skip over blank, short or commented lines
    int len = (end_ptr - start_ptr) + 1;
    if (len < 15 || *start_ptr == '#')
    {
      total_len += len + 1;
      start_ptr = end_ptr + 1; // Skip past the newline
      ++line_no;
      continue;
    }

    // Copy the line into a separate buffer for parsing.

    if (len > (int)PARSE_LINE_LEN - 1)
    {
      cerr << "WARNING: " << method_name << endl;
      cerr << "Cannot handle lines in the WINDS section longer than "
	   << (PARSE_LINE_LEN - 1) << " characters" << endl;
      cerr << "--- SKIPPING LINE ---" << endl;
      
      total_len += len + 1;   // Count characters processed 
      start_ptr = end_ptr + 1; // Skip past the newline
      ++line_no;

      continue;
    }
    
    STRcopy(_lineBuffer, start_ptr, len);
    _parseLine(main_params, _lineBuffer, len, default_marker_type);
      
    total_len += len + 1;   // Count characters processed 
    start_ptr = end_ptr + 1; // Skip past the newline
    ++line_no;
  }

  return true;
}


/**********************************************************************
 * _parseLine()
 */

bool WindsParams::_parseLine(const MainParams &main_params,
			     const char *parse_line, const long line_len,
			     const WindField::marker_type_t default_marker_type)
{
  static const string method_name = "WindsParams::_parseLine()";
  
  WindField wind_field;
  
  // Parse the line into tokens

  int num_fields;
  
  num_fields =
    STRparse(parse_line, _tokens, line_len, MAX_TOKENS, MAX_TOKEN_LEN);

  if (num_fields < 7)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Couldn't parse WINDS line: " << parse_line << endl;
    cerr << "Expected at least 7 tokens, found " << num_fields << " tokens" << endl;
    return false;
  }

  wind_field.legendLabel = _tokens[0];

  // Set the URL value.  Note that we have to take the '&' character off the
  // end of the string

  _tokens[1][strlen(_tokens[1]) - 1] = '\0';
  wind_field.url = _tokens[1];

  if (!main_params.isHtmlMode())
    wind_field.replaceUnderscores();

  wind_field.uFieldName = _tokens[2];
  wind_field.vFieldName = _tokens[3];
  if (strncasecmp(_tokens[4], "None", strlen("None")) != 0)
    wind_field.wFieldName = _tokens[4];
  wind_field.units = _tokens[5];
  if (atoi(_tokens[6]) > 0)
    wind_field.isOn = true;
  wind_field.lineWidth = abs(atoi(_tokens[6]));

  // Sanity check the line width value

  if (wind_field.lineWidth == 0 || wind_field.lineWidth > 10)
    wind_field.lineWidth = 1;
  
  // Pick out Optional Marker type fields
  
  wind_field.markerType = default_marker_type;
  if (strstr(_tokens[6], ",tuft") != NULL)
    wind_field.markerType  = WindField::MARKER_TUFT;
  else if (strstr(_tokens[6], ",barb") != NULL)
    wind_field.markerType = WindField::MARKER_BARB;
  else if (strstr(_tokens[6], ",vector") != NULL)
    wind_field.markerType = WindField::MARKER_VECTOR;
  else if (strstr(_tokens[6], ",tickvector") != NULL)
    wind_field.markerType = WindField::MARKER_TICKVECTOR; 
  else if (strstr(_tokens[6], ",labeledbarb") != NULL)
    wind_field.markerType = WindField::MARKER_LABELEDBARB;
  else if (strstr(_tokens[6], ",metbarb") != NULL)
    wind_field.markerType = WindField::MARKER_METBARB;
  else if (strstr(_tokens[6], ",barb_sh") != NULL)
    wind_field.markerType = WindField::MARKER_BARB_SH;
  else if (strstr(_tokens[6], ",labeledbarb_sh") != NULL)
    wind_field.markerType = WindField::MARKER_LABELEDBARB_SH;
  
  // Pull together all of the tokens that represent the color name

  wind_field.color = _tokens[7];
  for (int i = 8; i < num_fields; ++i)
  {
    wind_field.color += " ";
    wind_field.color += _tokens[i];
  }

  // Add the new wind field to the list

  _windFields.push_back(wind_field);
  
  return true;
}
