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
///////////////////////////////////////////////////////////////
// Server.cc
//
// File Server object
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2000
//
///////////////////////////////////////////////////////////////


#include <vector>

#include <euclid/GridPoint.hh>
#include <rapformats/GenPt.hh>
#include <toolsa/str.h>

#include "IconDef.hh"
#include "Server.hh"
using namespace std;



/*********************************************************************
 * Constructor
 *
 * Inherits from DsSymprodServer
 */

Server::Server(const string &prog_name,
	       Params *initialParams) :
  DsSymprodServer(prog_name,
		  initialParams->instance,
		  (void*)(initialParams),
		  initialParams->port,
		  initialParams->qmax,
		  initialParams->max_clients,
		  initialParams->no_threads,
		  initialParams->debug >= Params::DEBUG_NORM,
		  initialParams->debug >= Params::DEBUG_VERBOSE)
{
  // Do nothing
}


/*********************************************************************
 * loadLocalParams() - Load local params if they are to be overridden.
 */

int Server::loadLocalParams(const string &paramFile,
			    void **serverParams)
{
  Params  *localParams;
  char   **tdrpOverrideList = NULL;
  bool     expandEnvVars = true;

  const string method_name = "loadLocalParams()";

  if (_isDebug)
    cerr << "Loading new params from file: " << paramFile << endl;

  localParams = new Params(*((Params*)_initialParams));
  if (localParams->load((char*)paramFile.c_str(),
			tdrpOverrideList,
			expandEnvVars,
			_isVerbose) != 0)
  {
    cerr << "ERROR - " << _executableName << "::" << method_name << endl;
    cerr << "Cannot load parameter file: " << paramFile << endl;
    
    delete localParams;
    return -1;
  }

  if (_isVerbose)
    localParams->print(stderr, PRINT_SHORT);

  *serverParams = (void*)localParams;

  // Update objects dependent on the local parameters

  for (int i = 0; i < localParams->icon_defs_n; ++i)
  {
    // Create the list of icon points

    vector< GridPoint > point_list;
    
    char *x_string = strtok(localParams->_icon_defs[i].icon_points, " ");
    char *y_string;
    
    if (x_string == (char *)NULL)
    {
      cerr << "ERROR: Server::" << method_name << endl;
      cerr << "Error in icon_points string for icon " <<
	localParams->_icon_defs[i].icon_name << endl;
      cerr << "The string must contain at least 1 point" << endl;
      
      continue;
    }
    
    bool string_error = false;
    
    while (x_string != (char *)NULL)
    {
      // Get the string representing the Y coordinate of the icon point

      y_string = strtok(NULL, " ");
      
      if (y_string == (char *)NULL)
      {
	cerr << "ERROR: Server::" << method_name << endl;
	cerr << "Error in icon_points string for icon " <<
	  localParams->_icon_defs[i].icon_name << endl;
	cerr << "The string must contain an even number of values" << endl;
      
	string_error = true;
	
	break;
      }
      
      // Convert the string values to points

      GridPoint point(atoi(x_string), atoi(y_string));
      point_list.push_back(point);
      
      // Get the string representing the X coordinate of the icon point

      x_string = strtok(NULL, " ");
      
    } /* endwhile - x_string != (char *)NULL */
    
    // See if there was an error in the icon point processing

    if (string_error)
      continue;
    
    // Create the icon definition object and add it to our list

    string icon_name = localParams->_icon_defs[i].icon_name;
    
    IconDef *icon_def = new IconDef(icon_name, point_list);
    _iconDefList[icon_name] = icon_def;
    
  } /* endfor - i */
  
  return 0;
}


/*********************************************************************
 * convertToSymprod() - Convert the given data chunk from the SPDB
 *                      database to symprod format.
 *
 * Returns 0 on success, -1 on failure
 */

