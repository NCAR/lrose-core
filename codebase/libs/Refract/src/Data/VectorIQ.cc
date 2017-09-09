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
 * @file VectorIQ.cc
 */

#include <Refract/VectorIQ.hh>
#include <Refract/FieldWithData.hh>
#include <toolsa/LogStream.hh>

//-------------------------------------------------------------------------
VectorIQ::VectorIQ(int scanSize, double value) : _scanSize(scanSize)
{
  _iq.reserve(scanSize);
  for (int ind=0; ind<scanSize; ++ind)
  {
    _iq.push_back(IQ(value, value));
  }
}

//-------------------------------------------------------------------------
VectorIQ::VectorIQ(fl32 *i, fl32 *q, int scanSize) : _scanSize(scanSize)
{
  _iq.reserve(scanSize);
  for (int ind=0; ind<scanSize; ++ind)
  {
    _iq.push_back(IQ(i[ind], q[ind]));
  }
}

//-------------------------------------------------------------------------
void VectorIQ::initialize(fl32 *i, fl32 *q, int scanSize)
{
  _iq.clear();
  _scanSize = scanSize;
  _iq.reserve(scanSize);
  for (int ind=0; ind<scanSize; ++ind)
  {
    _iq.push_back(IQ(i[ind], q[ind]));
  }
}

//-------------------------------------------------------------------------
bool VectorIQ::copyToArrays(fl32 *i, fl32 *q, int scanSize) const
{
  if (scanSize != _scanSize)
  {
    LOG(ERROR) << "Mismatching scan sizes " << scanSize << " " << _scanSize;
    return false;
  }
  for (int ind=0; ind<scanSize; ++ind)
  {
    i[ind] = _iq[ind].i();
    q[ind] = _iq[ind].q();
  }
  return true;
}

//-------------------------------------------------------------------------
void VectorIQ::copyLargeR(int rMin, int numRange, int numAz,
			  const VectorIQ &inp)
{
  for (int i=0; i<_scanSize; ++i)
  {
    int r = FieldWithData::rIndex(i, numAz, numRange);
    if (r >= rMin)
    {
      _iq[i] = inp._iq[i];
    }
  }
}

//-----------------------------------------------------------------------
std::vector<double> VectorIQ::createDebugPhaseVector(void) const
{  
  vector<double> phase_targ;
  
  if (_scanSize == 0)
  {
    return phase_targ;
  }
  phase_targ.reserve(_scanSize);
  for (int index=0; index<_scanSize; ++index)
  {
    phase_targ.push_back(_iq[index].phase());
  }
  return phase_targ;
}

//-----------------------------------------------------------------------
std::vector<double> VectorIQ::createNormSquaredVector(void) const
{  
  vector<double> ret;
  
  if (_scanSize == 0)
  {
    return ret;
  }
  ret.reserve(_scanSize);
  for (int index=0; index<_scanSize; ++index)
  {
    ret.push_back(_iq[index].normSquared());
  }
  return ret;
}

//-----------------------------------------------------------------------
std::vector<double> VectorIQ::createNormVector(void) const
{  
  vector<double> ret;
  
  if (_scanSize == 0)
  {
    return ret;
  }
  ret.reserve(_scanSize);
  for (int index=0; index<_scanSize; ++index)
  {
    ret.push_back(_iq[index].norm());
  }
  return ret;
}

//-----------------------------------------------------------------------
void VectorIQ::setAllZero(void)
{
  for (int i=0; i<_scanSize; ++i)
  {
    _iq[i].set(0.0, 0.0);
  }
}

//-----------------------------------------------------------------------
void VectorIQ::setRangeZero(int i0, int i1)
{
  for (int i=i0; i<=i1; ++i)
  {
    _iq[i].set(0.0, 0.0);
  }
}

//-----------------------------------------------------------------------
void VectorIQ::normalize(void)
{
  for (int index = 0; index < _scanSize; ++index)
  {
    _iq[index].normalize();
  }
}

