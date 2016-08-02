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


#include <toolsa/gridLineConnect.hh>
//
// See comments in gridLineConnect.hh - Niles Oien June 2004
//
using namespace std;
//
// Constructor. Pass in the two points to be connected.
//
gridLineConnect::gridLineConnect(int x1, int y1, int x2, int y2){

  //
  // Make copies of the input args.
  //
  _x1 = x1; _y1 = y1; _x2 = x2; _y2 = y2;
  //
  // Set the first point.
  //
  _x = x1; _y = y1;
  //
  // Figure out if we are stepping in X or in Y
  //
  double Dx = x2-x1;
  double Dy = y2-y1;
  //
  const double bigNum = 10000.0;
  //
  _DxOnDy = bigNum;
  if (Dy != 0.0) _DxOnDy = Dx/Dy;
  //
  _DyOnDx = bigNum;
  if (Dx != 0.0) _DyOnDx = Dy/Dx;
  //
  _steppingInX = 0;
  if (fabs(_DxOnDy) > 1.0){
    _steppingInX = 1;
  }
  //
  // Set the increment.
  //
  _inc = 1;
  if ( (_steppingInX) && (x2 < x1) ){
    _inc = -1;
  }
  //
  if ( (!(_steppingInX)) && (y2 < y1) ){
    _inc = -1;
  }
  //
  return;
  //
}

// 
// Method to return the integer x,y of the next point.
// returns 0 if this is the last point, else 1.
//
int gridLineConnect::nextPoint(int &ix, int &iy, double &fractionDone){
  //
  // Load up the current point and calculate the fraction done.
  //
  ix = _x;  iy = _y;
  //
  if (_steppingInX){
    fractionDone = 1.0;
    if (_x2 != _x1){
      fractionDone = double(_x - _x1)/double(_x2 - _x1);
    }
  } else {
    fractionDone = 1.0;
    if (_y2 != _y1){
      fractionDone = double(_y - _y1)/double(_y2 - _y1);
    }
  }
  //
  // If this is the last point, return 0.
  //
  if ((ix == _x2) && (iy == _y2)){
    return 0;
  }
  //
  // Otherwise, get ready to return the next point.
  //
  if (_steppingInX){
    _x += _inc;
    double delX = (_x - _x1);
    double delY = delX * _DyOnDx;
    int yInc = (int)rint( delY );
    //
    _y = _y1 + yInc;
  } else {
    _y += _inc;
    double delY = (_y - _y1);
    double delX = delY * _DxOnDy;
    int xInc = (int)rint( delX );
    //
    _x = _x1 + xInc;
    //
  }
  //
  return 1;
  //
}

//
// Destructor.
//
gridLineConnect::~gridLineConnect(){
  return;
}

  
