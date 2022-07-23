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
#ifndef SOLODEFAULTCOLORWRAPPER_H
#define SOLODEFAULTCOLORWRAPPER_H

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include "SiiPalette.hh"
#include <qtplot/ColorMap.hh>
#include "ColorTableManager.hh"

using namespace std;

/// Wraps the internal default color scales from Solo3 code.

class SoloDefaultColorWrapper{

public:

  static SoloDefaultColorWrapper& getInstance() {
    static SoloDefaultColorWrapper instance;
    return instance;
  }

private: 
  // color map entries
  // these define the color scale colors, values and limits
  SoloDefaultColorWrapper();

  bool _debug;

public: 

  /// Destructor

  virtual ~SoloDefaultColorWrapper();
 
  // Find the ColorMap by the palette name                                                                            
  // e.g. lookupColorTable("carbone17");                                               
  ColorMap lookupColorTable(string parm);

  //ColorMap ToColorMap(vector<string> &colors);
  ColorMap ToColorMap(vector<string> colors); 

  vector<float> ToRGB(string colorStr);

  // map <usual parm, palette name>   _usualParmToPaletteName;
  //map<string, string> _usualParmToPaletteName;
  map<string, ColorMap> ColorMapForUsualParm;  // keep

  // map <palette name, color table name> _paletteNameToColorTable;
  map<string, string> _paletteNameToColorTable;

  // map <color table name, string containing color definitions > _asciiColorTables;   
  map<string, ColorMap> _SoloColorTableToHawkEyeColorMap; // keep

  // map <color table name, string containing color definitions > _asciiColorTables;   
  map<string, vector< vector<float> > > _colorTableNameToRgbList; // keep

  // map <palette name, ColorMap> 
  map<string, ColorMap> _paletteToHawkEyeColorMap;

  vector<double> _minMaxValue(double _centerDataValue, int  _numColors, double _colorWidth);

  ColorMap constructColorMap(double center, double width, string colorTableName);

  void makeAssociations();

  void PrintColorScales();

  void FindNiceMinMax(double min, double max, int tickCount,
    double *newMin, double *newMax);

  //private:
  void ImportSoloPalettes();

};

#endif