//-----------------------------------------------------------------------
void VectorIQ::copyIQ(const VectorIQ &f)
{
  _scanSize = f._scanSize;
  _iq = f._iq;
}

//-----------------------------------------------------------------------
void VectorIQ::copyIQFilterBadOrMissing(const VectorIQ &inp, double iBad,
					double iMissing, double qBad,
					double qMissing)
{
  for (int i=0; i<_scanSize; ++i)
  {
    if (inp._iq[i].isBadOrMissing(iBad, iMissing, qBad, qMissing))
    {
      _iq[i].set(0.0, 0.0);
    }
    else
    {
      _iq[i] = inp._iq[i];
    }
  }
}

//-----------------------------------------------------------------------
void VectorIQ::phaseDiffV(const VectorIQ &b)
{
  if (_scanSize != b._scanSize)
  {
    LOG(ERROR) << "Change in dimension:";
    return;
  }

  for (int i=0; i<_scanSize; ++i)
  {
    _iq[i].phaseDiffV(b._iq[i]);
  }      
}  

//-----------------------------------------------------------------------
void VectorIQ::phaseDiffV(const VectorIQ &b, const VectorIQ &c)
{
  if (_scanSize != b._scanSize || _scanSize != c._scanSize)
  {
    LOG(ERROR) << "Change in dimension:";
    return;
  }

  for (int i=0; i<_scanSize; ++i)
  {
    _iq[i] = b[i].phaseDiffC(c._iq[i]);
  }      
}  

//-----------------------------------------------------------------------
void VectorIQ::phaseDiff2V(const VectorIQ &b)
{
  for (int i=0; i<_scanSize; ++i)
  {
    _iq[i].phaseDiff2V(b._iq[i]);
  }      
}  

//-----------------------------------------------------------------------
IQ VectorIQ::normalized3ptAverage(int icenter) const
{  
  IQ ret = _iq[icenter-1] + _iq[icenter] + _iq[icenter+1];
  ret.normalize();
  return ret;
}

//-----------------------------------------------------------------------
double VectorIQ::refractivity(int icenter, double slope) const
{
  double tmp_a = (_iq[icenter].iqProduct(_iq[icenter-1]) +
		  _iq[icenter+1].iqProduct(_iq[icenter]));
  
  double tmp_b = (_iq[icenter].iqCorrelation(_iq[icenter-1]) +
		  _iq[icenter+1].iqCorrelation(_iq[icenter]));
  
  return atan2(tmp_b, tmp_a)*slope;
}

//-----------------------------------------------------------------------
void VectorIQ::normalizeUsingSNR(const FieldWithData &SNR)
{
  if (_scanSize != SNR.scanSize())
  {
    LOG(ERROR) << "Change in dimension:";
    return;
  }
  for (int i=0; i<_scanSize; ++i)
  {
    double normv = _iq[i].norm();
    if (!SNR.isBadAtIndex(i))
    {
      float strength_correction = 1.0/(1.0+pow(10.0, -0.1*SNR[i]));
      if (strength_correction > 0.001)
      {
	normv /= strength_correction;
      }
      else
      {
	normv = refract::VERY_LARGE;
      }
    }
    if (normv != 0.0)
    {
      _iq[i] /= normv;
    }
  }      
}

//-----------------------------------------------------------------------
void VectorIQ::normalizeWithVector(const std::vector<double> &norm)
{
  for (int i=0; i<_scanSize; ++i)
  {
    if (norm[i] != 0.0)
    {
      _iq[i] /= norm[i];
    }
  }
}
//-----------------------------------------------------------------------
void VectorIQ::normalizeWithQuality(const std::vector<double> quality)
{
  for (int i=0; i<_scanSize; ++i)
  {
    double n = _iq[i].norm();
    if (n != 0.0)
    {
      _iq[i] *= (quality[i]/n);
    }
  }
}

