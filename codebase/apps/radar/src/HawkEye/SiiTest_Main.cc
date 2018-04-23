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
///////////////////////////////////////////////////////////////
//
// main for QtTest
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2010
//
///////////////////////////////////////////////////////////////

//#include "ColorMap.hh"
#include "SoloDefaultColorWrapper.hh"
#include "SiiPalette.hh"

// main

int main(int argc, char **argv)

{

  SoloDefaultColorWrapper sd;
  string param;
  param = "RHO";
  //  SiiPalette *sp = sd.lookupByUsualName(param); // eventually, this would return ColorMap
  //if (sp == NULL) {
  //	cerr << "WARNING: didn't find default color map for parameter name " << param << endl;
  //} else {
    // we have an SiiPalette; convert it to a ColorMap object
//      ColorMap *colorMap;
//      colorMap = sd.ToColorMap(sp);
  //}

  // sd.ImportSoloPalettes();
  ColorMap colorMap;
  colorMap = sd.lookupColorTable("carbone17");
  colorMap.print(cout);


  colorMap = sd.lookupColorTable("theodore16");
  colorMap.print(cout);

  vector<double> bounds = sd._minMaxValue(0, 3, 10);
  cout << "bounds (min,max) for (center=0, ncolors=3, width=10): " << bounds[0] << ", " << bounds[1] << endl;

  colorMap = sd.ColorMapForUsualParm["AV"];
  colorMap.print(cout);

  colorMap = sd.ColorMapForUsualParm["KAC"];
  colorMap.print(cout);

/*
  vector<unsigned int> *rgbValues = sd.ToRGB( "   0.539   0.066   0.559");
  cout << "rgb for 0.539   0.066   0.559 ... ";
  for (unsigned int n : *rgbValues) {
    cout << n << ", ";
  }
  cout << endl;
  rgbValues->clear();
  delete(rgbValues); 
*/

  //SiiPalette *sp = sd.lookup("p_ahav"); // eventually, this would return ColorMap
  //sp = sd.lookupByUsualName("AH"); // eventually, this would return ColorMap

}

