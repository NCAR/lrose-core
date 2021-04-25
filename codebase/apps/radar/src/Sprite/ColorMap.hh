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
#ifndef COLORMAP_H
#define COLORMAP_H

#ifndef DLL_EXPORT
#ifdef WIN32
#ifdef QT_PLUGIN
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __declspec(dllimport)
#endif
#else
#define DLL_EXPORT
#endif
#endif

#include <vector>
#include <string>
#include <iostream>

#include <QtGui/QBrush>

using namespace std;

/// A color map manager. It holds the RGB values for each entry in the
/// color map, and will map a data value into the color map. It will also
/// provide the complete color tables, which is useful to clients.
/// If a color is referenced as an int, it will hae the range 0-255. If it is 
/// referenced as a float or a double, it will be in the range 0-1. 
/// The internal tables are stored in the range of 0-255, so when float values
/// are specified or returned, they must be scaled by 255.

class DLL_EXPORT ColorMap {

 public:

  // color map entries
  // these define the color scale colos, values and limits

  class CmapEntry {

  public:
    string colorName;
    int red;
    int green;
    int blue;
    QBrush *brush;
    double minVal;
    double maxVal;

    // default constructor

    CmapEntry();

    // construct from RGB

    CmapEntry(int red, int green, int blue, double minVal, double maxVal);

    // copy constructor

    CmapEntry(const CmapEntry &rhs) {
      _copy(rhs);
    }

    // operator =

    CmapEntry & operator= (const CmapEntry &rhs) {
      return _copy(rhs);
    }

    // destructor
    
    ~CmapEntry() {
      delete brush;
    }

    // set color from RGB

    void setColor(const int r, const int g, const int b);

  private:
    
    CmapEntry &_copy(const CmapEntry &rhs);

  };

  class CmapLabel {
  public:
    double value;    // value for label
    double position; // fractional position relative to min/max
    string text;     // label text
  };

  /// Default constructor for a ColorMap
  /// Will create a map with the standard colors

  ColorMap();

  /// Create a color map using the built-in
  /// color table.
  /// @param rangeMin The minimum value for the color map.
  /// @param rangeMax The maximum value for the color map.

  ColorMap(double rangeMin, double rangeMax);

  /// Create a color map using the provided colors.
  /// The color table will be constructed from the color
  /// vectors, using the number of entries found in
  /// the shortest of the three color vectors. (They 
  /// really should all be the same length).

  ColorMap
    (double rangeMin,        ///< The minimum map range
     double rangeMax,        ///< The maximum map range
     std::vector<int> red,   ///< vector of red hues, between 0 and 255
     std::vector<int> green, ///< vector of green hues, between 0 and 255
     std::vector<int> blue); ///< vector of blue hues, between 0 and 255

  /// Create a color map using the builtin color map 
  /// with that name. If that name is not located,
  /// use the dfault color map.

  ColorMap
    (double rangeMin,          ///< The minimum map range
     double rangeMax,          ///< The maximum map range
     std::string builtinName); ///< The builtin map name..

  /// Create a color map using the provided colors.
  /// The color table will be constructed from the color
  /// vectors, using the number of entries found in
  /// the outer vector. The inner vectors must be of
  /// length three. If they are not, a color map using
  /// the default color table will be constructed.

  ColorMap
    (double rangeMin, ///< The minimum map range
     double rangeMax, ///< The maximum map range
     std::vector<std::vector<int> >colors); ///< A vector of vectors of hues, ranging between 0 and 255. The inner vector values correcpond to red, green and blue.

  /// Create a color map using the provided colors.
  /// The color table will be constructed from the color
  /// vectors, using the number of entries found in
  /// the outer vector. The inner vectors must be of
  /// length three. If they are not, a color map using
  /// the default color table will be constructed.

  ColorMap
    (double rangeMin, ///< The minimum map range
     double rangeMax, ///< The maximum map range
     std::vector<std::vector<float> >colors); ///< A vector of vectors of hues, ranging between 0 and 1. The inner vector values correcpond to red, green and blue.

  // Construct map by reading RAL-style color map file.
  // On failure, uses the default constructor, which
  // uses rainbow colors and a range of 0.0 to 1.0.
  
  ColorMap(const std::string &file_path,
           bool debug = false);
  
  // copy constructor
  
  ColorMap(const ColorMap &rhs) {
    _copy(rhs);
  }
  
  // operator =
  
  ColorMap & operator= (const ColorMap &rhs) {
    return _copy(rhs);
  }

  /// Destructor

  virtual ~ColorMap();

  /// set debugging

  void setDebug(bool state) { _debug = state; }

  /// Change the color map using the provided colors.
  /// The color table will be constructed from the color
  /// vectors, using the number of entries found in
  /// the shortest of the three color vectors. (They 
  /// really should all be the same length).

  void setMap
    (double rangeMin,        ///< The minimum map range
     double rangeMax,        ///< The maximum map range
     std::vector<int> red,   ///< vector of red hues, between 0 and 255
     std::vector<int> green, ///< vector of green hues, between 0 and 255
     std::vector<int> blue); ///< vector of blue hues, between 0 and 255

  /// Change the color map using the provided colors.
  /// The color table will be constructed from the color
  /// vectors, using the number of entries found in
  /// the shortest of the three color vectors. (They 
  /// really should all be the same length).

  void setMap
    (double rangeMin,           ///< The minimum map range
     double rangeMax,          ///< The maximum map range
     std::vector<float> red,   ///< vector of red hues, between 0 and 1
     std::vector<float> green, ///< vector of green hues, between 0 and 1
     std::vector<float> blue); ///< vector of blue hues, between 0 and 1

