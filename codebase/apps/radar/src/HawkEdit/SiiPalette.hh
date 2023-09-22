#ifndef SiiPalette_hh
#define SiiPalette_hh

#include <algorithm>
#include <iostream>
#include <string.h>
#include <vector>
#include <cstdint>

//#include <glibmm/ustring.h>
//#include <bgdkmm/color.h>

#define MAX_COLOR_TABLE_SIZE 128
#define SIZEOF_COLOR_NAMES 32

using namespace std;

class SiiPalette
{

public:

  //////////////////
  // Public types //
  //////////////////

  typedef enum
  {
    FEATURE_ZERO,
    FEATURE_RNG_AZ_GRID,
    FEATURE_TIC_MARKS,
    FEATURE_BACKGROUND,
    FEATURE_ANNOTATION,
    FEATURE_EXCEEDED,
    FEATURE_MISSING,
    FEATURE_EMPHASIS,
    FEATURE_BND,
    FEATURE_OVERLAY1,
    FEATURE_OVERLAY2,
    FEATURE_OVERLAY3,
    FEATURE_OVERLAY4,
    FEATURE_OVERLAY5,
    MAX_FEATURE_COLORS
  } features_t;

  /**
   * @brief Constructor
   */

  SiiPalette();
  
  /**
   * @brief Constructor
   *
   * @param[in] palette_name        The name of the palette.
   * @param[in] usual_parms         The usual parameters.
   */

  SiiPalette(const string &palette_name,
	     const string &usual_parms);
  
  /**
   * @brief Constructor
   *
   * @param[in] palette_name        The name of the palette.
   * @param[in] usual_parms         The usual parameters.
   * @param[in] center_data_value   The data value at the center of the palette.
   * @param[in] color_width         The width of each color in parameter units.
   * @param[in] color_table_name    The color table name.
   */

  SiiPalette(const string &palette_name,
	     const string &usual_parms,
	     const double center_data_value, const double color_width,
	     const string &color_table_name);
  
  /**
   * @brief Destructor
   */

  virtual ~SiiPalette();
  

  // Input/Output methods //

  /**
   * @brief Read the palette information from the specified buffer.
   *
   * @param[in] buf_ptr      Pointer to the input buffer.
   * @param[in] gotta_swap   Flag indicating whether we need to swap the data.
   */

  void read(char *buf_ptr, const bool gotta_swap);
  

  /**
   * @brief Write the palette information to the specified stream.
   *
   * @param[in,out] stream   The output stream.
   *
   * @return Returns true on success, false on failure.
   */

  bool write(FILE *stream) const;
  

  // Access methods //

  /////////////////
  // get methods //
  /////////////////

  /**
   * @brief Get the palette name.
   *
   * @return Returns the palette name.
   */

  inline string getPaletteName() const
  {
    return _paletteName;
  }
  
  /**
   * @brief Get the usual parameters list.
   *
   * @return Returns the usual parameters list.
   */

  inline string getUsualParametersStr() const
  {
    string param_string;
    
    std::vector< string >::const_iterator param;
    for (param = _usualParams.begin(); param != _usualParams.end(); ++param)
      param_string += *param + ",";
    
    return param_string;
  }
  
  /**
   * @brief See if the indicated parameter is included in the usual parameters.
   *
   * @param[in] parameter  The parameter to look for.
   *
   * @return Returns true if this parameter is in the usual parameters, false
   *         otherwise.
   */

  inline bool isParameterIncluded(const string parameter) const
  {
    if (std::find(_usualParams.begin(), _usualParams.end(), parameter) ==
	_usualParams.end())
      return false;
    
    return true;
  }
  
  /**
   * @brief Get the color table name.
   *
   * @return Returns the color table name.
   */

  inline string getColorTableName() const
  {
    return _colorTableName;
  }

  /////////////////
  // set methods //
  /////////////////

  /**
   * @brief Set the palette name.
   *
   * @param[in] name The palette name.
   */

