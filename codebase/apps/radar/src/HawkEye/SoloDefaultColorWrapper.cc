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
  _debug = false;

  ImportSoloPalettes();
}

SoloDefaultColorWrapper::~SoloDefaultColorWrapper() 
{
}

// 
// Find the ColorMap by the palette name
// e.g. lookupColorTable("carbone17");
// It throws out_of_range if parm is not recognized.
ColorMap SoloDefaultColorWrapper::lookupColorTable(string parm) 
{
  if (_debug)
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

  vector<float> rgb;
  rgb = ToRGB(" 0.5 0.3 0.2");
  numericColors.push_back(rgb);

  ColorMap colorMap(0.0, 100.0, numericColors);
  
  return colorMap;
}

// set min and max value for associated ColorMap
vector<double> SoloDefaultColorWrapper::_minMaxValue(double _centerDataValue, int  _numColors, double _colorWidth){
  vector<double> minMax;
  minMax.push_back( _centerDataValue - 0.5 * _numColors * _colorWidth);
  minMax.push_back( _centerDataValue + 0.5 * _numColors * _colorWidth);
  return minMax;
}


ColorMap SoloDefaultColorWrapper::constructColorMap(double center, double width, string colorTableName) {

  vector< vector <float> >  baseColorTable;

  baseColorTable = _colorTableNameToRgbList[colorTableName];
  // const vector<ColorMap::CmapEntry> colors = colorMap.getEntries();
  vector<double> bounds = _minMaxValue(center, baseColorTable.size(), width);
  //ColorMap colorMap(bounds[0], bounds[1], baseColorTable); 
  //return colorMap;
  return  ColorMap(bounds[0], bounds[1], baseColorTable); 

}

/*
For each (usual parms, center, width, color table name):
  ColorMap ConstructColorMap(center, width, color table name)
  For each parm in usual parms:
      Dictionary[parm] = ColorMap
  End For
End For
*/


