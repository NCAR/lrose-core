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
// October 2002
//
///////////////////////////////////////////////////////////////

#include <rapformats/HydroStation.hh>
#include <toolsa/str.h>

#include "Server.hh"
using namespace std;


const int Server::FIELD_VALUE_STRING_LEN = 100;


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

  if (prod_id != SPDB_HYDRO_STATION_ID)
  {
    cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
    cerr << "  Incorrect prod_id: " << prod_id << endl;
    cerr << "  Should be SPDB_HYDRO_STATION_ID: " << SPDB_HYDRO_STATION_ID << endl;

    return -1;
  }

  const string routine_name = "convertToSymprod";
  Params *serverParams = (Params*) params;

  // Convert the SPDB data to a hydro station object

  HydroStation station;
  
  station.disassemble(spdb_data, spdb_len);
  
  // create Symprod object

  time_t now = time(NULL);

  Symprod prod(now, now,
	       chunk_ref.valid_time,
	       chunk_ref.expire_time,
	       chunk_ref.data_type,
	       chunk_ref.data_type2,
	       SPDB_HYDRO_STATION_LABEL);

  // Render each of the indicated fields

  char field_value_string[FIELD_VALUE_STRING_LEN];
  
  for (int i = 0; i < serverParams->field_render_list_n; ++i)
  {
    double field_data_value = 0.0;
    
    switch (serverParams->_field_render_list[i].field)
    {
    case Params::WIND_SPEED_FIELD :
      field_data_value = station.getWindSpeed();
      break;
      
    case Params::WIND_DIR_FIELD :
      field_data_value = station.getWindDirection();
      break;
      
    case Params::TEMPERATURE_FIELD :
      field_data_value = station.getTemperature();
      break;
      
    case Params::REL_HUM_FIELD :
      field_data_value = station.getRelativeHumidity();
      break;
      
    case Params::RAINFALL_FIELD :
      field_data_value = station.getRainfall();
      break;
      
    case Params::SOLAR_RAD_FIELD :
      field_data_value = station.getSolarRadiation();
      break;
      
    case Params::PRESSURE_FIELD :
      field_data_value = station.getPressure();
      break;
      
    case Params::SOIL_MOIST1_FIELD :
      field_data_value = station.getSoilMoisture1();
      break;
      
    case Params::SOIL_MOIST2_FIELD :
      field_data_value = station.getSoilMoisture2();
      break;
      
    case Params::SOIL_MOIST3_FIELD :
      field_data_value = station.getSoilMoisture3();
      break;
      
    case Params::SOIL_MOIST4_FIELD :
      field_data_value = station.getSoilMoisture4();
      break;
      
    case Params::SOIL_TEMP_FIELD :
      field_data_value = station.getSoilTemperature();
      break;
    }
    
    if (field_data_value == HydroStation::DATA_NOT_AVAILABLE)
    {
      sprintf(field_value_string, "N/A");
    }
    else
    {
      field_data_value *= serverParams->_field_render_list[i].multiplier;

      sprintf(field_value_string,
	      serverParams->_field_render_list[i].value_format_string,
	      field_data_value);
    }

    prod.addText(field_value_string,
		 station.getLatitude(), station.getLongitude(),
		 serverParams->_field_render_list[i].color_name, 
		 serverParams->_field_render_list[i].background_color_name,
		 serverParams->_field_render_list[i].x_offset,
		 serverParams->_field_render_list[i].y_offset,
		 _convertVertAlign(serverParams->_field_render_list[i].vert_align),
		 _convertHorizAlign(serverParams->_field_render_list[i].horiz_align),
		 serverParams->_field_render_list[i].font_size,
		 _convertFontStyle(serverParams->_field_render_list[i].font_style),
		 serverParams->_field_render_list[i].font_name);
  }
  
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
