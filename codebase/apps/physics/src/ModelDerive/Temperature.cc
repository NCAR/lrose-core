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
 *   $Revision: 1.9 $
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/************************************************************************
 * Temperature.cc: File containing classes to derive temperature
 *  related variables.
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
 * RAP, NCAR, Boulder CO
 * Jason Craig
 * Nov 2007
 *
 ************************************************************************/
#include <cmath>

#include "Temperature.hh"

void AirTempFromVptmpMixrPres::setOutputNames() 
{
  output_short_names.clear();
  output_long_names.clear();
  output_units.clear();

  numberOfInputs = 3;
  numberOfOutputs = 1;
  
  // "TMP", "Temperature", "C"
  output_short_names.push_back(Grib2::ProdDefTemp::_meteoTemp[0].name);
  output_long_names.push_back(Grib2::ProdDefTemp::_meteoTemp[0].comment);
  output_units.push_back("C");

}

AirTempFromVptmpMixrPres::AirTempFromVptmpMixrPres(const float missing, const float bad, int nx, int ny, int nz) 
  : DeriveBase(bad, missing, nx, ny, nz)
{
  setOutputNames();
}

AirTempFromVptmpMixrPres::~AirTempFromVptmpMixrPres() { }

int AirTempFromVptmpMixrPres::derive(vector<float*> *inputs, vector<float*> *outputs) 
{
  int returnValue;
  if(returnValue = DeriveBase::derive(inputs, outputs))
    return returnValue;
  
  const float *vptmpPtr = (*inputs)[0];
  const float *mixrPtr = (*inputs)[1];
  const float *presPtr = (*inputs)[2];

  float *tmp = new float[nPts];
  outputs->push_back(tmp);
  
  float *tmpPtr = tmp;
  float hspec, theta;

  for (int i = 0; i < nPts; i++, vptmpPtr++, mixrPtr++, presPtr++, tmpPtr++) {
    if((*vptmpPtr == missing) || (*vptmpPtr == bad) ||
       (*mixrPtr == missing) || (*mixrPtr == bad) ||
       (*presPtr == missing) || (*presPtr == bad)) {
      *tmpPtr = missing;
    } else {
      //hspec = (*mixrPtr)/(1.0 + (*mixrPtr));
      //theta = (*vptmpPtr)/(1.0+0.6078*hspec);
      //*tmpPtr = kelvin2Celsius( pow(theta*((*presPtr)/100000.0), 0.286) );

      theta = (*vptmpPtr) / (1.0 + 0.61 * (*mixrPtr) / 1000.0);
      *tmpPtr = kelvin2Celsius( theta / pow( ( 100000.0 / (*presPtr) ), 0.286 ) );
      
    }
  }
  return 0;
}

void RhFromTmpMixrPres::setOutputNames() 
{
  output_short_names.clear();
  output_long_names.clear();
  output_units.clear();

  numberOfInputs = 3;
  numberOfOutputs = 1;

  // "RH", "Relative Humidity", "%"
  //output_short_names.push_back(Grib2::ProdDefTemp::_meteoMoist[1].name);
  output_short_names.push_back("R_H");
  output_long_names.push_back(Grib2::ProdDefTemp::_meteoMoist[1].comment);
  output_units.push_back(Grib2::ProdDefTemp::_meteoMoist[1].unit);

}

RhFromTmpMixrPres::RhFromTmpMixrPres(const float missing, const float bad, int nx, int ny, int nz) 
  : DeriveBase(bad, missing, nx, ny, nz)
{
  setOutputNames();
}

RhFromTmpMixrPres::~RhFromTmpMixrPres() { }

int RhFromTmpMixrPres::derive(vector<float*> *inputs, vector<float*> *outputs) 
{
  int returnValue;
  if(returnValue = DeriveBase::derive(inputs, outputs))
    return returnValue;
  
  const float *tmpPtr = (*inputs)[0];
  const float *mixrPtr = (*inputs)[1];
  const float *presPtr = (*inputs)[2];

  float *rh = new float[nPts];
  outputs->push_back(rh);
  
  float *rhPtr = rh;
  float e, esat;

  for (int i = 0; i < nPts; i++, tmpPtr++, mixrPtr++, presPtr++, rhPtr++) {
    if((*tmpPtr == missing) || (*tmpPtr == bad) ||
       (*mixrPtr == missing) || (*mixrPtr == bad) ||
       (*presPtr == missing) || (*presPtr == bad)) {
      *rhPtr = missing;
    } else {
      e = ((*mixrPtr)*(*presPtr)) / (.622-(*mixrPtr));
      esat = e_sub_s(celsius2Kelvin(*tmpPtr));
      *rhPtr = e/esat;
      if(*rhPtr > 1.0)
	 *rhPtr = 1.0;
    }
  }
  return 0;
}

