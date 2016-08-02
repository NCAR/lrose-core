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
//   $Id: LinearPtFunc.cc,v 1.10 2016/03/03 18:46:08 dixon Exp $
//   $Revision: 1.10 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * LinearPtFunc.cc: class implementing a function defined by a series of
 *                  points.  The function value for points is calculated
 *                  by using piecewise linear interpolation.
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
#include <algorithm>

#include <math.h>
#include <cstdio>

#include <rapmath/LinearPtFunc.hh>
using namespace std;


/**********************************************************************
 * Constructors
 */

LinearPtFunc::LinearPtFunc(map< double, double, less<double> > function) :
  PtFunction(function)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

LinearPtFunc::~LinearPtFunc(void)
{
}
  

/**********************************************************************
 * computeFunctionValue() - Compute the function value using linear
 *                          interpolation.
 */

double LinearPtFunc::computeFunctionValue(const double x)
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
  
  // Do the linear interpolation.

  double x1 = (*before_pt).first;
  double y1 = (*before_pt).second;
  
  double x2 = (*after_pt).first;
  double y2 = (*after_pt).second;
  
  return y1 + (((x - x1) / (x2 - x1)) * (y2 - y1));
}


/**********************************************************************
 * print() - Print the function information to the given stream.
 */

void LinearPtFunc::print(FILE *stream)
{
  fprintf(stream, "Linear Interpolation Function:\n");
  fprintf(stream, "\n");
  
  _printFunction(stream);
}

void LinearPtFunc::print(ostream &stream)
{
  stream << "Linear Interpolation Function:" << endl;
  stream << endl;
  
  _printFunction(stream);
}


/**********************************************************************
 * parseString() -  take a string of form:
 * "CloudTopTemperatureMap, 2, {{0.0,0.0},{0.5,0.0},{1.0,1.0}}"
 * return name & map ready for passing to constructor
 */

void LinearPtFunc::parseString(const string s, string& name, map< double, double, less<double> >& function)
{
	function.clear();

  // get non-const version to manipulate
  string imd(s);

  _initialParsing(imd,2,name);
  _fillMapDef(imd,function,name);

}

/**********************************************************************
 * initialParsing() - removes white space, parses/removes the name, 
 * and parses/removes/verifies the dimension 
 * returns true if no errors, otherwise false
 */
bool LinearPtFunc::_initialParsing(string &imd, int dim, string &nameReturn)
{
	// remove any whitespace
	imd.erase(remove_if(imd.begin(), imd.end(), ::isspace), imd.end());

	// get the interest map name (i.e. everything up to the first comma)
	nameReturn = imd.substr(0,imd.find(','));

	// remove name and first comma from definition
	imd.erase(0,imd.find(',')+1);

	// get the dimension and verify it is "2"
	string iMapDim = imd.substr(0,imd.find(','));
	if (atoi(iMapDim.c_str()) != dim){
		cerr << "Interest map " << nameReturn
		     << " should have "<< dim << " dimensions, but has " 
		     << atoi(iMapDim.c_str()) << " returning ..." << endl;
		return false;
	}

	// remove dimension and first comma from definition
	imd.erase(0,imd.find(',')+1);    

	return true;
}
/**********************************************************************
 * This is a helper function to parse value pairs from an piecewise definition
 * The input string imd should be of the form: "{{0.0,0.0},{0.5,0.0},{1.0,1.0}}"
 *  The output mapDef can be passed to directly to the LinearPtFunc constructor
 * returns true if no errors, otherwise false
 */
bool LinearPtFunc::_fillMapDef(string &imd,  map< double, double, less<double> > &mapDef, const string &iMapName)
{

	//verify beginning/ending {} and remove
	if ((imd[0] != '{') || (imd[imd.length()-1] != '}')){
		cerr << "Interest map " << iMapName
		     << " is missing beginning or ending '{' '}'" << endl;
		return false;
	}
	imd.erase(imd.length()-1, 1);
	imd.erase(0,1);

	while(imd.length() > 0){
		//verify beginning { and remove
		if (imd[0] != '{'){
			cerr << "Interest map " << iMapName 
			     << " is badly formated: missing '{'" << endl;
			return false;
		}
		imd.erase(0,1);

		size_t commaPos = imd.find(',');
		size_t endBPos = imd.find('}');
    
		if (commaPos == string::npos){
			cerr << "Interest map " << iMapName 
			     << " is badly formated: missing ','" << endl;
			return false;       
		}
     
		if (endBPos == string::npos){
			cerr << "Interest map " << iMapName 
			     << " is badly formated: missing '}'" << endl;
			return false;       
		}
		double x = atof(imd.substr(0,commaPos).c_str());
		double y = atof(imd.substr(commaPos+1,endBPos).c_str());
		mapDef[x]=y;

		//erase all the way to comma between value pairs
		imd.erase(0,endBPos+2);
	}

	return true;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

