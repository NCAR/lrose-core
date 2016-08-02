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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/03 18:46:09 $
//   $Id: StepPtFunc.cc,v 1.7 2016/03/03 18:46:09 dixon Exp $
//   $Revision: 1.7 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * StepPtFunc.cc: class implementing a function defined by a series of
 *                points.  The function value for points is calculated
 *                by using the indicated interpolation method.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/


#include <map>
#include <iostream>

#include <math.h>
#include <cstdio>

#include <rapmath/StepPtFunc.hh>
using namespace std;


/**********************************************************************
 * Constructors
 */

StepPtFunc::StepPtFunc(map< double, double, less<double> > function) :
  PtFunction(function)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

StepPtFunc::~StepPtFunc(void)
{
}
  

/**********************************************************************
 * computeFunctionValue() - Compute the data value for the step function.
 */

double StepPtFunc::computeFunctionValue(const double x)
{
  // Find the surrounding points for the X value.

  map< double, double, less<double> >::iterator before_pt;
  map< double, double, less<double> >::iterator after_pt;
  
  _findBoundingPoints(x, before_pt, after_pt);
  
  // Check for special cases.

  if (before_pt == _function.end() && after_pt == _function.end())
    return 0.0;
  else if (before_pt == _function.end() && after_pt == _function.begin())
    return (*after_pt).second;
  else if (after_pt == _function.end())
    return (*before_pt).second;
  else if (before_pt == after_pt)
    return (*before_pt).second;

  // Calculate the step function.

  double y1 = (*before_pt).second;
  
  return y1;
}


/**********************************************************************
 * print() - Print the function information to the given stream.
 */

void StepPtFunc::print(FILE *stream)
{
  fprintf(stream, "Stepwise Interpolation Function:\n");
  fprintf(stream, "\n");
  
  _printFunction(stream);
}


void StepPtFunc::print(ostream &stream)
{
  stream << "Stepwise Interpolation Function:" << endl;
  stream << endl;
  
  _printFunction(stream);
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

