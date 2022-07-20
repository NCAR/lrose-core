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

#ifndef BBOX_H
#define BBOX_H

class Bbox {

public:


long nob;
double coordzMin;
double coordzMax;
double coordyMin;
double coordyMax;
double coordxMin;
double coordxMax;



Bbox() {
  nob = 0;
  coordzMin = numeric_limits<double>::quiet_NaN();
  coordzMax = numeric_limits<double>::quiet_NaN();
  coordyMin = numeric_limits<double>::quiet_NaN();
  coordyMax = numeric_limits<double>::quiet_NaN();
  coordxMin = numeric_limits<double>::quiet_NaN();
  coordxMax = numeric_limits<double>::quiet_NaN();
} // end constructor




void print() {
  cout
    << "bbox: nob: " << nob << endl
    << "  coordz: min: " << coordzMin << "  max: " << coordzMax << endl
    << "  coordy: min: " << coordyMin << "  max: " << coordyMax << endl
    << "  coordx: min: " << coordxMin << "  max: " << coordxMax << endl;
  cout << endl;
}


void addOb(
  double coordz,
  double coordy,
  double coordx)
{
  nob++;
  if (std::isnan( coordzMin) || coordz < coordzMin) coordzMin = coordz;
  if (std::isnan( coordzMax) || coordz > coordzMax) coordzMax = coordz;
  if (std::isnan( coordyMin) || coordy < coordyMin) coordyMin = coordy;
  if (std::isnan( coordyMax) || coordy > coordyMax) coordyMax = coordy;
  if (std::isnan( coordxMin) || coordx < coordxMin) coordxMin = coordx;
  if (std::isnan( coordxMax) || coordx > coordxMax) coordxMax = coordx;
} // end addOb




void throwerr( const char * msg) {
  cout << "Bbox: throwerr: " << msg << endl;
  cout.flush();
  throw msg;
}


}; // end class Bbox

#endif


