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
// ContourAlg.hh
//
// Base class for classes implementing contouring algorithms.
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 2000
//
///////////////////////////////////////////////////////////////

#ifndef ContourAlg_H
#define ContourAlg_H

#include <iostream>

#include <contour/Contour.hh>

using namespace std;

class ContourAlg
{
  
public:

  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  // constructor

  ContourAlg(const bool& debug_flag = false);

  // destructor
  
  virtual ~ContourAlg();


  ////////////////////////
  // Contouring methods //
  ////////////////////////

  // calculate the contours
  //
  // nx - number of points in x-direction
  // ny - number of points in y-direction
  // dx - grid spacing in x-direction
  // dy - grid spacing in y-direction
  // min_x - leftmost value of grid in x-direction
  // min_y - lowest value of grid in y-direction
  // levels - list of contour levsls
  // data - the data of which we will contour
  //
  // Returns the calculated contour on success, 0 on failure

  virtual Contour* generateContour(const int& nx, const int& ny,
				   const float& dx, const float& dy,
				   const float& min_x, const float& min_y,
				   const vector<float>& levels,
				   const float* data) = 0;

protected:
  
  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _debugFlag;
  
  ///////////////////////
  // Protected methods //
  ///////////////////////

private:

  /////////////////////
  // Private methods //
  /////////////////////

};

#endif
