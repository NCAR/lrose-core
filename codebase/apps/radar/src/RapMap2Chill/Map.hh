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
//////////////////////////////////////////////////////////
// Map.hh
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2007
//
//////////////////////////////////////////////////////////

#ifndef _MAP_HH
#define _MAP_HH

#include <string>
#include <vector>
#include <iostream>
#include "Params.hh"

using namespace std;

#define ICONDEF "ICONDEF"
#define ICON "ICON"
#define POLYLINE "POLYLINE"
#define SIMPLELABEL "SIMPLELABEL"
#define PROJ "PROJECTION"

#define ICON_FLAG 32767
#define POLYLINE_FILE_FLAG -999.0
#define POLYLINE_PLOT_PENUP -1000000.0
#define POLYLINE_PLOT_FLAG -999999.0

class Map {
  
public:

  Map(const Params &params);
  ~Map();
  
  // Reads in map file
  // Returns 0 on success, -1 on failure

  int readFile(const string &filePath);

  // translate to GTK display format
  
  void translate2GtkDisplay();

  // print map
  
  void print(ostream &out);

protected:
private:

  const Params &_params;

  class DoubleCoord {
  public:
    double x;
    double y;
  };
  
  class IntCoord {
  public:
    int x;
    int y;
  };
  
  class IconDef {
  public:
    string name;
    vector<IntCoord> pts;
  };
  
  class Icon {
  public:
    int defnum;
    string name;
    DoubleCoord loc;
    IntCoord label_offset;
    string label;
  };
  
  class Polyline {
  public:
    string name;
    vector<DoubleCoord> pts;
  };
  
  class SimpleLabel {
  public:
    string text;
    DoubleCoord loc;
  };
  
  vector<IconDef> _icondefs;
  vector<Icon> _icons;
  vector<Polyline> _polylines;
  vector<SimpleLabel> _simplelabels;
  string _label;
  string _filePath;

  void _freeUp();

  void _printError(int lineNum,
                   const string &line,
                   const string &message,
                   const string &filePath);
  
};

#endif
