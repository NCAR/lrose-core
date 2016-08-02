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
// FieldInfo.cc
//
// FieldInfo object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2008
//
///////////////////////////////////////////////////////////////

#include <iostream>
#include "FieldInfo.hh"

FieldInfo::FieldInfo(Display *dpy,
		     const string &name,
		     const string &color,
		     double minY,
		     double maxY)

{

  _dpy = dpy;
  _name = name;
  _colorCell = 0;
  _minY = minY;
  _maxY = maxY;

  setColor(color);

}

void FieldInfo::setColor(const string &color)
{

  _color = color;
  XColor cell_def;
  XColor rgb_def;
  Colormap cmap = DefaultColormap(_dpy,DefaultScreen(_dpy));
  XAllocNamedColor(_dpy, cmap, _color.c_str(), &cell_def,&rgb_def);
  _colorCell = cell_def.pixel;

}

FieldInfo::~FieldInfo()
{
  clearData();
}

void FieldInfo::clearData()
{
  _yy.clear();
  _xx.clear();
}

void FieldInfo::addData(double xx, double yy)
{
  _xx.push_back(xx);
  _yy.push_back(yy);
}