  /// Change the range of an existing map.

  void setRange(double rangeMin,  ///< The minimum map range
                double rangeMax); ///< The maximum map range

  // set map by reading color map file
  // Returns 0 on success, -1 on failure
        
  int readMap(const std::string &file_path);

  // set map by reading RAL-style color map file
  // Returns 0 on success, -1 on failure
        
  int readRalMap(const std::string &file_path);

  // set map by reading XML-style color map file
  // Returns 0 on success, -1 on failure
        
  int readXmlMap(const std::string &file_path);

  // set/get name and units

  void setName(const string &name) { _name = name; }
  void setUnits(const string &units) { _units = units; }

  const string &getName() const { return _name; }
  const string &getUnits() const { return _units; }
  const string &getPath() const { return _path; }
  
  /**
   * @returns the color table
   */

  const std::vector<CmapEntry> &getEntries() const { return _entries; }
  const std::vector<double> &getTransitions() const { return _transitionVals; }

  /// Map the data value to a color. Return values will be luminances
  /// from the supplied color tables.
  /// @param data The data value
  /// Data values less than _rangeMin will be mapped
  ///    to the minimum color table color.
  /// Data values greater than _rangeMax will be mapped
  ///    to the maximum color table color.
  /// @param red value returned here.
  /// @param green value returned here.
  /// @param blue value returned here.
  void dataColor(double data, int& red, int& green, int& blue) const ;

  /// Map the data value to a color.
  /// Return values will be luminances in the supplied color tables.
  /// @param data The data value
  /// Data values less than _rangeMin will be mapped
  ///     to the minimum color table color.
  /// Data values greater than _rangeMax will be mapped
  ///     to the maximum color table color.
  /// @return  Returns a pointer to the appropriate brush.
  const QBrush *dataBrush(double data) const;
  
  /// Map the data value to a color.
  /// Return values will be luminances in the range 0-255.
  /// @param data The data value
  /// Data values less than _rangeMin will be mapped
  ///     to the minimum color table color.
  /// Data values greater than _rangeMax will be mapped
  ///     to the maximum color table color.
  /// @param red value returned here.
  /// @param green value returned here.
  /// @param blue value returned here.
  void dataColor(double data, double& red, double& green, double& blue) const;

  /// Map the data value to a color.
  /// Return values will be luminances in the range 0-255.
  /// @param data The data value
  /// Data values less than _rangeMin will be mapped
  ///    to the minimum color table color.
  /// Data values greater than _rangeMax will be mapped
  ///    to the maximum color table color.
  /// @param red value returned here.
  /// @param green value returned here.
  /// @param blue value returned here.
  void dataColor(double data, float& red, float& green, float& blue) const ;

  /// @return The minimum range value
  double rangeMin() const;

  /// @return The maximum range value
  double rangeMax() const;

  /// @return The name of the builtin color maps.
  static std::vector<std::string> builtinMaps();

  /// Get specified labels

  bool labelsSetByValue() const { return _labelsSetByValue; }
  const vector<CmapLabel> &getSpecifiedLabels() const
  {
    return _specifiedLabels;
  }
  
  // print map

  void print(ostream &out) const;

  // print lookup table

  void printLut(ostream &out) const;

  // is this a default map

  bool isDefault() const { return _isDefault; }

 protected:

  bool _debug;
  bool _isDefault;

  string _path;
  string _name;
  string _units;

  // transform using log() for lut with large dynamic range

  bool _useLog10Transform;
  bool _useLog10ForLut;

  // value for saturated color

  double _saturation;

  // map entries
  // size is number of colors

  vector<CmapEntry> _entries;

  // values at color transitions
  // size will be number of colors + 1

  vector<double> _transitionVals;

  // color scale range
  // lookup table is computed for this range

  double _rangeMin;
  double _rangeMax;
  double _range;

  // specified labels and their values

  bool _labelsSetByValue;
  vector<CmapLabel> _specifiedLabels;
  
  // lookup table of RGB colors, evenly distributed
  // through the range

  class LutEntry {
  public:
    int red, green, blue;
    const QBrush *brush;
    double redNorm, greenNorm, blueNorm;
    LutEntry(const int r, const int g, const int b,
	     const QBrush *brush,
	     const double rn, const double gn, const double bn) :
      red(r),
      green(g),
      blue(b),
      brush(brush),
      redNorm(rn),
      greenNorm(gn),
      blueNorm(bn) {
    }

    virtual ~LutEntry() {}

  };
  vector<LutEntry> _lut;
  static const int LUT_SIZE = 10000;

  // initialize for construction

  void _init();

  // parse a line from a color table

  int _parseRalColorScaleLine(const char *line, 
                              double &start_val, 
                              double &end_val, 
                              string &colorname);

  // split a line into tokens

  void _tokenize(const string &str,
                 const string &spacer,
                 vector<string> &toks);

  // reading in XML file

  int _readRangeTag(const string &rangeTag);
  int _readLabelTag(const string &labelTag);

  // interpolate missing vals (nans)

  void _interpForNans();
  void _doInterp(vector<double> &vals,
                 size_t lower, size_t upper);

  // load up transitions

  void _loadTransitions();

  // compute the lookup table from the entries

  void _computeLut();
  int _getLutIndex(double data) const;

 private:

  ColorMap &_copy(const ColorMap &rhs);

};

#endif