void SoloDefaultColorWrapper::makeAssociations() {

  /*  keep this ... it helps document the notation below 
SiiPalette(const string &palette_name,
             const string &usual_parms,
             const double center_data_value, const double color_width,
             const string &color_table_name);
  */

  ColorMap colorMap;
  // set min and max value for ColorMap, along with the color table

  //pal = new SiiPalette("p_ahav", "AH,AV,", 0.0, 22.0, "carbone17");
  colorMap = constructColorMap(0.0, 22.0, "carbone17");
  colorMap.setName("p_ahav");
  // colorMap.setRange(0.0, 33.0); // I bet this doesn't recalculate CmapEntry min & max
  ColorMapForUsualParm["AH"] = colorMap;
  ColorMapForUsualParm["AV"] = colorMap;
  //_paletteNameToColorTable["p_ahav"] = "carbone17";
  //
  //>>>>>>
  /*
  colorMap = _SoloColorTableToHawkEyeColorMap["carbone17"];
          //   _SoloColorTableToHawkEyeColorMap[key] = colorMap;
  vector<double> bounds = _minMaxValue(0.0, 17, 22.0);
  colorMap.setRange(bounds[0], bounds[1]);
  _paletteToHawkEyeColorMap["p_ahav"] = colorMap;  
  */

  //pal = new SiiPalette("p_chcv", "CH,CV,", 0.5, 0.1, "carbone17");
  //colorMap = constructColorMap(0.5, 0.1, "carbone17");
  //ColorMapForUsualParm["CH"] = colorMap;
  //ColorMapForUsualParm["CV"] = colorMap;
  // _paletteNameToColorTable["p_chcv"] = "carbone17";

  //pal = new SiiPalette("p_pdesc", "PD,WPD,HYDID,", 9.0, 1.0, "pd17");
  colorMap = constructColorMap(9.0, 1.0, "pd17");
  colorMap.setName("p_pdesc");
  ColorMapForUsualParm["PD"] = colorMap; // "p_pdesc";
  ColorMapForUsualParm["WPD"] = colorMap; // "p_pdesc";
  ColorMapForUsualParm["HYDID"] = colorMap; // "p_pdesc";
  // _paletteNameToColorTable["p_pdesc"] = "pd17";

  //pal = new SiiPalette("p_raccum", "KAC,ZAC,DAC,HAC,NAC,GAC,",
		       //50.0, 10.0, "bluebrown10");
  colorMap = constructColorMap(50.0, 10.0, "bluebrown10");
  colorMap.setName("p_raccum");
  ColorMapForUsualParm["KAC"] = colorMap; // "p_raccum";
  ColorMapForUsualParm["ZAC"] = colorMap; // "p_raccum";
  ColorMapForUsualParm["DAC"] = colorMap; // "p_raccum";
  ColorMapForUsualParm["HAC"] = colorMap; // "p_raccum";
  ColorMapForUsualParm["NAC"] = colorMap; // "p_raccum";
  ColorMapForUsualParm["GAC"] = colorMap; // "p_raccum";
  // _paletteNameToColorTable["p_raccum"] = "bluebrown10";

  //pal = new SiiPalette("p_chcv", "CH,CV,", 0.5, 0.1, "carbone17");
  colorMap = constructColorMap(0.5, 0.1, "carbone17");
  colorMap.setName("p_chcv");
  ColorMapForUsualParm["CH"] = colorMap;
  ColorMapForUsualParm["CV"] = colorMap;
  // _paletteNameToColorTable["p_chcv"] = "carbone17";

  //pal = new SiiPalette("p_rho", "RHOHV,RHO,RH,RX,", 0.5, 0.1, "carbone17");
  colorMap = constructColorMap(0.5, 0.1, "carbone17");
  colorMap.setName("p_rho");
  ColorMapForUsualParm["RHOHV"] = colorMap; // "p_rho";
  ColorMapForUsualParm["RHO"] = colorMap; // "p_rho";
  ColorMapForUsualParm["RH"] = colorMap; // "p_rho";
  ColorMapForUsualParm["RX"] = colorMap; // "p_rho";
  // _paletteNameToColorTable["p_rho"] = "carbone17";

  //pal = new SiiPalette("p_kdp", "KDP,CKDP,NKDP,MKDP,DKD_DSD,",
		       //0.7, 0.12, "carbone17");
  colorMap = constructColorMap(0.7, 0.12, "carbone17");
  colorMap.setName("p_kdp");
  ColorMapForUsualParm["KDP"] = colorMap;//"p_kdp";
  ColorMapForUsualParm["CKDP"] = colorMap;//"p_kdp";
  ColorMapForUsualParm["NKDP"] = colorMap;//"p_kdp";
  ColorMapForUsualParm["MKDP"] = colorMap;//"p_kdp";
  ColorMapForUsualParm["DKD_DSD"] = colorMap;//"p_kdp";
  // _paletteNameToColorTable["p_kdp"] = colorMap;//"carbone17";

  //pal = new SiiPalette("p_res", "RES_DSD,", 5.0, 0.6, "carbone17");
  colorMap = constructColorMap(5.0, 0.6, "carbone17");
  colorMap.setName("p_res");
  ColorMapForUsualParm["RES_DSD"] = colorMap;//"p_res";
  // _paletteNameToColorTable["p_res"] = colorMap;//"carbone17";

  // pal = new SiiPalette("p_d0", "D0_DSD,", 2.0, 0.25, "carbone17");
  colorMap = constructColorMap(2.0, 0.25, "carbone17");
  colorMap.setName("p_d0");
  ColorMapForUsualParm["D0_DSD"] = colorMap;//"p_d0";
  // _paletteNameToColorTable["p_d0"] = colorMap;//"carbone17";

  // pal = new SiiPalette("p_lam", "LAM_DSD,", 5.0, 0.6, "carbone17");
  colorMap = constructColorMap(5.0, 0.6, "carbone17");
  colorMap.setName("p_lam");
  ColorMapForUsualParm["LAM_DSD"] = colorMap;//"p_lam";
  // _paletteNameToColorTable["p_lam"] = colorMap;//"carbone17";

  // pal = new SiiPalette("p_lwd", "LWC_DSD,", 0.8, 0.1, "carbone17");
  colorMap = constructColorMap(0.8, 0.1, "carbone17");
  colorMap.setName("p_lwd");
  ColorMapForUsualParm["LWC_DSD"] = colorMap;//"p_lwd";
  // _paletteNameToColorTable["p_lwd"] = colorMap;//"carbone17";

  // pal = new SiiPalette("p_mu", "MU_DSD,", 5.0, 0.6, "carbone17");
  colorMap = constructColorMap(5.0, 0.6, "carbone17");
  colorMap.setName("p_mu");
  ColorMapForUsualParm["MU_DSD"] = colorMap;//"p_mu";
  // _paletteNameToColorTable["p_mu"] = colorMap;//"carbone17";

  // pal = new SiiPalette("p_n0", "N0_DSD,", 4.0, 0.5, "carbone17");
  colorMap = constructColorMap(4.0, 0.5, "carbone17");
  colorMap.setName("p_n0");
  ColorMapForUsualParm["N0_DSD"] = colorMap;//"p_n0";
  // _paletteNameToColorTable["p_n0"] = colorMap;//"carbone17";

  // pal = new SiiPalette("p_phi", "PHIDP,PHI,PH,DP,NPHI,CPHI",
		       // 70.0, 10.0, "carbone17");
  colorMap = constructColorMap(70.0, 10.0, "carbone17");
  colorMap.setName("p_phi");
  ColorMapForUsualParm["PHIDP"] = colorMap;//"p_phi";
  ColorMapForUsualParm["PHI"] = colorMap;//"p_phi";
  ColorMapForUsualParm["PH"] = colorMap;//"p_phi";
  ColorMapForUsualParm["DP"] = colorMap;//"p_phi";
  ColorMapForUsualParm["NPHI"] = colorMap;//"p_phi";
  ColorMapForUsualParm["CPHI"] = colorMap;//"p_phi";
  //_paletteNameToColorTable["p_phi"] = colorMap;//"carbone17";

  // pal = new SiiPalette("p_zdr", "ZDR,ZD,DR,UZDR,", 4.0, 0.7, "carbone17");
  colorMap = constructColorMap(4.0, 0.7, "carbone17");
  colorMap.setName("p_zdr");
  colorMap.setUnits("dB");
  ColorMapForUsualParm["ZDR"] = colorMap;//"p_zdr";
  ColorMapForUsualParm["ZD"] = colorMap;//"p_zdr";
  ColorMapForUsualParm["DR"] = colorMap;//"p_zdr";
  ColorMapForUsualParm["UZDR"] = colorMap;//"p_zdr";
  //_paletteNameToColorTable["p_zdr"] = colorMap;//"carbone17";

  // pal = new SiiPalette("p_ldr", "LDR,TLDR,ULDR,LVDR,LH,LV",
		       // -6.0, 4.0, "carbone17");
  colorMap = constructColorMap(-6.0, 4.0, "carbone17");
  colorMap.setName("p_ldr");
  ColorMapForUsualParm["LDR"] = colorMap;//"p_ldr";
  ColorMapForUsualParm["TLDR"] = colorMap;//"p_ldr";
  ColorMapForUsualParm["ULDR"] = colorMap;//"p_ldr";
  ColorMapForUsualParm["LVDR"] = colorMap;//"p_ldr";
  ColorMapForUsualParm["LH"] = colorMap;//"p_ldr";
  ColorMapForUsualParm["LV"] = colorMap;//"p_ldr";
  //_paletteNameToColorTable["p_ldr"] = colorMap;//"carbone17";

  // pal = new SiiPalette("p_dBm", "DM,LM,XM,XL,DL,DX", -80.0, 5.0, "carbone17");
  colorMap = constructColorMap(-80.0, 5.0, "carbone17");
  colorMap.setName("p_dBm");
  ColorMapForUsualParm["DM"] = colorMap;//"p_dBm";
  ColorMapForUsualParm["LM"] = colorMap;//"p_dBm";
  ColorMapForUsualParm["XM"] = colorMap;//"p_dBm";
  ColorMapForUsualParm["XL"] = colorMap;//"p_dBm";
  ColorMapForUsualParm["DL"] = colorMap;//"p_dBm";
  ColorMapForUsualParm["DX"] = colorMap;//"p_dBm";
  //_paletteNameToColorTable["p_dBm"] = colorMap;//"carbone17";

  // pal = new SiiPalette("p_dBz", "DBZ,DZ,XZ,DB,Z,UDBZ,CDZ,DCZ,",
		       // 15.0, 5.0, "carbone17");
  colorMap = constructColorMap(15.0, 5.0, "carbone17");
  colorMap.setName("p_dBz");
  ColorMapForUsualParm["DBZ"] = colorMap;//"p_dBz";
  ColorMapForUsualParm["DZ"] = colorMap;//"p_dBz";
  ColorMapForUsualParm["XZ"] = colorMap;//"p_dBz";
  ColorMapForUsualParm["DB"] = colorMap;//"p_dBz";
  ColorMapForUsualParm["Z"] = colorMap;//"p_dBz";
  ColorMapForUsualParm["UDBZ"] = colorMap;//"p_dBz";
  ColorMapForUsualParm["CDZ"] = colorMap;//"p_dBz";
  ColorMapForUsualParm["DCZ"] = colorMap;//"p_dBz";
  ColorMapForUsualParm["REF"] = colorMap;//"p_dBz";
  //_paletteNameToColorTable["p_dBz"] = colorMap;//"carbone17";

  // pal = new SiiPalette("p_spectral", "SR,SW,S1,S2", 8.0, 1.0, "carbone17");
  colorMap = constructColorMap(8.0, 1.0, "carbone17");
  colorMap.setName("p_spectral");
  ColorMapForUsualParm["SR"] = colorMap;//"p_spectral";
  ColorMapForUsualParm["SW"] = colorMap;//"p_spectral";
  ColorMapForUsualParm["S1"] = colorMap;//"p_spectral";
  ColorMapForUsualParm["S2"] = colorMap;//"p_spectral";
  ColorMapForUsualParm["WIDTH"] = colorMap;//"p_spectral";
  //_paletteNameToColorTable["p_spectral"] = colorMap;//"carbone17";

  // pal = new SiiPalette("p_ncp", "NCP,NC,", 0.5, 0.1, "carbone17");
  colorMap = constructColorMap(0.5, 0.1, "carbone17");
  colorMap.setName("p_ncp");
  ColorMapForUsualParm["NCP"] = colorMap;//"p_ncp";
  ColorMapForUsualParm["NC"] = colorMap;//"p_ncp";
  //_paletteNameToColorTable["p_ncp"] = colorMap;//"carbone17";

  // pal = new SiiPalette("p_vel", "VR,VF,VG,VH,VN,VE,VU,VT,V1,V2,VELOCITY,",
		       // 0.0, 3.0, "carbone17");
  colorMap = constructColorMap(0.0, 3.0, "carbone17");
  colorMap.setName("p_vel");
  colorMap.setUnits("m/s");
  ColorMapForUsualParm["VR"] = colorMap;//"p_vel";
  ColorMapForUsualParm["VF"] = colorMap;//"p_vel";
  ColorMapForUsualParm["VG"] = colorMap;//"p_vel";
  ColorMapForUsualParm["VH"] = colorMap;//"p_vel";
  ColorMapForUsualParm["VN"] = colorMap;//"p_vel";
  ColorMapForUsualParm["VE"] = colorMap;//"p_vel";
  ColorMapForUsualParm["VU"] = colorMap;//"p_vel";
  ColorMapForUsualParm["VT"] = colorMap;//"p_vel";
  ColorMapForUsualParm["V1"] = colorMap;//"p_vel";
  ColorMapForUsualParm["V2"] = colorMap;//"p_vel";
  ColorMapForUsualParm["VEL"] = colorMap;//"p_vel";
  ColorMapForUsualParm["VELOCITY"] = colorMap;//"p_vel";
  // cerr << " ==> ColorMap for VEL " << endl;
  // colorMap.print(cout);

  //_paletteNameToColorTable["p_vel"] = colorMap; //colorMap;//"carbone17";

  // pal = new SiiPalette("p_rrate", "RR_DSD,RNX,RZD,RKD,", 0.0, 0.4, "rrate11");
  colorMap = constructColorMap(0.0, 0.4, "rrate11");
  colorMap.setName("p_rrate");
  ColorMapForUsualParm["RR_DSD"] = colorMap; //"p_rrate";
  ColorMapForUsualParm["RNX"] = colorMap; //"p_rrate";
  ColorMapForUsualParm["RZD"] = colorMap; //"p_rrate";
  ColorMapForUsualParm["RKD"] = colorMap; //"p_rrate";
  //_paletteNameToColorTable["p_rrate"] = colorMap; //"rrate11";

  // pal = new SiiPalette("p_niq", "NIQ,", -60.0, 7.0, "carbone17");
  colorMap = constructColorMap(-60.0, 7.0, "carbone17");
  colorMap.setName("p_niq");
  ColorMapForUsualParm["NIQ"] = colorMap; //"p_niq";
  //_paletteNameToColorTable["p_niq"] = colorMap; //"carbone17";

  // pal = new SiiPalette("p_aiq", "AIQ,", 0.0, 22.0, "carbone17");
  colorMap = constructColorMap(0.0, 22.0, "carbone17");
  colorMap.setName("p_aiq");
  ColorMapForUsualParm["AIQ"] = colorMap; //"p_aiq";
  //_paletteNameToColorTable["p_aiq"] = colorMap; //"carbone17";

}