//-----------------------------------------------------------------------
void VectorIQ::smoothClose(int rMin, int rMax, int numAzimuth,
			   int numGates, const VectorIQ &difPrevScan)
{
  for (int r = rMin; r <= rMax; ++r)
  {
    _iq[r].set(0.0, 0.0);
    for (int az = 0, index = r; az < numAzimuth; ++az, index += numGates)
    {
      _iq[r] += difPrevScan[index];
    }
  } /* endfor - r */

  for (int r = rMin; r < rMax; ++r)
  {
    IQ tmp = normalized3ptAverage(r);
    for (int az = 1, index = numGates + r; az < numAzimuth;
	 ++az, index += numGates)
    {
      _iq[index] = tmp;
    } /* endfor - az */
  } /* endfor - r */

  for (int r = rMin; r < rMax; ++r)
  {
    _iq[r] = _iq[r+numGates];
  }
}

//-----------------------------------------------------------------------
void VectorIQ::smoothFar(int rMax, int numAzimuth, int numGates,
			 const VectorIQ &difPrevScan)
{
  for (int r = rMax; r < numGates; ++r)
  {
    IQ smooth_iq(0.0, 0.0);
      
    for (int dr = -numGates / 10; dr <= numGates / 10; ++dr)
    {
      if (r + dr < numGates)
      {
	for (int daz = -numAzimuth / 16; daz <= numAzimuth / 16; ++daz)
	{
	  int meas_az = daz;
	  if (meas_az < 0)  meas_az += numAzimuth;
	  if (meas_az >= numAzimuth)  meas_az -= numAzimuth;
	  int index = meas_az*numGates + r + dr;
	  
	  smooth_iq += difPrevScan[index];
	} /* endfor - daz */
      }
    } /* endfor - dr */

    _iq[r] = smooth_iq;

    for (int az = 1; az < numAzimuth; ++az)
    {
      for (int dr = -numGates / 10; dr <= numGates / 10; ++dr)
      {
	if (r + dr < numGates)
	{
	  int meas_az = az - 1 - (numAzimuth / 16);
	  if (meas_az < 0)  meas_az += numAzimuth;

	  int index = meas_az*numGates + r + dr;
	  
	  smooth_iq -= difPrevScan[index];

	  meas_az = az + (numAzimuth / 16);
	  if (meas_az >= numAzimuth) meas_az -= numAzimuth;

	  index = meas_az*numGates + r + dr;
	  smooth_iq += difPrevScan[index];
	}
      } /* endfor - dr */
	
      int index = az*numGates + r;
      _iq[index] = smooth_iq;
    } /* endfor - az */

    for (int az = 0; az < numAzimuth; ++az)
    {
      int index = az*numGates + r;
      _iq[index].normalize();
    } 
  } /* endfor - r */
}

void VectorIQ::shiftDown(int i0, int i1)
{
  for (int i=i0; i<i1; ++i)
  {
    _iq[i] = _iq[i+1];
  }
  _iq[i1].set(0.0, 0.0);
}

void VectorIQ::setSlopes(int numBeams, int smoothRange)
{
  if (numBeams*(2*smoothRange+1) != _scanSize)
  {
    LOG(ERROR) << "Mismatch";
    return;
  }
  for (int az=0,index=0; az<numBeams; ++az)
  {
    for (int dr=0; dr<=2*smoothRange; ++dr, ++index)
    {
      _iq[index] = IQ((smoothRange-dr)*az);
    }
    // for (int r=-smoothRange; r<=smoothRange; ++r)
    // {
    //   int index = r + smoothRange;
    //   if (index < 0 || index >= _scanSize)
    //   {
    // 	LOG(ERROR) << "Out of bounds";
    //   }
    //   else
    //   {
    // 	_iq[index] = IQ(r*az);
    //   }
    // }
  }
}