  inline void setPaletteName(const string &name)
  {
    _paletteName = name;
  }

  /**
   * @brief Set the color table name.
   *
   * @param[in] name   The color table name.
   */
/*
  inline void setColorTableName(const string &name)
  {
    _colorTableName = name;
  }
*/
  /**
   * @brief Set the number of colors in the color table.
   *
   * @param[in] num_colors  The number of colors in the color table.
   */

  inline void setNumColors(const int num_colors)
  {
    _numColors = num_colors;
  }

  /**
   * @brief Set the data value at the center of the palette.
   *
   * @param[in] value  The data value at the center of the palette.
   */

  inline void setCenterDataValue(const float value)
  {
    _centerDataValue = value;
  }

  /**
   * @brief Set the width of each color in the palette in data units.
   *
   * @param[in] width  The width of each color in the palette.
   */

  inline void setColorWidth(const float width)
  {
    _colorWidth = width;
  }

  /**
   * @brief Set the minimum data value in the palette.
   *
   * @param[in] value   The minimum data value in the palette.
   */

  inline void setMinValue(const float value)
  {
    _minValue = value;
  }

  
  /**
   * @brief Set the usual parameters list.
   *
   * @param[in] params The usual parameters list.
   */

  inline void setUsualParameters(const string &params)
  {
    // Clear out any existing entries in the vector

    _usualParams.clear();
    
    // Parse the param string and add the parameters to the vector

    char str[512];
    char *sptrs[64];
    
    strcpy(str, params.c_str());
    int num_tokens = _tokenize(str, sptrs, ",");
    
    for (int i = 0; i < num_tokens; ++i)
      _usualParams.push_back(sptrs[i]);
  }
  
  /**
   * @brief Append the given parameter to the usual parameters.  In this case,
   *        no checking is done to see if the parameter is already in the
   *        list.
   *
   * @param[in] new_parameter  The new parameter name.
   */

  void appendToUsualParameters(const string &new_parameter)
  {
    _usualParams.push_back(new_parameter);
  }
  
  /**
   * @brief Prepend the given parameter to the usual parameters.  If this
   *        parameter is already in the parameter list, move it from it's
   *        current location to the front of the list.
   *
   * @param[in] new_parameter  The new parameter name.
   */

  void prependToUsualParameters(const string &new_parameter);
  
  /**
   * @brief Remove the given parameter from the usual parameters.
   *
   * @param[in] parameter  The parameter to remove.
   */

  void removeFromUsualParameters(const string &parameter);
  
  /**
   * @brief Set the color table name.
   *
   * @param[in] name   The color table name.
   */

  inline void setColorTableName(const string &name)
  {
    _colorTableName = name;
  }

protected:

  /////////////////////
  // Protected types //
  /////////////////////

  struct solo_palette {
    char name_struct[4];	/* "SPAL" */
    int32_t sizeof_struct;
    int32_t window_num;
    int32_t time_modified;
    int32_t changed;

    float center;
    float increment;
    float emphasis_zone_lower;
    float emphasis_zone_upper;
    float bnd_line_width_pix;
    int32_t num_colors;
    char usual_parms[128];
    char palette_name[16];
    char units[16];
    char color_table_dir[128];
    char color_table_name[128];
    char emphasis_color[SIZEOF_COLOR_NAMES];
    char exceeded_color[SIZEOF_COLOR_NAMES];
    char missing_data_color[SIZEOF_COLOR_NAMES];
    char background_color[SIZEOF_COLOR_NAMES];
    char annotation_color[SIZEOF_COLOR_NAMES];
    char grid_color[SIZEOF_COLOR_NAMES];
    /* boundary info */
    char bnd_color[SIZEOF_COLOR_NAMES];
    char bnd_alert[SIZEOF_COLOR_NAMES];
    char bnd_last_line[SIZEOF_COLOR_NAMES];
    char message[48];		/* display min/max colorbar vals */
  };

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief The name of the palette.
   */