void SoloDefaultColorWrapper::ImportSoloPalettes() {
  
  // create dictionary to query by usual parms
  // map <usual parm, palette name>   _usualParmToPaletteName;                              
  // map <palette name, color table name> _paletteNameToColorTable;                         
 
  // create dictionary to query by color table name and get ColorMap returned
  
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

    std::vector<std::vector<float> > numericColors;

    for (vector<string>::iterator it=colors.begin(); it != colors.end(); ++it) {
      vector<float> rgb;
      rgb = ToRGB(*it);
      if (rgb.size() == 3) {
        numericColors.push_back(rgb);
      } else {
	if (_debug) cerr << " discarding line: " << *it << endl;
      }
    }

// ok, constructing as a ColorMap doesn't work, because the min and max values
// are not recalulated when the range is changed.  
// Instead, store the color table as a vector < vector <float> > for each
// string name

    ColorMap colorMap(0.0, 100.0, numericColors);

    if (_debug) 
      cout << "key is " << key << endl;

    _SoloColorTableToHawkEyeColorMap[key] = colorMap;
   // map<string, vector< vector <float> > _colorTableNameToRgbList; 
   _colorTableNameToRgbList[key] = numericColors;

    if (_debug) 
      cout << " after colorMap insert into dictionary " << endl;
  }
 
  makeAssociations();
 
 
}

