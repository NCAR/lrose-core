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
#include "ColorMap.hh"
#include "rgb.hh"
#include <toolsa/TaXml.hh>
#include <toolsa/TaFile.hh>
#include <toolsa/MemBuf.hh>
#include <toolsa/LogStream.hh>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <cassert>
#include <iostream>
using namespace std;

typedef  struct {unsigned char r,g, b;} RGB;
#define RGBSIZE(table) (sizeof(table)/sizeof(table[0]))

const char* builtinNames[] = {
  "default",
  "rainbow",
  "eldoraDbz",
  "spolDbz",
  "eldoraVel",
  "spolVel",
  "spolDiv"
};

static RGB
  rainbowRGB[] = {
  { 0, 0, 0}, { 84, 0, 155}, { 45, 0, 232}, { 0, 30, 255},
  { 0, 114, 255}, { 0, 199, 255}, { 0, 255, 220}, { 0, 255, 135},
  { 0, 255, 55}, { 73, 255, 0}, { 157, 255, 0}, { 246, 255, 0}, 
  { 255, 224, 0}, { 255, 136, 0}, { 255, 53, 0}, { 255, 255, 255}
};


// eldora dbZ
static RGB
  eldoraDbzRGB[] = {
  {60, 60, 60}, {0, 69, 0}, {0, 101, 10},
  {0, 158, 30}, {0, 177, 59}, {0, 205, 116}, {0, 191, 150},
  {0, 159, 206}, {8, 127, 219}, {28, 71, 232}, {56, 48, 222},
  {110, 13, 198}, {144, 12, 174}, {200, 15, 134}, {196, 67, 134},
  {192, 100, 135}, {191, 104, 101}, {190, 108, 68}, {210, 136, 59},
  {250, 196, 49}, {254, 217, 33}, {254, 250, 3}, {254, 221, 28},
  {254, 154, 88}, {254, 130, 64}, {254, 95, 5}, {249, 79, 8},
  {253, 52, 28}, {200, 117, 104}, {215, 183, 181}, {210, 210, 210},
  {151, 151, 0}, {219, 219, 0}
};

// eldora velocity
static RGB
  eldoraVelocityRGB[] = {
  {255, 0,   255}, {242, 0,   254}, {222, 0,   254},
  {200, 0,   254}, {186, 0,   254}, {175, 0,   253}, {165, 0,   252},
  {139, 0,   248}, {113, 1,   242}, {71,  19,  236}, {50,  75,  229},
  {0,   110, 229}, {0,   182, 228}, {4,   232, 152}, {151, 151, 0}, 
  {2,   116,  76}, {125, 125, 125}, {217, 149, 49}, {238, 184, 31},
  {252, 218, 18},   {219, 219, 0},  {230, 218, 33},  {230, 177, 100},
  {230, 145, 150}, {230, 131, 131}, {230, 108, 108},  {230, 75,  75}, 
  {254, 120,  0},   {254, 90,  0},   {225, 0, 0},   {200, 0, 0},
  {175, 0,   0}, {150, 0,   0},
};

/*      spol NCAR official reflectivity color map */
static RGB
  spolDbzRGB[] = {
  {0,0,0},{60,60,60},{0,69,0},{0,101,10},
  {0,158,30},{0,177,59},{0,205,116},{0,191,150},
  {0,159,206},{8,127,219},{28,71,232},{56,48,222},
  {110,13,198},{144,12,174},{200,15,134},{196,67,134},
  {192,100,135},{191,104,101},{190,108,68},{210,136,59},
  {250,196,49},{254,217,33},{254,250,3},{254,221,28},
  {254,154,88},{254,130,64},{254,95,5},{249,79,8},
  {253,52,28},{200,117,104},{215,183,181},{210,210,210}
};

/*      spol NCAR official divergence color map */
static RGB
  spolDivRGB[] = {
  {0,0,0},{254,0,254},{253,0,254},{248,0,254},
  {222,0,254},{186,0,254},{175,0,253},{165,0,252},
  {139,0,248},{113,1,242},{71,19,236},{19,55,229},
  {0,110,229},{0,182,228},{4,232,152},
  /*  {2,116,76},  */
  {125,125,125},{125,125,125},{125,125,125},
  /*  {226,193,133},  */
  {217,149,49},{238,184,31},
  {252,218,18},{254,218,33},{254,177,100},{254,145,150},
  {254,131,131},{254,108,58},{254,93,7},{254,86,0},
  {254,55,0},{254,13,0},{254,0,0},{255,0,0},{0,0,0}
};

/*      spol NCAR official velocity color map */
static RGB
  spolVelocityRGB[] = {
  {0,0,0},{254,0,254},{253,0,254},{248,0,254},
  {222,0,254},{186,0,254},{175,0,253},{165,0,252},
  {139,0,248},{113,1,242},{71,19,236},{19,55,229},
  {0,110,229},{0,182,228},{4,232,152},{2,116,76},
  {125,125,125},{226,193,133},{217,149,49},{238,184,31},
  {252,218,18},{254,218,33},{254,177,100},{254,145,150},
  {254,131,131},{254,108,58},{254,93,7},{254,86,0},
  {254,55,0},{254,13,0},{254,0,0},{255,0,0},{0,0,0}
};

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
// ColorMap::CmapEntry inner class
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

// default constructor

