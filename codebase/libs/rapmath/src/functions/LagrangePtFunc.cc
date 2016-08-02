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
//   $Id: LagrangePtFunc.cc,v 1.7 2016/03/03 18:46:08 dixon Exp $
//   $Revision: 1.7 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * LagrangePtFunc.cc: class implementing a function defined by a series
 *                    of points.  The function value for points is
 *                    calculated by using Lagrange polynomials.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 2000
 *
 * Nancy Rehak
 *
 *********************************************************************/


#include <map>
#include <iostream>

#include <math.h>
#include <cstdio>

#include <rapmath/LagrangePtFunc.hh>
using namespace std;


/**********************************************************************
 * Constructors
 */

LagrangePtFunc::LagrangePtFunc(map< double, double, less<double> > function) :
  PtFunction(function)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

LagrangePtFunc::~LagrangePtFunc(void)
{
}
  

/**********************************************************************
 * computeFunctionValue() - Compute the function value using linear
 *                          interpolation.
 */

double LagrangePtFunc::computeFunctionValue(const double x)
{
  double l;
  double p = 0.0;
  
  map< double, double, less<double> >::iterator k_iter;
  map< double, double, less<double> >::iterator i_iter;
  
  int k, i;
  
  for (k_iter = _function.begin(), k = 0;
       k_iter != _function.end();
       ++k_iter, ++k)
  {
    l = 1.0;
    
    // Obtain lk(x) for the next term to add to the
    // interpolating polynomial.

    for (i_iter = _function.begin(), i = 0;
	 i_iter != _function.end(); ++i_iter, ++i)
    {
      if (i != k)
	l = l * ((x - (*i_iter).first) / ((*k_iter).first - (*i_iter).first));
    }
    
    // Add the next term computed from lk(x) to the
    // interpolating polynomial.

    p = p + ((*k_iter).second * l);
  }
  
  return p;
}


/**********************************************************************
 * print() - Print the function information to the given stream.
 */

void LagrangePtFunc::print(FILE *stream)
{
  fprintf(stream, "Lagrange Polynomial Interpolation Function:\n");
  fprintf(stream, "\n");
  
  _printFunction(stream);
}


void LagrangePtFunc::print(ostream &stream)
{
  stream << "Lagrange Polynomial Interpolation Function:" << endl;
  stream << endl;
  
  _printFunction(stream);
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

