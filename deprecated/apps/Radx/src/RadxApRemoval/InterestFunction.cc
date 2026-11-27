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
 *
 * @file InterestFunction.cc
 *
 * @class InterestFunction
 *
 * InterestFunction handles interest functions.
 *  
 * @date 4/8/2002
 *
 */

using namespace std;

#include <toolsa/MsgLog.hh>
#include "InterestFunction.hh"
#include "Feature.hh"
#include "ApRemoval.hh"

//
// Constants
//

const double InterestFunction::MISSING_INTEREST  = -99;

const float  InterestFunction::INTEREST_SCALE      = 0.008;
const float  InterestFunction::INTEREST_BIAS       = -1.0;

const int    InterestFunction::SCALED_MISSING_INTEREST = USHRT_MAX;
const int    InterestFunction::SCALED_MIN_INTEREST = 0;
const int    InterestFunction::SCALED_MAX_INTEREST = USHRT_MAX - 1;


/**
 * Constructor
 */

InterestFunction::InterestFunction() :
  _x1(0),
  _x2(0),
  _x3(0),
  _x4(0),
  _x5(0),
  _x6(0),
  _y1(0),
  _y2(0),
  _y3(0),
  _y4(0),
  _y5(0),
  _y6(0),
  _weight(0),
  _functionSet(false)
{
}
   

/**
 * setFunction()
 */

int InterestFunction::setFunction(const double x1, const double y1, 
				  const double x2, const double y2, 
				  const double x3, const double y3, 
				  const double x4, const double y4,
				  const double x5, const double y5,
				  const double x6, const double y6,
				  const double weight) 
{
   if (x1 >= x2 ||
       x2 >= x3 || 
       x3 >= x4 ||
       x4 >= x5 ||
       x5 >= x6)
   {
     POSTMSG(ERROR, "Points for interest function do not represent "
	     "a true function");
     return -1;
   }

   if (y1 > 1 || y1 < -1 || y2 > 1 || y2 < -1 ||
       y3 > 1 || y3 < -1 || y4 > 1 || y4 < -1 ||
       y5 > 1 || y5 < -1 || y6 > 1 || y6 < -1)
   {
     POSTMSG(ERROR, "Interest values must be between -1.0 and 1.0");
     return -1;
   }
   
   _x1 = x1;
   _x2 = x2;
   _x3 = x3;
   _x4 = x4;
   _x5 = x5;
   _x6 = x6;
   
   _y1 = y1;
   _y2 = y2;
   _y3 = y3;
   _y4 = y4;
   _y5 = y5;
   _y6 = y6;

   _weight = weight;
   
   _functionSet = true;

   return 0;
}


/**
 * apply()
 */

double InterestFunction::apply(const double val) const
{
  if (!_functionSet || val == Feature::MISSING_VALUE)
    return MISSING_INTEREST;

  double slope;
   
  if (val <= _x1)
  {
    return _y1;
  }
  else if (val <= _x2)
  {
    slope = (_y2 - _y1) / (_x2 - _x1);
    return slope * (val - _x2) + _y2;
  }
  else if (val <= _x3)
  {
    slope = (_y3 - _y2) / (_x3 - _x2);
    return slope * (val - _x3) + _y3;
  }
  else if (val <= _x4)
  {
    slope = (_y4 - _y3) / (_x4 - _x3);
    return slope * (val - _x4) + _y4;
  }
  else if (val <= _x5)
  {
    slope = (_y5 - _y4) / (_x5 - _x4);
    return slope * (val - _x5) + _y5;
  }
  else if (val <= _x6)
  {
    slope = (_y6 - _y5) / (_x6 - _x5);
    return slope * (val - _x6) + _y6;
  }
      
  // Else it's greater than _x6

  return _y6;
}