ColorMap::CmapEntry::CmapEntry() :
        colorName(""),
        red(0),
        green(0),
        blue(0),
        brush(new QBrush(QColor(red, green, blue))),
        minVal(0.0),
        maxVal(0.0)
{
}

//////////////////////////////////////////////////////////////
// Construct from RGB values

ColorMap::CmapEntry::CmapEntry(int red,
                               int green,
                               int blue,
                               double minVal,
                               double maxVal) :
        red(red),
        green(green),
        blue(blue),
	brush(new QBrush(QColor(red, green, blue))),
        minVal(minVal),
        maxVal(maxVal)

{

  char name[64];
  sprintf(name, "#%.2x%.2x%.2x", red, green, blue);
  colorName = name;

}

//////////////////////////////////////////////////
// Copy object

ColorMap &ColorMap::_copy(const ColorMap &rhs)
{

  if (this != &rhs) {

    // copy members

    _debug = rhs._debug;
    _isDefault = rhs._isDefault;

    _path = rhs._path;
    _name = rhs._name;
    _units = rhs._units;

    _useLog10Transform = rhs._useLog10Transform;
    _useLog10ForLut = rhs._useLog10ForLut;

    _entries = rhs._entries;
    _transitionVals = rhs._transitionVals;

    _rangeMin = rhs._rangeMin;
    _rangeMax = rhs._rangeMax;
    _range = rhs._range;

    _labelsSetByValue = rhs._labelsSetByValue;
    _specifiedLabels = rhs._specifiedLabels;

    // compute the lookup table
    
    _computeLut();

  }

  return *this;

}

////////////////////////////////////////////////////
// Copy entry

ColorMap::CmapEntry &ColorMap::CmapEntry::_copy(const CmapEntry &rhs)
{
  if (this != &rhs) {
    colorName = rhs.colorName;
    red = rhs.red;
    green = rhs.green;
    blue = rhs.blue;
    brush = new QBrush(QColor(red, green, blue));
    minVal = rhs.minVal;
    maxVal = rhs.maxVal;
  }
  return *this;
}

// Set entry

