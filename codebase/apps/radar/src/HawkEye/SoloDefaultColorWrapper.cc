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
#include "SoloDefaultColorWrapper.hh"
//#include "rgb.hh"
//#include "SiiPalette.hh"
//#include "PaletteManager.hh"
//#include "ColorTableManager.hh"
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <cassert>
#include <iostream>
using namespace std;


//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

// default constructor

SoloDefaultColorWrapper::SoloDefaultColorWrapper() 
{
  ImportSoloPalettes();
}

SoloDefaultColorWrapper::~SoloDefaultColorWrapper() 
{
}
/*
//
// Find the SiiPalette for the usual parameter name
//
SiiPalette *SoloDefaultColorWrapper::lookup(string name) 
{
  cerr << "looking for " << name << endl;

  // TODO: convert the list of Palettes to a dictionary in order to perform the lookup
 
 // PaletteManager::getInstance()->getPalettesList(); 
  PaletteManager *pi = PaletteManager::getInstance();
  SiiPalette *si;
  // seek gets the palette by the paletteName "p_ahav"
  si = pi->seek(name);

  // isParameterIncluded returns true/false if palette

  return si;
}

// 
// Find the SiiPalette by the palette name
//
SiiPalette *SoloDefaultColorWrapper::lookupByUsualName(string parm) 
{
  cerr << "looking for " << parm << endl;
 
  PaletteManager *pi = PaletteManager::getInstance();
  SiiPalette *si;
  // get the palette by the usual parameter name associated  "AH"
  si = pi->_paletteForParam(parm);

  return si;
}
*/

// 
// Find the ColorMap by the palette name
// e.g. lookupColorTable("carbone17");
ColorMap SoloDefaultColorWrapper::lookupColorTable(string parm) 
{
  cerr << "looking for " << parm << endl;
  return _SoloColorTableToHawkEyeColorMap[parm];
}



//
// Convert an SiiPalette to a ColorMap
//

// colors is a list of strings like "r g b" where r,g,b are floats
//
//ColorMap *SoloDefaultColorWrapper::ToColorMap(SiiPalette *palette) {
ColorMap SoloDefaultColorWrapper::ToColorMap(vector<string> colors) {
  // try to create a CMapEntry from the list of colors
  //colorMap->
  std::vector<std::vector<float> > numericColors;

  //for (vector<string>::iterator it=colors.begin(); it != colors.end(); it++) {
  //    vector<float> rgb = ToRGB(*it);
  vector<float> rgb;
    rgb = ToRGB(" 0.5 0.3 0.2");
    numericColors.push_back(rgb);
    /*
    ColorMap::CmapEntry *cme = new ColorMap::CmapEntry(0.0, 100.0, colorNumbers);
    int r,g,b;
    r = g = b = 0;
    // will need to convert unsigned int to int
    r = (int) rgb->at(0);
    g = (int) rgb->at(1);
    b = (int) rgb->at(2);
    cme->setColor(r,g,b);
    */
    //}

  ColorMap colorMap(0.0, 100.0, numericColors);
  
  return colorMap;
}


int  SoloDefaultColorWrapper::ToSomething(vector<string> colors) {
  // try to create a CMapEntry from the list of colors
  //colorMap->
  /*
  std::vector<std::vector<float> > numericColors;

  for (vector<string>::iterator it=colors.begin(); it != colors.end(); it++) {
    vector<float> *rgb = ToRGB(*it);
    numericColors.push_back(*rgb);
    
    ColorMap::CmapEntry *cme = new ColorMap::CmapEntry(0.0, 100.0, colorNumbers);
    int r,g,b;
    r = g = b = 0;
    // will need to convert unsigned int to int
    r = (int) rgb->at(0);
    g = (int) rgb->at(1);
    b = (int) rgb->at(2);
    cme->setColor(r,g,b);
    
  }

  ColorMap colorMap(0.0, 100.0, numericColors);
  */
  return 3;
}