vector<float> SoloDefaultColorWrapper::ToRGB(string colorStr) {

  vector<float> colors;
  float fr, fg, fb;
      
  if (sscanf(colorStr.c_str(), "%g%g%g", &fr, &fg, &fb) == 3) {
    if (_debug) 
      cout << "read " << fr << ", " << fg << ", " << fb << endl;
    colors.push_back(fr);
    colors.push_back(fg);
    colors.push_back(fb);
  }  else {
    if (_debug) cerr << "Error reading RBG color from line " << colorStr <<  endl;
  }

  return colors;
}

// Pulled this code from stackoverflow site 
void SoloDefaultColorWrapper::FindNiceMinMax(double min, double max, int tickCount,
					     double *newMin, double *newMax) {
  
  if (min > max) {
    cerr << "WARNING - min is greater than max when computing colorscale boundaries\n";
    cerr << "   attempting recovery by switching min and max\n";
    double save;
    save = min;
    min = max;
    max = save;
  }
  double range = max - min;
  double unroundedTickSize = range/tickCount;
  if (unroundedTickSize > 0) {
    double x = ceil(log10(unroundedTickSize)-1);
    double pow10x = pow(10, x);
    double roundedTickRange = ceil(unroundedTickSize / pow10x) * pow10x;
    *newMin = roundedTickRange * floor(min/roundedTickRange);
    *newMax = roundedTickRange * ceil(max/roundedTickRange);
  } else {
    cerr << "WARNING - min and max are almost equal when computing colorscale boundaries\n";
    cerr << "   attempting recovery by using (min -1, max) for the colorscale boundaries\n";
    *newMin = min - 1.0;
    *newMax = max;
  } 
}


void SoloDefaultColorWrapper::PrintColorScales() {

  std::map<string,bool> printed;


  cout << " ====== Default Color Scales and Their Usual Parameters ====== \n";
  // print all the usual parameter names and the associated color scale name
  // print the color scale just the first time; throw the name into the printed list.
 
  for (std::map<string,ColorMap>::iterator it=ColorMapForUsualParm.begin();
    it!=ColorMapForUsualParm.end(); ++it) {
    ColorMap colorMap = it->second;
    string colorMapName = colorMap.getName();
    std::cout << it->first << " => " << colorMapName << '\n';  
    if (printed.find(colorMapName) == printed.end()) {
      colorMap.print(cout);
      printed[colorMapName] = true;
    }
  }

  cout << " ========================================================== \n";
}
