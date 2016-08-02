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
//   $Id: NewtonPtFunc.cc,v 1.8 2016/03/03 18:46:08 dixon Exp $
//   $Revision: 1.8 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * NewtonPtFunc.cc: class implementing a function defined by a series
 *                    of points.  The function value for points is
 *                    calculated by using Newton polynomials.
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

#include <cmath>
#include <cstdio>
#include <cfloat>

#include <rapmath/NewtonPtFunc.hh>
using namespace std;


/**********************************************************************
 * Constructors
 */

NewtonPtFunc::NewtonPtFunc(map< double, double, less<double> > function,
			   const double tolerance) :
  PtFunction(function),
  _tolerance(tolerance),
  _degree(0)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

NewtonPtFunc::~NewtonPtFunc(void)
{
}
  

/**********************************************************************
 * computeFunctionValue() - Compute the function value using Newton
 *                          polynomial interpolation.
 */

double NewtonPtFunc::computeFunctionValue(const double x)
{
  double *table;
  double delta;
  double m = 1.0;
  double p;
  
  // Allocate space for the table

  table = new double[_function.size()];
  
  // Set the initial interpolating polynomial and first element
  // of the divided-difference table to f(x0).

  p = table[0] = (*_function.begin()).second;
  _degree = 0;
  
  map< double, double, less<double> >::iterator k_iter;
  map< double, double, less<double> >::iterator k_1_iter;
  map< double, double, less<double> >::iterator i_iter;
  
  unsigned int k;
  int i;
  
  for (k = 0, k_iter = _function.begin();
       k < _function.size() - 1;
       ++k, ++k_iter)
  {
    k_1_iter = k_iter;
    ++k_1_iter;
    
    ++_degree;
    
    table[k + 1] = (*k_1_iter).second;
    
    // Compute the next diagonal placed in the
    // divided-difference table to obtain the next coefficient.

    for (i = k, i_iter = k_iter; i >= 0; --i, --i_iter)
      table[i] =
	(table[i + 1] - table[i])/((*k_1_iter).first - (*i_iter).first);
      
    // Compute the next pk(x), one degree larger than the
    // last, and determine whether the specified tolerance will
    // be met.

    m = m * (x - (*k_iter).first);
    delta = table[0] * m;
    p = p + delta;
    
    if (fabs(delta) < _tolerance)
    {
      delete [] table;
      
      return p;
    }
    
  } /* endfor - k */
  
  // The specified tolerance was never met for differences
  // between successive interpolating polynomials.

  delete [] table;
  
  return DBL_MAX;
}


/**********************************************************************
 * print() - Print the function information to the given stream.
 */

void NewtonPtFunc::print(FILE *stream)
{
  fprintf(stream, "Newton Polynomial Interpolation Function:\n");
  fprintf(stream, "\n");
  fprintf(stream, "   tolerance = %f\n", _tolerance);
  fprintf(stream, "\n");
  
  _printFunction(stream);
}


void NewtonPtFunc::print(ostream &stream)
{
  stream << "Newton Polynomial Interpolation Function:" << endl;
  stream << endl;
  stream << "   tolerance = " << _tolerance << endl;
  stream << endl;
  
  _printFunction(stream);
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