  string _paletteName;

  /**
   * @brief The list of usual parameters using this palette.  This list is a
   *        string of the parameter names separated by commas.
   */

  std::vector< string > _usualParams;

  /**
   * @brief The name of the color table.
   */

  string _colorTableName;

  /**
   * @brief The number of colors in the table.
   */

  uint32_t _numColors;

  /**
   * @brief The data value at the center of the color palette.
   */

  float _centerDataValue;
  
  /**
   * @brief The width of each color in the palette in data units.
   */

  float _colorWidth;
  
  /**
   * @brief Minimum data value for the palette.
   */

  float _minValue;
  
  /**
   * @brief Maximum data value for the palette.
   */

  float _maxValue;
  
  /**
   * @brief Lower bound of the emphasis zone.
   */

  float _emphasisZoneLower;
  
  /**
   * @brief Upper bound of the emphasis zone.
   */

  float _emphasisZoneUpper;
  
  /**
   * @brief The name of the color used for the grid overlay.
   */

  string _gridColor;

  /**
   * @brief The name of the color used to signal missing data.
   */

  string _missingDataColor;

  /**
   * @brief The name of the exceeded color.
   */

  string _exceededColor;

  /**
   * @brief The name of the color used for annotations.
   */

  string _annotationColor;

  /**
   * @brief The name of the background color.
   */

  string _backgroundColor;

  /**
   * @brief The name of the emphasis color.
   */

  string _emphasisColor;

  /**
   * @brief The name of the boundary color.
   */

  string _boundaryColor;
 
  /**
   * @brief The name of the boundary alert color.
   */

  string _boundaryAlertColor;
 
  /**
   * @brief The name of the boundary last line color.
   */

  string _boundaryLastLineColor;
 
  /**
   * @brief List of feature colors, in the order of the values in the features_t
   *        enumeration.
   */

  //  std::vector< Gdk::Color > _featureColor;

  //guchar _featureRgbs[MAX_FEATURE_COLORS][3];

  /**
   * @brief List of data colors.
   */

  //  std::vector< Gdk::Color > _dataColorTable;

  /**
   * @brief The RGB values for the color palette.  They are stored by scaled
   *        data value.  So, the values are accessed as:
   *             red = color_table_rgbx[(scaled_data_value * 3)]
   *             green = color_table_rgbx[(scaled_data_value * 3) + 1]
   *             blue = color_table_rgbx[(scaled_data_value * 3) + 2]
   */

  //guchar _colorTableRgbs[MAX_COLOR_TABLE_SIZE][3];


  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Check that the color table name is valid.  If it isn't valid,
   *        change it to a valid name.
   */

  void _checkColorTableName();
  

  /**
   * @brief Fill the given write buffer with the palette information.
   *
   * @param[in] write_buffer     The buffer to fill.
   */

  void _fillWriteBuffer(struct solo_palette &write_buffer) const;
  

  /**
   * @brief "Fix" the color.  For some reason, we need "gray" rather than
   *        "grey".
   *
   * @param[in] old_color    The old color name.
   *
   * @return Returns the "fixed" color name.
   */

  static std::string _fixColor(const std::string old_color)
  {
    if (old_color.find("grey") == std::string::npos)
      return old_color;

    return "gray";
  }

  /**
   * @brief Initialize the internal color tables.
   */

  void _initColorTables();
  
  /**
   * @brief Tokenize the given string.
   *
   * @param[in] token_line    The line to tokenize.
   * @param[out] token_ptrs   The pointers to the tokens.
   * @param[in] delimiters    The delimiters.
   *
   * @return Returns the number of tokens found.
   */

  static int _tokenize(char *token_line, char *token_ptrs[],
		       const char *delimiters)
  {
    int num_tokens = 0;
    char *b = token_line;

    for (b = strtok(b, delimiters); b != 0; b = strtok(NULL, delimiters))
      token_ptrs[num_tokens++] = b;

    return num_tokens;
  }

};

#endif
