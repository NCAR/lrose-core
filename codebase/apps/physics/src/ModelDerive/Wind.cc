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
 *   $Date: 2016/03/06 23:15:37 $
 *   $Revision: 1.5 $
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/************************************************************************
 * Wind.cc: File containing classes to derive wind related variables.
 *
 * How to add a new derivable variable:
 *  1) Create a new function class declaration. (in Wind.hh or Temperature.hh etc.)
 *  2) Create the constructor/derive/destruct and setOutputNames class functions
 *     in the accompanying .cc file. (Example below)
 *  3) Add the class to the DeriveInterface.hh file getDeriveClassFromName and
 *     setOutputNamesFromClassName functions.
 *  4) Class can now be called by name from the parameter file. Add documentation
 *     about the derive function in paramdef.ModelDerive derive_functions variable.
 *
 * Example class derive function:
 *

void VariableFromVar1Var2::setOutputNames() 
{
  output_short_names.clear();
  output_long_names.clear();
  output_units.clear();

  numberOfInputs = 3;            // Number of inputs can be 1 to N
  numberOfOutputs = 1;           // Number of outputs can be 1 to N

  // List each outputs short name, long name, and units
  output_short_names.push_back("Output short name1");
  output_long_names.push_back("Output long name1");
  output_units.push_back("Output Units1");

  output_short_names.push_back("Output short name2");
  output_long_names.push_back("Output long name2");
  output_units.push_back("Output Units2");
}

// Construct and Destructor should be left as is
VariableFromVar1Var2::VariableFromVar1Var2(const float missing, const float bad, int nx, int ny, int nz) 
  : DeriveBase(bad, missing, nx, ny, nz)
{
  setOutputNames();
}

VariableFromVar1Var2::~VariableFromVar1Var2() { }

// The actual deriving funciton:
int VariableFromVar1Var2::derive(vector<float*> *inputs, vector<float*> *outputs) 
{
  int returnValue;
  if(returnValue = DeriveBase::derive(inputs, outputs))
    return returnValue;

  // Add your variable deriving function here

  return 0;
}

 *
 *
 * RAP, NCAR, Boulder CO
 * Jason Craig
 * Nov 2007
 *
 ************************************************************************/
#include <cmath>

#include "Wind.hh"

void WindSpeedDirectionFromUV::setOutputNames() 
{
  output_short_names.clear();
  output_long_names.clear();
  output_units.clear();

  numberOfInputs = 2;
  numberOfOutputs = 2;

  // "WDIR", "Wind direction (from which blowing)", "deg_true"
  output_short_names.push_back(Grib2::ProdDefTemp::_meteoMoment[0].name);
  output_long_names.push_back(Grib2::ProdDefTemp::_meteoMoment[0].comment);
  output_units.push_back("deg_true");

  // "WIND", "Wind speed", "m/s"  
  output_short_names.push_back(Grib2::ProdDefTemp::_meteoMoment[1].name);
  output_long_names.push_back(Grib2::ProdDefTemp::_meteoMoment[1].comment);
  output_units.push_back("inputs");  

}

WindSpeedDirectionFromUV::WindSpeedDirectionFromUV(const float missing, const float bad, int nx, int ny, int nz) 
  : DeriveBase(bad, missing, nx, ny, nz)
{
  setOutputNames();
}

WindSpeedDirectionFromUV::~WindSpeedDirectionFromUV() { }

int WindSpeedDirectionFromUV::derive(vector<float*> *inputs, vector<float*> *outputs) 
{
  int returnValue;
  if(returnValue = DeriveBase::derive(inputs, outputs))
    return returnValue;
  
  const float *uPtr = (*inputs)[0];
  const float *vPtr = (*inputs)[1];
  
  float *wd = new float[nPts];
  float *ws = new float[nPts];
  outputs->push_back(wd);
  outputs->push_back(ws);
  
  float *wdPtr = wd;
  float *wsPtr = ws;
  
  for (int i = 0; i < nPts; i++, uPtr++, vPtr++, wdPtr++, wsPtr++) {
    if((*uPtr == missing) || (*uPtr == bad) ||
       (*vPtr == missing) || (*vPtr == bad)) {
      *wdPtr = missing;
      *wsPtr = missing;
    } else {
      *wdPtr = atan2(-(*uPtr),-(*vPtr))*180.0/M_PI;
      if (*wdPtr < 0.0)
	*wdPtr += 360.0;
      *wsPtr = hypot(*uPtr, *vPtr);
    }
  }
  return 0;
}
