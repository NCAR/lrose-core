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

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/03 19:24:05 $
 *   $Id: PtFunction.hh,v 1.8 2016/03/03 19:24:05 dixon Exp $
 *   $Revision: 1.8 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * PtFunction.hh: class implementing a function defined by a series of
 *                points.  The function value for points is calculated
 *                by using the indicated interpolation method.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 1999
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef PtFunction_HH
#define PtFunction_HH

/*
 **************************** includes **********************************
 */


#include <map>
#include <iostream>

#include <cstdio>
using namespace std;

/*
 ******************************* defines ********************************
 */

/*
 ******************************* structures *****************************
 */

/*
 ************************* global variables *****************************
 */

/*
 ***************************** function prototypes **********************
 */

/*
 ************************* class definitions ****************************
 */

class PtFunction
{
public:

  ////////////////////////////////
  // Constructors & destructors //
  ////////////////////////////////

  // Constructors

  PtFunction(map< double, double, less<double> > function);
  
  // Destructor

  virtual ~PtFunction(void);
  

  /////////////////////////
  // Computation methods //
  /////////////////////////

  // Compute the given function value using the implicit interpolation
  // method.

  virtual double computeFunctionValue(double x) = 0;
  

  //////////////////////////
  // Input/output methods //
  //////////////////////////

  // Print the function information to the given stream.

  virtual void print(FILE *stream) = 0;
  virtual void print(ostream &stream) = 0;
  

  ////////////////////
  // Access methods //
  ////////////////////

  double getMinX(void)
  {
    return (*(_function.begin())).first;
  }
  
  double getMaxX(void)
  {
    return (*(_function.rbegin())).first;
  }
  
  map< double, double, less<double> >::iterator begin(void)
  {
    return _function.begin();
  }
  
  map< double, double, less<double> >::iterator end(void)
  {
    return _function.end();
  }
  

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  map< double, double, less<double> > _function;
  
  ///////////////////////
  // Protected methods //
  ///////////////////////

  // Find the two defining function points that surround the given point.
  //
  // Note:  If the given X value precedes the defined portion of the
  //        function, before_pt will be _function.end() and after_pt
  //        will be _function.begin().  Likewise, if the given X value
  //        exceeds the defined portion of the function, before_pt will
  //        be _function.rbegin() and after_pt will be _function.end().
  //        If the given X is one of the defining points for the function,
  //        the same point is returned for both bounding points.
  //
  //        If there was an error, _function.end() will be returned for
  //        both points.

  void _findBoundingPoints(const double x,
			   map< double, double, less<double> >::iterator &before_pt,
			   map< double, double, less<double> >::iterator &after_pt);
  

  // Print the defining points of the function to the given stream.

  void _printFunction(FILE *stream);
  void _printFunction(ostream &stream);
  

private:

  /////////////////////
  // Private methods //
  /////////////////////

  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("PtFunction");
  }
  
};


#endif
