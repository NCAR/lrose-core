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
 * @file CloudHandler.hh
 *
 * @class CloudHandler
 *
 * Class for handling cloud data.
 *  
 * @date 10/10/2009
 *
 */

#ifndef _CloudHandler_hh
#define _CloudHandler_hh

#include <map>
#include <string>

#include <Spdb/Spdb.hh>
#include <Spdb/Symprod.hh>
#include <toolsa/MemBuf.hh>
#include <xmlformats/SigwxCloud.hh>

#include "BoundingBox.hh"
#include "IconDef.hh"
#include "Params.hh"
#include "ProductHandler.hh"

using namespace std;

/**
 * @class CloudHandler
 */

class CloudHandler : public ProductHandler
{

public:

  /**
   * @brief Constructor.
   */

  CloudHandler(Params *params, const int debug_level = 0);
  
  /**
   * @brief Destructor.
   */

  virtual ~CloudHandler();
  

  ///////////////////////
  // Rendering methods //
  ///////////////////////

  /**
   * @brief Convert the cloud data to Symprod format.
   *
   * @param[in] icon_def_list List of defined icons.
   * @param[in] dir_path Data directory path.
   * @param[in] prod_id SPDB product ID.
   * @param[in] prod_label SPDB product label.
   * @param[in] chunk_ref Chunk header for this data.
   * @param[in] aux_ref Auxilliary chunk header for this data.
   * @param[in] spdb_data Data pointer.
   * @param[in] spdb_len Length of data buffer.
   * @param[in,out] symprod_buf Symprod buffer.
   * @param[in] bbox Display bounding box.
   *
   * @return Returns 0 on success, -1 on failure.
   */

  int convertToSymprod(const map< string, IconDef > &icon_def_list,
		       const string &dir_path,
		       const int prod_id,
		       const string &prod_label,
		       const Spdb::chunk_ref_t &chunk_ref,
		       const Spdb::aux_ref_t &aux_ref,
		       const void *spdb_data,
		       const int spdb_len,
		       MemBuf &symprod_buf,
		       const BoundingBox &bbox) const;


protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  /**
   * @brief The name of the cloud icon.
   */

  static const string CLOUD_ICON_NAME;
  
  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Add a text box to the Symprod buffer.
   *
   * @param[in,out] symprod Symprod buffer.
   * @param[in] box_ll_lat Box lower left latitude.
   * @param[in] box_ll_lon Box lower left longitude.
   * @param[in] cloud_pt_lat Latitude of the point on the cloud outline
   *                         where the text box arrow should point.
   * @param[in] cloud_pt_lon Longitude of the point on the cloud outline
   *                         where the text box arrow should point.
   * @param[in] msgs List of text messages.
   * @param[in] text_color Text color.
   * @param[in] text_bg_color Text background color.
   */

  void _addTextBoxToSymprod(Symprod *symprod,
			    const double box_ll_lat,
			    const double box_ll_lon,
			    const double cloud_pt_lat,
			    const double cloud_pt_lon,
			    const vector< string > &msgs,
			    const string &text_color,
			    const string &text_bg_color) const;
  

  /**
   * @brief Add the given cloud to the Symprod buffer.
   *
   * @param[in] bbox Bounding box of display area.
   * @param[in] icon_def_list List of defined icons.
   * @param[in,out] symprod Symrpod buffer.
   * @param[in] cloud Cloud to display.
   *
   * @return Returns true on success, false on failure.
   */

  bool _displayCloud(const BoundingBox &bbox,
		     const map< string, IconDef > &icon_def_list,
		     Symprod * symprod,
		     const SigwxCloud &cloud) const;
  

  /**
   * @brief Display a text box for the given cloud.
   *
   * @param[in] cloud The cloud information.
   * @param[in] bbox Bounding box of the display area.
   * @param[in,out] symprod Symprod buffer.
   *
   * @return Returns true on success, false on failure.
   */

  bool _displayTextBox(const SigwxCloud &cloud,
		       const BoundingBox &bbox,
		       Symprod *symprod) const;
  

  /**
   * @brief Display the list of points as directed.
   *
   * @param[in] points List of points to render.
   * @param[in,out] symprod Symprod buffer.
   * @param[in] icon_def_list List of defined icons.
   * @param[in] bbox Bounding box of the display area.
   * @param[in] line_color Line color to use when rendering.
   * @param[in] line_type Line type to use when rendering.
   * @param[in] line_width Line width to use when rendering.
   * @param[in] screenSize Approximate screen size in pixels.
   * @param[in] scallopSize Approximate scallop size in pixels.
   * @param[in] fill_type Fill type to use when rendering.
   * @param[in] text_color Text color to use when rendering.
   * @param[in] text_bg_color Text background color to use when rendering.
   * @param[in] font_name Name of font to use.
   * @param[in] font_size Font size in points.
   * @param[in] hidden_text_flag Flag indicating whether to render hidden
   *                             text.
   * @param[in] msgs List of messages to render.
   *
   * @return Returns true on success, false on failure.
   */

  bool _displayList(const vector< SigwxPoint > &points,
		    Symprod * symprod,
		    const map< string, IconDef > &icon_def_list,
		    const BoundingBox &bbox,
		    const string &line_color,
		    const Symprod::linetype_t line_type,
		    const int line_width,
		    const int screenSize,
		    const int scallopSize,
		    const int fill_type,
		    const string &text_color,
		    const string &text_bg_color,
		    const string &font_name,
		    const int font_size,
		    const bool hidden_text_flag,
		    const vector< string > &msgs,
		    const vector< bool > &hiddens) const;


  /**
   * @brief Make a line with scallops as described.
   *
   * @param[in] pt1_lat Latitude of first point.
   * @param[in] pt1_lon Longitude of first point.
   * @param[in] pt2_lat Latitude of second point.
   * @param[in] pt2_lon Longitude of second point.
   * @param[in,out] symprod Symprod buffer.
   * @param[in] line_color Line color to use when rendering.
   * @param[in] line_width Line width to use when rendering.
   * @param[in] screenSize Approximate screen size in pixels.
   * @param[in] scallopSize Approximate scallop size in pixels.
   * @param[in] bbox Bounding box of the display area.
   */

  void _mkScallops(const double pt1_lat,
		   const double pt1_lon,
		   const double pt2_lat,
		   const double pt2_lon,
		   Symprod * symprod,
		   const string &line_color,
		   const int line_width,
		   const int screenSize,
		   const int scallopSize,
		   const BoundingBox &bbox) const;
  

};

  
#endif
