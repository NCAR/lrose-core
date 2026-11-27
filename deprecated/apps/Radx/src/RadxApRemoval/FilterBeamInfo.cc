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
/**
 *
 * @file FilterBeamInfo.cc
 *
 * @class FilterBeamInfo
 *
 * FilterBeamInfo is a class the manages the information for a beam in the
 * filtering process.
 *  
 * @date 8/5/2008
 *
 */

using namespace std;

#include <math.h>
#include <cstring>

#include "FilterBeamInfo.hh"
#include "InterestFunction.hh"


/**
 * Constructor
 */

FilterBeamInfo::FilterBeamInfo() :
  _azimuth(0.0),
  _numGates(0),
  _interestSum(0),
  _interestWeightSum(0),
  _interestPtrs(0),
  _confidencePtrs(0)
{
}
   

/**
 * Destructor
 */

FilterBeamInfo::~FilterBeamInfo() 
{
  // Clear interest data arrays

  delete [] _interestPtrs;
  delete [] _interestSum;
  delete [] _interestWeightSum;

  // Clear confidence data arrays

  delete [] _confidencePtrs;
}


/**
 * setBeam()
 */

void FilterBeamInfo::setBeam(const double azimuth, const int num_gates)
{
  // Save the beam information

  _azimuth = azimuth;
  _numGates = num_gates;

  // Allocate space for the arrays

  _interestSum = new double[_numGates];
  _interestWeightSum = new double[_numGates];
  _interestPtrs = new float[N_INTEREST_FIELDS * _numGates];

  _confidencePtrs = new float[N_INTEREST_FIELDS * _numGates];
   
  // Initialize the array values

  memset((void *)_interestSum, 0, sizeof(double) * _numGates);
  memset((void *)_interestWeightSum, 0, sizeof(double) * _numGates);

  for (int igate = 0; igate < _numGates; ++igate)
  {
    for (int ifield = 0; ifield < N_INTEREST_FIELDS; ++ifield)
    {
      _interestPtrs[igate * N_INTEREST_FIELDS + ifield] =
	InterestFunction::MISSING_INTEREST;

      _confidencePtrs[igate * N_INTEREST_FIELDS + ifield] =
	InterestFunction::MISSING_INTEREST;
    } /* endfor - ifield */

  } /* endfor - igate */
}


/**
 * calcInterest()
 */

void FilterBeamInfo::calcInterest(const InterestType int_type,
				  const int igate,
				  const double interest_val,
				  const double interest_weight,
				  const double confidence_val,
				  const double confidence_weight)
{
  if (igate < 0 || igate >= _numGates)
    return;
  
  // Set the interest value and update the final accumulations

  int field_index = igate * N_INTEREST_FIELDS + int_type;
  
  _interestPtrs[field_index] = interest_val;
 
  if (interest_val != InterestFunction::MISSING_INTEREST)
  {
    _interestSum[igate] += interest_weight * interest_val;
    _interestWeightSum[igate] += interest_weight;
  }

  // Set the confidence value and update the final value

  _confidencePtrs[field_index] = confidence_val;
 
  if (confidence_val != InterestFunction::MISSING_INTEREST)
  {
    int final_index = igate * N_INTEREST_FIELDS + FINAL;
    
    if (_confidencePtrs[final_index] == InterestFunction::MISSING_INTEREST)
      _confidencePtrs[final_index] = pow(confidence_val, confidence_weight);
    else
      _confidencePtrs[final_index] *= pow(confidence_val, confidence_weight);
  }
  
}


/**
 * calcFinal()
 */

void FilterBeamInfo::calcFinal(const int igate, const bool apply_confidence)
{
  if (igate < 0 || igate >= _numGates)
    return;
  
  int final_index = igate * N_INTEREST_FIELDS + FINAL;
  int finalc_index = igate * N_INTEREST_FIELDS + FINALC;

  if (_interestWeightSum[igate] != 0)
    _interestPtrs[final_index] =
      _interestSum[igate] / _interestWeightSum[igate];

  // Apply the final confidence to the final interest

  if (apply_confidence)
  {
    if (_confidencePtrs[final_index] == InterestFunction::MISSING_INTEREST)
      _interestPtrs[finalc_index] = InterestFunction::MISSING_INTEREST;
    else if (_interestWeightSum[igate] == 0)
      _interestPtrs[finalc_index] = InterestFunction::MISSING_INTEREST;
    else
      _interestPtrs[finalc_index] =
	_interestPtrs[final_index] * _confidencePtrs[final_index];
  }
  else
  {
    _interestPtrs[finalc_index] = _interestPtrs[final_index];
  }

}


/**
 * setFinal()
 */

void FilterBeamInfo::setFinal(const int igate, const float final_value)
{
  if (igate < 0 || igate >= _numGates)
    return;
  
  int final_index = igate * N_INTEREST_FIELDS + FINAL;
  int finalc_index = igate * N_INTEREST_FIELDS + FINALC;

  _interestPtrs[final_index] = final_value;
  _interestPtrs[finalc_index] = final_value;
}


/**
 * getScaledInterest()
 */

ui16 *FilterBeamInfo::getScaledInterest() const
{
  int num_values = _numGates * N_INTEREST_FIELDS;
  
  ui16 *scaled_interest = new ui16[num_values];

  for (int ivalue = 0; ivalue < num_values; ++ivalue)
  {
    float interest_val = _interestPtrs[ivalue];
    int scaled_value;
	
    if (interest_val == InterestFunction::MISSING_INTEREST)
    {
      scaled_value = InterestFunction::SCALED_MISSING_INTEREST;
    }
    else
    {
      scaled_value =
	(int) ((interest_val - InterestFunction::INTEREST_BIAS) / 
	       InterestFunction::INTEREST_SCALE + 0.5);

      if (scaled_value < InterestFunction::SCALED_MIN_INTEREST)
	scaled_value = InterestFunction::SCALED_MIN_INTEREST;
      else if (scaled_value > InterestFunction::SCALED_MAX_INTEREST)
	scaled_value = InterestFunction::SCALED_MAX_INTEREST;
    }
	
    scaled_interest[ivalue] = (ui16)scaled_value;
  } /* endfor - ivalue */

  return scaled_interest;
}


/**
 * getScaledConfidence()
 */

ui16 *FilterBeamInfo::getScaledConfidence() const
{
  int num_values = _numGates * N_INTEREST_FIELDS;
  
  ui16 *scaled_confidence = new ui16[num_values];

  for (int ivalue = 0; ivalue < num_values; ++ivalue)
  {
    float confidence_val = _confidencePtrs[ivalue];
    int scaled_value;
	
    if (confidence_val == InterestFunction::MISSING_INTEREST)
    {
      scaled_value = InterestFunction::SCALED_MISSING_INTEREST;
    }
    else
    {
      scaled_value =
	(int) ((confidence_val - InterestFunction::INTEREST_BIAS) / 
	       InterestFunction::INTEREST_SCALE + 0.5);

      if (scaled_value < InterestFunction::SCALED_MIN_INTEREST)
	scaled_value = InterestFunction::SCALED_MIN_INTEREST;
      else if (scaled_value > InterestFunction::SCALED_MAX_INTEREST)
	scaled_value = InterestFunction::SCALED_MAX_INTEREST;
    }
	
    scaled_confidence[ivalue] = (ui16)scaled_value;
  } /* endfor - ivalue */

  return scaled_confidence;
}

