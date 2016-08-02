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
/*
 *  $Id: SeviriConverter.cc,v 1.5 2016/03/07 01:23:05 dixon Exp $
 *
 */

# ifndef    lint
static char RCSid[] = "$Id: SeviriConverter.cc,v 1.5 2016/03/07 01:23:05 dixon Exp $";
# endif     /* not lint */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/////////////////////////////////////////////////////////////////////////
//
// Class:	SeviriConverter
//
// Author:	G M Cunning
//
// Date:	Tue Aug 21 23:47:31 2007
//
// Description: The class handles the conversion from radiance or 
//		counts to albedo and temperatures.
//
//


// C++ include files
#include <cmath>
#include <algorithm>
#include <numeric>
#include <iostream>

// System/RAP include files

// Local include files
#include "SeviriConverter.hh"

using namespace std;

// define any constants
const string SeviriConverter::_className    = "SeviriConverter";

const float SeviriConverter::_radainceCalScale[NUM_BANDS] = 
  { 0.023128100, 0.029726600, 0.023621900, 0.0036586667, 0.0083181079, 0.038621984, 
    0.12674358, 0.10396123, 0.20503445, 0.22231142, 0.15760693, 0.031999301 };

const float SeviriConverter::_radainceCalBias[NUM_BANDS] = 
  { -1.1795331, -1.5160566, -1.2047169, -0.18659200, -0.42422350, -1.9697212,
    -6.4639227, -5.3020227, -10.456757, -11.337882, -8.0379535, -1.6319644};

const float SeviriConverter::_nuC[NUM_BANDS] = 
  { 0.0, 0.0, 0.0, 2569.094, 1598.566, 1362.142, 
    1149.083, 1034.345, 930.659, 839.661, 752.381, 0.0 };

const float SeviriConverter::_alpha[NUM_BANDS] = 
  { 0.0, 0.0, 0.0, 0.9959, 0.9963, 0.9991, 
    0.9996, 0.9999, 0.9983, 0.9988, 0.9981, 0.0 };

const float SeviriConverter::_beta[NUM_BANDS] = 
  { 0.0, 0.0, 0.0, 3.471, 2.219, 0.485, 
    0.181,  0.060, 0.627, 0.397, 0.576, 0.0 };

const float SeviriConverter::_toARad[NUM_BANDS] = 
  { 20.76, 23.24, 19.85, 1.0, 1.0, 1.0, 1.0, 1.0 , 1.0, 1.0, 1.0, 25.11 };

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Constructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

SeviriConverter::SeviriConverter()
{

}

SeviriConverter::SeviriConverter(const SeviriConverter &)
{

}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Destructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
  
SeviriConverter::~SeviriConverter()
{

}



/////////////////////////////////////////////////////////////////////////
//
// Method Name:	SeviriConverter::calculateRadiances
//
// Description:	calculates radainces from counts.
//
// Returns:	
//
// Notes:	This method should be ran on all bands. It should also
//		be ran prior to running calculateBrightnessTemps
//
//

void 
SeviriConverter::calculateRadiances(int band_num, int npts, float missing, 
				    float** data)
{
  const string methodName = _className + ":::calculateRadiances";

  int bandIdx = band_num - 1; 

  for (int i = 0; i < npts; i++) {
    float countVal = (*data)[i];

    float radVal = 0.0;

    if ((countVal > MIN_COUNT_VALUE) && (countVal < MAX_COUNT_VALUE) &&
	(fabs(countVal - _missing) < EPSILON)) {
      radVal = missing;
      continue;
    }

    radVal = 
      _radainceCalBias[bandIdx] + _radainceCalScale[bandIdx]*countVal;

    radVal = max(radVal, 0.0f);

    (*data)[i] = radVal;
  }

}

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	SeviriConverter::calculateBrightnessTemps
//
// Description:	calclutes the brightness temperatures from radiances
//
// Returns:	none
//
// Notes:       This method should be only run on IR bands, which are
//		bands 4 (START_IR_BANDS) through 11 (END_IR_BANDS).
//
//

void 
SeviriConverter::calculateBrightnessTemps(int band_num, int npts, float missing, 
					  float** data)
{
  const string methodName = _className + "::calculateBrightnessTemps";

  int bandIdx = band_num - 1; 

  float nuCubed = pow(_nuC[bandIdx], 3.0f);
  float nuPrime = _coeff2*_nuC[bandIdx];

  for (int i = 0; i < npts; i++) {

    float radVal = (*data)[i];
    
    float tempVal = 0.0;

    if (fabs(radVal - missing) < EPSILON) {
      tempVal = missing;
      continue;
    }
      
    if ((band_num >= START_IR_BANDS) && (band_num <= END_IR_BANDS)) {


      float logVal = log(_coeff1*nuCubed/radVal + 1.0);
      float tempVal = (nuPrime/logVal - _beta[bandIdx])/_alpha[bandIdx];
	
      tempVal = max(tempVal, 0.0f);
	
      if (tempVal < 0.01) {
	tempVal = _missing;
      }

      (*data)[i] = tempVal;

    }
    else {

      float albedoVal = 100.0*radVal/_toARad[bandIdx];

      albedoVal = max(albedoVal, 0.0f);
      albedoVal = min(albedoVal, 100.0f);

      if (albedoVal < 0.01) {
	albedoVal = _missing;
      }
	
      (*data)[i] = albedoVal;
    } 
      
  }

}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Public Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	SeviriConverter::operator=
//
// Description:	
//
// Returns:	
//
// Notes:
//
//

SeviriConverter& 
SeviriConverter::operator=(const SeviriConverter& from)
{
  _copy(from);

  return *this;

}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Private Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	SeviriConverter::_copy
//
// Description:	take care of making a copy
//
// Returns:	
//
// Notes:
//
//

void
SeviriConverter::_copy(const SeviriConverter& from)
{
  
}

