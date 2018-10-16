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
/**
 * @file RayCloudEdge.cc
 */
#include "RayCloudEdge.hh"
#include <toolsa/LogStream.hh>
#include <cstdio>

using std::string;

// /*----------------------------------------------------------------*/
// RayCloudEdge::RayCloudEdge(int y, const RaySubset &cloud,
// 		     const RaySubset &outside,
// 		     // int x0, int x1, int xout0, int xout1,
// 		     double v, bool moving_in) :
//   _y(y), _edge(cloud, outside, v),
//   // cloud(x0, x1, y), _outside(xout0, xout1, y), _v(v),
//   _movingIn(moving_in)
// {
//   // _y = y;
  // _x0 = x0;
  // _x1 = x1;
  // _xout0 = xout0;
  // _xout1 = xout1;
  // _v = v;
  // _movingIn = moving_in;
// }

// /*----------------------------------------------------------------*/
// RayCloudEdge::~RayCloudEdge()
// {
// }

/*----------------------------------------------------------------*/
string RayCloudEdge::sprint(void) const
{
  string s0;
  if (_movingIn)
    s0 = "Out->In";
  else
    s0 = "In->Out";

  char buf[1000];
  sprintf(buf, "%s  %s  %.2lf %s", _cloud.sprint("Cloud").c_str(),
	  _outside.sprint("Outside").c_str(), _value, s0.c_str());
    
  std::string s = buf;
  return s;

  // sprintf(buf, "%s  %s",  _edge.sprint().c_str(), s0.c_str());
  // string s = buf;
  // return s;
}

/*----------------------------------------------------------------*/
void RayCloudEdge::print(void) const
{
  string s = sprint();
  printf("%s\n", s.c_str());
}

/*----------------------------------------------------------------*/
void RayCloudEdge::log(const string &message) const
{
  string sp = sprint();
  LOG(DEBUG_VERBOSE) << message << " - " << sp;
}

