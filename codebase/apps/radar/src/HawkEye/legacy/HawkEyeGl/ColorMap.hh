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

#include <vector>
#include <string>
#include <iostream>
using namespace std;

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

/// A color map manager. It holds the RGB values for each entry in the
/// color map, and will map a data value into the color map. It will also
/// provide the complete color tables, which is useful to clients such
/// as plotlib::ColorBar. 
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
    double minVal;
    double maxVal;
    CmapEntry() {
      red = green = blue = 0;
      minVal = maxVal = 0.0;
    }
    CmapEntry(int red, int green, int blue, double minVal, double maxVal);
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
  
  ColorMap(const std::string &file_path);
  
  /// Destructor
  virtual ~ColorMap();

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

  // set map by reading RAL-style color map file
  // Returns 0 on success, -1 on failure
        
  int readRalMap(const std::string &file_path);

  // set/get name and units

  void setName(const string &name) { _name = name; }
  void setUnits(const string &units) { _units = units; }

  const string &getName() const { return _name; }
  const string &getUnits() const { return _units; }
  
  /**
   * @returns the color table
   */

  const std::vector<CmapEntry> &getEntries() const { return _entries; }

  /// Map the data value to a color. Return values will be luminances
  /// from the supplied color tables.
  /// @param data The data value
  /// Data values less than _rangeMin will be mapped to the minimum color table
  /// color.
  /// Data values greater than _rangeMax will be mapped to the maximum color table
  /// color.
  /// @param red value returned here.
  /// @param green value returned here.
  /// @param blue value returned here.
  void dataColor(
          double data, int& red, int& green, int& blue) const ;

  /// Map the data value to a color. Return values will be luminances
  /// in the range 0-255.
  /// @param data The data value
  /// Data values less than _rangeMin will be mapped to the minimum color table
  /// color.
  /// Data values greater than _rangeMax will be mapped to the maximum color table
  /// color.
  /// @param red value returned here.
  /// @param green value returned here.
  /// @param blue value returned here.
  void dataColor(
          double data, double& red, double& green, double& blue) const;

  /// Map the data value to a color. Return values will be luminances
  /// in the range 0-255.
  /// @param data The data value
  /// Data values less than _rangeMin will be mapped to the minimum color table
  /// color.
  /// Data values greater than _rangeMax will be mapped to the maximum color table
  /// color.
  /// @param red value returned here.
  /// @param green value returned here.
  /// @param blue value returned here.
  void dataColor(
          double data, float& red, float& green, float& blue) const ;

  /// @return The minimum range value
  double rangeMin() const;

  /// @return The maximum range value
  double rangeMax() const;

  /// @return The name of the builtin color maps.
  static std::vector<std::string> builtinMaps();

  // print map

  void print(ostream &out) const;

  // print lookup table

  void printLut(ostream &out) const;

  // is this a default map

  bool isDefault() const { return _isDefault; }

 protected:

  string _name;
  string _units;
  bool _isDefault;

  vector<CmapEntry> _entries;

  // color scale range
  // lookup table is computed for this range

  double _rangeMin;
  double _rangeMax;
  double _range;

  // lookup table of RGB colors, evenly distributed
  // through the range

  class LutEntry {
  public:
    int red, green, blue;
    double redNorm, greenNorm, blueNorm;
    LutEntry() {
      red = green = blue = 0;
      redNorm = greenNorm = blueNorm = 0.0;
    }
  };
  vector<LutEntry> _lut;
  static const int LUT_SIZE = 10000;

  // compute the lookup table from the entries

  void _computeLut();

  // parse a line from a color table

  int _parseColorScaleLine(const char *line, 
                           double &start_val, 
                           double &end_val, 
                           string &colorname);

  // split a line into tokens

  void _tokenize(const string &str,
                 const string &spacer,
                 vector<string> &toks);

};

#endif