void RhFromVptmpMixrPres::setOutputNames() 
{
  output_short_names.clear();
  output_long_names.clear();
  output_units.clear();

  numberOfInputs = 3;
  numberOfOutputs = 1;

  // "RH", "Relative Humidity", "%"
  output_short_names.push_back(Grib2::ProdDefTemp::_meteoMoist[1].name);
  output_long_names.push_back(Grib2::ProdDefTemp::_meteoMoist[1].comment);
  output_units.push_back(Grib2::ProdDefTemp::_meteoMoist[1].unit);

}

RhFromVptmpMixrPres::RhFromVptmpMixrPres(const float missing, const float bad, int nx, int ny, int nz) 
  : DeriveBase(bad, missing, nx, ny, nz)
{
  setOutputNames();
}

RhFromVptmpMixrPres::~RhFromVptmpMixrPres() { }

int RhFromVptmpMixrPres::derive(vector<float*> *inputs, vector<float*> *outputs) 
{
  int returnValue;
  if(returnValue = DeriveBase::derive(inputs, outputs))
    return returnValue;
  
  const float *vptmpPtr = (*inputs)[0];
  const float *mixrPtr = (*inputs)[1];
  const float *presPtr = (*inputs)[2];

  float *rh = new float[nPts];
  outputs->push_back(rh);
  
  float *rhPtr = rh;
  float theta, saturationMR;

  for (int i = 0; i < nPts; i++, vptmpPtr++, mixrPtr++, presPtr++, rhPtr++) {
    if((*vptmpPtr == missing) || (*vptmpPtr == bad) ||
       (*mixrPtr == missing) || (*mixrPtr == bad) ||
       (*presPtr == missing) || (*presPtr == bad)) {
      *rhPtr = missing;
    } else {
	theta = celsius2Kelvin(*vptmpPtr) /
	  (1.0 + 0.61 * (*mixrPtr) / 1000.0);
	theta /= pow( ( 1000.0 / (*presPtr) ), 0.286 );
	saturationMR = ( 3.8 / (*presPtr) ) *
		    exp(17.27*kelvin2Celsius(theta)/(theta - 35.86));
	*rhPtr = 100.0 * (*mixrPtr) / 1000.0 / saturationMR;
    }
  }
  return 0;
}


//+---+-----------------------------------------------------------------+

void RhFromTmpSpecPres::setOutputNames() 
{
  output_short_names.clear();
  output_long_names.clear();
  output_units.clear();

  numberOfInputs = 3;
  numberOfOutputs = 1;

  // "RH", "Relative Humidity", "%"
  //output_short_names.push_back(Grib2::ProdDefTemp::_meteoMoist[1].name);
  output_short_names.push_back("R_H");
  output_long_names.push_back(Grib2::ProdDefTemp::_meteoMoist[1].comment);
  output_units.push_back(Grib2::ProdDefTemp::_meteoMoist[1].unit);

}

RhFromTmpSpecPres::RhFromTmpSpecPres(const float missing, const float bad, int nx, int ny, int nz) 
  : DeriveBase(bad, missing, nx, ny, nz)
{
  setOutputNames();
}

RhFromTmpSpecPres::~RhFromTmpSpecPres() { }

int RhFromTmpSpecPres::derive(vector<float*> *inputs, vector<float*> *outputs) 
{
  int returnValue;
  if(returnValue = DeriveBase::derive(inputs, outputs))
    return returnValue;
  
  const float *tmpPtr = (*inputs)[0];
  const float *specPtr = (*inputs)[1];
  const float *presPtr = (*inputs)[2];

  float *rh = new float[nPts];
  outputs->push_back(rh);
  
  float *rhPtr = rh;
  float e, esat;

  for (int i = 0; i < nPts; i++, tmpPtr++, specPtr++, presPtr++, rhPtr++) {
    if((*tmpPtr == missing) || (*tmpPtr == bad) ||
       (*specPtr == missing) || (*specPtr == bad) ||
       (*presPtr == missing) || (*presPtr == bad)) {
      *rhPtr = missing;
    } else {
      e = ((*specPtr)*(*presPtr)) / (.622-(*specPtr));
      esat = e_sub_s(celsius2Kelvin(*tmpPtr));
      *rhPtr = e/esat;
      if(*rhPtr > 1.0)
	 *rhPtr = 1.0;
    }
  }
  return 0;
}

//+---+-----------------------------------------------------------------+
