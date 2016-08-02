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
 * @file JazzXml.cc
 *
 * @class JazzXml
 *
 * Jazz XML handler.
 *  
 * @date 9/24/2010
 *
 */

#include <iomanip>
#include <stdlib.h>
#include <vector>

#include <Mdv/MdvxProj.hh>
#include <toolsa/DateTime.hh>

#include "JazzXml.hh"
#include "Zoom.hh"

using namespace std;

/**********************************************************************
 * Constructor
 */

JazzXml::JazzXml ()
{
}


/**********************************************************************
 * Destructor
 */

JazzXml::~JazzXml(void)
{
}
  

/**********************************************************************
 * init()
 */

bool JazzXml::init()
{
  return true;
}
  

/**********************************************************************
 * generateXml()
 */

bool JazzXml::generateXml(const CiddParamFile &cidd_param_file) const
{
  if (!_generateHeader(cidd_param_file))
    return false;
  
  if (!_generateLayers(cidd_param_file))
    return false;
  
  if (!_generateMenuGroups(cidd_param_file))
    return false;
  
  if (!_generateTimeConfiguration(cidd_param_file))
    return false;
  
  if (!_generateAnimationConfiguration(cidd_param_file))
    return false;
  
  if (!_generateAltitudeConfiguration(cidd_param_file))
    return false;
  
  if (!_generateViewConfiguration(cidd_param_file))
    return false;
  
  if (!_generateAreasOfInterest(cidd_param_file))
    return false;
  
  if (!_generateWindowParameters(cidd_param_file))
    return false;
  
  if (!_generateXSections(cidd_param_file))
    return false;
  
  if (!_generateSkewTParameters(cidd_param_file))
    return false;
  
  if (!_generateOptionalToolParameters(cidd_param_file))
    return false;
  
  if (!_generateColorScales(cidd_param_file))
    return false;
  
  if (!_generateFooter(cidd_param_file))
    return false;
  
  return true;
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _generateHeader()
 */

bool JazzXml::_generateHeader(const CiddParamFile &cidd_param_file) const
{
  cout << "<?xml version=\"1.0\"?>" << endl;
  cout << "" << endl;
  cout << "<!--" << endl;
  cout << "####################################################################################################################################" << endl;
  cout << "" << endl;
  cout << " Sample Jazz file" << endl;
  cout << "" << endl;
  cout << " Direct comments to:" << endl;
  cout << " Nancy Rehak (rehak@ucar.edu)" << endl;
  cout << "" << endl;
  cout << "####################################################################################################################################" << endl;
  cout << "-->" << endl;
  cout << "" << endl;
  cout << "<Jazz version=\"1.0\">" << endl;
  cout << "" << endl;
  cout << "<!--" << endl;
  cout << "####################################################################################################################################" << endl;
  cout << "" << endl;
  cout << " XML Tips:" << endl;
  cout << "" << endl;
  cout << " - XML is comprised of Elements and Attributes, as shown below:" << endl;
  cout << "" << endl;
  cout << "   <Element attribute=\"value\">" << endl;
  cout << "     <ChildElement1/>" << endl;
  cout << "     <ChildElement2 attribute1=\"foo\" anotherAttribute=\"bar\"/>" << endl;
  cout << "   </Element>" << endl;
  cout << "" << endl;
  cout << " - Do NOT use ampersand, apostrophe, greater-than, less-than, dashes, or exclamation characters for your content!" << endl;
  cout << "   These are special XML characters." << endl;
  cout << " - Element names begin with a capital letter, attribute names with a lowercase." << endl;
  cout << " - Elements must be terminated with a slash, either inline such as \"<Element/>\" or with a matching tag such as \"<Element></Element>\"" << endl;
  cout << " - Attribute values must always be double quoted" << endl;
  cout << " - Comments are bounded by less-than, exclamation, and 2 dashes at the beginning and 2 dashes followed by a greater-than at the end." << endl;
  cout << "   2 dashes (and thus comments) cannot appear anywhere within a comment (which is why I can't show them to you here)." << endl;
  cout << " - Blank lines, whitespace, carriage returns, and comments are completely ignored by the parser." << endl;
  cout << "" << endl;
  cout << " Jazz Tips:" << endl;
  cout << "" << endl;
  cout << " - Configuration elements contain attributes which have meaning to that element (see descriptions of each below)." << endl;
  cout << " - Most configuration elements can be specified on a single line, but you are free to split them on several lines for readability." << endl;
  cout << " - Some elements may appear more than once in a file (e.g., \"Layer,\" \"Colorscale,\" and \"Area\"). Others should only appear once." << endl;
  cout << "   The comment for each element will indicate whether that element can be repeated." << endl;
  cout << " - Some attributes may be left out or their values left empty. In those cases, the default value for that attribute applies." << endl;
  cout << "   These are all indicated in the comments." << endl;
  cout << "" << endl;
  cout << "####################################################################################################################################" << endl;
  cout << "-->" << endl;
  cout << "" << endl;

  return true;
}


/**********************************************************************
 * _generateLayers()
 */

bool JazzXml::_generateLayers(const CiddParamFile &cidd_param_file) const
{
  const MainParams &main_params = cidd_param_file.getMainParams();
  
  cout << "<!--" << endl;
  cout << "####################################################################################################################################" << endl;
  cout << "" << endl;
  cout << " <Layer> element (optional, any number of these allowed)." << endl;
  cout << "" << endl;
  cout << " Required attributes:" << endl;
  cout << "" << endl;
  cout << " vis        = whether the layer should be on or off when the application starts" << endl;
  cout << " type       = the datatype of this layer (e.g., MDV, MDVWIND, MDVTOPO, CIDDMAP, SYMPROD)" << endl;
  cout << "              This determines which other configuration options, below, are valid" << endl;
  cout << " name       = the name shown in the display" << endl;
  cout << "" << endl;
  cout << "" << endl;
  cout << " Optional attributes:" << endl;
  cout << "" << endl;
  cout << " menuGroup        = the name of a group to add this layer to, defining where the layer shows up in the menus. By default" << endl;
  cout << "                    layers are added to a default group based on their type (example: MDV layers are added to GRIDS_MENU)." << endl;
  cout << "                    Specifying a different membership or menuGroup=\"none\" overrides the default behavior." << endl;
  cout << " menuName         = the name shown in the menu.  By default, the display name is used." << endl;
  cout << " visibilityGroups = the name of a group that will control the visibility of the layer. By default layers may be added to" << endl;
  cout << "                    a group based on their type (example: MDV layers are added to the exclusive GRIDS_VISIBILITY group" << endl;
  cout << "                    that maintains a maximum of one visible layer within the group).  Layers are not required to be in a" << endl;
  cout << "                    visibility group." << endl;
  cout << "" << endl;
  cout << " Notes:" << endl;
  cout << "  - Layers are rendered in the same order in which they are defined. First layer is on bottom, last layer on top. However," << endl;
  cout << "    the GUI allows you to change the order by selecting \"Configure -> Layer Order\" from the menubar." << endl;
  cout << "" << endl;
  cout << "" << endl;
  cout << " Layer Descriptions:" << endl;
  cout << "" << endl;
  cout << "   MDV:" << endl;
  cout << "     This is the layer type to use for any MDV grids that don't fall under a more specific category below." << endl;
  cout << "" << endl;
  cout << "     Attributes:" << endl;
  cout << "       field      = the MDV field name to retrieve (required)" << endl;
  cout << "       is2D       = true or false (default); if 2D = true, then altitude changes do not cause a reloading of data" << endl;
  cout << "       render     = rendering scheme which may be: grid, fillcontours, linecontours, or none (required)" << endl;
  cout << "       colorscale = the colorscale to use. May be the name of a colorscale defined in this file" << endl;
  cout << "                    or a reference to a remote CIDD (*.colors) or JADE (*.xml) colorscale. (required)" << endl;
  cout << "       location   = must be in the form \"mdvp::\". Mdv files are NOT supported (yet). (required)" << endl;
  cout << "" << endl;
  cout << "     Default menu group is GRIDS_MENU, default visibility group is GRIDS_EXCLUSIVE." << endl;
  cout << "" << endl;
  cout << "" << endl;
  cout << "   MDVTOPO:" << endl;
  cout << "     This is the layer type to use for MDV topography grids." << endl;
  cout << "" << endl;
  cout << "     Attributes:" << endl;
  cout << "       field      = the short MDV field name to retrieve (required)" << endl;
  cout << "       colorscale = the colorscale to use. May be the name of a colorscale defined in this file or a reference to a" << endl;
  cout << "                    remote CIDD (\"*.colors\") or JADE (\"*.xml\") colorscale (required)" << endl;
  cout << "       location   = must be in the form \"mdvp::\". (required)" << endl;
  cout << "" << endl;
  cout << "     Default menu group is FEATURES_MENU." << endl;
  cout << "" << endl;
  cout << "" << endl;
  cout << "   MDVWIND:" << endl;
  cout << "     This is the layer type to use for MDV wind grids that are rendered as gridded layers.  This is not a wind/motion" << endl;
  cout << "     overlay layer which will need to be added some time in the futurel." << endl;
  cout << "" << endl;
  cout << "     Attributes:" << endl;
  cout << "       isUandV    = true if the following fields will contain U and V vectors or false if speed and direction will be used (required)" << endl;
  cout << "       field      = the short MDV field name containing either the U wind vector or the wind speed in knots, depending on the value of \"isUandV\" (required)" << endl;
  cout << "       extraField = the short MDV field name containing the V wind vector or the wind direction in degrees, depending on the value of \"isUandV\" (required)" << endl;
  cout << "       is2D       = true or false (default); if 2D = true, then altitude changes do not cause a reloading of data" << endl;
  cout << "       render     = rendering scheme (\"grid\" for filled grid, \"fillcontours\" for color-filled contours, or \"linecontours\" for line contours) (required)" << endl;
  cout << "       colorscale = the colorscale to use. May be the name of a colorscale defined in this file or a reference to a" << endl;
  cout << "                    remote CIDD (*.colors) or JADE (*.xml) colorscale (required)" << endl;
  cout << "       location   = must be in the form \"mdvp::\". (required)" << endl;
  cout << "" << endl;
  cout << "     Default menu group is GRIDS_MENU, default visibility group is GRIDS_EXCLUSIVE." << endl;
  cout << "" << endl;
  cout << "" << endl;
  cout << "   SYMPROD:" << endl;
  cout << "     This is the layer type to use for SPDB data that is being rendered using a Symprod server." << endl;
  cout << "" << endl;
  cout << "     Attributes:" << endl;
  cout << "       before     = the amount of time before the currently selected time to look for products (e.g. \"75mins\" or \"1hrs\"). (required)" << endl;
  cout << "       after      = the amount of time after the currently selected time to look for products (can be \"\" or \"now\" to indicate zero offset). (required)" << endl;
  cout << "       textOff    = the spacing factor past which to stop rendering text labels.  Defaults to 0.0." << endl;
  cout << "       request    = the methodology to use for requesting the data (\"closest\", \"interval\", \"latest\", \"valid\" or \"firstBefore\"). (required)" << endl;
  cout << "" << endl;
  cout << "     Default menu group is FEATURES_MENU." << endl;
  cout << "" << endl;
  cout << "" << endl;
  cout << "   CIDDMAP:" << endl;
  cout << "     This is the layer type to use for maps in the format used by CIDD and Rview." << endl;
  cout << "" << endl;
  cout << "     Attributes:" << endl;
  cout << "       color                  = the color with which to render the line, as an X11-defined color name, RGB triplet (each value between 0..255)" << endl;
  cout << "                                or hex group (\"wheat1\", \"153,153,153\", \"#ffffcc\"). Defaults to black." << endl;
  cout << "       width                  = the width of the line in pixels. Defaults to 1." << endl;
  cout << "" << endl;
  cout << "     Default menu group is MAPS_MENU." << endl;
  cout << "" << endl;
  cout << "" << endl;
  cout << "   RANGE_RINGS:" << endl;
  cout << "     This layer type creates an overlay of range rings.  The range rings may be centered on a predefined location or may be centered on" << endl;
  cout << "     the origin of the underlying base MDV data layer.  This layer is most useful if the base MDV layer is radar data in a polar or flat" << endl;
  cout << "     projection." << endl;
  cout << "" << endl;
  cout << "     Attributes:" << endl;
  cout << "       followData             = flag indicating whether the range rings should follow the data origin." << endl;
  cout << "                                If true, the rings will be centered on the projection origin for the underlying base gridded data field." << endl;
  cout << "                                Note that the range rings will not be displayed if the origin of the data is set to 0.0, 0.0 (which is" << endl;
  cout << "                                the case for most lat/lon projections)." << endl;
  cout << "                                If false, the rings will always be rendered centered on radarLat, radarLon." << endl;
  cout << "                                Defaults to false." << endl;
  cout << "       radarLat               = the latitude of the radar. Defaults to 0.0." << endl;
  cout << "       radarLon               = the longitude of the radar. Defaults to 0.0." << endl;
  cout << "       color                  = the color with which to render the line, as an X11-defined color name, RGB triplet (each value between 0..255)" << endl;
  cout << "                                or hex group (\"wheat1\", \"153,153,153\", \"#ffffcc\") (required)." << endl;
  cout << "       width                  = the width of the lines in pixels.  Defaults to 1." << endl;
  cout << "       radiusIncr             = the radius increment of the rings in kilometers.  Rings will be rendered at all integral multiples of this" << endl;
  cout << "                                value.  Defaults to 100." << endl;
  cout << "       numRings               = number of rings to render.  Defaults to 4." << endl;
  cout << "       displayAzLines         = flag indicating whether to display azimuth lines (\"true\" or \"false\", defaults to false)" << endl;
  cout << "       startAz                = the azimuth for the first azimuth line in degrees.  This is also the line along which the range labels will be" << endl;
  cout << "                                rendered.  Defaults to 45.0." << endl;
  cout << "       numAzLines             = the number of azimuth lines to display.  Defaults to 8." << endl;
  cout << "" << endl;
  cout << "     Default menu group is MAPS_MENU." << endl;
  cout << "" << endl;
  cout << "####################################################################################################################################" << endl;
  cout << "-->" << endl;
  cout << "" << endl;

  // Generate the MDV layers

  cout << "<!-- MDV Layers -->" << endl;
  cout << endl;
  
  vector< GridField > mdv_layers = cidd_param_file.getGridFields();
  vector< GridField >::const_iterator mdv_layer;
  bool first = true;
  
  for (mdv_layer = mdv_layers.begin(); mdv_layer != mdv_layers.end();
       ++mdv_layer)
  {
    if (!_generateMdvLayer(cidd_param_file, *mdv_layer, first))
      return false;

    first = false;
  }
  
  // Generate the MDVTOPO layers

  const TerrainParams &terrain_params = cidd_param_file.getTerrainParams();

  if (terrain_params.getTerrainUrl() != "")
  {
    cout << "<!-- MDVTOPO Layers -->" << endl;
    cout << endl;

    cout << "<Layer vis=\"on\" type=\"MDVTOPO\" name=\"";
    cout << terrain_params.getIdLabel();
    cout << "\" location=\"";
    cout << terrain_params.getTerrainUrl();
    cout << "\"" << endl;
  
    cout << "       field=\"";
    cout << terrain_params.getTerrainFieldName();
    cout << "\" colorscale=\"";
    cout << terrain_params.getLanduseColorscale();
    cout << "\" menuName=\"";
    cout << terrain_params.getIdLabel();
    cout << "\" />" << endl;
  
    cout << endl;
  }
  
  // Generate the MDVWIND layers

  cout << "<!-- MDVWIND Layers -->" << endl;
  cout << endl;

  vector< WindField > mdvwind_layers = cidd_param_file.getWindFields();
  vector< WindField >::const_iterator mdvwind_layer;
  
  for (mdvwind_layer = mdvwind_layers.begin();
       mdvwind_layer != mdvwind_layers.end(); ++mdvwind_layer)
  {
    if (!_generateMdvwindLayer(cidd_param_file, *mdvwind_layer))
      return false;
  }
  
  if (mdvwind_layers.size() > 0)
  {
    cerr << "WARNING: " << endl;
    cerr << "Your CIDD parameter file contains fields in the \"WINDS\" section" << endl;
    cerr << "Jazz does have an MDVWINDS layer, however it is not currently" << endl;
    cerr << "implemented in the same manner as the CIDD winds rendering.  I am " << endl;
    cerr << "converting the WINDS section but it will have to be updated again " << endl;
    cerr << "once an appropriate Jazz layer is created." << endl;
    cerr << endl;
  }
  
  // Generate the SYMPROD layers

  cout << "<!-- SYMPROD Layers -->" << endl;
  cout << endl;

  vector< SymprodField > symprod_layers = cidd_param_file.getSymprodFields();
  vector< SymprodField >::const_iterator symprod_layer;
  
  for (symprod_layer = symprod_layers.begin();
       symprod_layer != symprod_layers.end(); ++symprod_layer)
  {
    if (!_generateSymprodLayer(cidd_param_file, *symprod_layer))
      return false;
  }
  
  if (symprod_layers.size() > 0)
  {
    cerr << "WARNING: " << endl;
    cerr << "Your CIDD parameter file contains products in the \"SYMPRODS\" section." << endl;
    cerr << "The symprod data timing in Jazz is different than it is in CIDD.  You" << endl;
    cerr << "will need to check the \"request\" attribute for each of your SYMPROD layers" << endl;
    cerr << "to make sure they reflect what you really want to do." << endl;
    cerr << endl;
  }
  
  // Generate the CIDDMAP layers

  cout << "<!-- CIDDMAP Layers -->" << endl;
  cout << endl;

  vector< MapField > ciddmap_layers = cidd_param_file.getMapFields();
  vector< MapField >::const_iterator ciddmap_layer;
  
  for (ciddmap_layer = ciddmap_layers.begin();
       ciddmap_layer != ciddmap_layers.end(); ++ciddmap_layer)
  {
    if (!_generateCiddmapLayer(cidd_param_file, *ciddmap_layer))
      return false;
  }
  
  // Generate the RANGE_RING layer

  if (main_params.isRenderRangeRings() || main_params.isRenderAzimuthLines())
  {
    int num_rings = 0;
    
    if (main_params.isRenderRangeRings())
      num_rings = (int)(main_params.getAzimuthRadius() /
			main_params.getRangeRingSpacing());
    
    cout << "<!-- RANGE_RING Layers -->" << endl;
    cout << endl;

    cout << "<Layer vis=\"on\" type=\"RANGE_RINGS\" name=\"Range Rings\"" << endl;
    cout << "       followData=\"false\" radarLat=\"" << main_params.getOriginLatitude()
	 << "\" radarLon=\"" << main_params.getOriginLongitude() << "\"" << endl;
    cout << "       color=\"" << main_params.getRangeRingColor() << "\" width=\"1\"" << endl;
    cout << "       numRings=\"" << num_rings
	 << "\" radiusIncr=\"" << main_params.getRangeRingSpacing() << "\""
	 << endl;

    cout << "       displayAzLines=\"";
    if (main_params.isRenderAzimuthLines())
      cout << "true";
    else
      cout << "false";
    cout << "\" />" << endl;
  }
  
  cout << endl;
  
  return true;
}


/**********************************************************************
 * _generateMdvLayer()
 */

bool JazzXml::_generateMdvLayer(const CiddParamFile &cidd_param_file,
				const GridField &field,
				const bool vis_flag) const
{
  // See if there are any menu groups for this layer

  const GuiConfigParams gui_config = cidd_param_file.getGuiConfigParams();
  vector< string > menu_groups =
    gui_config.getMenus(field.legendName);
  
  // Generate the layer

  cout << "<Layer vis=\"";
  if (vis_flag)
    cout << "on";
  else
    cout << "off";
  cout << "\" type=\"MDV\" name=\"";
  cout << field.legendName;
  cout << "\" location=\"";
  cout << field.url;
  cout << "\"" << endl;

  cout << "       field=\"";
  cout << field.fieldName;
  cout << "\" render=\"";
  switch (field.renderMethod)
  {
  case GridField::RENDER_POLYGONS :
    cout << "grid";
    break;
  case GridField::RENDER_FILLED_CONTOURS :
    cout << "fillcontours";
    break;
  case GridField::RENDER_DYNAMIC_CONTOURS :     // TODO - what should this be???
    cout << "fillcontours";
    break;
  case GridField::RENDER_LINE_CONTOURS :
    cout << "linecontours";
    break;
  }
  cout << "\" colorscale=\"";
  cout << field.colorFile;
  cout << "\"";
  
  if (menu_groups.size() > 0)
  {
    if (menu_groups.size() > 1)
    {
    cerr << "WARNING: " << endl;
    cerr << "CIDD field " << field.legendName << " belongs to more than 1 menu" << endl;
    cerr << "on the CIDD display.  Jazz currently allows fields to appear in only" << endl;
    cerr << "one menu so this field is only being included in the " << menu_groups[0] << " menu." << endl;
    cerr << endl;
    }
    
    cout << endl;
    cout << "       menuGroup=\"" << menu_groups[0] << "\"";
  }
  
  cout << " menuName=\"";
  cout << field.buttonName;
  cout << "\" />" << endl;
  cout << endl;

  return true;
}


/**********************************************************************
 * _generateMdvwindLayer()
 */

bool JazzXml::_generateMdvwindLayer(const CiddParamFile &cidd_param_file,
				    const WindField &field) const
{
  cout << "<Layer vis=\"";
  cout << "off";
  cout << "\" type=\"MDVWIND\" name=\"";
  cout << field.legendLabel;
  cout << "\" menuName=\"";
  cout << field.legendLabel;
  cout << "\" location=\"";
  cout << field.url;
  cout << "\"" << endl;
  
  cout << "       isUandV=\"true\" field=\"";
  cout << field.uFieldName;
  cout << "\" extraField=\"";
  cout << field.vFieldName;
  cout << "\" render=\"grid\" renderBarbs=\"true\" color=\"";
  cout << field.color;
  cout << "\" colorscale=\"windColors\" />" << endl;

  cout << endl;
  
  return true;
}


/**********************************************************************
 * _generateSymprodLayer()
 */

bool JazzXml::_generateSymprodLayer(const CiddParamFile &cidd_param_file,
				    const SymprodField &field) const
{
  cout << "<Layer vis=\"";
  if (field.onByDefault)
    cout << "on";
  else
    cout << "off";
  cout << "\" type=\"SYMPROD\" name=\"";
  cout << field.menuLabel;
  cout << "\" location=\"";
  cout << field.url;
  cout << "\"" << endl;
  
  cout << "       before=\"";
  _generateTimeInterval(field.searchBefore);
  cout << "\" after=\"";
  _generateTimeInterval(field.searchAfter);
  cout << "\" textOff=\"";
  cout << field.textOffThreshold;
  cout << "\" request=\"";
  switch (field.renderType)
  {
  case SymprodField::RENDER_ALL :
  case SymprodField::RENDER_ALL_BEFORE_DATA_TIME :
  case SymprodField::RENDER_ALL_AFTER_DATA_TIME :
    cout << "interval";
    break;
  case SymprodField::RENDER_ALL_VALID :
  case SymprodField::RENDER_VALID_IN_LAST_FRAME :
  case SymprodField::RENDER_GET_VALID :
  case SymprodField::RENDER_GET_VALID_AT_FRAME_TIME :
    cout << "valid";
    break;
  case SymprodField::RENDER_LATEST_IN_FRAME :
  case SymprodField::RENDER_LATEST_IN_LOOP :
    cout << "latest";
    break;
  case SymprodField::RENDER_FIRST_BEFORE_FRAME_TIME :
  case SymprodField::RENDER_FIRST_BEFORE_DATA_TIME :
    cout << "firstBefore";
    break;
  case SymprodField::RENDER_FIRST_AFTER_DATA_TIME :
    cout << "closest";
    break;
  }
  
  cout << "\" menuName=\"";
  cout << field.menuLabel;
  cout << "\" />" << endl;

  cout << endl;
  
  return true;
}


/**********************************************************************
 * _generateCiddmapLayer()
 */

bool JazzXml::_generateCiddmapLayer(const CiddParamFile &cidd_param_file,
				    const MapField &field) const
{
  cout << "<Layer vis=\"";
  if (field.onFlag)
    cout << "on";
  else
    cout << "off";
  cout << "\" type=\"CIDDMAP\" name=\"";
  cout << field.controlLabel;
  cout << "\" location=\"";
  cout << field.filePath;
  cout << "\"" << endl;

  cout << "       color=\"";
  cout << field.color;
  cout << "\" width=\"";
  cout << field.lineWidth;
  cout << "\" />" << endl;

  cout << endl;

  return true;
}


/**********************************************************************
 * _generateMenuGroups()
 */

bool JazzXml::_generateMenuGroups(const CiddParamFile &cidd_param_file) const
{
  cout << "<!--" << endl;
  cout << " ####################################################################################################################################" << endl;
  cout << "" << endl;
  cout << " Groups" << endl;
  cout << "" << endl;
  cout << " There are three types of groups:" << endl;
  cout << " MenuGroups       - Are groups that determine where layers end up in the GUI menus. If a layer is added to a default" << endl;
  cout << "                    group (GRIDS_MENU, FEATURES_MENU, MAPS_MENU) the layer will appear in the corresponding menu in the" << endl;
  cout << "                    GUI. If a layer is added to a MenuGroup that has one of the default groups as a parent, the layer will" << endl;
  cout << "                    be put in a submenu corresponding to that MenuGroup." << endl;
  cout << " VisibilityGroups - Are used to control the visibility of layers. There is one default VisibilityGroup called" << endl;
  cout << "                    GRIDS_EXCLUSIVE that ensures no more than one grid is visible at a time. Gridded layers are added to" << endl;
  cout << "                    this group by default. To override, specify visibilityGroups=\"none\" for the layer, and you will be" << endl;
  cout << "                    able to show the layer at the same time as other grids." << endl;
  cout << " SpecialGroups    - Are defined in the JAM XML and known to the application code. They enable any sort of custom event" << endl;
  cout << "                    handling within the application. There is one special group available to all apps that have" << endl;
  cout << "                    vertical xsections: XSECT_FEATURES - add feature layers to this group if you want them to show up" << endl;
  cout << "                    in the vertical section display." << endl;
  cout << "" << endl;
  cout << " <MenuGroup> element (optional, any number of these allowed)." << endl;
  cout << " Required attributes:" << endl;
  cout << "" << endl;
  cout << " name        = the name of the group (has to be unique)" << endl;
  cout << " label       = the label to be used on the sub-menu in the GUI." << endl;
  cout << " parentGroup = the name of a menu group to act as the parent of this group. Use one of the default groups (GRIDS_MENU, FEATURES_MENU, or MAPS_MENU)," << endl;
  cout << "               another MenuGroup defined here, or reference a special group that is used to build special menus in the application code." << endl;
  cout << "" << endl;
  cout << "" << endl;
  cout << "<VisibilityGroup> element (optional, any number of these allowed)." << endl;
  cout << " Required attributes:" << endl;
  cout << " name       = the name of the group (has to be unique)" << endl;
  cout << " type       = specifies what control the group will exercise on the visibility its members ('exclusive', or 'synchronized')" << endl;
  cout << "" << endl;
  cout << " -->" << endl;
  cout << endl;
  
  const GuiConfigParams gui_config = cidd_param_file.getGuiConfigParams();
  
  if (gui_config.hasMenusDefined())
  {
    const vector< GridMenu > &menus = gui_config.getMenuList();
    vector< GridMenu >::const_iterator menu;
    
    for (menu = menus.begin(); menu != menus.end(); ++menu)
      cout << "  <MenuGroup name=\"" << menu->menuName
	   << "\" label=\"" << menu->menuLabel
	   << "\" parentGroup=\"GRIDS_MENU\" />" << endl;

  }

  cout << endl;
  
  return true;
}


/**********************************************************************
 * _generateTimeConfiguration()
 */

bool JazzXml::_generateTimeConfiguration(const CiddParamFile &cidd_param_file) const
{
  const MainParams &main_params = cidd_param_file.getMainParams();
  
  cout << "<!--" << endl;
  cout << "####################################################################################################################################" << endl;
  cout << endl;
  cout << " Time configuration (optional, zero or one):" << endl;
  cout << endl;
  cout << " mode     = \"realtime\" or \"archive\" - determines data refresh and time window marching" << endl;
  cout << " start    = in realtime mode, start is relative to now (+ or - number of hrs or mins, or \"now\"), e.g., \"-12hrs\"" << endl;
  cout << "            in archive mode, start must be an explicit time in the format \"yyyy-MM-dd'T'HH:mm\" (e.g. 2008-04-30T23:41)" << endl;
  cout << " end      = in realtime mode, end is relative to now (+ or - number of hrs or mins, or \"now\"), e.g., \"+15mins\"" << endl;
  cout << "            in archive mode, end must be an explicit time in the format \"yyyy-MM-dd'T'HH:mm\" (e.g. 2008-04-30T23:41)" << endl;
  cout << " interval = time between selectable (and animated) ticks, beginning with the start time and ending at or before the end time" << endl;
  cout << " update   = time between checks for data updates (in realtime only). For performance reasons, keep this value >= 5mins." << endl;
  cout << " timeZone = time zone to use for the interpretation of start/end, and to use for the display. Defaults to UTC if omitted." << endl;
  cout << "            Abbreviations and full descriptions are ok (eg: MST or America/Denver).  See http://en.wikipedia.org/wiki/Time_zone" << endl;
  cout << "            for a description, and http://www.timezoneconverter.com/ for a listing" << endl;
  cout << endl;
  cout << "####################################################################################################################################" << endl;
  cout << "-->" << endl;
  cout << endl;

  DateTime demo_time = main_params.getDemoTime();
  
  if (demo_time == DateTime::NEVER)
  {
    int offset_secs =
      main_params.getNumFrames() * main_params.getTimeInterval();
    
    cout << "<Time mode=\"realtime\" start=\"";
    _generateTimeOffset(-offset_secs);
    cout << "\" end=\"+0hrs\" interval=\"";
    _generateTimeInterval(main_params.getTimeInterval());
    cout << "\" update=\"";
    _generateTimeInterval(main_params.getUpdateInterval());
    cout << "\"/>" << endl;
  }
  else
  {
    cout << "<Time mode=\"archive\" start=\"";
    _generateTime(demo_time);
    cout << "\" end=\"";
    _generateTime(demo_time +
		  (main_params.getNumFrames() * main_params.getTimeInterval()));
    cout << "\" interval=\"";
    _generateTimeInterval(main_params.getTimeInterval());
    cout << "\"/>" << endl;
  }
  
  cout << endl;

  return true;
}


/**********************************************************************
 * _generateTimeOffset()
 */

void JazzXml::_generateTimeOffset(const int offset_secs) const
{
  int secs = offset_secs;
  
  // First generate the sign

  if (secs < 0)
    cout << "-";
  else
    cout << "+";
  
  secs = abs(secs);
  
  // Now generate the value.  Use the largest units that make sense.

  if (secs % 3600 == 0)
  {
    // We can use hours

    cout << (secs / 3600) << "hrs";
  }
  else if (secs % 60 == 0)
  {
    // We can use minutes

    cout << (secs / 60) << "mins";
  }
  else
  {
    // We have to use seconds

    cout << secs << "secs";
  }
}

/**********************************************************************
 * _generateTimeInterval()
 */

void JazzXml::_generateTimeInterval(const int seconds) const
{
  if (seconds % 3600 == 0)
    cout << (seconds / 3600) << "hrs";
  else if (seconds % 60 == 0)
    cout << (seconds / 60) << "mins";
  else
    cout << seconds << "secs";
}

/**********************************************************************
 * _generateTime()
 */

void JazzXml::_generateTime(const DateTime &value) const
{
  cout << value.getYear();
  cout << "-";
  cout << setfill('0') << setw(2) << value.getMonth();
  cout << "-";
  cout << setfill('0') << setw(2) << value.getDay();
  cout << "T";
  cout << setfill('0') << setw(2) << value.getHour();
  cout << ":";
  cout << setfill('0') << setw(2) << value.getMin();
  if (value.getSec() != 0)
  {
    cout << ":";
    cout << setfill('0') << setw(2) << value.getSec();
  }
}

/**********************************************************************
 * _generateAnimationConfiguration()
 */

bool JazzXml::_generateAnimationConfiguration(const CiddParamFile &cidd_param_file) const
{
  const MainParams &main_params = cidd_param_file.getMainParams();
  
  cout << "<!--" << endl;
  cout << "####################################################################################################################################" << endl;
  cout << endl;
  cout << " Animation configuration (optional, zero or one):" << endl;
  cout << endl;
  cout << " delay = minimum milliseconds delay between animation frames" << endl;
  cout << " dwell = minimum milliseconds pause at the end of an animation sequence" << endl;
  cout << endl;
  cout << "####################################################################################################################################" << endl;
  cout << "-->" << endl;
  cout << endl;
  cout << "<Animation delay=\"";
  cout << main_params.getMovieSpeed();
  cout << "\" dwell=\"";
  cout << main_params.getMovieDelay();
  cout << "\" />" << endl;
  cout << endl;

  return true;
}


/**********************************************************************
 * _generateAltitudeConfiguration()
 */

bool JazzXml::_generateAltitudeConfiguration(const CiddParamFile &cidd_param_file) const
{
  cout << "<!--" << endl;
  cout << "####################################################################################################################################" << endl;
  cout << endl;
  cout << " Altitude configuration (optional, zero or one):" << endl;
  cout << endl;
  cout << " units    = the units for the vertical scale (km, m, ft, FL, or mb)" << endl;
  cout << " bottom   = lowest altitude of range, in the same units defined above (if in mb, may be greater numerically than top)" << endl;
  cout << " top      = highest altitude of range, in the same units defined above (if in mb, may be lesser numerically than bottom)" << endl;
  cout << " interval = the interval to use for dividing up the distance between bottom and top" << endl;
  cout << " default  = optional initial altitude, in the same units defined above (defaults to same value as \"bottom\")" << endl;
  cout << endl;
  cout << " or" << endl;
  cout << endl;
  cout << " dataDrivenProperties = optional. Overrides other attributes. If true, altitude's range and levels will be" << endl;
  cout << "                        dynamically set according to selected/on layer." << endl;
  cout << endl;
  cout << "####################################################################################################################################" << endl;
  cout << "-->" << endl;
  cout << endl;
  cout << "<!--Altitude units=\"ft\" bottom=\"0\" top=\"40000\" interval=\"1000\" default=\"6000\"/-->" << endl;
  cout << "<!--Altitude units=\"FL\" bottom=\"0\" top=\"300\" interval=\"10\" default=\"100\"/-->" << endl;
  cout << "<!--Altitude units=\"km\" bottom=\"0\" top=\"16\" interval=\"0.25\" default=\"3\"/-->" << endl;
  cout << "<!--Altitude units=\"mb\" bottom=\"1000\" top=\"0\" interval=\"50\" default=\"700\" /-->" << endl;

  // CIDD always uses the data driven properties

  cout << "<Altitude dataDrivenProperties=\"true\" />" << endl;
  cout << endl;

  return true;
}


/**********************************************************************
 * _generateViewConfiguration()
 */

bool JazzXml::_generateViewConfiguration(const CiddParamFile &cidd_param_file) const
{
  const MainParams &main_params = cidd_param_file.getMainParams();
  
  cout << "<!--" << endl;
  cout << "####################################################################################################################################" << endl;
  cout << endl;
  cout << " View configuration (optional, zero or one):" << endl;
  cout << endl;
  cout << "   projection = the projection in which to view the data. One of the below list." << endl;
  cout << endl;
  cout << " Projection-specific configuration:" << endl;
  cout << endl;
  cout << " LatLon:" << endl;
  cout << "   none, defaults to -180 to +180 longitude and -90 to +90 latitude" << endl;
  cout << endl;
  cout << " Flat:" << endl;
  cout << "   originLon  = longitude of the origin" << endl;
  cout << "   originLat  = latitude of the origin" << endl;
  cout << "   rotation = rotation of the projection" << endl;
  cout << endl;
  cout << " LambertConformal:" << endl;
  cout << "   originLat = origin's latitude" << endl;
  cout << "   originLon = origin's longitude" << endl;
  cout << "   stdLat1 = standard latitude 1" << endl;
  cout << "   stdLat2 = standard latitude 2 (optional)" << endl;
  cout << endl;
  cout << " Stereographic:" << endl;
  cout << "   tangentLon = tangent longitude" << endl;
  cout << "   tangentLat = tangent latitude" << endl;
  cout << "   secantLat = secant latitude, if applicable due to secant plane" << endl;
  cout << endl;
  cout << " PolarStereographic:" << endl;
  cout << "   tangentLon = tangent longitude" << endl;
  cout << "   poleIsNorth = \"true\" or \"false\" (optional - defaults to \"true\")" << endl;
  cout << "   centralScale = central scale of projection (optional - defaults to 1.0)" << endl;
  cout << endl;
  cout << " Albers:" << endl;
  cout << "   originLat = origin's latitude" << endl;
  cout << "   originLon = origin's longitude" << endl;
  cout << "   lat1 = standard latitude 1" << endl;
  cout << "   lat2 = standard latitude 2" << endl;
  cout << endl;
  cout << " AzimEquidist:" << endl;
  cout << "   originLat = origin's latitude" << endl;
  cout << "   originLon = origin's longitude" << endl;
  cout << "   rotation = rotation of the projection (optional - defaults to 0.0)" << endl;
  cout << endl;
  cout << " LambertAzimuthal:" << endl;
  cout << "   originLat = origin's latitude" << endl;
  cout << "   originLon = origin's longitude" << endl;
  cout << endl;
  cout << " Mercator:" << endl;
  cout << "   originLat = origin's latitude" << endl;
  cout << "   originLon = origin's longitude" << endl;
  cout << endl;
  cout << " TransMercator:" << endl;
  cout << "   originLat = origin's latitude" << endl;
  cout << "   originLon = origin's longitude" << endl;
  cout << "   centralScale = central scale of the projection (optional - defaults to 1.0)" << endl;
  cout << endl;
  cout << " VerticalPerspective:" << endl;
  cout << "   originLat = origin's latitude" << endl;
  cout << "   originLon = origin's longitude" << endl;
  cout << "   perspRadius = the perspective radius of the projection" << endl;
  cout << endl;
  cout << "####################################################################################################################################" << endl;
  cout << "-->" << endl;
  cout << endl;
  cout << "<!--View projection=\"LambertConformal\" originLon=\"-95.0\" originLat=\"25.0\" stdLat1=\"25.0\" /-->" << endl;
  cout << "<!--View projection=\"Stereographic\" tangentLon=\"-135.0\" tangentLat=\"90.0\"/-->" << endl;
  cout << "<!--View projection=\"LatLon\" /-->" << endl;
  cout << "<!--View projection=\"Flat\" originLon=\"120.433502\" originLat=\"22.526699\" rotation=\"0.0\" /-->" << endl;

  MdvxProj proj = main_params.getDisplayProjection();
  PjgMath math = proj.getPjgMath();
  
  switch (proj.getProjType())
  {
  case Mdvx::PROJ_LATLON :
    _generateLatLonView(math);
    break;
    
  case Mdvx::PROJ_FLAT :
    _generateFlatView(math);
    break;
    
  case Mdvx::PROJ_LAMBERT_CONF :
    _generateLambertConfView(math);
    break;
    
  case Mdvx::PROJ_OBLIQUE_STEREO :
    _generateObliqueStereoView(math);
    break;
    
  case Mdvx::PROJ_ALBERS :
    _generateAlbersView(math);
    break;
    
  case Mdvx::PROJ_LAMBERT_AZIM :
    _generateLambertAzimuthalView(math);
    break;
    
  case Mdvx::PROJ_MERCATOR :
    _generateMercatorView(math);
    break;
    
  case Mdvx::PROJ_POLAR_STEREO :
    _generatePolarStereographicView(math);
    break;
    
  case Mdvx::PROJ_TRANS_MERCATOR :
    _generateTransMercatorView(math);
    break;
    
  case Mdvx::PROJ_VERT_PERSP :
    _generateVerticalPerspectiveView(math);
    break;
    
  default:
    _generateDefaultView();
  }
  
  cout << endl;

  return true;
}


/**********************************************************************
 * _generateDefaultView()
 */

void JazzXml::_generateDefaultView() const
{
  cerr << "WARNING: Unsupported projection specified in CIDD file" << endl;
  cerr << "Using lat/lon projection" << endl;
  
  cout << "<View projection=\"LatLon\" />" << endl;
}


/**********************************************************************
 * _generateLatLonView()
 */

void JazzXml::_generateLatLonView(const PjgMath &math) const
{
  cout << "<View projection=\"LatLon\" />" << endl;
}


/**********************************************************************
 * _generateFlatView()
 */

void JazzXml::_generateFlatView(const PjgMath &math) const
{
  cout << "<View projection=\"Flat\" originLon=\"";
  cout << math.getOriginLon();
  cout << "\" originLat=\"";
  cout << math.getOriginLat();
  cout << "\" rotation=\"";
  cout << math.getRotation();
  cout << "\" />" << endl;
}


/**********************************************************************
 * _generateLambertConfView()
 */

void JazzXml::_generateLambertConfView(const PjgMath &math) const
{
  cout << "<View projection=\"LambertConformal\" originLon=\"";
  cout << math.getOriginLon();
  cout << "\" originLat=\"";
  cout << math.getOriginLat();
  cout << "\" stdLat1=\"";
  cout << math.getLat1();
  cout << "\" />" << endl;
}


/**********************************************************************
 * _generateObliqueStereoView()
 */

void JazzXml::_generateObliqueStereoView(const PjgMath &math) const
{
  cout << "<View projection=\"Stereographic\" tangentLon=\"";
  cout << math.getTangentLon();
  cout << "\" tangentLat=\"";
  cout << math.getTangentLat();
  cout << "\"/>" << endl;
}


/**********************************************************************
 * _generateAlbersView()
 */

void JazzXml::_generateAlbersView(const PjgMath &math) const
{
  cout << "<View projection=\"Albers\" originLon=\"";
  cout << math.getOriginLon();
  cout << "\" originLat=\"";
  cout << math.getOriginLat();
  cout << "\" lat1=\"";
  cout << math.getLat1();
  cout << "\" lat2=\"";
  cout << math.getLat2();
  cout << "\" />" << endl;
}


/**********************************************************************
 * _generateLambertAzimuthalView()
 */

void JazzXml::_generateLambertAzimuthalView(const PjgMath &math) const
{
  cout << "<View projection=\"LambertAzimuthal\" originLon=\"";
  cout << math.getOriginLon();
  cout << "\" originLat=\"";
  cout << math.getOriginLat();
  cout << "\" />" << endl;
}


/**********************************************************************
 * _generateMercatorView()
 */

void JazzXml::_generateMercatorView(const PjgMath &math) const
{
  cout << "<View projection=\"Mercator\" originLon=\"";
  cout << math.getOriginLon();
  cout << "\" originLat=\"";
  cout << math.getOriginLat();
  cout << "\" />" << endl;
}


/**********************************************************************
 * _generatePolarStereographicView()
 */

void JazzXml::_generatePolarStereographicView(const PjgMath &math) const
{
  cout << "<View projection=\"PolarStereographic\" tangentLon=\"";
  cout << math.getTangentLon();
  cout << "\" poleIsNorth=\"";
  if (math.getPole() == PjgTypes::POLE_NORTH)
    cout << "true";
  else
    cout << "false";
  cout << "\" centralScale=\"";
  cout << math.getCentralScale();
  cout << "\" />" << endl;
}


/**********************************************************************
 * _generateTransMercatorView()
 */

void JazzXml::_generateTransMercatorView(const PjgMath &math) const
{
  cout << "<View projection=\"TransMercator\" originLon=\"";
  cout << math.getOriginLon();
  cout << "\" originLat=\"";
  cout << math.getOriginLat();
  cout << "\" centralScale=\"";
  cout << math.getCentralScale();
  cout << "\" />" << endl;
}


/**********************************************************************
 * _generateVerticalPerspectiveView()
 */

void JazzXml::_generateVerticalPerspectiveView(const PjgMath &math) const
{
  cout << "<View projection=\"VerticalPerspective\" originLon=\"";
  cout << math.getOriginLon();
  cout << "\" originLat=\"";
  cout << math.getOriginLat();
  cout << "\" perspRadius=\"";
  cout << math.getPerspRadius();
  cout << "\" />" << endl;
}


/**********************************************************************
 * _generateAreasOfInterest()
 */

bool JazzXml::_generateAreasOfInterest(const CiddParamFile &cidd_param_file) const
{
  const MainParams &main_params = cidd_param_file.getMainParams();
  
  cout << "<!--" << endl;
  cout << "####################################################################################################################################" << endl;
  cout << endl;
  cout << " Areas of Interest (optional, zero or more):" << endl;
  cout << endl;
  cout << "   name = the name of the Area shown in the display" << endl;
  cout << " minLon = the minimum longitude of the lat/lon bounding box defining the Area in decimal degrees or degrees, minutes, seconds" << endl;
  cout << " minLat = the minimum latitude of the lat/lon bounding box defining the Area in decimal degrees or degrees, minutes, seconds" << endl;
  cout << " maxLon = the maximum longitude of the lat/lon bounding box defining the Area in decimal degrees or degrees, minutes, seconds" << endl;
  cout << " maxLat = the maximum latitude of the lat/lon bounding box defining the Area in decimal degrees or degrees, minutes, seconds" << endl;
  cout << " defaultView = (optional) true or false; if true, then the Area will be the default view for the main window" << endl;
  cout << endl;
  cout << " Note:" << endl;
  cout << "   - If more than one Area has a defaultView of true, then the last one listed will be the default view" << endl;
  cout << endl;
  cout << "    name  defaultView   minLon   minLat maxLon  maxLat" << endl;
  cout << endl;
  cout << "####################################################################################################################################" << endl;
  cout << "-->" << endl;
  cout << endl;

  vector< Zoom > zooms = main_params.getZooms();
  vector< Zoom >::const_iterator zoom;
  
  MdvxProj proj = main_params.getDisplayProjection();
  
  for (zoom = zooms.begin(); zoom != zooms.end(); ++zoom)
  {
    double min_lat, min_lon;
    double max_lat, max_lon;
    
    proj.xy2latlon(zoom->minX, zoom->minY, min_lat, min_lon);
    proj.xy2latlon(zoom->maxX, zoom->maxY, max_lat, max_lon);
    
    cout << "<Area name=\"";
    cout << zoom->name;
    cout << "\" minLon=\"";
    cout << min_lon;
    cout << "\" maxLon=\"";
    cout << max_lon;
    cout << "\" minLat=\"";
    cout << min_lat;
    cout << "\" maxLat=\"";
    cout << max_lat;
    cout << "\"";
    if (zoom->isDefault)
      cout << " defaultView=\"true\"";
    cout << "/>" << endl;
  }
  
  cout << endl;

  return true;
}


/**********************************************************************
 * _generateWindowParameters()
 */

bool JazzXml::_generateWindowParameters(const CiddParamFile &cidd_param_file) const
{
  const MainParams &main_params = cidd_param_file.getMainParams();
  
  cout << "<!--" << endl;
  cout << "####################################################################################################################################" << endl;
  cout << endl;
  cout << " Window parameters (optional element, zero or one):" << endl;
  cout << endl;
  cout << " width = pixel width of the window, if the screen is big enough" << endl;
  cout << " height = pixel height of the window, if the screen is big enough" << endl;
  cout << " xOrigin = (optional) pixel x location of the upper left of the window, measured from upper left of the screen" << endl;
  cout << " yOrigin = (optional) pixel y location of the upper left of the window, measured from upper left of the screen (positive downwards)" << endl;
  cout << " backgroundColor = (optional) defaults to white" << endl;
  cout << endl;
  cout << "####################################################################################################################################" << endl;
  cout << "-->" << endl;

  cout << "<Window width=\"";
  cout << main_params.getHorizDefaultWidth();
  cout << "\" height=\"";
  cout << main_params.getHorizDefaultHeight();
  cout << "\" xOrigin=\"";
  cout << main_params.getHorizDefaultXPos();
  cout << "\" yOrigin=\"";
  cout << main_params.getHorizDefaultYPos();
  cout << "\" backgroundColor=\"";
  cout << main_params.getBackgroundColor();
  cout << "\"/>" << endl;

  cout << endl;

  return true;
}


/**********************************************************************
 * _generateXSections()
 */

bool JazzXml::_generateXSections(const CiddParamFile &cidd_param_file) const
{
  cout << "<!--" << endl;
  cout << "####################################################################################################################################" << endl;
  cout << endl;
  cout << " This section MUST appear *below* the Layer definitions!" << endl;
  cout << endl;
  cout << " XSections:" << endl;
  cout << endl;
  cout << " XSections = optional element. Zero or one allowed. If this element is missing, then cross-sections will be disabled." << endl;
  cout << " Path = optional element representing a preconfigured xsect path. Zero or more allowed.  Leave this out to enable arbitrary," << endl;
  cout << "        user-entered cross sections." << endl;
  cout << "   name = name attribute for the xsection Path element" << endl;
  cout << "   Waypoint = 2 or more elements required for each Path. Ordered waypoints for the path. Requires lat and lon attributes." << endl;
  cout << endl;
  cout << "####################################################################################################################################" << endl;
  cout << "-->" << endl;
  cout << endl;
  cout << "<!--XSections>" << endl;
  cout << "  <Path name=\"Test 1\">" << endl;
  cout << "    <Waypoint lat=\"23.147\" lon=\"120.056\"/>" << endl;
  cout << "    <Waypoint lat=\"22.862\" lon=\"120.134\"/>" << endl;
  cout << "  </Path>" << endl;
  cout << "  <Path name=\"Test 2\">" << endl;
  cout << "    <Waypoint lat=\"22.309\" lon=\"119.75\"/>" << endl;
  cout << "    <Waypoint lat=\"22.494\" lon=\"120.244\"/>" << endl;
  cout << "  </Path>" << endl;
  cout << "</XSections-->" << endl;
  cout << "" << endl;
  cout << "<XSections>" << endl;
  cout << "</XSections>" << endl;
  cout << "" << endl;

  return true;
}


/**********************************************************************
 * _generateSkewTParameters()
 */

bool JazzXml::_generateSkewTParameters(const CiddParamFile &cidd_param_file) const
{
  cout << "<!--" << endl;
  cout << "####################################################################################################################################" << endl;
  cout << endl;
  cout << " SkewT parameters:" << endl;
  cout << endl;
  cout << " SkewT            = optional element. Zero or one allowed. If this element is missing, then SkewTs will be disabled." << endl;
  cout << " location         = Required. where to get the data eg: mdvp:://yaw:10000:ruc/mdv/lambert" << endl;
  cout << " temperatureField = Required. Name of the MDV 3-D temperature field." << endl;
  cout << " rhField          = either rhField or dewpointField required.  Name of the MDV 3-D relative humidity field. Server side units" << endl;
  cout << "                    must be correct (ie: % means 0..100, whereas unitless means 0..1)" << endl;
  cout << " dewpointField    = either rhField or dewpointField required. Name of the MDV 3-D dewpoint field." << endl;
  cout << " uField           = Required. Name of the MDV 3-D u-wind field. Speed/dir not supported (yet)" << endl;
  cout << " vField           = Required. Name of the MDV 3-D v-wind field. Speed/dir not supported (yet)" << endl;
  cout << endl;
  cout << "####################################################################################################################################" << endl;
  cout << "-->" << endl;
  cout << endl;
  cout << "<!--SkewT location=\"mdvp:://yaw:10000:ruc/mdv/lambert\"" << endl;
  cout << "       temperatureField=\"TMP\"" << endl;
  cout << "       rhField=\"R_H\"" << endl;
  cout << "       uField=\"UGRD\"" << endl;
  cout << "       vField=\"VGRD\">" << endl;
  cout << "</SkewT-->" << endl;
  cout << endl;
  cout << "<!--SkewT location=\"mdvp:://yaw:10000:ruc/mdv/lambert\"" << endl;
  cout << "       temperatureField=\"TMP\"" << endl;
  cout << "       rhField=\"R_H\"" << endl;
  cout << "       uField=\"UGRD\"" << endl;
  cout << "       vField=\"VGRD\">" << endl;
  cout << "</SkewT-->" << endl;
  cout << endl;

  return true;
}


/**********************************************************************
 * _generateOptionalToolParameters()
 */

bool JazzXml::_generateOptionalToolParameters(const CiddParamFile &cidd_param_file) const
{

  cout << "<!--" << endl;
  cout << "####################################################################################################################################" << endl;
  cout << endl;
  cout << " Optional tools.  Each tool will only appear if the associated element is found in this file." << endl;
  cout << " You can comment out tools that are not desired." << endl;
  cout << endl;
  cout << "####################################################################################################################################" << endl;
  cout << "-->" << endl;
  cout << endl;
  cout << "<SwapFieldsTool/>" << endl;
  cout << endl;

  return true;
}

/**********************************************************************
 * _generateColorScales()
 */

bool JazzXml::_generateColorScales(const CiddParamFile &cidd_param_file) const
{
  cout << "<!--" << endl;
  cout << "  ####################################################################################################################################" << endl;
  cout << endl;
  cout << "   ColorScale configuration (zero or more):" << endl;
  cout << endl;
  cout << "   id    = the name of the colorscale, used to refer to it from \"Layer\" defintions" << endl;
  cout << "   Range = one or more Range elements required" << endl;
  cout << "   min   = the minimum value of the bin; attribute of the Range element, required" << endl;
  cout << "   max   = the maximum value of the bin; attribute of the Range element, required" << endl;
  cout << "   color = an X11-defined color name, RGB triplet (0..255 for each value), or hex group (\"wheat1\", \"153,153,153\", \"#ffffcc\");" << endl;
  cout << "           attribute of the Range element, required" << endl;
  cout << "   label = the optional custom label to apply to this bin. Defaults to the numeric values of the bin boundaries." << endl;
  cout << "           ; attribute of the Range element, optional" << endl;
  cout << endl;
  cout << "   Note:" << endl;
  cout << "      - when a value can be in more than one bin (overlapping) the last matching (descending) bin color is used" << endl;
  cout << endl;
  cout << "  ####################################################################################################################################" << endl;
  cout << "  -->" << endl;
  cout << "<ColorScale id=\"airColorscale\">" << endl;
  cout << "  <Range min=\"-80\" max=\"-35\" color=\"LightYellow1\"      label=\"-35<\" />" << endl;
  cout << "  <Range min=\"-35\" max=\"-30\" color=\"LightYellow2\"      label=\"\" />" << endl;
  cout << "  <Range min=\"-30\" max=\"-25\" color=\"wheat1\"            label=\"\" />" << endl;
  cout << "  <Range min=\"-25\" max=\"-15\" color=\"LightYellow3\"      label=\"\" />" << endl;
  cout << "  <Range min=\"-15\" max=\"-10\" color=\"RosyBrown3\"        label=\"\" />" << endl;
  cout << "  <Range min=\"-10\" max=\"-5\"  color=\"medium slate blue\" label=\"\" />" << endl;
  cout << "  <Range min=\"-5\"  max=\"0\"   color=\"dark slate blue\"   label=\"\" />" << endl;
  cout << "  <Range min=\"0\"   max=\"5\"   color=\"blue\"              label=\"\" />" << endl;
  cout << "  <Range min=\"5\"   max=\"10\"  color=\"royal blue\"        label=\"\" />" << endl;
  cout << "  <Range min=\"10\"  max=\"14\"  color=\"slate gray\"        label=\"\" />" << endl;
  cout << "  <Range min=\"14\"  max=\"17\"  color=\"dark slate gray\"   label=\"\" />" << endl;
  cout << "  <Range min=\"17\"  max=\"20\"  color=\"dark green\"        label=\"\" />" << endl;
  cout << "  <Range min=\"20\"  max=\"23\"  color=\"forest green\"      label=\"\" />" << endl;
  cout << "  <Range min=\"23\"  max=\"26\"  color=\"lime green\"        label=\"\" />" << endl;
  cout << "  <Range min=\"26\"  max=\"29\"  color=\"lawn green\"        label=\"\" />" << endl;
  cout << "  <Range min=\"29\"  max=\"32\"  color=\"yellow\"            label=\"\" />" << endl;
  cout << "  <Range min=\"32\"  max=\"35\"  color=\"gold1\"             label=\"\" />" << endl;
  cout << "  <Range min=\"35\"  max=\"38\"  color=\"DarkGoldenrod1\"    label=\"\" />" << endl;
  cout << "  <Range min=\"38\"  max=\"41\"  color=\"orange\"            label=\"\" />" << endl;
  cout << "  <Range min=\"41\"  max=\"44\"  color=\"sienna1\"           label=\"\" />" << endl;
  cout << "  <Range min=\"44\"  max=\"47\"  color=\"orange red\"        label=\"\" />" << endl;
  cout << "  <Range min=\"47\"  max=\"50\"  color=\"red1\" />" << endl;
  cout << "</ColorScale>" << endl;
  cout << endl;
  cout << "<ColorScale id=\"tempColorscale\">" << endl;
  cout << "  <Range min=\"-50\" max=\"-45\"  color=\"204,153,255\"/>" << endl;
  cout << "  <Range min=\"-45\" max=\"-40\"  color=\"153,102,255\"/>" << endl;
  cout << "  <Range min=\"-40\" max=\"-35\"  color=\"102,51,153\"/>" << endl;
  cout << "  <Range min=\"-35\" max=\"-30\"  color=\"0,0,204\"/>" << endl;
  cout << "  <Range min=\"-30\" max=\"-25\"  color=\"0,0,255\"/>" << endl;
  cout << "  <Range min=\"-25\" max=\"-20\"  color=\"51,102,255\"/>" << endl;
  cout << "  <Range min=\"-20\" max=\"-15\"  color=\"51,153,255\"/>" << endl;
  cout << "  <Range min=\"-15\" max=\"-10\"  color=\"102,204,255\"/>" << endl;
  cout << "  <Range min=\"-10\" max=\"-5\"   color=\"153,204,255\"/>" << endl;
  cout << "  <Range min=\"-5\"  max=\"0\"    color=\"204,204,255\"/>" << endl;
  cout << "  <Range min=\"0\"   max=\"5\"    color=\"255,255,153\"/>" << endl;
  cout << "  <Range min=\"5\"   max=\"10\"   color=\"255,204,0\"/>" << endl;
  cout << "  <Range min=\"10\"  max=\"15\"   color=\"255,153,0\"/>" << endl;
  cout << "  <Range min=\"15\"  max=\"20\"   color=\"255,102,0\"/>" << endl;
  cout << "  <Range min=\"20\"  max=\"25\"   color=\"255,51,0\"/>" << endl;
  cout << "  <Range min=\"25\"  max=\"30\"   color=\"255,0,0\"/>" << endl;
  cout << "  <Range min=\"30\"  max=\"35\"   color=\"204,0,0\"/>" << endl;
  cout << "</ColorScale>" << endl;
  cout << endl;
  cout << "<!--" << endl;
  cout << " wind.colors" << endl;
  cout << endl;
  cout << " For the Taiwan system. Winds below 60" << endl;
  cout << " are cool colours - over 60" << endl;
  cout << " they are hot colours." << endl;
  cout << "-->" << endl;
  cout << "<ColorScale id=\"windColorsTaiwan\">" << endl;
  cout << "  <Range min=\"0\"   max=\"15\"  color=\"75,75,75\" />" << endl;
  cout << "  <Range min=\"15\"  max=\"30\"  color=\"102,102,102\" />" << endl;
  cout << "  <Range min=\"30\"  max=\"45\"  color=\"128,128,128\" />" << endl;
  cout << "  <Range min=\"45\"  max=\"60\"  color=\"153,153,153\" />" << endl;
  cout << "  <Range min=\"60\"  max=\"70\"  color=\"dark slate blue\" />" << endl;
  cout << "  <Range min=\"70\"  max=\"80\"  color=\"RoyalBlue\" />" << endl;
  cout << "  <Range min=\"80\"  max=\"90\"  color=\"SteelBlue1\" />" << endl;
  cout << "  <Range min=\"90\"  max=\"100\" color=\"purple\" />" << endl;
  cout << "  <Range min=\"100\" max=\"110\" color=\"sea green\" />" << endl;
  cout << "  <Range min=\"110\" max=\"120\" color=\"lime green\" />" << endl;
  cout << "  <Range min=\"120\" max=\"150\" color=\"orange\" />" << endl;
  cout << "  <Range min=\"150\" max=\"200\" color=\"chocolate\" />" << endl;
  cout << "</ColorScale>" << endl;
  cout << endl;
  cout << "<ColorScale id=\"windColors\">" << endl;
  cout << "  <Range min=\"0\"   max=\"15\"  color=\"255,255,255\"/>" << endl;
  cout << "  <Range min=\"15\"   max=\"30\"  color=\"204,255,255\"/>" << endl;
  cout << "  <Range min=\"30\"   max=\"45\"  color=\"153,255,255\"/>" << endl;
  cout << "  <Range min=\"45\"   max=\"60\"  color=\"153,255,204\"/>" << endl;
  cout << "  <Range min=\"60\"   max=\"75\"  color=\"153,255,102\"/>" << endl;
  cout << "  <Range min=\"75\"   max=\"90\"  color=\"204,255,102\"/>" << endl;
  cout << "  <Range min=\"90\"   max=\"105\"  color=\"255,255,153\"/>" << endl;
  cout << "  <Range min=\"105\"   max=\"120\"  color=\"255,204,0\"/>" << endl;
  cout << "  <Range min=\"120\"   max=\"135\"  color=\"255,153,0\"/>" << endl;
  cout << "  <Range min=\"135\"   max=\"150\"  color=\"255,102,0\"/>" << endl;
  cout << "  <Range min=\"150\"   max=\"165\"  color=\"255,51,0\"/>" << endl;
  cout << "  <Range min=\"165\"   max=\"180\"  color=\"204,0,0\"/>" << endl;
  cout << "</ColorScale>" << endl;
  cout << endl;
  cout << "<ColorScale id=\"radar_colorscale\">" << endl;
  cout << "  <Range min=\"5.0\" max=\"10.0\" color=\"102,255,255\"/>" << endl;
  cout << "  <Range min=\"10.0\" max=\"15.0\" color=\"51,153,255\"/>" << endl;
  cout << "  <Range min=\"15.0\" max=\"20.0\" color=\"0,0,255\"/>" << endl;
  cout << "  <Range min=\"20.0\" max=\"25.0\" color=\"0,255,0\"/>" << endl;
  cout << "  <Range min=\"25.0\" max=\"30.0\" color=\"0,204,0\"/>" << endl;
  cout << "  <Range min=\"30.0\" max=\"35.0\" color=\"0,153,0\"/>" << endl;
  cout << "  <Range min=\"35.0\" max=\"40.0\" color=\"255,255,0\"/>" << endl;
  cout << "  <Range min=\"40.0\" max=\"45.0\" color=\"255,204,0\"/>" << endl;
  cout << "  <Range min=\"45.0\" max=\"50.0\" color=\"255,102,0\"/>" << endl;
  cout << "  <Range min=\"50.0\" max=\"55.0\" color=\"255,0,0\"/>" << endl;
  cout << "  <Range min=\"55.0\" max=\"60.0\" color=\"204,51,0\"/>" << endl;
  cout << "  <Range min=\"60.0\" max=\"65.0\" color=\"153,0,0\"/>" << endl;
  cout << "  <Range min=\"65.0\" max=\"70.0\" color=\"255,0,255\"/>" << endl;
  cout << "  <Range min=\"70.0\" max=\"75.0\" color=\"153,51,204\"/>" << endl;
  cout << "</ColorScale>" << endl;
  cout << endl;
  cout << "<ColorScale id=\"saudi_dbz_colorscale\">" << endl;
  cout << "  <Range min=\"-10.0\" max=\"-5.0\" color=\"53,53,53\"/>" << endl;
  cout << "  <Range min=\"-5.0\" max=\"0.0\" color=\"117,117,117\"/>" << endl;
  cout << "  <Range min=\"0.0\" max=\"5.0\" color=\"0,90,0\"/>" << endl;
  cout << "  <Range min=\"5.0\" max=\"10.0\" color=\"0,126,0\"/>" << endl;
  cout << "  <Range min=\"10.0\" max=\"15.0\" color=\"28,71,232\"/>" << endl;
  cout << "  <Range min=\"15.0\" max=\"20.0\" color=\"8,127,219\"/>" << endl;
  cout << "  <Range min=\"20.0\" max=\"25.0\" color=\"110,13,198\"/>" << endl;
  cout << "  <Range min=\"25.0\" max=\"30.0\" color=\"200,15,134\"/>" << endl;
  cout << "  <Range min=\"30.0\" max=\"35.0\" color=\"192,100,135\"/>" << endl;
  cout << "  <Range min=\"35.0\" max=\"40.0\" color=\"210,136,59\"/>" << endl;
  cout << "  <Range min=\"40.0\" max=\"45.0\" color=\"250,196,49\"/>" << endl;
  cout << "  <Range min=\"45.0\" max=\"50.0\" color=\"254,250,3\"/>" << endl;
  cout << "  <Range min=\"50.0\" max=\"54.0\" color=\"254,154,88\"/>" << endl;
  cout << "  <Range min=\"54.0\" max=\"58.0\" color=\"254,95,5\"/>" << endl;
  cout << "  <Range min=\"58.0\" max=\"62.0\" color=\"253,52,28\"/>" << endl;
  cout << "  <Range min=\"62.0\" max=\"66.0\" color=\"190,190,190\"/>" << endl;
  cout << "  <Range min=\"66.0\" max=\"70.0\" color=\"211,211,211\"/>" << endl;
  cout << "</ColorScale>" << endl;
  cout << endl;
  cout << "<ColorScale id=\"torp_colorscale\">" << endl;
  cout << "  <Range min=\"-35\" max=\"-30\" color=\"102,255,255\"/>" << endl;
  cout << "  <Range min=\"-30\" max=\"-25\" color=\"51,153,255\"/>" << endl;
  cout << "  <Range min=\"-25\" max=\"-20\" color=\"0,0,255\"/>" << endl;
  cout << "  <Range min=\"-20.0\" max=\"-15.0\" color=\"0,255,0\"/>" << endl;
  cout << "  <Range min=\"-15.0\" max=\"-10.0\" color=\"0,204,0\"/>" << endl;
  cout << "  <Range min=\"-10.0\" max=\"-5.0\" color=\"0,153,0\"/>" << endl;
  cout << "  <Range min=\"-5.0\" max=\"0.0\" color=\"255,255,0\"/>" << endl;
  cout << "  <Range min=\"0.0\" max=\"5.0\" color=\"255,204,0\"/>" << endl;
  cout << "  <Range min=\"5.0\" max=\"10.0\" color=\"255,102,0\"/>" << endl;
  cout << "  <Range min=\"10.0\" max=\"15.0\" color=\"255,0,0\"/>" << endl;
  cout << "  <Range min=\"15.0\" max=\"20.0\" color=\"204,51,0\"/>" << endl;
  cout << "  <Range min=\"20.0\" max=\"25.0\" color=\"153,0,0\"/>" << endl;
  cout << "  <Range min=\"25.0\" max=\"30.0\" color=\"255,0,255\"/>" << endl;
  cout << "  <Range min=\"30.0\" max=\"35.0\" color=\"153,51,204\"/>" << endl;
  cout << "</ColorScale>" << endl;
  cout << endl;
  cout << "<ColorScale id=\"topo_colorscale\">" << endl;
  cout << "  <Range min=\"0\" max=\"315\" color=\"102,51,0\"/>" << endl;
  cout << "  <Range min=\"315\" max=\"630\" color=\"111,60,9\"/>" << endl;
  cout << "  <Range min=\"630\" max=\"945\" color=\"119,68,17\"/>" << endl;
  cout << "  <Range min=\"945\" max=\"1260\" color=\"128,77,26\"/>" << endl;
  cout << "  <Range min=\"1260\" max=\"1575\" color=\"137,86,35\"/>" << endl;
  cout << "  <Range min=\"1575\" max=\"1890\" color=\"145,94,43\"/>" << endl;
  cout << "  <Range min=\"1890\" max=\"2215\" color=\"153,102,51\"/>" << endl;
  cout << "  <Range min=\"2215\" max=\"2530\" color=\"162,111,60\"/>" << endl;
  cout << "  <Range min=\"2530\" max=\"2845\" color=\"170,119,68\"/>" << endl;
  cout << "  <Range min=\"2845\" max=\"3160\" color=\"179,128,77\"/>" << endl;
  cout << "  <Range min=\"3160\" max=\"3475\" color=\"188,137,86\"/>" << endl;
  cout << "  <Range min=\"3475\" max=\"3790\" color=\"196,145,94\"/>" << endl;
  cout << "  <Range min=\"3790\" max=\"4105\" color=\"204,153,102\"/>" << endl;
  cout << "  <Range min=\"4105\" max=\"4420\" color=\"213,162,111\"/>" << endl;
  cout << "  <Range min=\"4420\" max=\"4735\" color=\"221,170,119\"/>" << endl;
  cout << "  <Range min=\"4735\" max=\"5050\" color=\"230,179,128\"/>" << endl;
  cout << "  <Range min=\"5050\" max=\"5365\" color=\"239,188,137\"/>" << endl;
  cout << "  <Range min=\"5365\" max=\"5680\" color=\"247,196,145\"/>" << endl;
  cout << "  <Range min=\"5680\" max=\"6000\" color=\"255,204,153\"/>" << endl;
  cout << "</ColorScale>" << endl;
  cout << endl;
  cout << "<!--ColorScale" << endl;
  cout << "      id = \"radar_colorscale\"" << endl;
  cout << "      className = \"edu.ucar.rap.jade.view.legend.ColorScale\"" << endl;
  cout << "      axisLabel = \"dBZ\"" << endl;
  cout << "      unitLabel = \"dBZ\">" << endl;
  cout << "      <ColorRange minValue = \"-15.0\" maxValue = \"-10.0\">" << endl;
  cout << "        <Color  red = \"0\" green = \"100\" blue = \"0\" />" << endl;
  cout << "      </ColorRange>" << endl;
  cout << "      <ColorRange minValue = \"-10.0\" maxValue = \"-6.0\">" << endl;
  cout << "        <Color  red = \"85\" green = \"107\" blue = \"47\" />" << endl;
  cout << "      </ColorRange>" << endl;
  cout << "      <ColorRange minValue = \"-6.0\" maxValue = \"-3.0\">" << endl;
  cout << "        <Color  red = \"34\" green = \"139\" blue = \"34\" />" << endl;
  cout << "      </ColorRange>" << endl;
  cout << "      <ColorRange minValue = \"-3.0\" maxValue = \"0.0\">" << endl;
  cout << "        <Color  red = \"0\" green = \"205\" blue = \"102\" />" << endl;
  cout << "      </ColorRange>" << endl;
  cout << "      <ColorRange minValue = \"0.0\" maxValue = \"3.0\">" << endl;
  cout << "        <Color  red = \"60\" green = \"179\" blue = \"113\" />" << endl;
  cout << "      </ColorRange>" << endl;
  cout << "      <ColorRange minValue = \"3.0\" maxValue = \"6.0\">" << endl;
  cout << "        <Color  red = \"102\" green = \"205\" blue = \"170\" />" << endl;
  cout << "      </ColorRange>" << endl;
  cout << "      <ColorRange minValue = \"6.0\" maxValue = \"9.0\">" << endl;
  cout << "        <Color  red = \"123\" green = \"104\" blue = \"238\" />" << endl;
  cout << "      </ColorRange>" << endl;
  cout << "      <ColorRange minValue = \"9.0\" maxValue = \"12.0\">" << endl;
  cout << "        <Color  red = \"0\" green = \"0\" blue = \"255\" />" << endl;
  cout << "      </ColorRange>" << endl;
  cout << "      <ColorRange minValue = \"12.0\" maxValue = \"15.0\">" << endl;
  cout << "        <Color  red = \"0\" green = \"0\" blue = \"139\" />" << endl;
  cout << "      </ColorRange>" << endl;
  cout << "      <ColorRange minValue = \"15.0\" maxValue = \"18.0\">" << endl;
  cout << "        <Color  red = \"104\" green = \"34\" blue = \"139\" />" << endl;
  cout << "      </ColorRange>" << endl;
  cout << "      <ColorRange minValue = \"18.0\" maxValue = \"21.0\">" << endl;
  cout << "        <Color  red = \"139\" green = \"58\" blue = \"98\" />" << endl;
  cout << "      </ColorRange>" << endl;
  cout << "      <ColorRange minValue = \"21.0\" maxValue = \"24.0\">" << endl;
  cout << "        <Color  red = \"176\" green = \"48\" blue = \"96\" />" << endl;
  cout << "      </ColorRange>" << endl;
  cout << "      <ColorRange minValue = \"24.0\" maxValue = \"27.0\">" << endl;
  cout << "        <Color  red = \"139\" green = \"34\" blue = \"82\" />" << endl;
  cout << "      </ColorRange>" << endl;
  cout << "      <ColorRange minValue = \"27.0\" maxValue = \"31.0\">" << endl;
  cout << "        <Color  red = \"160\" green = \"82\" blue = \"45\" />" << endl;
  cout << "      </ColorRange>" << endl;
  cout << "      <ColorRange minValue = \"31.0\" maxValue = \"35.0\">" << endl;
  cout << "        <Color  red = \"210\" green = \"105\" blue = \"30\" />" << endl;
  cout << "      </ColorRange>" << endl;
  cout << "      <ColorRange minValue = \"35.0\" maxValue = \"40.0\">" << endl;
  cout << "        <Color  red = \"218\" green = \"165\" blue = \"32\" />" << endl;
  cout << "      </ColorRange>" << endl;
  cout << "      <ColorRange minValue = \"40.0\" maxValue = \"45.0\">" << endl;
  cout << "        <Color  red = \"255\" green = \"255\" blue = \"0\" />" << endl;
  cout << "      </ColorRange>" << endl;
  cout << "      <ColorRange minValue = \"45.0\" maxValue = \"50.0\">" << endl;
  cout << "        <Color  red = \"233\" green = \"150\" blue = \"122\" />" << endl;
  cout << "      </ColorRange>" << endl;
  cout << "      <ColorRange minValue = \"50.0\" maxValue = \"55.0\">" << endl;
  cout << "        <Color  red = \"250\" green = \"128\" blue = \"114\" />" << endl;
  cout << "      </ColorRange>" << endl;
  cout << "      <ColorRange minValue = \"55.0\" maxValue = \"60.0\">" << endl;
  cout << "        <Color  red = \"238\" green = \"44\" blue = \"44\" />" << endl;
  cout << "      </ColorRange>" << endl;
  cout << "      <ColorRange minValue = \"60.0\" maxValue = \"65.0\">" << endl;
  cout << "        <Color  red = \"255\" green = \"20\" blue = \"147\" />" << endl;
  cout << "      </ColorRange>" << endl;
  cout << "      <ColorRange minValue = \"65.0\" maxValue = \"70.0\">" << endl;
  cout << "        <Color  red = \"211\" green = \"211\" blue = \"211\" />" << endl;
  cout << "      </ColorRange>" << endl;
  cout << "      <ColorRange minValue = \"70.0\" maxValue = \"80.0\">" << endl;
  cout << "        <Color  red = \"250\" green = \"250\" blue = \"250\" />" << endl;
  cout << "      </ColorRange>" << endl;
  cout << "</ColorScale-->" << endl;
  cout << endl;

  return true;
}


/**********************************************************************
 * _generateFooter()
 */

bool JazzXml::_generateFooter(const CiddParamFile &cidd_param_file) const
{
  cout << "<!--" << endl;
  cout << "####################################################################################################################################" << endl;
  cout << endl;
  cout << " Other configuration parameters:" << endl;
  cout << endl;
  cout << "####################################################################################################################################" << endl;
  cout << "-->" << endl;
  cout << endl;
  cout << "</Jazz>" << endl;
  
  return true;
}
