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
//   $Id: ScaleFuzzyFunc.cc,v 1.4 2016/03/03 18:46:09 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * ScaleFuzzyFunc.cc: class implementing a fuzzy function.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2000
 *
 * Nancy Rehak
 *
 *********************************************************************/


#include <iostream>

#include <rapmath/ScaleFuzzyFunc.hh>
using namespace std;


/**********************************************************************
 * Constructors
 */

ScaleFuzzyFunc::ScaleFuzzyFunc(const double scale) :
  _scale(scale)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

ScaleFuzzyFunc::~ScaleFuzzyFunc(void)
{
}
  

/**********************************************************************
 * apply() - Apply the fuzzy function to the given data value.
 */

double ScaleFuzzyFunc::apply(const double x) const
{
  return x * _scale;
}
  

/**********************************************************************
 * print() - Print the function information to the given stream.
 */

void ScaleFuzzyFunc::print(ostream &stream) const
{
  stream << "ScaleFuzzyFunc object:" << endl;
  stream << "    scale = " << _scale << endl;
  stream << endl;
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