void ColorMap::CmapEntry::setColor(const int r, const int g, const int b)
{
  red = r;
  green = g;
  blue = b;
  brush->setColor(QColor(red, green, blue));
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
// ColorMap::CmapEntry inner class - end
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
// ColorMap class
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

ColorMap::ColorMap()
{
  _init();
  *this = ColorMap(0.0, 1.0);
  _isDefault = true;
}
///////////////////////////////////////////////////////////
// No colors specified, use rainbow colors

ColorMap::ColorMap(double rangeMin, 
                   double rangeMax):
        _rangeMin(rangeMin),
        _rangeMax(rangeMax),
        _range(rangeMax-rangeMin)
{ 
  _init();
  int s = sizeof(rainbowRGB)/(sizeof(rainbowRGB[0])/sizeof(rainbowRGB[0].r));
  vector<int> red, green, blue;
  for (int i = 0; i < s; i++) {
    red.push_back(rainbowRGB[i].r);
    green.push_back(rainbowRGB[i].g);
    blue.push_back(rainbowRGB[i].b);
  }
  setMap(rangeMin, rangeMax, red, blue, green);
}

/**********************************************************/
ColorMap::ColorMap
  (double rangeMin,        ///< The minimum map range
   double rangeMax,        ///< The maximum map range
   std::vector<int> red ,  ///< vector of red hues, ranging between 0 and 255
   std::vector<int> green, ///< vector of green hues, ranging between 0 and 255
   std::vector<int> blue)  ///< vector of blue hues, ranging between 0 and 255
{
  _init();
  setMap(rangeMin, rangeMax, red, blue, green);
}

/**********************************************************/
ColorMap::ColorMap
  (double rangeMin,         ///< The minimum map range
   double rangeMax,         ///< The maximum map range
   std::vector<std::vector<int> >colors)
{

  _init();

  for (unsigned int i = 0; i < colors.size(); i++) {
    assert(colors[i].size() == 3);
  }
  vector<int> red, green, blue;
  for (unsigned int i = 0; i < colors.size(); i++) {
    red.push_back(colors[i][0]);
    green.push_back(colors[i][1]);
    blue.push_back(colors[i][2]);
  }
  setMap(rangeMin, rangeMax, red, blue, green);
}

/**********************************************************/
ColorMap::ColorMap
  (double rangeMin,         ///< The minimum map range
   double rangeMax,         ///< The maximum map range
   std::vector<std::vector<float> >colors)
{
  _init();
  for (unsigned int i = 0; i < colors.size(); i++) {
    assert(colors[i].size() == 3);
  }
  vector<int> red, green, blue;
  for (unsigned int i = 0; i < colors.size(); i++) {
    red.push_back((int) (colors[i][0] * 255 + 0.5));
    green.push_back((int) (colors[i][1] * 255 + 0.5));
    blue.push_back((int) (colors[i][2] * 255 + 0.5));
  }
  setMap(rangeMin, rangeMax, red, green, blue);
}

/**********************************************************/
ColorMap::ColorMap(
        double rangeMin,         ///< The minimum map range
        double rangeMax,         ///< The maximum map range
        std::string builtinName) /// the builtin map name

{

  _init();

  RGB* colors;
  int tableSize = 0;

  if (!builtinName.compare("eldoraVel")) {
    colors = &eldoraVelocityRGB[0];
    tableSize = RGBSIZE(eldoraVelocityRGB);
  } else if (!builtinName.compare("spolVel")) { 
    colors = &spolVelocityRGB[0];
    tableSize = RGBSIZE(spolVelocityRGB);
  } else if (!builtinName.compare("eldoraDbz")) {
    colors = &eldoraDbzRGB[0];
    tableSize = RGBSIZE(eldoraDbzRGB);
  } else if (!builtinName.compare("spolDbz")) {
    colors = &spolDbzRGB[0];
    tableSize = RGBSIZE(spolDbzRGB);
  } else if (!builtinName.compare("spolDiv")) {
    colors = &spolDivRGB[0];
    tableSize = RGBSIZE(spolDivRGB);
  } else if (!builtinName.compare("rainbow")) {
    colors = &rainbowRGB[0];
    tableSize = RGBSIZE(rainbowRGB);
  } else {
    // try to read from file path ... 
    //if (readMap(builtinName)) {
      // rainbow will be our default map.
      colors = &rainbowRGB[0];
      tableSize = RGBSIZE(rainbowRGB);
    //}
  }

  vector<int> red, green, blue;
  for (int i = 0; i < tableSize; i++) {
    red.push_back(colors[i].r);
    green.push_back(colors[i].g);
    blue.push_back(colors[i].b);
  }
  setMap(rangeMin, rangeMax, red, green, blue);

  _name = builtinName;

}

////////////////////////////////////////////////////////
// Construct map by reading RAL-style color map file.
// On failure, uses the default constructor, which
// uses rainbow colors and a range of 0.0 to 1.0.

ColorMap::ColorMap(const std::string &file_path,
                   bool debug /* = false */)
  
{

  _init();
  _debug = debug;

  // first try XML style map

  if (readMap(file_path)) {
    
    // failure 0 set to default
    *this = ColorMap(0.0, 1.0);
    _isDefault = true;
    
  }

}

////////////////////////////////////////////////////////
// Initialize for construction

void ColorMap::_init()
  
{

  _debug = false;
  _isDefault = false;
  _useLog10Transform = false;
  _useLog10ForLut = false;
  _labelsSetByValue = false;

  _rangeMin = 0.0;
  _rangeMax = 0.0;
  _range = 0.0;

}

/**********************************************************/
void ColorMap::setMap
  (double rangeMin,       ///< The minimum map range
   double rangeMax,       ///< The maximum map range
   std::vector<int> red,  ///< A vector of red hues, ranging between 0 and 255
   std::vector<int> green,///< A vector of green hues, ranging between 0 and 255
   std::vector<int> blue) ///< A vector of blue hues, ranging between 0 and 255

{

  assert(green.size() == red.size());
  assert(blue.size() == red.size());
  
  setRange(rangeMin, rangeMax);

  _entries.clear();
  
  int s = red.size();
  double delta = _range / (double) s;
  double minVal = _rangeMin;
  double maxVal = minVal + delta;
  for (int i = 0; i < s; i++) {
    CmapEntry entry(red[i], green[i], blue[i], minVal, maxVal);
    _entries.push_back(entry);
    minVal += delta;
    maxVal += delta;
  }

  _computeLut();

  _isDefault = false;

}

/**********************************************************/
void
  ColorMap::setMap
  (double rangeMin,          ///< The minimum map range
   double rangeMax,          ///< The maximum map range
   std::vector<float> red,   ///< vector of red hues, between 0 and 255
   std::vector<float> green, ///< vector of green hues, between 0 and 255
   std::vector<float> blue)  ///< vector of blue hues, between 0 and 255
{

  assert(green.size() == red.size());
  assert(blue.size() == red.size());
  
  vector<int> ired, igreen, iblue;
  for (size_t i = 0; i < red.size(); i++) {
    ired.push_back((int) (red[i] * 255 + 0.5));
    igreen.push_back((int) (green[i] * 255 + 0.5));
    iblue.push_back((int) (blue[i] * 255 + 0.5));
  }
  setMap(rangeMin, rangeMax, red, green, blue);
  
}

/**********************************************************/
void
  ColorMap::setRange(double rangeMin, double rangeMax) {
  _rangeMin = rangeMin;
  _rangeMax = rangeMax;
  _range = rangeMax - rangeMin;
}

/**********************************************************/
void
  ColorMap::setRangeMin(double rangeMin) {

  _rangeMin = rangeMin;
  _resetMap();

}

/**********************************************************/
void
  ColorMap::setRangeMax(double rangeMax) {

  _rangeMax = rangeMax;
  _resetMap();

}

/**********************************************************/
ColorMap::~ColorMap()
{
  //LOG(DEBUG) << "entry " << _name;
  //LOG(DEBUG) << "exit"; 
}

/**********************************************************/
double ColorMap::rangeMin() const
{
  return _rangeMin;
}

/**********************************************************/
double ColorMap::rangeMax() const
{
  return _rangeMax;
}

/**********************************************************/
std::vector<std::string>
  ColorMap::builtinMaps() {
  std::vector<std::string> result;
  for (unsigned int i = 0; i < sizeof(builtinNames)/sizeof(char*); i++) {
    result.push_back(std::string(builtinNames[i]));
  }
  return result;
}

/**********************************************************/
void ColorMap::dataColor
  (double data, int& red, int& green, int& blue) const 

{
  
  int index = _getLutIndex(data);
  const LutEntry &entry = _lut[index];
  red = entry.red;
  green = entry.green;
  blue = entry.blue;

}

/**********************************************************/
const QBrush *ColorMap::dataBrush(double data) const

{

  int index = _getLutIndex(data);
  const LutEntry &entry = _lut[index];
  return entry.brush;

}

/**********************************************************/
void ColorMap::dataColor
  (double data, float& red, float& green, float& blue) const 

{

  int index = _getLutIndex(data);
  const LutEntry &entry = _lut[index];
  red = entry.redNorm;
  green = entry.greenNorm;
  blue = entry.blueNorm;

}

/**********************************************************/
void ColorMap::dataColor
  (double data, double& red, double& green, double& blue) const 

{

  int index = _getLutIndex(data);
  const LutEntry &entry = _lut[index];
  red = entry.redNorm;
  green = entry.greenNorm;
  blue = entry.blueNorm;

}

////////////////////////////////////////////////////////
// set color map
//
// Returns 0 on success, -1 on failure

int ColorMap::readMap(const std::string &file_path)
  
{

  // first try XML style map
  
  if (readXmlMap(file_path) == 0) {
    return 0;
  }

  // then try RAL stype map
  
  if (readRalMap(file_path) == 0) {
    return 0;
  }
      
  // failure

  return -1;

}
  
////////////////////////////////////////////////////////
// set map by reading RAL-style color map file
//
// Returns 0 on success, -1 on failure

int ColorMap::readRalMap(const std::string &file_path)
  
{

  _path = file_path;
  _entries.clear();
  
  // open color_scale file

  FILE *file;
  if ((file = fopen(_path.c_str(), "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - ColorMap::readRalMap" << endl;
    cerr << "  Cannot open color map file: " << _path << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }

  // read in the color data for the levels
  
  char line[BUFSIZ];
  while(fgets(line, BUFSIZ, file) != NULL) {
    
    string colorname;
    double start_val, end_val;
    if (_parseRalColorScaleLine(line, start_val, end_val, colorname) == 0) {
      CmapEntry entry;
      // strip the color name of any "!" characters
      colorname.erase(std::remove(colorname.begin(), colorname.end(), '!'), colorname.end());
      entry.colorName = colorname;
      entry.minVal = start_val;
      entry.maxVal = end_val;
      _entries.push_back(entry);
    }

  } // while
  
  fclose(file);

  // need at least 1 entry for a real color table

  if (_entries.size() < 1) {
    cerr << "ERROR - ColorMap::readRalMap" << endl;
    cerr << "  No valid entries found in file: " << _path << endl;
    return -1;
  }

  // determine the RGB values

  for (size_t ii = 0; ii < _entries.size(); ii++) {

    CmapEntry &entry = _entries[ii];
    
    if (entry.colorName[0] == '#') {

      // RGB in hex #rrggbb
      
      unsigned int red, green, blue;
      if (sscanf(entry.colorName.c_str(), "#%2x%2x%2x",
                 &red, &green, &blue) == 3) {
	entry.setColor(red, green, blue);
        continue;
      }
      
    } else {

      // is this an X color? ignore case

      bool found = false;
      for (int jj = 0; jj < N_X_COLORS; jj++) {
        if (strcasecmp(xColors[jj].name, entry.colorName.c_str()) == 0) {
          // found a match
	  entry.setColor(xColors[jj].red, xColors[jj].green, xColors[jj].blue);
          found = true;
          break;
        }
      }
      if (found) {
        continue;
      }

    }

    // could not decode - set to black

    cerr << "WARNING - ColorMap::readRalMap" << endl;
    cerr << "  Cannot get RGB values for color: " << entry.colorName << endl;
    cerr << "  Setting to black" << endl;
    entry.setColor(0, 0, 0);
    
  }

  // set range

  setRange(_entries[0].minVal, _entries[_entries.size()-1].maxVal);

  // compute the lookup table

  _computeLut();

  return 0;
  
}


/////////////////////////////////////////////////////////////////////
// parses line in RAL color scale file, and fills the start,
// end and colorname fields.
//
// Expected line format is:
//
//   minval maxval colorname
//
// Returns 0 on success, 1 on failure

int ColorMap::_parseRalColorScaleLine(const char *line, 
                                      double &start_val, 
                                      double &end_val, 
                                      string &colorname)
  
{

  // check for starting #

  if (strlen(line) < 8) {
    return -1;
  }
  if (line[0] == '#') {
    // comment line
    return -1;
  }
  
  // tokenize the line on white space

  string lineStr(line);
  for (size_t ii = 0; ii < lineStr.size(); ii++) {
    if (isspace(lineStr[ii])) {
      lineStr[ii] = ' ';
    }
  }
  vector<string> toks;
  _tokenize(lineStr, " ", toks);
  for (size_t ii = 0; ii < toks.size(); ii++) {
  }

  // expect at least 3 tokens: minval, maxval and color name

  if (toks.size() < 3) {
    return -1;
  }

  // get start and end val from first two tokens
  
  double val;
  if (sscanf(toks[0].c_str(), "%lg", &val) != 1) {
    return -1;
  }
  start_val = val;
  if (sscanf(toks[1].c_str(), "%lg", &val) != 1) {
    return -1;
  }
  end_val = val;

  // color name is concatentation of remaining tokens

  colorname.clear();
  for (size_t ii = 2; ii < toks.size(); ii++) {
    colorname += toks[ii];
  }
  
  return 0;

}

//////////////////////////////////////////////
// tokenize a string into a vector of strings
// given a spacer

void ColorMap::_tokenize(const string &str,
                         const string &spacer,
                         vector<string> &toks)
  
{
    
  toks.clear();
  size_t pos = 0;
  while (true) {
    size_t start = str.find_first_not_of(spacer, pos);
    size_t end = str.find_first_of(spacer, start);
    if (start == string::npos) {
      return;
    } else if (end == string::npos) {
      string tok;
      tok.assign(str, start, string::npos);
      toks.push_back(tok);
      return;
    } else {
      string tok;
      tok.assign(str, start, end - start);
      toks.push_back(tok);
    }
    pos = end;
  }
}

//////////////////////////////////////////////
// Print map

void ColorMap::print(ostream &out) const

{

  out << "========= ColorMap =========" << endl;
  out << "  name: " << _name << endl;
  out << "  units: " << _units << endl;
  for (size_t ii = 0; ii < _entries.size(); ii++) {
    const CmapEntry &entry = _entries[ii];
    out << "  minVal, maxVal, color, r, g, b: "
	<< entry.minVal << ", "
	<< entry.maxVal << ", "
	<< entry.colorName << ", "
	<< entry.red << ", "
	<< entry.green << ", "
	<< entry.blue << endl;
  }
  out << "============================" << endl;

}

//////////////////////////////////////////////
// Print lookup table

void ColorMap::printLut(ostream &out) const

{

  out << "========= ColorMap LUT =========" << endl;
  out << "  name: " << _name << endl;
  out << "  units: " << _units << endl;
  for (size_t ii = 0; ii < _lut.size(); ii++) {
    const LutEntry &entry = _lut[ii];
    out << "  r, g, b, rnorm, gnorm, bnorm: "
	<< entry.red << ", "
	<< entry.green << ", "
	<< entry.blue << ", "
	<< entry.redNorm << ", "
	<< entry.greenNorm << ", "
	<< entry.blueNorm << endl;
  }
  out << "================================" << endl;
}

////////////////////////////////////////////////////////
// set map by reading XML-style color map file
//
// Returns 0 on success, -1 on failure

int ColorMap::readXmlMap(const std::string &file_path)
  
{

  _path = file_path;
  _entries.clear();

  // open color_scale file

  TaFile colFile;
  if (colFile.fopen(_path.c_str(), "r") == NULL) {
    int errNum = errno;
    cerr << "ERROR - ColorMap::readXmlMap" << endl;
    cerr << "  Cannot open color map file: " << _path << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }
  if (colFile.fstat()) {
    int errNum = errno;
    cerr << "ERROR - ColorMap::readXmlMap" << endl;
    cerr << "  Cannot stat color map file: " << _path << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }
  off_t fileSize = colFile.getStat().st_size;

  // read entire file contents into buffer

  vector<char> buf;
  buf.reserve(fileSize + 1);
  memset(buf.data(), 0, fileSize + 1);
  if (colFile.fread(buf.data(), 1, fileSize) != fileSize) {
    int errNum = errno;
    cerr << "ERROR - ColorMap::readXmlMap" << endl;
    cerr << "  Cannot read color map file: " << _path << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }
  colFile.fclose();

  // create XML string

  string xmlBuf(buf.data());

  // read in main tag and attributes

  string body;
  vector<TaXml::attribute> attrs;
  if (TaXml::readString(xmlBuf, "ColorScale", body, attrs)) {
    if (_debug) {
      cerr << "WARNING - cannot find <ColorScale> in color map file" << endl;
      cerr << "  file not XML: " << _path << endl;
    }
    return -1;
  }
  
    LOG(DEBUG_VERBOSE) << "Reading in color scale file: " << _path;
    LOG(DEBUG_VERBOSE) << "Contents:";
    LOG(DEBUG_VERBOSE) << xmlBuf;

  string id;
  if (TaXml::readStringAttr(attrs, "id", id)) {
    cerr << "ERROR - no 'id' attribute in XML colorscale" << endl;
    cerr << "  file: " << _path << endl;
    id = "NotSet";
  }
  _name = id;

  string gradation = "linear";
  _useLog10Transform = false;
  if (TaXml::readStringAttr(attrs, "gradation", gradation) == 0) {
    if (gradation == "log") {
      _useLog10Transform = true;
    }
  }

  _saturation = 1.0;
  string satStr;
  if (TaXml::readStringAttr(attrs, "saturation", satStr) == 0) {
    double satVal;
    if (sscanf(satStr.c_str(), "%lg", &satVal) != 1) {
      _saturation = satVal;
    }
  }

  string labelsSetByValue = "false";
  if (TaXml::readStringAttr(attrs, "labelsSetByValue", labelsSetByValue) == 0) {
    if (labelsSetByValue == "true") {
      _labelsSetByValue = true;
    }
  }

    LOG(DEBUG_VERBOSE) << "DEBUG - reading colorScale name: " << _name;
    LOG(DEBUG_VERBOSE) << "=====>> id: " << id;
    LOG(DEBUG_VERBOSE) << "=====>> gradation: " << gradation;
    LOG(DEBUG_VERBOSE) << "=====>> saturation: " << _saturation;
    LOG(DEBUG_VERBOSE) << "=====>> labelsSetByValue: " << labelsSetByValue;

  // read in color range tags
  
  vector<string> rangeTags;
  if (TaXml::readTagBufArray(body, "Range", rangeTags)) {
    cerr << "ERROR - no <Range> tags in XML colorscale" << endl;
    cerr << "  file: " << _path << endl;
    cerr << "  name: " << _name << endl;
    return -1;
  }
  if (rangeTags.size() < 2) {
    cerr << "ERROR - not enough <Range> tags in XML colorscale" << endl;
    cerr << "  file: " << _path << endl;
    cerr << "  name: " << _name << endl;
    return -1;
  }
  for (size_t ii = 0; ii < rangeTags.size(); ii++) {
    if (_readRangeTag(rangeTags[ii])) {
      return -1;
    }
  }

  // check that min and max vals are not NAN

  if (std::isnan(_entries[0].minVal)) {
    cerr << "ERROR - min val for first color must be specified" << endl;
    cerr << "  file: " << _path << endl;
    return -1;
  }

  if (std::isnan(_entries[_entries.size()-1].maxVal)) {
    cerr << "ERROR - max val for last color must be specified" << endl;
    cerr << "  file: " << _path << endl;
    return -1;
  }

  // set range
  
  setRange(_entries[0].minVal, _entries[_entries.size()-1].maxVal);
  
  // copy missing min/max values as appropriate

  for (size_t ii = 1; ii < _entries.size(); ii++) {
    if (std::isnan(_entries[ii].minVal) &&
        !std::isnan(_entries[ii-1].maxVal)) {
      // copy previous max val to this min val
      _entries[ii].minVal = _entries[ii-1].maxVal;
    } else if (!std::isnan(_entries[ii].minVal) &&
               std::isnan(_entries[ii-1].maxVal)) {
      // copy this min val to previous max val
      _entries[ii-1].maxVal = _entries[ii].minVal;
    } 
  }
  
  // check for remaining missing values

  bool missFound = false;
  for (size_t ii = 0; ii < _entries.size(); ii++) {
    if (std::isnan(_entries[ii].minVal) || std::isnan(_entries[ii].maxVal)) {
      missFound = true;
      break;
    }
  }

  if (missFound) {
    // have missing values, we must interpolate
    // load transition values
    _loadTransitions();
    // interpolate
    _interpForNans();
  }
    
  // read in labels if present

  _specifiedLabels.clear();
  if (_labelsSetByValue) {
    vector<string> labelTags;
    if (TaXml::readTagBufArray(body, "Label", labelTags)) {
      cerr << "ERROR - no <Label> tags in XML colorscale," << endl;
      cerr << "  but labelsSetByValue is set true" << endl;
      cerr << "  file: " << _path << endl;
      cerr << "  name: " << _name << endl;
      return -1;
    }
    if (labelTags.size() < 1) {
      cerr << "ERROR - no <Label> tags in XML colorscale," << endl;
      cerr << "  but labelsSetByValue is set true" << endl;
      cerr << "  file: " << _path << endl;
      cerr << "  name: " << _name << endl;
      return -1;
    }
    for (size_t ii = 0; ii < labelTags.size(); ii++) {
      if (_readLabelTag(labelTags[ii])) {
        return -1;
      }
    }
  }

  // compute the lookup table

  _computeLut();

  return 0;
  
}

////////////////////////////////////////////////////////
// Read in Range tag

int ColorMap::_readRangeTag(const string &rangeTag)

{

  vector<TaXml::attribute> attrs;

  string val;
  if (TaXml::readString(rangeTag, "Range", val, attrs)) {
    cerr << "ERROR - bad <Range> tag in XML colorscale" << endl;
    cerr << "  entry: " << rangeTag << endl;
    cerr << "  file: " << _path << endl;
    cerr << "  name: " << _name << endl;
    return -1;
  }
    
  string minStr;
  if (TaXml::readStringAttr(attrs, "min", minStr)) {
    cerr << "ERROR - no 'min' attribute in Range entry" << endl;
    cerr << "  entry: " << rangeTag << endl;
    cerr << "  file: " << _path << endl;
    cerr << "  name: " << _name << endl;
    return -1;
  }

  double minVal;
  if (sscanf(minStr.c_str(), "%lg", &minVal) != 1) {
    minVal = NAN;
  }
  
  string maxStr;
  if (TaXml::readStringAttr(attrs, "max", maxStr)) {
    cerr << "ERROR - no 'max' attribute in Range entry" << endl;
    cerr << "  entry: " << rangeTag << endl;
    cerr << "  file: " << _path << endl;
    cerr << "  name: " << _name << endl;
    return -1;
  }
  double maxVal;
  if (sscanf(maxStr.c_str(), "%lg", &maxVal) != 1) {
    maxVal = NAN;
  }
  
  string colorStr;
  if (TaXml::readStringAttr(attrs, "color", colorStr)) {
    cerr << "ERROR - no 'color' attribute in Range entry" << endl;
    cerr << "  entry: " << rangeTag << endl;
    cerr << "  file: " << _path << endl;
    cerr << "  name: " << _name << endl;
    return -1;
  }
  
  unsigned int red = 0, green = 0, blue = 0;
  string xcolor;
  if (colorStr.find(",") == string::npos) {
    // no commas, must be xcolor
    xcolor = colorStr;
    for (int jj = 0; jj < N_X_COLORS; jj++) {
      if (strcasecmp(xColors[jj].name, xcolor.c_str()) == 0) {
        // found a match
        red = xColors[jj].red;
        green = xColors[jj].green;
        blue = xColors[jj].blue;
        break;
      }
    } // jj
  } else {
    // read in RGB
    double fred, fgreen, fblue;
    if (sscanf(colorStr.c_str(), "%lg,%lg,%lg",
               &fred, &fgreen, &fblue) == 3) {
      // scale for saturation
      double mult = 255.0 / _saturation;
      red = (int) floor(fred * mult + 0.5);
      green = (int) floor(fgreen * mult + 0.5);
      blue = (int) floor(fblue * mult + 0.5);
      // if (fred >= 1 || fgreen >= 1 || fblue >= 1) {
      //   // assume ints from 0 to 255
      //   red = (int) floor(fred + 0.5);
      //   green = (int) floor(fgreen + 0.5);
      //   blue = (int) floor(fblue + 0.5);
      // } else {
      //   // floats from 0 to 1, scale up to 255
      //   red = (int) floor(fred * 255.0 + 0.5);
      //   green = (int) floor(fgreen * 255.0 + 0.5);
      //   blue = (int) floor(fblue * 255.0 + 0.5);
      // }
      if (red > 255) red = 255;
      if (green > 255) green = 255;
      if (blue > 255) blue = 255;
    } else {
      red = green = blue = 0;
    }
    
  }
  
  CmapEntry entry;
  entry.colorName = colorStr;
  entry.minVal = minVal;
  entry.maxVal = maxVal;
  entry.setColor(red, green, blue);
  _entries.push_back(entry);

  return 0;
  
}

////////////////////////////////////////////////////////
// Read in label tag

int ColorMap::_readLabelTag(const string &labelTag)

{

  vector<TaXml::attribute> attrs;

  string val;
  if (TaXml::readString(labelTag, "Label", val, attrs)) {
    cerr << "ERROR - bad <Label> tag in XML colorscale" << endl;
    cerr << "  label: " << labelTag << endl;
    cerr << "  file: " << _path << endl;
    cerr << "  name: " << _name << endl;
    return -1;
  }
    
  string valueStr;
  if (TaXml::readStringAttr(attrs, "value", valueStr)) {
    cerr << "ERROR - no 'value' attribute in Label entry" << endl;
    cerr << "  label: " << labelTag << endl;
    cerr << "  file: " << _path << endl;
    cerr << "  name: " << _name << endl;
    return -1;
  }

  double value;
  if (sscanf(valueStr.c_str(), "%lg", &value) != 1) {
    cerr << "ERROR - bad 'value' attribute in Label entry" << endl;
    cerr << "  label: " << labelTag << endl;
    cerr << "  file: " << _path << endl;
    cerr << "  name: " << _name << endl;
    return -1;
  }
  
  string textStr;
  if (TaXml::readStringAttr(attrs, "text", textStr)) {
    cerr << "ERROR - no 'text' attribute in Label entry" << endl;
    cerr << "  label: " << labelTag << endl;
    cerr << "  file: " << _path << endl;
    cerr << "  name: " << _name << endl;
    return -1;
  }

  // compute the position in the color scale

  double fractionPos = (value - _rangeMin) / (_rangeMax - _rangeMin);
  for (size_t ii = 0; ii < _entries.size(); ii++) {
    const CmapEntry &entry = _entries[ii];
    double entryPos = 0.0;
    if (value >= entry.minVal && value <= entry.maxVal) {
      entryPos = (value - entry.minVal) / (entry.maxVal - entry.minVal);
      if (_useLog10Transform && entryPos > 0) {
        if (value > 0 && entry.minVal > 0 && entry.maxVal > 0) {
          double logVal = log10(value);
          double logMin = log10(entry.minVal);
          double logMax = log10(entry.maxVal);
          entryPos = (logVal - logMin) / (logMax - logMin);
        }
      }
      fractionPos = ((ii + entryPos) / (double) _entries.size());
      // cerr << "text, entryPos, fractionPos: " << textStr << ", "
      //      << entryPos << ", " << fractionPos << endl;
      break;
    }
  }
    
  CmapLabel label;
  label.value = value;
  label.text = textStr;
  label.position = fractionPos;
  _specifiedLabels.push_back(label);

  return 0;
  
}

////////////////////////////////////////////////////////
// Interpolate missing vals

void ColorMap::_interpForNans()

{

  // interpolate between valid transition vals
  // i.e. replace nans

  bool done = false;
  while (!done) {
    int lower = -1;
    int upper = -1;
    for (size_t ii = 1; ii < _transitionVals.size(); ii++) {
      if (lower < 0 && std::isnan(_transitionVals[ii])) {
        lower = ii - 1;
      } else if (lower >= 0 && !std::isnan(_transitionVals[ii])) {
        upper = ii;
        break;
      }
    } // ii
    if (lower < 0 && upper < 0) {
      done = true;
    } else {
      _doInterp(_transitionVals, lower, upper);
    }
  } // while

  // set the values back into the entries

  for (size_t ii = 1; ii < _transitionVals.size(); ii++) {
    _entries[ii-1].maxVal = _transitionVals[ii];
    _entries[ii].minVal = _transitionVals[ii];
  }

}
  
////////////////////////////////////////////////////////
// Interpolate values to fill in missing ranges

void ColorMap::_doInterp(vector<double> &vals,
                         size_t lower, size_t upper)
{

  double lowerVal = vals[lower];
  double upperVal = vals[upper];

  bool useLog = false;
  if (_useLog10Transform) {
    if (lowerVal > 0 && upperVal > 0) {
      lowerVal = log10(lowerVal);
      upperVal = log10(upperVal);
      useLog = true;
    } else { 
      cerr << "WARNING - non-positive value in log color scale" << endl;
      cerr << "  file: " << _path << endl;
      cerr << "  name: " << _name << endl;
      cerr << "  lower index, val: " << lower << ", " << lowerVal << endl;
      cerr << "  upper index, val: " << upper << ", " << upperVal << endl;
      cerr << "  Using linear interpolation instead of log" << endl;
    }
  }

  double range = upperVal - lowerVal;
  double nn = upper - lower; 
  double delta = range / nn;
  double diff = delta;

  for (size_t ii = lower + 1; ii < upper; ii++) {
    double interpVal = lowerVal + diff;
    if (useLog) {
      vals[ii] = pow(10.0, interpVal);
    } else {
      vals[ii] = interpVal;
    }
    diff += delta;
  }

}

////////////////////////////////////////////////////////
// Load up the transition vals vector

void ColorMap::_loadTransitions()
{

  _transitionVals.clear();
  _transitionVals.push_back(_entries[0].minVal);
  for (size_t ii = 0; ii < _entries.size() - 1; ii++) {
    if (!std::isnan(_entries[ii].maxVal)) {
      _transitionVals.push_back(_entries[ii].maxVal);
    } else {
      _transitionVals.push_back(_entries[ii+1].minVal);
    }
  }
  _transitionVals.push_back(_entries[_entries.size()-1].maxVal);

}

/////////////////////////////////////////////////////
// compute the lookup table given the entry list

void ColorMap::_computeLut()

{

  assert(_entries.size() > 0);

  // should we use a log transform?

  if (!_useLog10ForLut) {

    double minDelta = 1.0e99;
    double maxDelta = 0;
    bool allPos = true;
    
    for (size_t ii = 0; ii < _entries.size(); ii++) {
      if (_entries[ii].maxVal <= 0.0 || _entries[ii].minVal <= 0) {
        allPos = false;
      }
      double delta = fabs(_entries[ii].maxVal - _entries[ii].minVal);
      if ((delta > 0) && (delta < minDelta)) {
        minDelta = delta;
      } else if (delta > maxDelta) {
        maxDelta = delta;
      }
    }
    
    if (allPos) {
      double ratio = maxDelta / minDelta;
      if (ratio > 1000) {
        _useLog10ForLut = true;
      }
    }

    if (_useLog10ForLut) {
      ColorMap::setRange(log10(_rangeMin), log10(_rangeMax));
    }
  
  }

  // compute the beakpoints between the colors
  
  vector<int> breaks;
  for (size_t ii = 0; ii < _entries.size(); ii++) {
    double fraction = (_entries[ii].maxVal - _rangeMin) / _range;
    if (_useLog10ForLut) {
      fraction = (log10(_entries[ii].maxVal) - _rangeMin) / _range;
    }
    int bval = (int) ((fraction * LUT_SIZE) + 0.5);
    if (bval < 0) bval = 0;
    if (bval > LUT_SIZE) bval = LUT_SIZE;
    breaks.push_back(bval);
  }
  breaks[breaks.size()-1] = LUT_SIZE;

  int start = 0;
  _lut.clear();
  for (size_t ii = 0; ii < _entries.size(); ii++) {
    int red = _entries[ii].red;
    int green = _entries[ii].green;
    int blue = _entries[ii].blue;
    QBrush *brush = _entries[ii].brush;
    double redNorm = _entries[ii].red / 255.0;
    double greenNorm = _entries[ii].green / 255.0;
    double blueNorm = _entries[ii].blue / 255.0;
    for (int jj = start; jj < breaks[ii]; jj++) {
      _lut.push_back(LutEntry(red, green, blue, brush,
			      redNorm, greenNorm, blueNorm));
    }
    start = breaks[ii];
  }

}
  
////////////////////
// get lut index

int ColorMap::_getLutIndex(double data) const

{

  int index = 0;
  if (_useLog10ForLut) {
    if (data > 0) {
      index = (int) (LUT_SIZE * (log10(data) - _rangeMin)/(_range));
    } else {
      index = 0;
    }
  } else {
    index = (int) (LUT_SIZE * (data - _rangeMin)/(_range));
  }
  
  if (index < 0) {
    index = 0;
  } else if (index > (LUT_SIZE-1)) {
    index = LUT_SIZE - 1;
  }

  return index;

}

////////////////////
// extract the r,g,b values and reset the map

void
  ColorMap::_resetMap() {

  vector<CmapEntry>::iterator it;
  vector<int> red;
  vector<int> green;
  vector<int> blue;

  for (it = _entries.begin(); it != _entries.end(); it++) {
    red.push_back(it->red);
    green.push_back(it->green);
    blue.push_back(it->blue);
  }
  setMap(_rangeMin, _rangeMax, red, green, blue);

}

