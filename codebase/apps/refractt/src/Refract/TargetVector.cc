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
 * @file TargetVector.cc
 *
 * @class TargetVector
 *
 * Class representing a target data point.
 *  
 * @date 3/31/2010
 *
 */

#include "TargetVector.hh"
#include <Refract/FieldDataPair.hh>
#include <Refract/VectorData.hh>
#include <Refract/FieldWithData.hh>

/*********************************************************************
 * Constructors
 */

TargetVector::TargetVector(int scanSize) : _scanSize(scanSize)
					   // _strength(scanSize, 0.0),
					   // _phase_diff(scanSize, 0.0),
					   // _phase_diff_er(scanSize, 0.0),
					   // _dif_iq(scanSize, 0.0),
					   // _phase(scanSize, 0.0),
					   // _phase_er(scanSize, 0.0),
					   // _iq(scanSize, 0.0)
{
  if (scanSize > 0)
  {
    _strength = VectorData(scanSize, 0.0);
    _phase_diff = VectorData(scanSize, 0.0);
    _phase_diff_er = VectorData(scanSize, 0.0);
    _dif_iq = VectorIQ(scanSize, 0.0);
    _phase = VectorData(scanSize, 0.0);
    _phase_er = VectorData(scanSize, 0.0);
    _phase_cor = VectorData(scanSize, 0.0);
    _iq = VectorIQ(scanSize, 0.0);
  }
  
  // _target.reserve(scanSize);
  //   for (int i=0; i<scanSize; ++i)
  //   {
  //     _target.push_back(TargetInfo());
  //   }
  // }
}

/*********************************************************************
 * Destructor
 */

TargetVector::~TargetVector()
{
}

void TargetVector::compute_phase_diff(const FieldDataPair &difPrevScan,
				      const std::vector<double> &norm,
				      const FieldDataPair &difFromRef)
{
  for (int i=0; i<_scanSize; ++i)
  {
    // _target[i].compute_phase_diff(difPrevScan[i], norm[i], difFromRef[i]);
    if (norm[i] != 0.0)
    {
      _phase_diff[i] = difPrevScan[i].phase();
      _phase_diff_er[i] = sqrt(-2.0*log(norm[i])/norm[i])/DEG_TO_RAD;
      _dif_iq[i] = difPrevScan[i];
    }
    else
    {
      _phase_diff[i] = refract::INVALID;
      _phase_diff_er[i] = refract::INVALID;
      _dif_iq[i].set(0.0, 0.0);
    }
    if (difFromRef[i].hasZero())
    {
      _phase[i] = refract::INVALID;
      _phase_cor[i] = refract::INVALID;
    }
    else
    {
      _phase[i] = difFromRef[i].phase();
      _phase_cor[i] = _phase[i];
    }
  }
}

void TargetVector::setIQ(const FieldDataPair &difFromRef)
{
  for (int i=0; i<_scanSize; ++i)
  {
    _iq[i] = difFromRef[i];
    // _target[i].iq = difFromRef[i];
  }
}

void TargetVector::setStrengthAndPhaseErr(const FieldWithData &snr,
					  const FieldDataPair &rawPhase,
					  const std::vector<double> &phase_er)
{
  for (int i=0; i<_scanSize; ++i)
  {
    // _target[i].setStrength(snr[i], snr.isBadAtIndex(i));

    // Compute a data quality value based on the thresholds given
    if (snr.isBadAtIndex(i))
    {
      _strength[i] = refract::INVALID;
    }
    else
    {
      _strength[i] = snr[i];
    }

    if (rawPhase[i].norm() != 0.0)
    {
      _phase_er[i] = phase_er[i];
      // _target[i].phase_er = phase_er[i];
    }
    else
    {
      _phase_er[i] = refract::INVALID;
      // _target[i].phase_er = refract::INVALID;
    }
  }
}

void TargetVector::copyPhaseDiff(VectorData &p) const
{
  p = _phase_diff;
  // for (int i=0; i<_scanSize; ++i)
  // {
  //   p[i] = _target[i].phase_diff;
  // }
}


void TargetVector::copyPhase(VectorData &p) const
{
  p = _phase;
  // for (int i=0; i<_scanSize; ++i)
  // {
  //   p[i] = _target[i].phase;
  // }
}

bool TargetVector::phase_dif_er_is_valid(int offset) const
{
  return _phase_diff_er[offset] != refract::INVALID;
}

double TargetVector::iqNorm(int offset) const
{
  return _iq[offset].norm();
}
