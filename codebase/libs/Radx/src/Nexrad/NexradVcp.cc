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
// NexradVcp.cc
//
// Stores the angles for a VCP
//
// Mike Dixon, EOL, NCAR, Boulder, CO
// Sept 2013
//
///////////////////////////////////////////////////////////////

#include <Radx/NexradVcp.hh>
#include <cmath>

///////////////////////////////////////////////////////////////
// constructor

NexradVcp::NexradVcp(int num)

{

  _num = num;

  // load up the angle list

  // VCP 32

  if (num == 32) {
    _fixedAngles.push_back(0.5);
    _fixedAngles.push_back(0.5);
    _fixedAngles.push_back(1.5);
    _fixedAngles.push_back(1.5);
    _fixedAngles.push_back(2.5);
    _fixedAngles.push_back(3.5);
    _fixedAngles.push_back(4.5);
    return;
  }

  // VCP 31

  if (num == 31) {
    _fixedAngles.push_back(0.5);
    _fixedAngles.push_back(0.5);
    _fixedAngles.push_back(1.5);
    _fixedAngles.push_back(1.5);
    _fixedAngles.push_back(2.5);
    _fixedAngles.push_back(2.5);
    _fixedAngles.push_back(3.5);
    _fixedAngles.push_back(4.5);
    return;
  }

  // VCP 35

  if (num == 35) {
    _fixedAngles.push_back(0.5);
    _fixedAngles.push_back(0.9);
    _fixedAngles.push_back(1.3);
    _fixedAngles.push_back(1.8);
    _fixedAngles.push_back(2.4);
    _fixedAngles.push_back(3.1);
    _fixedAngles.push_back(4.0);
    _fixedAngles.push_back(5.1);
    _fixedAngles.push_back(6.4);
    return;
  }

  // VCP 21 or 221

  if (num == 21 || num == 221) {
    _fixedAngles.push_back(0.5);
    _fixedAngles.push_back(0.5);
    _fixedAngles.push_back(1.45);
    _fixedAngles.push_back(1.45);
    _fixedAngles.push_back(2.4);
    _fixedAngles.push_back(3.35);
    _fixedAngles.push_back(4.3);
    _fixedAngles.push_back(6.0);
    _fixedAngles.push_back(9.9);
    _fixedAngles.push_back(14.6);
    _fixedAngles.push_back(19.5);
    return;
  }

  // VCP 121

  if (num == 121) {
    _fixedAngles.push_back(0.5);
    _fixedAngles.push_back(0.5);
    _fixedAngles.push_back(0.5);
    _fixedAngles.push_back(0.5);
    _fixedAngles.push_back(1.45);
    _fixedAngles.push_back(1.45);
    _fixedAngles.push_back(1.45);
    _fixedAngles.push_back(1.45);
    _fixedAngles.push_back(2.4);
    _fixedAngles.push_back(2.4);
    _fixedAngles.push_back(2.4);
    _fixedAngles.push_back(3.35);
    _fixedAngles.push_back(3.35);
    _fixedAngles.push_back(3.35);
    _fixedAngles.push_back(4.3);
    _fixedAngles.push_back(4.3);
    _fixedAngles.push_back(6.0);
    _fixedAngles.push_back(9.9);
    _fixedAngles.push_back(14.6);
    _fixedAngles.push_back(19.5);
    return;
  }

  // VCP 12 or 212

  if (num == 12 || num == 212) {
    _fixedAngles.push_back(0.5);
    _fixedAngles.push_back(0.5);
    _fixedAngles.push_back(0.9);
    _fixedAngles.push_back(0.9);
    _fixedAngles.push_back(1.3);
    _fixedAngles.push_back(1.3);
    _fixedAngles.push_back(1.8);
    _fixedAngles.push_back(2.4);
    _fixedAngles.push_back(3.1);
    _fixedAngles.push_back(4.0);
    _fixedAngles.push_back(5.1);
    _fixedAngles.push_back(6.4);
    _fixedAngles.push_back(8.0);
    _fixedAngles.push_back(10.0);
    _fixedAngles.push_back(12.5);
    _fixedAngles.push_back(15.6);
    _fixedAngles.push_back(19.5);
    return;
  }

  // VCP 215

  if (num == 215) {
    _fixedAngles.push_back(0.5);
    _fixedAngles.push_back(0.9);
    _fixedAngles.push_back(1.3);
    _fixedAngles.push_back(1.8);
    _fixedAngles.push_back(2.4);
    _fixedAngles.push_back(3.1);
    _fixedAngles.push_back(4.0);
    _fixedAngles.push_back(5.1);
    _fixedAngles.push_back(6.4);
    _fixedAngles.push_back(8.0);
    _fixedAngles.push_back(10.0);
    _fixedAngles.push_back(12.0);
    _fixedAngles.push_back(14.0);
    _fixedAngles.push_back(16.7);
    _fixedAngles.push_back(19.5);
    return;
  }

  // VCP 11 or 211, or default

  _fixedAngles.push_back(0.5);
  _fixedAngles.push_back(0.5);
  _fixedAngles.push_back(1.45);
  _fixedAngles.push_back(1.45);
  _fixedAngles.push_back(2.4);
  _fixedAngles.push_back(3.35);
  _fixedAngles.push_back(4.3);
  _fixedAngles.push_back(5.25);
  _fixedAngles.push_back(6.2);
  _fixedAngles.push_back(7.5);
  _fixedAngles.push_back(8.7);
  _fixedAngles.push_back(10.0);
  _fixedAngles.push_back(12.0);
  _fixedAngles.push_back(14.0);
  _fixedAngles.push_back(16.7);
  _fixedAngles.push_back(19.5);

}

///////////////////////////////////////////////////////////////
// destructor

NexradVcp::~NexradVcp()

{
  _fixedAngles.clear();
}

////////////////////////////////////////////////////////////
// Printing

void NexradVcp::print(ostream &out) const
{
  out << "=============== NEXRAD VCP ===============" << endl;
  out << "  num: " << _num << endl;
  out << "  n fixed angles: " << getNFixedAngles() << endl;
  for (size_t ii = 0; ii < _fixedAngles.size(); ii++) {
    cerr << "  sweep, angle: " << ii << ", " << _fixedAngles[ii] << endl;
  }
  out << "==========================================" << endl;
}

/////////////////////////////////////////////
/// get fixed angle closest to supplied value
/// also set the sweep number

double NexradVcp::getClosestFixedAngle(double val,
                                       int &sweepNum) const

{

  sweepNum = 0;
  double minDiff = 1.0e99;
  double closestVal = 0.0;
  for (size_t ii = 0; ii < _fixedAngles.size(); ii++) {
    double diff = fabs(val - _fixedAngles[ii]);
    if (diff < minDiff) {
      closestVal = _fixedAngles[ii];
      minDiff = diff;
      sweepNum = ii;
    }
  }

  return closestVal;

}
