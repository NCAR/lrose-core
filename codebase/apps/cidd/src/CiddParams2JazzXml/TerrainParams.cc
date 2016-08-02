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
 * @file TerrainParams.cc
 *
 * @class TerrainParams
 *
 * Class controlling access to the TERRAIN section of the CIDD parameter
 * file.
 *  
 * @date 10/6/2010
 *
 */

#include "Cterrain_P.hh"
#include "TerrainParams.hh"

using namespace std;

/**********************************************************************
 * Constructor
 */

TerrainParams::TerrainParams () :
  TdrpParamSection()
{
}


/**********************************************************************
 * Destructor
 */

TerrainParams::~TerrainParams(void)
{
}
  

/**********************************************************************
 * init()
 */

bool TerrainParams::init(const MainParams &main_params,
			 const char *params_buf, const size_t buf_size)
{
  static const string method_name = "TerrainParams::init()";
  
  // Pull out the TERRAIN section of the parameters buffer

  const char *param_text;
  long param_text_line_no = 0;
  long param_text_len = 0;
  
  if ((param_text = _findTagText(params_buf, "TERRAIN",
				 &param_text_len, &param_text_line_no)) == 0 ||
      param_text == 0 || param_text_len <= 0)
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "Couldn't find TERRAIN section in CIDD parameter file" << endl;
    cerr << "Not processing TERRAIN parameters" << endl;
    
    return true;
  }
  
  // Load the parameters from the buffer

  Cterrain_P terrain_params;
  
  if (terrain_params.loadFromBuf("TERRAIN TDRP Section",
				 0, param_text, param_text_len,
				 param_text_line_no,
				 false, false) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error loading TDRP parameters from <TERRAIN> section" << endl;
    
    return false;
  }
  
  // Extract the terrain information

  _idLabel = terrain_params.id_label;
  _setUrlAndFieldName(terrain_params.terrain_url, _terrainUrl,
		      _terrainFieldName);
  _heightScaler = terrain_params.height_scaler;
  _setUrlAndFieldName(terrain_params.landuse_url, _landuseUrl,
		      _landuseFieldName);
  _landuseColorscale =
    main_params.getColorFileSubdir() + "/" + terrain_params.landuse_colorscale;
  switch (terrain_params.land_use_render_method)
  {
  case Cterrain_P::RENDER_FILLED_CONT :
    _landuseRenderMethod = RENDER_FILLED_CONT;
    break;
  case Cterrain_P::RENDER_RECTANGLES :
    _landuseRenderMethod = RENDER_RECTANGLES;
    break;
  case Cterrain_P::RENDER_DYNAMIC_CONTOURS :
    _landuseRenderMethod = RENDER_DYNAMIC_CONTOURS;
    break;
  }
  _earthSkinColor = terrain_params.earth_color1;
  _earthCoreColor = terrain_params.earth_color2;
  
  return true;
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

void TerrainParams::_setUrlAndFieldName(const string &url_field_string,
					string &url_string,
					string &field_string) const
{
  // Find the '&' character in the string.  If there isn't an '&', then set
  // the field name to "".

  size_t amp_pos;
  
  if ((amp_pos = url_field_string.find("&")) == string::npos)
  {
    url_string = url_field_string;
    field_string = "";
  }
  
  // Now separate the URL from the field name and set the class members

  url_string = url_field_string.substr(0, amp_pos);
  field_string = url_field_string.substr(amp_pos + 1);
}
