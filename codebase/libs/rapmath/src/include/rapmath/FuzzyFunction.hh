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
 *   $Id: FuzzyFunction.hh,v 1.4 2016/03/03 19:24:05 dixon Exp $
 *   $Revision: 1.4 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * FuzzyFunction.hh: class implementing a function defined by a series of
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

#ifndef FuzzyFunction_HH
#define FuzzyFunction_HH

/*
 **************************** includes **********************************
 */


#include <iostream>
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

class FuzzyFunction
{
public:

  ////////////////////////////////
  // Constructors & destructors //
  ////////////////////////////////

  // Constructors

  FuzzyFunction(void);
  
  // Destructor

  virtual ~FuzzyFunction(void);
  

  /////////////////////////
  // Computation methods //
  /////////////////////////

  // Apply the fuzzy function to the given data value.

  virtual double apply(const double x) const = 0;
  

  //////////////////////////
  // Input/output methods //
  //////////////////////////

  // Print the function information to the given stream.

  virtual void print(ostream &stream) const = 0;
  

private:

  /////////////////////
  // Private methods //
  /////////////////////

  // Don't let people use the copy constructor.  We do this because we
  // always use objects derived from this one and don't have a factory
  // to create these objects.

  FuzzyFunction(const FuzzyFunction& rhs);
  
  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("FuzzyFunction");
  }
  
};


#endif
