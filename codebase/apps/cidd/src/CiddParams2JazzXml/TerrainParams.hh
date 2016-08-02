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
 * @file TerrainParams.hh
 *
 * @class TerrainParams
 *
 * Class controlling access to the TERRAIN section of the CIDD parameter
 * file.
 *  
 * @date 10/6/2010
 *
 */

#ifndef TerrainParams_HH
#define TerrainParams_HH

#include "MainParams.hh"
#include "TdrpParamSection.hh"

using namespace std;


class TerrainParams : public TdrpParamSection
{
 public:

  //////////////////
  // Public types //
  //////////////////

  typedef enum
  {
    RENDER_FILLED_CONT,
    RENDER_RECTANGLES,
    RENDER_DYNAMIC_CONTOURS
  } render_type_t;
  
    
  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor
   */

  TerrainParams();
  
  /**
   * @brief Destructor
   */

  virtual ~TerrainParams(void);
  

  /**
   * @brief Initialize the parameters from the given buffer.
   *
   * @param[in] params_buf Parameter file buffer.  Must be null-terminated.
   * @param[in] buf_size   Size of the parameter buffer.
   *
   * @return Returns true on success, false on failure.
   */

  bool init(const MainParams &main_params,
	    const char *params_buf, const size_t buf_size);
  

  ////////////////////
  // Access methods //
  ////////////////////

  /**
   * @brief Get the ID label.
   *
   * @return Returns the ID label.
   */

  string getIdLabel() const
  {
    return _idLabel;
  }
  
  /**
   * @brief Get the land use colorscale file.
   *
   * @return Returns the land use colorscale file.
   */

  string getLanduseColorscale() const
  {
    return _landuseColorscale;
  }
  
  /**
   * @brief Get the terrain field name.
   *
   * @return Returns the terrain field name.
   */

  string getTerrainFieldName() const
  {
    return _terrainFieldName;
  }
  
  /**
   * @brief Get the terrain URL.
   *
   * @return Returns the terrain URL.
   */

  string getTerrainUrl() const
  {
    return _terrainUrl;
  }
  

 protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Label used for legends.
   */

  string _idLabel;
  
  /**
   * @brief URL used to locate gridded terrain data.
   */

  string _terrainUrl;
  
  /**
   * @brief Terrain field name.
   */

  string _terrainFieldName;
  
  /**
   * @brief Conversion from terrain units to local coords.
   */

  double _heightScaler;
  
  /**
   * @brief URL used to locate4 gridded land use data.
   */

  string _landuseUrl;
  
  /**
   * @brief Land use field name.
   */

  string _landuseFieldName;
  
  /**
   * @brief Color lookup table for land use.
   */

  string _landuseColorscale;
  
  /**
   * @brief Land use rendering style.
   */

  render_type_t _landuseRenderMethod;
  
  /**
   * @brief Earth's skin color.
   */

  string _earthSkinColor;
  
  /**
   * @brief Earth's core color.
   */

  string _earthCoreColor;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  void _setUrlAndFieldName(const string &url_field_string,
			   string &url_string,
			   string &field_string) const;
  
};


#endif
