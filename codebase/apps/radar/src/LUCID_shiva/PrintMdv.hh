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
/////////////////////////////////////////////////////////////
// PrintMdv.hh
//
// PrintMdv object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 1999
//
///////////////////////////////////////////////////////////////

#ifndef PrintMdv_H
#define PrintMdv_H

#include <Mdv/DsMdvxThreaded.hh>
#include "qtplot/ColorMap.hh"
#include "Args.hh"
#include "Params.hh"
#include <string>
using namespace std;

class PrintMdv {
  
public:

  // constructor

  PrintMdv(char *inputFile, char *outputDir);
//  PrintMdv(char *inputFile, char *outputDir, char *fieldName, 
//    char *colorScaleFileOrName);  

  // destructor
  
  ~PrintMdv();

  int plotAllFields();
  int plotField(char *fieldName, char *colorScaleFileOrName);

  // data members

  int OK;

  void readColorMap();
  void readColorMap(char *colorScaleFileOrName);

protected:

  time_t _readSearchTime;
  time_t _latestValidModTime;
  time_t _timeListStartTime;
  time_t _timeListEndTime;
  time_t _timeListGenTime;
  time_t _timeListSearchTime;
  
  void _setupRead(DsMdvx *mdvx, string filePath);

  int _handleVolume(DsMdvx *mdvx);
  bool _needData();
  int _handleVsection(DsMdvx *mdvx);
  int _handleAllHeaders(DsMdvx *mdvx);
  int _handleTimeList(DsMdvx *mdvx);
  int _handleTimeHeight(DsMdvx *mdvx);

  int _getVolume(DsMdvx *mdvx);
  int _getAllHeaders(DsMdvx *mdvx);
  int _getVsection(DsMdvx *mdvx);
  int _getTimeList(DsMdvx *mdvx);
  int _getTimeHeight(DsMdvx *mdvx);
  
  void _setTimeListMode(DsMdvx *mdvx);

  void _printVolume(const DsMdvx *mdvx) const;
  void _printVsection(const DsMdvx *mdvx) const;
  void _doPrintVol(const DsMdvx *mdvx) const;

  int _plotVolume(DsMdvx *mdvx);
  int _plotVolume(DsMdvx *mdvx, char *fieldName);
  void _plotField(const DsMdvx *mdvx, char *plotFieldName);
  void _plotAllFields(const DsMdvx *mdvx);

  unsigned int _mapToColorScale(float value, float range, float min);
  unsigned int _mapToColorScale(float value);

  void _generateImage(DsMdvx *mdvx, char *fieldName);
  QImage *_generateQtImage(int width, int height, const float* plane2);

  int _doTest(DsMdvx *mdvx);
  void _printSizes(ostream &out);

  //MdvxTimeList::time_list_mode_t getTimeListMode(Params::time_list_mode_t mode);

private:

  string _progName;
  char *_paramsPath;
  //Args _args;
  //Params _params;

  string _filePath;


  /* this is temp.colors

-75     -70     DarkSlateGray1 
-70     -65     SkyBlue1
-65     -60     deep sky blue     
-60     -55     medium slate blue 
-55     -50     slate blue        
-50     -45     blue4            
-45     -40     MediumPurple4    
-40     -35     DarkOrchid4      
-35     -30     MediumOrchid4    
-30     -25     dark orchid      
-25     -20     purple           
-20     -15     hot pink          
-15     -10     pink        
-10     -5      DarkOrchid4
-5      -1      RoyalBlue4
-1      1       gray
1       5       SpringGreen2
5       10      SpringGreen3
10      15      SpringGreen4
15      20      yellow1
20      23      yellow3
23      26      gold1
26      29      DarkGoldenrod1
29      32      orange
32      35      sienna1
35      40      coral1
40      50      red1
*/

constexpr static float cmin[] = {-75,
-70,
-65,
-60,
-55,
-50,
-45,
-40,
-35,
-30,
-25,
-20,
-15,
-10,
-5,
-1,
1,
5,
10,
15,
20,
23,
26,
29,
32,
35,
40};

constexpr static float cmax[] = {
-70,
-65,
-60,
-55,
-50,
-45,
-40,
-35,
-30,
-25,
-20,
-15,
-10,
-5,
-1,
1,
5,
10,
15,
20,
23,
26,
29,
32,
35,
40,
50
};


  int _nbins;
  unsigned int *_colorMapRGB;

  bool _printAllFields;
  DsMdvx *_mdvx;

  char *_outputDir;

  ColorMap *_colorMap;

  int init(char *filePath, char *outputDir);

};

#endif
