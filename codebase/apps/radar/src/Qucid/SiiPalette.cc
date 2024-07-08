#include <iostream>

//#include <dd_crackers.hh>
//#include <sii_utils.hh>
//#include <solo_window_structs.h>

#include "SiiPalette.hh"
#include "ColorTableManager.hh"

/**********************************************************************
 * Constructor
 */

SiiPalette::SiiPalette() :
  _paletteName(""),
  _colorTableName("carbone17"),
  _numColors(17) ,
  _centerDataValue(0.0),
  _colorWidth(5.0),
  _minValue(_centerDataValue - 0.5 * _numColors * _colorWidth),
  _maxValue(_centerDataValue + _numColors * _colorWidth),
  _emphasisZoneLower(0.0),
  _emphasisZoneUpper(0.0),
  _gridColor("gray90"),
  _missingDataColor("darkslateblue"),
  _exceededColor("gray70"),
  _annotationColor("gray90"),
  _backgroundColor("midnightblue"),
  _emphasisColor("hotpink"),
  _boundaryColor("orangered")
{
  _initColorTables();
}


/**********************************************************************
 * Constructor
 */

SiiPalette::SiiPalette(const string &palette_name,
		       const string &usual_parms) :
  _paletteName(palette_name),
  _colorTableName("carbone17"),
  _numColors(17) ,
  _centerDataValue(0.0),
  _colorWidth(5.0),
  _minValue(_centerDataValue - 0.5 * _numColors * _colorWidth),
  _maxValue(_centerDataValue + _numColors * _colorWidth),
  _emphasisZoneLower(0.0),
  _emphasisZoneUpper(0.0),
  _gridColor("gray90"),
  _missingDataColor("darkslateblue"),
  _exceededColor("gray70"),
  _annotationColor("gray90"),
  _backgroundColor("midnightblue"),
  _emphasisColor("hotpink"),
  _boundaryColor("orangered")
{
  setUsualParameters(usual_parms);
  
  _initColorTables();
}


/**********************************************************************
 * Constructor
 */

SiiPalette::SiiPalette(const string &palette_name,
		       const string &usual_parms,
		       const double center_data_value, const double color_width,
		       const string &color_table_name) :
  _paletteName(palette_name),
  _colorTableName(color_table_name),
  _numColors(17) ,
  _centerDataValue(center_data_value),
  _colorWidth(color_width),
  _minValue(_centerDataValue - 0.5 * _numColors * _colorWidth),
  _maxValue(_centerDataValue + _numColors * _colorWidth),
  _emphasisZoneLower(0.0),
  _emphasisZoneUpper(0.0),
  _gridColor("gray90"),
  _missingDataColor("darkslateblue"),
  _exceededColor("gray70"),
  _annotationColor("gray90"),
  _backgroundColor("midnightblue"),
  _emphasisColor("hotpink"),
  _boundaryColor("orangered")
{
  setUsualParameters(usual_parms);
  
  _initColorTables();
}


/**********************************************************************
 * Destructor
 */

SiiPalette::~SiiPalette()
{
}


/**********************************************************************
 * prependToUsualParameters()
 */

void SiiPalette::prependToUsualParameters(const string &new_parameter)
{
  // Create the new list and put the parameter at the front

  std::vector< string > new_param_list;
  new_param_list.push_back(new_parameter);
  
  // Loop through the old list, copying all parameters except the newly
  // added one.

  std::vector< string >::const_iterator param;
  for (param = _usualParams.begin(); param != _usualParams.end(); ++param)
  {
    if (*param != new_parameter)
      new_param_list.push_back(*param);
  }

  // Replace the old list with the new one

  _usualParams = new_param_list;
}


/**********************************************************************
 * read()
 */
/*
void SiiPalette::read(char *buf_ptr, const bool gotta_swap)
{
  // Read the palette information from the buffer

  struct solo_palette spal;
      
  if (gotta_swap)
  {
//    sp_crack_spal(buf_ptr, (char *)&spal, (int)0);
  }
  else
  {
    memcpy(&spal, buf_ptr, sizeof(struct solo_palette));
  }

  // Fill the object with the read information
  
  setUsualParameters(spal.usual_parms);
  setPaletteName(spal.palette_name);
  setCenterDataValue(spal.center);
  setColorWidth(spal.increment);
  setNumColors(spal.num_colors);
  setEmphasisZone(spal.emphasis_zone_lower, spal.emphasis_zone_upper);
  setGridColor(_fixColor(spal.grid_color));
  setMissingDataColor(_fixColor(spal.missing_data_color));
  setExceededColor(_fixColor(spal.exceeded_color));
  setAnnotationColor(_fixColor(spal.annotation_color));
  setBackgroundColor(_fixColor(spal.background_color));
  setEmphasisColor(_fixColor(spal.emphasis_color));
  setBoundaryColor(_fixColor(spal.bnd_color));

  // Make sure that we have a valid color table name

  _checkColorTableName();
}
*/

