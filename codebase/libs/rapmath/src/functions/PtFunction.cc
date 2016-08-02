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
//   $Date: 2016/03/03 18:46:08 $
//   $Id: PtFunction.cc,v 1.8 2016/03/03 18:46:08 dixon Exp $
//   $Revision: 1.8 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * PtFunction.cc: class implementing a function defined by a series of
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

#include <rapmath/PtFunction.hh>
using namespace std;


/**********************************************************************
 * Constructors
 */

PtFunction::PtFunction(map< double, double, less<double> > function) :
  _function(function)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

PtFunction::~PtFunction(void)
{
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _findBoundingPoints() - Find the two defining function points that
 *                         surround the given point.
 *
 * Note:  If the given X value precedes the defined portion of the
 *        function, before_pt will be _function.end() and after_pt
 *        will be _function.begin().  Likewise, if the given X value
 *        exceeds the defined portion of the function, before_pt will
 *        be _function.rbegin() and after_pt will be _function.end().
 *        If the given X is one of the defining points for the function,
 *        the same point is returned for both bounding points.
 *
 *        If there was an error, _function.end() will be returned for
 *        both points.
 */

void PtFunction::_findBoundingPoints(const double x,
				     map< double, double, less<double> >::iterator &before_pt,
				     map< double, double, less<double> >::iterator &after_pt)
{
  // Check for the point being outside of the defined range for the
  // function.

  if (x < (*_function.begin()).first)
  {
    before_pt = _function.end();
    after_pt = _function.begin();
    
    return;
  }
  
  // Look through the function points to find the position of the given
  // X value.

  map< double, double, less<double> >::iterator curr_point;
  
  map< double, double, less<double> >::iterator prev_point = _function.end();
  
  for (curr_point = _function.begin();
       curr_point != _function.end();
       curr_point++)
  {
    double func_x = (*curr_point).first;
    
    if (func_x == x)
    {
      before_pt = curr_point;
      after_pt = curr_point;
      
      return;
    }
    
    if (func_x < x)
    {
      prev_point = curr_point;
      continue;
    }
    
    // If we get here, the value lies between last_point and
    // point.  Calculate the interpolation between these points.

      before_pt = prev_point;
      after_pt = curr_point;
      
      return;
  }

  // If we get here, the point is past the end of the function.

  before_pt = prev_point;
  after_pt = _function.end();
  
  return;
}


/**********************************************************************
 * _printFunction() - Print the defining points of the function to the
 *                    given stream.
 */

void PtFunction::_printFunction(FILE *stream)
{
  fprintf(stream, "Defining function points:\n");
  
  map< double, double, less<double> >::iterator it;
  
  for (it = _function.begin(); it != _function.end(); ++it)
    fprintf(stream, "   x = %15.9f, y = %15.9f\n",
	    (*it).first, (*it).second);
  
  fprintf(stream, "\n");
}


void PtFunction::_printFunction(ostream &stream)
{
  stream << "Defining function points:" << endl;
  
  map< double, double, less<double> >::iterator it;
  
  for (it = _function.begin(); it != _function.end(); ++it)
    stream <<  "   x = " << (*it).first << ", y = " << (*it).second << endl;
  
  stream << endl;
}
