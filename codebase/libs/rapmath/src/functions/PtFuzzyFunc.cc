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
//   $Id: PtFuzzyFunc.cc,v 1.4 2016/03/03 18:46:08 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * PtFuzzyFunc.cc: class implementing a fuzzy function defined by a
 *                 point function.  The point function can be interpolated
 *                 in several ways (see the classes derived from
 *                 PtFunction).
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2000
 *
 * Nancy Rehak
 *
 *********************************************************************/


#include <iostream>

#include <rapmath/PtFunction.hh>
#include <rapmath/PtFuzzyFunc.hh>
using namespace std;


/**********************************************************************
 * Constructors
 */

PtFuzzyFunc::PtFuzzyFunc(PtFunction *function) :
  _function(function)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

PtFuzzyFunc::~PtFuzzyFunc(void)
{
  delete _function;
}
  

/**********************************************************************
 * apply() - Apply the fuzzy function to the given data value.
 */

double PtFuzzyFunc::apply(const double x) const
{
  return _function->computeFunctionValue(x);
}
  

/**********************************************************************
 * print() - Print the function information to the given stream.
 */

void PtFuzzyFunc::print(ostream &out) const
{
  out << "PtFuzzyFunc object:" << endl;
  _function->print(out);
  out << endl;
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
