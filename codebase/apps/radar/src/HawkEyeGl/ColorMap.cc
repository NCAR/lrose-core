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
// Construct entry from RGB values
//////////////////////////////////////////////////////////////

ColorMap::CmapEntry::CmapEntry(int red,
                               int green,
                               int blue,
                               double minVal,
                               double maxVal) :
        red(red),
        green(green),
        blue(blue),
        minVal(minVal),
        maxVal(maxVal)

{

  char name[64];
  sprintf(name, "#%.2x%.2x%.2x", red, green, blue);
  colorName = name;

}

/**********************************************************/
ColorMap::ColorMap()
{
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
  setMap(rangeMin, rangeMax, red, blue, green);
}

/**********************************************************/
ColorMap::ColorMap
  (double rangeMin,         ///< The minimum map range
   double rangeMax,         ///< The maximum map range
   std::vector<std::vector<int> >colors)
{

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
ColorMap::ColorMap(
        double rangeMin,         ///< The minimum map range
        double rangeMax,         ///< The maximum map range
        std::vector<std::vector<float> >colors)
{
  for (unsigned int i = 0; i < colors.size(); i++) {
    assert(colors[i].size() == 3);
  }
  vector<int> red, green, blue;
  for (unsigned int i = 0; i < colors.size(); i++) {
    red.push_back((int) (colors[i][0] * 255 + 0.5));
    green.push_back((int) (colors[i][1] * 255 + 0.5));
    blue.push_back((int) (colors[i][2] * 255 + 0.5));
  }
  setMap(rangeMin, rangeMax, red, blue, green);
}

/**********************************************************/
ColorMap::ColorMap(
        double rangeMin,         ///< The minimum map range
        double rangeMax,         ///< The maximum map range
        std::string builtinName) /// the builtin map name

{

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
    // rainbow will be our default map.
    colors = &rainbowRGB[0];
    tableSize = RGBSIZE(rainbowRGB);
  }

  vector<int> red, green, blue;
  for (int i = 0; i < tableSize; i++) {
    red.push_back(colors[i].r);
    green.push_back(colors[i].g);
    blue.push_back(colors[i].b);
  }
  setMap(rangeMin, rangeMax, red, green, blue);
}

////////////////////////////////////////////////////////
// Construct map by reading RAL-style color map file.
// On failure, uses the default constructor, which
// uses rainbow colors and a range of 0.0 to 1.0.

ColorMap::ColorMap(const std::string &file_path)
  
{

  if (readRalMap(file_path)) {
    *this = ColorMap(0.0, 1.0);
    _isDefault = true;
  }

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
  setMap(rangeMin, rangeMax, red, blue, green);
  
}

/////////////////////////////////////////////////////
// compute the lookup table given the entry list

void ColorMap::_computeLut()

{

  assert(_entries.size() > 0);

  // compute the beakpoints between the colors

  vector<int> breaks;
  for (size_t ii = 0; ii < _entries.size(); ii++) {
    double fraction = (_entries[ii].maxVal - _rangeMin) / _range;
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
    double redNorm = _entries[ii].red / 255.0;
    double greenNorm = _entries[ii].green / 255.0;
    double blueNorm = _entries[ii].blue / 255.0;
    for (int jj = start; jj < breaks[ii]; jj++) {
      LutEntry entry;
      entry.red = red;
      entry.green = green;
      entry.blue = blue;
      entry.redNorm = redNorm;
      entry.greenNorm = greenNorm;
      entry.blueNorm = blueNorm;
      _lut.push_back(entry);
    }
    start = breaks[ii];
  }

}
  
/**********************************************************/
void
  ColorMap::setRange(double rangeMin, double rangeMax) {
  _rangeMin = rangeMin;
  _rangeMax = rangeMax;
  _range = rangeMax - rangeMin;
}

/**********************************************************/
ColorMap::~ColorMap()
{
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
  
  int index = (int) (LUT_SIZE * (data - _rangeMin)/(_range));
  
  if (index < 0) {
    index = 0;
  } else if (index > (LUT_SIZE-1)) {
    index = LUT_SIZE - 1;
  }

  const LutEntry &entry = _lut[index];
  red = entry.red;
  green = entry.green;
  blue = entry.blue;

}

/**********************************************************/
void ColorMap::dataColor
  (double data, float& red, float& green, float& blue) const 

{

  int index = (int) (LUT_SIZE * (data - _rangeMin)/(_range));
  
  if (index < 0) {
    index = 0;
  } else if (index > (LUT_SIZE-1)) {
    index = LUT_SIZE - 1;
  }

  const LutEntry &entry = _lut[index];
  red = entry.redNorm;
  green = entry.greenNorm;
  blue = entry.blueNorm;

}

/**********************************************************/
void ColorMap::dataColor
  (double data, double& red, double& green, double& blue) const 

{

  int index = (int) (LUT_SIZE * (data - _rangeMin)/(_range));
  
  if (index < 0) {
    index = 0;
  } else if (index > (LUT_SIZE-1)) {
    index = LUT_SIZE - 1;
  }

  const LutEntry &entry = _lut[index];
  red = entry.redNorm;
  green = entry.greenNorm;
  blue = entry.blueNorm;

}

////////////////////////////////////////////////////////
// set map by reading RAL-style color map file
//
// Returns 0 on success, -1 on failure

int ColorMap::readRalMap(const std::string &file_path)
  
{
  
  _entries.clear();
  
  // open color_scale file

  FILE *file;
  if ((file = fopen(file_path.c_str(), "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - ColorMap::readRalMap" << endl;
    cerr << "  Cannot open color map file: " << file_path << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }

  // read in the color data for the levels
  
  char line[BUFSIZ];
  while(fgets(line, BUFSIZ, file) != NULL) {
    
    string colorname;
    double start_val, end_val;
    if (_parseColorScaleLine(line, start_val, end_val, colorname) == 0) {
      CmapEntry entry;
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
    cerr << "  No valid entries found in file: " << file_path << endl;
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
        entry.red = red;
        entry.green = green;
        entry.blue = blue;
        continue;
      }
      
    } else {

      // is this an X color? ignore case

      bool found = false;
      for (int jj = 0; jj < N_X_COLORS; jj++) {
        if (strcasecmp(xColors[jj].name, entry.colorName.c_str()) == 0) {
          // found a match
          entry.red = xColors[jj].red;
          entry.green = xColors[jj].green;
          entry.blue = xColors[jj].blue;
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
    entry.red = 0;
    entry.green = 0;
    entry.blue = 0;
    
  }

  // set range

  setRange(_entries[0].minVal, _entries[_entries.size()-1].maxVal);

  // compute the lookup table

  _computeLut();

  return 0;
  
}


/////////////////////////////////////////////////////////////////////
// parses line in color scale file, and fills the start, end and
// colorname fields.
//
// Expected line format is:
//
//   minval maxval colorname
//
// Returns 0 on success, 1 on failure

int ColorMap::_parseColorScaleLine(const char *line, 
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

    
