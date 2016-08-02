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
 * @file GridsParams.cc
 *
 * @class GridsParams
 *
 * Class controlling access to the GRIDS section of the CIDD parameter
 * file.
 *  
 * @date 9/24/2010
 *
 */

#include <stdlib.h>
#include <string.h>

#include <toolsa/str.h>

#include "GridsParams.hh"

using namespace std;

// Globals

const size_t GridsParams::PARSE_LINE_LEN = 2048;
const int GridsParams::MAX_TOKENS = 32;
const int GridsParams::MAX_TOKEN_LEN = 1024;

/**********************************************************************
 * Constructor
 */

GridsParams::GridsParams () :
  ParamSection(),
  _lineBuffer(0),
  _tokens(0)
{
}


/**********************************************************************
 * Destructor
 */

GridsParams::~GridsParams(void)
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

bool GridsParams::init(const MainParams &main_params,
		       const char *params_buf, const size_t buf_size)
{
  static const string method_name = "GridsParams::init()";
  
  // Allocate space for the parsing buffers

  _lineBuffer = new char[PARSE_LINE_LEN];
  
  _tokens = new char*[MAX_TOKENS];
  for (int i = 0; i < MAX_TOKENS; ++i)
    _tokens[i] = new char[MAX_TOKEN_LEN];
  
  // Pull out the GRIDS section of the parameters buffer

  const char *param_text;
  long param_text_line_no = 0;
  long param_text_len = 0;
  
  if ((param_text = _findTagText(params_buf, "GRIDS",
				 &param_text_len, &param_text_line_no)) == 0)
    return false;
  
  if (param_text == 0 || param_text_len <= 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Couldn't find GRIDS section in CIDD parameter file" << endl;
    
    return false;
  }
  
  if (!_initDataLinks(main_params,
		      param_text, param_text_len, param_text_line_no))
    return false;
  
  return true;
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _initDataLinks()
 */

bool GridsParams::_initDataLinks(const MainParams &main_params,
				 const char *param_buf,
				 long param_buf_len, long line_no)
{
  static const string method_name = "GridsParams::_initDataLinks()";
  
  // Read all the lines in the data information buffer

  int total_len = 0;
  const char *start_ptr = param_buf;
  const char *end_ptr;

  while ((end_ptr = strchr(start_ptr,'\n')) != NULL &&
	 (total_len < param_buf_len))
  {
    // Skip over blank, short or commented lines
    int len = (end_ptr - start_ptr) + 1;
    if (len < 20 || *start_ptr == '#')
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
      cerr << "Cannot handle lines in the GRIDS section longer than "
	   << (PARSE_LINE_LEN - 1) << " characters" << endl;
      cerr << "--- SKIPPING LINE ---" << endl;
      
      total_len += len + 1;   // Count characters processed 
      start_ptr = end_ptr + 1; // Skip past the newline
      ++line_no;

      continue;
    }
    
    STRcopy(_lineBuffer, start_ptr, len);
    _parseLine(main_params, _lineBuffer, len);
      
    total_len += len + 1;   // Count characters processed 
    start_ptr = end_ptr + 1; // Skip past the newline
    ++line_no;
  }

  return true;
}


/**********************************************************************
 * _parseLine()
 */

bool GridsParams::_parseLine(const MainParams &main_params,
			     const char *parse_line, const long line_len)
{
  static const string method_name = "GridsParams::_parseLine()";
  
  GridField grid_field;
  
  // Parse the line into tokens

  int num_fields;

  if ((num_fields =
       STRparse(parse_line, _tokens, line_len, MAX_TOKENS, MAX_TOKEN_LEN))
      < 11)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Couldn't parse grid line:"  << parse_line << endl;
    cerr << "Expected at least 11 fields, found " << num_fields << endl;

    return false;
  }
  

  grid_field.legendName = _tokens[0];
  grid_field.buttonName = _tokens[1];
  if (!grid_field.setUrlAndFieldName(_tokens[2]))
    return false;
  grid_field.colorFile = main_params.getColorFileSubdir() + "/" + _tokens[3];

  // if units are "" or --, set to zero-length string
  if (!strcmp(_tokens[4], "\"\"") || !strcmp(_tokens[4], "--"))
    grid_field.units = "";
  else
    grid_field.units = _tokens[4];

  grid_field.contourLow = atof(_tokens[5]);
  grid_field.contourHigh = atof(_tokens[6]);
  grid_field.contourInterval = atof(_tokens[7]);

  grid_field.renderMethod = GridField::RENDER_POLYGONS;

  if (strncasecmp(_tokens[8], "cont", 4) == 0)
    grid_field.renderMethod = GridField::RENDER_FILLED_CONTOURS;

  if (strncasecmp(_tokens[8], "lcont", 4) == 0)
    grid_field.renderMethod = GridField::RENDER_LINE_CONTOURS;

  if (strncasecmp(_tokens[8], "dcont", 4) == 0)
    grid_field.renderMethod = GridField::RENDER_DYNAMIC_CONTOURS;
  
  if (strstr(_tokens[8],"comp") != 0)
    grid_field.compositeMode = true;
  
  if (strstr(_tokens[8], "autoscale") != 0)
    grid_field.autoScale = true;

  if (atoi(_tokens[9]) == 0)
    grid_field.displayInMenu = false;
  else
    grid_field.displayInMenu = true;
  
  if (main_params.isRunOnceAndExit())
  {
    grid_field.backgroundRender = true;
  }
  else
  {
    if (atoi(_tokens[10]) == 0)
      grid_field.backgroundRender = false;
    else
      grid_field.backgroundRender = true;
  }

  if (!main_params.isHtmlMode() && main_params.isReplaceUnderscores())
    grid_field.replaceUnderscores();

  _gridFields.push_back(grid_field);
  
  return true;
}