void SoloDefaultColorWrapper::ImportSoloPalettes() {
  
  // create dictionary to query by usual parms

  // create dictionary to query by color table name and get ColorMap returned

  //vector <string> *SoloColorTable;
  //SoloColorTable = lookup("p_ahav");
  // convert SoloColorTable to ColorMap
  
  // stuff ColorMap into dictionary with name as key

  map<string, std::vector<string> >::iterator it;
  ColorTableManager *ctm = ColorTableManager::getInstance();
  
  for ( it = ctm->_asciiColorTables.begin(); it != ctm->_asciiColorTables.end(); ++it ) {
    // std::cout << it->first  // string (key)
    //		<< ':'
    //	<< it->second   // string's value 
      //	<< std::endl ;
    string key = it->first;
    //    ColorMap colorMap;
    vector<string> colors;
    colors = it->second;
    // colorMap = ToColorMap(listOfColors);
    int varthing;
    //    varthing = ToSomething(listOfColors);

    //-----
    std::vector<std::vector<float> > numericColors;

    for (vector<string>::iterator it=colors.begin(); it != colors.end(); ++it) {
      vector<float> rgb;
      rgb = ToRGB(*it);
      if (rgb.size() == 3) {
        numericColors.push_back(rgb);
      } else {
	cerr << " discarding line: " << *it << endl;
      }
    }


    ColorMap colorMap(0.0, 100.0, numericColors);

  //---------

    cout << "key is " << key << endl;

    _SoloColorTableToHawkEyeColorMap[key] = colorMap;
    cout << " after colorMap insert into dictionary " << endl;
  }
  
}

/*
vector<unsigned int> *SoloDefaultColorWrapper::ToRGB(string colorStr,
  double saturation) {

  unsigned int rgb;

  char *token;
  char *colorString;
  colorString = new char[colorStr.length() + 1];
  strcpy(colorString, colorStr.c_str());
  token = std::strtok(colorString, " ,\t");

  vector<unsigned int> *colors = new vector<unsigned int>[3];

  for (int i=0; i<3; i++) {
    rgb = 0;  
    if (token == NULL) {
      cerr << "Error reading RGB color from string " << colorStr << endl;
      cerr << " setting values to 0" << endl;
    } else {
      // read in RGB                                                                          
      double fcolor;
      if (sscanf(token, "%lg",
             &fcolor) == 1) {
        cout << "read " << fcolor;
        // scale for saturation                                                               
        double mult = 255.0 / saturation;
        rgb = (unsigned int) floor(fcolor * mult + 0.5);
        if (rgb > 255) rgb = 255;
        cout << " converted to " << rgb << endl;
      }
    }
    colors->push_back(rgb);
    // get the next color                                                                
    token = std::strtok(NULL, " ");
  } 
  free(colorString);

  return colors;
}
*/


vector<float> SoloDefaultColorWrapper::ToRGB(string colorStr) {

  vector<float> colors;
  float fr, fg, fb;
      
  if (sscanf(colorStr.c_str(), "%g%g%g", &fr, &fg, &fb) == 3) {
    cout << "read " << fr << ", " << fg << ", " << fb << endl;
	colors.push_back(fr);
	colors.push_back(fg);
	colors.push_back(fb);

  }  else {
    cerr << "Error reading RBG color from line " << colorStr <<  endl;
  }

  return colors;
}

/*
vector<float> SoloDefaultColorWrapper::ToRGB(string colorStr) {
  //double saturation = 1.0;

  unsigned int rgb;

  char *token;
  char colorString[1024];;
  //colorString = nchar[colorStr.length() + 1];
  strcpy(colorString, colorStr.c_str());
  token = std::strtok(colorString, " ,\t");

  vector<float> colors;
  float fcolor;
      
  for (int i=0; i<3; i++) {
    rgb = 0;  
    if (token == NULL) {
      cerr << "Error reading RGB color from string " << colorStr << endl;
      cerr << " setting values to 0" << endl;
    } else {
      // read in RGB                                                                          
      if (sscanf(token, "%g",
             &fcolor) == 1) {
        cout << "read " << fcolor << endl;
        // scale for saturation                                                               
        //double mult = 255.0 / saturation;
        //rgb = (unsigned int) floor(fcolor * mult + 0.5);
        //if (rgb > 255) rgb = 255;
        //cout << " converted to " << rgb << endl;
      }
    }
    colors.push_back(fcolor);
    // get the next color                                                                
    token = std::strtok(NULL, " ");
  } 
  //free(colorString);

  return colors;
}
*/
