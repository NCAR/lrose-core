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
// FieldInfo.hh
//
// FieldInfo object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2008
//
///////////////////////////////////////////////////////////////

#ifndef FieldInfo_H
#define FieldInfo_H

#include <string>
#include <vector>
#include <X11/Xlib.h>       /* X11 - XView */
using namespace std;

class FieldInfo {
public:

  FieldInfo(Display *dpy,
	    const string &name,
	    const string &color,
	    double minY,
	    double maxY);
  ~FieldInfo();

  // clear all data

  void clearData();

  // data a data value to the array
  
  void addData(double xx, double yy);

  // set methods

  void setColor(const string &color);

  // get methods

  const string &getName() const { return _name; }
  int getColorCell() const { return _colorCell; }
  const string &getColor() const { return _color; }
  double getMinY() const { return _minY; }
  double getMaxY() const { return _maxY; }
  const vector<double> &getXData() const { return _xx; }
  const vector<double> &getYData() const { return _yy; }

private:

  // data members

  Display *_dpy;
  string _name;
  int _colorCell;  // color for data
  string _color;
  double _minY, _maxY;
  vector<double> _xx;
  vector<double> _yy;
  
};

#endif

