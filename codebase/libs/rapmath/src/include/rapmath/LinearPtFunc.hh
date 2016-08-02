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
 *   $Id: LinearPtFunc.hh,v 1.8 2016/03/03 19:24:05 dixon Exp $
 *   $Revision: 1.8 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * LinearPtFunc.hh: class implementing a function defined by a series of
 *                  points.  The function value for points is calculated
 *                  by using piecewise linear interpolation.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 1999
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef LinearPtFunc_HH
#define LinearPtFunc_HH

/*
 **************************** includes **********************************
 */


#include <map>
#include <iostream>
#include <string>
#include <cstdio>

#include <rapmath/PtFunction.hh>
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

class LinearPtFunc : public PtFunction
{
public:

  ////////////////////////////////
  // Constructors & destructors //
  ////////////////////////////////

  // Constructors

  LinearPtFunc(map< double, double, less<double> > function);	
  
  // Destructor

  ~LinearPtFunc(void);
  

  /////////////////////////
  // Computation methods //
  /////////////////////////

  // Compute the function value using linear interpolation.

  double computeFunctionValue(double x);

  double operator()(double x) { return computeFunctionValue(x); }

  //////////////////////////
  // Input/output methods //
  //////////////////////////

  // Print the function information to the given stream.

  void print(FILE *stream);
  void print(ostream &stream);

	//////////////////////////
	// Helper methods      	//
	//////////////////////////
	
	// take a string of form:
	// "CloudTopTemperatureMap, 2, {{0.0,0.0},{0.5,0.0},{1.0,1.0}}"
	// return name & map ready for passing to constructor
	static void parseString(const string s, string& name, map< double, double, less<double> >& function);

  

protected:

private:

  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("LinearPtFunc");
  }
  
	// some helper functions for parseString()
	static bool _initialParsing(string &imd, int dim, string &nameReturn);
	static bool _fillMapDef(string &imd,  map< double, double, less<double> > &mapDef, 
	                       const string &iMapName);

};


#endif