int Server::convertToSymprod(const void *params,
			     const string &dir_path,
			     const int prod_id,
			     const string &prod_label,
			     const Spdb::chunk_ref_t &chunk_ref,
                             const Spdb::aux_ref_t &aux_ref,
			     const void *spdb_data,
			     const int spdb_len,
			     MemBuf &symprod_buf)
{
  const string method_name = "convertToSymprod()";
  
  // check prod_id

  if (prod_id != SPDB_GENERIC_POINT_ID)
  {
    cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
    cerr << "  Incorrect prod_id: " << prod_id << endl;
    cerr << "  Should be SPDB_GENERIC_POINT_ID: " << SPDB_GENERIC_POINT_ID << endl;

    return -1;
  }

  const string routine_name = "convertToSymprod";
  Params *serverParams = (Params*) params;

  // Convert the SPDB data to a weather hazards buffer

  GenPt point;
  
  point.disassemble(spdb_data, spdb_len);
  
  // create Symprod object

  time_t now = time(NULL);

  Symprod prod(now, now,
	       chunk_ref.valid_time,
	       chunk_ref.expire_time,
	       chunk_ref.data_type,
	       chunk_ref.data_type2,
	       SPDB_GENERIC_POINT_LABEL);

  // Get the field data value

  char field_value_string[BUFSIZ];
    
  int field_num = point.getFieldNum(serverParams->field_name);
  
  if (field_num < 0)
  {
    cerr << "ERROR: Server::" << method_name << endl;
    cerr << "Field <" << serverParams->field_name <<
      "> not found in database" << endl;
    
    return -1;
  }
  
  double field_value = point.get1DVal(field_num) * serverParams->multiplier;
  
  if (serverParams->debug >= Params::DEBUG_NORM)
    cerr << "---> Field value: " << field_value << endl;
  
  // See if this field should be displayed as text

  bool displayed_as_text = false;
  
  for (int i = 0; i < serverParams->text_values_n; ++i)
  {
    if (field_value >= serverParams->_text_values[i].min_value &&
	field_value < serverParams->_text_values[i].max_value)
    {
      // Add the text to the product

      sprintf(field_value_string,
	      serverParams->_text_values[i].value_format_string,
	      field_value);

      prod.addText(field_value_string,
		   point.getLat(), point.getLon(),
		   serverParams->_text_values[i].color_name, "",
		   serverParams->_text_values[i].x_offset,
		   serverParams->_text_values[i].y_offset,
		   _convertVertAlign(serverParams->_text_values[i].vert_align),
		   _convertHorizAlign(serverParams->_text_values[i].horiz_align),
		   serverParams->_text_values[i].font_size,
		   _convertFontStyle(serverParams->_text_values[i].font_style),
		   serverParams->_text_values[i].font_name);
  
      displayed_as_text = true;
      
      break;
    }
  } /* endfor - i */
  
  // Now see if the field should be displayed as an icon

  if (!displayed_as_text)
  {
    for (int i = 0; i < serverParams->icon_values_n; ++i)
    {
      if (field_value >= serverParams->_icon_values[i].min_value &&
	  field_value < serverParams->_icon_values[i].max_value)
      {
	// Find the icon in the icon list

	if (_iconDefList.find(serverParams->_icon_values[i].icon_name)
	    == _iconDefList.end())
	{
	  cerr << "ERROR: Server::" << method_name << endl;
	  cerr << "Icon <" << serverParams->_icon_values[i].icon_name <<
	    "> not found in icon list created from parameter file" << endl;
	  cerr << "Not rendering icon on display" << endl;
	  
	  break;
	}
	
	IconDef *icon_def =
	  _iconDefList[serverParams->_icon_values[i].icon_name];
	
	// Add the icon to the product

	Symprod::wpt_t icon_origin;
	
	icon_origin.lat = point.getLat();
	icon_origin.lon = point.getLon();
	
	prod.addStrokedIcons(serverParams->_icon_values[i].color_name,
			     icon_def->getNumPoints(),
			     icon_def->getPointList(),
			     1,
			     &icon_origin);
	
      }
    } /* endfor - i */
    
  } /* endif - !displayed_as_text */
  
  // Add the string to the Symprod object

  // set return buffer

  if (_isVerbose)
    prod.print(cerr);

  prod.serialize(symprod_buf);

  return 0;
}

/*********************************************************************
 * Private methods
 *********************************************************************/

/*********************************************************************
 * _convertFontStyle() - Convert the font style parameter from the 
 *                       parameter file into the value needed for the
 *                       Symprod class.
 */

Symprod::font_style_t Server::_convertFontStyle(const Params::font_style_t font_style)
{
  switch (font_style)
  {
  case Params::TEXT_NORM :
    return Symprod::TEXT_NORM;
    
  case Params::TEXT_BOLD :
    return Symprod::TEXT_BOLD;
    
  case Params::TEXT_ITALICS :
    return Symprod::TEXT_ITALICS;
    
  case Params::TEXT_SUBSCRIPT :
    return Symprod::TEXT_SUBSCRIPT;
    
  case Params::TEXT_SUPERSCRIPT :
    return Symprod::TEXT_SUPERSCRIPT;
    
  case Params::TEXT_UNDERLINE :
    return Symprod::TEXT_UNDERLINE;
    
  case Params::TEXT_STRIKETHROUGH :
    return Symprod::TEXT_STRIKETHROUGH;
  } /* endswitch - font_style */
  
  return Symprod::TEXT_NORM;
}


/*********************************************************************
 * _convertHorizAlign() - Convert the horizontal alignment parameter
 *                        from the parameter file into the value needed
 *                        for the Symprod class.
 */

Symprod::horiz_align_t Server::_convertHorizAlign(const Params::horiz_align_t horiz_align)
{
  switch (horiz_align)
  {
  case Params::HORIZ_ALIGN_LEFT :
    return Symprod::HORIZ_ALIGN_LEFT;
    
  case Params::HORIZ_ALIGN_CENTER :
    return Symprod::HORIZ_ALIGN_CENTER;
    
  case Params::HORIZ_ALIGN_RIGHT :
    return Symprod::HORIZ_ALIGN_RIGHT;
  } /* endswitch - horiz_align */
  
  return Symprod::HORIZ_ALIGN_CENTER;
}


/*********************************************************************
 * _convertVertAlign() - Convert the vertical alignment parameter from
 *                       the parameter file into the value needed for
 *                       the Symprod class.
 */

Symprod::vert_align_t Server::_convertVertAlign(const Params::vert_align_t vert_align)
{
  switch (vert_align)
  {
  case Params::VERT_ALIGN_TOP :
    return Symprod::VERT_ALIGN_TOP;
    
  case Params::VERT_ALIGN_CENTER :
    return Symprod::VERT_ALIGN_CENTER;
    
  case Params::VERT_ALIGN_BOTTOM :
    return Symprod::VERT_ALIGN_BOTTOM;
  } /* endswitch - vert_align */
  
  return Symprod::VERT_ALIGN_CENTER;
}