/**********************************************************************
 * removeFromUsualParameters()
 */

void SiiPalette::removeFromUsualParameters(const string &parameter)
{
  std::vector< string >::iterator param_ptr;
  
  if ((param_ptr = std::find(_usualParams.begin(), _usualParams.end(),
			     parameter)) != _usualParams.end())
  {
    _usualParams.erase(param_ptr);
  }
}


/**********************************************************************
 * write()
 */

bool SiiPalette::write(FILE *stream) const
{
  // Put the palette information into the structure used in the file
/*
  struct solo_palette palette_buffer;
  _fillWriteBuffer(palette_buffer);
    
  if (fwrite(&palette_buffer, sizeof(char),
	     (std::size_t)palette_buffer.sizeof_struct, stream)
      < (std::size_t)palette_buffer.sizeof_struct)
  {
    char message[256];
      
    snprintf(message, "Problem writing palette %s\n",
	    palette_buffer.palette_name);
    sii_message(message);
    return false;
  }
*/
  return true;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _checkColorTableName()
 */
/*
void SiiPalette::_checkColorTableName()
{
  // For the color table name, make sure that the indicated color table
  // exists.  If it doesn't, use the default color table.

  char str[256];

  char *color_table_entry =
    ColorTableManager::getInstance()->getAsciiTable(_colorTableName.c_str());
    
  if (color_table_entry == 0)
  {
    strcpy (str, _colorTableName.c_str());
    if ((color_table_entry = strstr(str, ".colors")) != 0)
    {
      color_table_entry[0] = '\0';
      _colorTableName = str;
      color_table_entry =
	ColorTableManager::getInstance()->getAsciiTable(_colorTableName.c_str());
      if (color_table_entry == 0)
      {
	_colorTableName = "carbone17";
	_numColors = 17;
      }
    }
    else
    {
      _colorTableName = "carbone17";
      _numColors = 17;
    }
  }
}
*/

/**********************************************************************
 * _fillWriteBuffer()
 */

void SiiPalette::_fillWriteBuffer(struct solo_palette &write_buffer) const
{
  memset(&write_buffer, 0, sizeof(write_buffer));
  
  strcpy(write_buffer.name_struct, "SPAL");
  write_buffer.sizeof_struct = sizeof(struct solo_palette);

  strncpy(write_buffer.palette_name, _paletteName.c_str(), 
          sizeof(write_buffer.palette_name) - 1);
  strcpy (write_buffer.usual_parms, getUsualParametersStr().c_str());
  write_buffer.center = _centerDataValue;
  write_buffer.increment = _colorWidth;
  write_buffer.num_colors = _numColors;
  write_buffer.emphasis_zone_lower = _emphasisZoneLower;
  write_buffer.emphasis_zone_upper = _emphasisZoneUpper;
  strcpy (write_buffer.color_table_name, _colorTableName.c_str());
  strcpy(write_buffer.grid_color, _gridColor.c_str());
  strcpy (write_buffer.missing_data_color, _missingDataColor.c_str());
  strcpy (write_buffer.exceeded_color, _exceededColor.c_str());
  strcpy (write_buffer.annotation_color, _annotationColor.c_str());
  strcpy (write_buffer.background_color, _backgroundColor.c_str());
  strcpy (write_buffer.emphasis_color, _emphasisColor.c_str());
  strcpy (write_buffer.bnd_color, _boundaryColor.c_str());
}


/**********************************************************************
 * _initColorTables()
 */

void SiiPalette::_initColorTables()
{
/*
  // Initialize the data color table

  _dataColorTable.reserve(MAX_COLOR_TABLE_SIZE);
  for (size_t i = 0; i < MAX_COLOR_TABLE_SIZE; ++i)
    _dataColorTable.push_back(Gdk::Color());
  
  // Initialize the feature color table

  _featureColor.reserve(MAX_FEATURE_COLORS);
  for (size_t i = 0; i < MAX_FEATURE_COLORS; ++i)
    _featureColor.push_back(Gdk::Color());
*/
}
