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
 * @file Data.cc
 */

#include <cstdio>
#include <cmath>
#include <Radx/RayxData.hh>
#include <Radx/RadxField.hh>

//-----------------------------------------------------------------
RayxData::RayxData(void) :
        _name("unknown"),
        _units("unknown"),
        _npt(0),
        _missing(0),
        _az(0),
        _elev(0),
        _gate_spacing(0),
        _start_range(0),
        _debug(false)
{
}

//-----------------------------------------------------------------
RayxData::RayxData(const std::string &name, const std::string &units,
                   const int npt, const double missing, const double az,
                   const double elev, const double gate_spacing, 
                   const double start_range) :
        _name(name),
        _units(units),
        _npt(npt),
        _missing(missing),
        _az(az),
        _elev(elev),
        _gate_spacing(gate_spacing),
        _start_range(start_range),
        _debug(false)
{
  _data.resize(_npt);
  for (int i=0; i<npt; ++i)
  {
    _data[i] = _missing;
  }
}

//-----------------------------------------------------------------
RayxData::RayxData(const std::string &name, const std::string &units,
                   const int npt, const double missing, const double az,
                   const double elev, const double gate_spacing, 
                   const double start_range, const RadxField &data) :
        _name(name),
        _units(units),
        _npt(npt),
        _missing(missing),
        _az(az),
        _elev(elev),
        _gate_spacing(gate_spacing),
        _start_range(start_range),
        _debug(false)
{
  int npoints = data.getNPoints();
  int npt0, npt1;
  const Radx::fl32 *d = data.getDataFl32();
  if (npoints != _npt)
  {
    if (_debug) {
      cerr << "WARNING - RayxData::RayxData" << endl;
      cerr << "  Constructor npt=" << _npt << ", RadxField npt=" << npoints << endl;
    }
    if (npoints > _npt)
    {
      npt0 = _npt;
      npt1 = npoints;
    }
    else
    {
      npt0 = npoints;
      npt1 = _npt;
    }
  }
  else
  {
    npt0 = _npt;
    npt1 = _npt;
  }

  _data.resize(_npt);
  for (int i=0; i<npt0; ++i)
  {
    _data[i] = d[i];
  }
  for (int i=npt0; i<npt1; ++i)
  {
    _data[i] = _missing;
  }
}


//-----------------------------------------------------------------
RayxData::~RayxData()
{
}

//-----------------------------------------------------------------
bool RayxData::retrieveData(Radx::fl32 *data, const int npt) const
{
  if (npt != _npt)
  {
    cerr << "ERROR - RayxData::retrieveData" << endl;
    cerr << "  Npt input " << npt << " versus local " << _npt << endl;
    return false;
  }
  for (int i=0; i<npt; ++i)
  {
    data[i] = static_cast<Radx::fl32>(_data[i]);
  }
  return true;
}

//-----------------------------------------------------------------
void RayxData::storeData(const Radx::fl32 *data, const int npt)
{
  int n = _npt;
  if (npt != _npt)
  {
    if (_debug) {
      cerr << "WARNING - RayxData::storeData" << endl;
      cerr << "  npt changed " << _npt << " to " << npt << endl;
    }
    if (_npt > npt)
    {
      n = npt;
    }
  }
  for (int i=0; i<n; ++i)
  {
    _data[i] = data[i];
  }
  for (int i=n+1; i<_npt; ++i)
  {
    _data[i] = _missing;
  }
}

//-----------------------------------------------------------------
void RayxData::setV(const int index, const double value)
{
  if (index < 0 || index >= _npt)
  {
    cerr << "ERROR - RayxData::setV" << endl;
    cerr << "  index out of range " << index << endl;
  }
  else
  {
    _data[index] = value;
  }
}

//-----------------------------------------------------------------
bool RayxData::getV(const int index, double &value) const
{
  if (index < 0 || index >= _npt)
  {
    cerr << "ERROR - RayxData::getV" << endl;
    cerr << "  index out of range " << index << endl;
    return false;
  }
  else
  {
    value = _data[index];
    return value != _missing;
  }
}

//-----------------------------------------------------------------
void RayxData::multiply(const double v)
{
  for (int i=0; i<_npt; ++i)
  {
    if (_data[i] != _missing)
    {
      _data[i] *= v;
    }
  }
}

//-----------------------------------------------------------------
void RayxData::multiply(const RayxData &inp, const bool missing_ok)
{
  if (inp._npt != _npt)
  {
    cerr << "ERROR - RayxData::multiply" << endl;
    cerr << "  input npt " << inp._npt << " not same as local " << _npt << endl;
    return;
  }
  for (int i=0; i<_npt; ++i)
  {
    if (inp._data[i] != inp._missing &&	_data[i] != _missing)
    {
      _data[i] *= inp._data[i];
    }
    else
    {
      pPassthrough(inp, i, missing_ok);
    }
  }
}

//-----------------------------------------------------------------
void RayxData::min(const RayxData &inp)
{
  if (inp._npt != _npt)
  {
    cerr << "ERROR - RayxData::min" << endl;
    cerr << "  input npt " << inp._npt << " not same as local " << _npt << endl;
    return;
  }
  for (int i=0; i<_npt; ++i)
  {
    if (inp._data[i] != inp._missing &&	_data[i] != _missing)
    {
      if (_data[i] > inp._data[i])
      {
	_data[i] = inp._data[i];
      }
    }
    else if (inp._data[i] != inp._missing && _data[i] == _missing)
    {
      _data[i] = inp._data[i];
    }
  }
}

//-----------------------------------------------------------------
void RayxData::max(const RayxData &inp)
{
  if (inp._npt != _npt)
  {
    cerr << "ERROR - RayxData::max" << endl;
    cerr << "  input npt " << inp._npt << " not same as local " << _npt << endl;
    return;
  }
  for (int i=0; i<_npt; ++i)
  {
    if (inp._data[i] != inp._missing &&	_data[i] != _missing)
    {
      if (_data[i] < inp._data[i])
      {
	_data[i] = inp._data[i];
      }
    }
    else if (inp._data[i] != inp._missing && _data[i] == _missing)
    {
      _data[i] = inp._data[i];
    }
    else
    {
      _data[i] = _missing;
    }
  }
}

//-----------------------------------------------------------------
void RayxData::divide(const RayxData &inp)
{
  if (inp._npt != _npt)
  {
    cerr << "ERROR - RayxData::divide" << endl;
    cerr << "  input npt " << inp._npt << " not same as local " << _npt << endl;
    return;
  }
  for (int i=0; i<_npt; ++i)
  {
    if (inp._data[i] != inp._missing &&	_data[i] != _missing)
    {
      if (fabs(inp._data[i]) > 0.000001)
      {
	_data[i] /= inp._data[i];
      }
      else
      {
	_data[i] = _missing;
      }
    }
    else
    {
      _data[i] = _missing;
    }
  }
}

//-----------------------------------------------------------------
void RayxData::divide(const double v)
{
  if (fabs(v) < 0.000001)
  {
    cerr << "ERROR - RayxData::divide" << endl;
    cerr << "  Trying to divide by too small a number " << v << endl;
    return;
  }

  for (int i=0; i<_npt; ++i)
  {
    if (_data[i] != _missing)
    {
      _data[i] /= v;
    }
  }
}

//-----------------------------------------------------------------
void RayxData::inc(const RayxData &inp, const bool missing_ok)
{
  if (inp._npt != _npt)
  {
    cerr << "ERROR - RayxData::inc" << endl;
    cerr << "  input npt " << inp._npt << " not same as local " << _npt << endl;
    return;
  }

  for (int i=0; i<_npt; ++i)
  {
    if (_data[i] != _missing && inp._data[i] != inp._missing)
    {
      _data[i] += inp._data[i];
    }
    else
    {
      pPassthrough(inp, i, missing_ok);
    }
  }
}

//-----------------------------------------------------------------
void RayxData::inc(const double v)
{
  for (int i=0; i<_npt; ++i)
  {
    if (_data[i] != _missing)
    {
      _data[i] += v;
    }
  }
}

//-----------------------------------------------------------------
void RayxData::dec(const RayxData &inp, const bool missing_ok)
{
  if (inp._npt != _npt)
  {
    cerr << "ERROR - RayxData::dec" << endl;
    cerr << "  input npt " << inp._npt << " not same as local " << _npt << endl;
    return;
  }

  for (int i=0; i<_npt; ++i)
  {
    if (_data[i] != _missing && inp._data[i] != inp._missing)
    {
      _data[i] -= inp._data[i];
    }
    else
    {
      pPassthrough(inp, i, missing_ok);
    }
  }
}

//-----------------------------------------------------------------
void RayxData::dec(const double v)
{
  for (int i=0; i<_npt; ++i)
  {
    if (_data[i] != _missing)
    {
      _data[i] -= v;
    }
  }
}

//-----------------------------------------------------------------
void RayxData::abs(void)
{
  for (int i=0; i<_npt; ++i)
  {
    if (_data[i] != _missing)
    {
      _data[i] = fabs(_data[i]);
    }
  }
}

//-----------------------------------------------------------------
void RayxData::incAtPoint(const int i, const double v)
{
  if (_data[i] != _missing)
  {
    _data[i] += v;
  }
  else
  {
    _data[i] = v;
  }
}

//-----------------------------------------------------------------
void RayxData::subtract(const RayxData &inp)
{
  if (inp._npt != _npt)
  {
    cerr << "ERROR - RayxData::subtract" << endl;
    cerr << "  input npt " << inp._npt << " not same as local " << _npt << endl;
    return;
  }

  for (int i=0; i<_npt; ++i)
  {
    if (_data[i] != _missing && inp._data[i] != inp._missing)
    {
      _data[i] -= inp._data[i];
    }
  }
}

//-----------------------------------------------------------------
void RayxData::remap(const double scale, const double offset)
{
  for (int i=0; i<_npt; ++i)
  {
    if (_data[i] != _missing)
    {
      _data[i] = offset + _data[i]*scale;
    }
  }
}

//-----------------------------------------------------------------
double RayxData::differenceAtPoint(const RayxData &other,
                                   const int i) const
{
  if (other._data[i] == other._missing || _data[i] == _missing)
  {
    return _missing;
  }
  else
  {
    return _data[i] - other._data[i];
  }
}

//-----------------------------------------------------------------
void RayxData::setAllToValue(const double v)
{
  for (int i=0; i<_npt; ++i)
  {
    _data[i] = v;
  }
}

//-----------------------------------------------------------------
bool RayxData::matchBeam(const double x0, const double dx) const
{
  bool ret = true;
  if (dx != _gate_spacing)
  {
    cerr << "ERROR - RayxData::matchBeam" << endl;
    cerr << "  dx input " << _gate_spacing << ", local " << dx << endl;
    ret = false;
  }
  if (x0 != _start_range)
  {
    cerr << "ERROR - RayxData::matchBeam" << endl;
    cerr << "  x0 input " << _start_range << ", local " << x0 << endl;
    ret = false;
  }
  return ret;
}

//-----------------------------------------------------------------
void RayxData::speckleMask(const double outlierThresh, 
                           const double maskOutputValue,
                           const double nonMaskOutputValue,
                           const bool up)
{
  if (maskOutputValue == _missing || nonMaskOutputValue == _missing)
  {
    cerr << "ERROR - RayxData::speckleMask" << endl;
    cerr << "  Missing value " << _missing << " = new mask value" << endl;
    for (int i=0; i<_npt; ++i)
    {
      if (_data[i] != _missing)
      {
	_data[i] = nonMaskOutputValue;
      }
    }
    return;
  }

  vector<double> d;
  d.resize(_npt);
  for (int i=0; i<_npt; ++i)
  {
    if (_data[i] != _missing)
    {
      d[i] = nonMaskOutputValue;
    }
    else
    {
      d[i] = _missing;
    }
  }

  for (int i=2; i<_npt-3; )
  {
    bool this_gate = false, next_gate = false;
    
    if (_data[i] == _missing || _data[i-2] == _missing ||
	_data[i+2] == _missing)
    {
      // can't be an outlier
      this_gate = false;
      i++;
      continue;
    }
    if (_data[i] > (_data[i-2] + outlierThresh) &&
	_data[i] > (_data[i+2] + outlierThresh))
    {
      this_gate = true;
      if (_data[i+1] == _missing || _data[i-1] == _missing ||
	  _data[i+3] == _missing)
      {
	// next can't be an outlier
	next_gate = false;
      }
      else
      {
	if (_data[i+1] > (_data[i-1] + outlierThresh) &&
	    _data[i+1] > (_data[i+3] + outlierThresh))
	{
	  next_gate = true;
	}
      }
    }
    if (!this_gate && !up)
    {
      if (_data[i] < (_data[i-2] - outlierThresh) &&
	  _data[i] < (_data[i+2] - outlierThresh))
      {
	this_gate = true;
	if (_data[i+1] == _missing || _data[i-1] == _missing ||
	    _data[i+3] == _missing)
	{
	  // next can't be an outlier
	  next_gate = false;
	}
	else
	{
	  if (_data[i+1] < (_data[i-1] - outlierThresh) &&
	      _data[i+1] < (_data[i+3] - outlierThresh))
	  {
	    next_gate = true;
	  }
	}
      }
    }
    if (this_gate)
    {
      if (!next_gate)
      {
	// only gate i has clutter, substitute accordingly
	d[i] = maskOutputValue;
	i++;
      }
      else
      {
	// both gate i and i+1 has clutter, substitute accordingly
	d[i] = d[i+1] = maskOutputValue;
	i += 2;
      }
    }
    else
    {
      i++;
    }
  }
  for (int i=0; i<_npt; ++i)
  {
    _data[i] = d[i];
  }
}

//-----------------------------------------------------------------
void RayxData::maskWhenEqual(const double maskValue, const double tolerance,
                             const double maskOutputValue,
                             const double nonMaskOutputValue)
{
  for (int i=0; i<_npt; ++i)
  {
    if (_data[i] != _missing)
    {
      if (fabs(_data[i]-maskValue) < tolerance)
      {
	_data[i] = maskOutputValue;
      }
      else
      {
	_data[i] = nonMaskOutputValue;
      }
    }
  }
}

//-----------------------------------------------------------------
void RayxData::maskWhenLessThan(double threshold, double replacement, 
                                bool replaceWithMissing)
{
  for (int i=0; i<_npt; ++i)
  {
    if (_data[i] != _missing)
    {
      if (_data[i] < threshold)
      {
	if (replaceWithMissing)
	{
	  _data[i] = _missing;
	}
	else
	{
	  _data[i] = replacement;
	}
      }
    }
  }
}

//-----------------------------------------------------------------
void RayxData::maskWhenLessThanOrEqual(double threshold, double replacement, 
                                       bool replaceWithMissing)
{
  for (int i=0; i<_npt; ++i)
  {
    if (_data[i] != _missing)
    {
      if (_data[i] <= threshold)
      {
	if (replaceWithMissing)
	{
	  _data[i] = _missing;
	}
	else
	{
	  _data[i] = replacement;
	}
      }
    }
  }
}

//-----------------------------------------------------------------
void RayxData::maskWhenGreaterThan(double threshold, double replacement, 
                                   bool replaceWithMissing)

{
  for (int i=0; i<_npt; ++i)
  {
    if (_data[i] != _missing)
    {
      if (_data[i] > threshold)
      {
	if (replaceWithMissing)
	{
	  _data[i] = _missing;
	}
	else
	{
	  _data[i] = replacement;
	}
      }
    }
  }
}

//-----------------------------------------------------------------
void RayxData::maskWhenGreaterThanOrEqual(double threshold, double replacement, 
                                          bool replaceWithMissing)

{
  for (int i=0; i<_npt; ++i)
  {
    if (_data[i] != _missing)
    {
      if (_data[i] >= threshold)
      {
	if (replaceWithMissing)
	{
	  _data[i] = _missing;
	}
	else
	{
	  _data[i] = replacement;
	}
      }
    }
  }
}

//-----------------------------------------------------------------
void RayxData::maskWhenMissing(double replacement)
{
  for (int i=0; i<_npt; ++i)
  {
    if (_data[i] == _missing)
    {
      _data[i] = replacement;
    }
  }
}

//-----------------------------------------------------------------
void RayxData::maskRestrict(const RayxData &mask, const double maskValue,
                            const double nonMaskOutputValue)
{
  if (mask._npt != _npt)
  {
    cerr << "ERROR - RayxData::maskRestrict" << endl;
    cerr << "  input npt " << mask._npt << " not same as local " << _npt << endl;
    return;
  }

  for (int i=0; i<_npt; ++i)
  {
    if (mask._data[i] != maskValue)
    {
      _data[i] = nonMaskOutputValue;
    }
  }
}

//-----------------------------------------------------------------
void RayxData::maskToMissing(const RayxData &mask, const double maskValue)
{
  if (mask._npt != _npt)
  {
    cerr << "ERROR - RayxData::maskToMissing" << endl;
    cerr << "  input npt " << mask._npt << " not same as local " << _npt << endl;
    return;
  }

  for (int i=0; i<_npt; ++i)
  {
    if (mask._data[i] != maskValue)
    {
      _data[i] = _missing;
    }
  }
}

//-----------------------------------------------------------------
void RayxData::maskFilter(const RayxData &mask, const double maskValue,
                          double dataValue)
{
  if (mask._npt != _npt)
  {
    cerr << "ERROR - RayxData::maskFilter" << endl;
    cerr << "  input npt " << mask._npt << " not same as local " << _npt << endl;
    return;
  }

  for (int i=0; i<_npt; ++i)
  {
    if (mask._data[i] == maskValue)
    {
      _data[i] = dataValue;
    }
  }
}

//-----------------------------------------------------------------
void RayxData::maskFilterLessThan(const RayxData &mask, const double maskValue,
                                  double dataValue)
{
  if (mask._npt != _npt)
  {
    cerr << "ERROR - RayxData::maskFilterLessThan" << endl;
    cerr << "  input npt " << mask._npt << " not same as local " << _npt << endl;
    return;
  }

  for (int i=0; i<_npt; ++i)
  {
    if (mask._data[i] < maskValue && mask._data[i] != mask._missing)
    {
      _data[i] = dataValue;
    }
  }
}

//-----------------------------------------------------------------
void RayxData::maskMissingFilter(const RayxData &mask, double dataValue)
{
  if (mask._npt != _npt)
  {
    cerr << "ERROR - RayxData::maskMissingFilter" << endl;
    cerr << "  input npt " << mask._npt << " not same as local " << _npt << endl;
    return;
  }

  for (int i=0; i<_npt; ++i)
  {
    if (mask._data[i] == mask._missing)
    {
      _data[i] = dataValue;
    }
  }
}

//-----------------------------------------------------------------
void RayxData::modifyWhenMaskLessThan(const RayxData &mask, double thresh, double dataValue,
                                      bool replaceWithMissing)
{
  if (mask._npt != _npt)
  {
    cerr << "ERROR - RayxData::modifyWhenMaskLessThan" << endl;
    cerr << "  input npt " << mask._npt << " not same as local " << _npt << endl;
    return;
  }

  for (int i=0; i<_npt; ++i)
  {
    if (mask._data[i] < thresh && mask._data[i] != mask._missing)
    {
      if (replaceWithMissing)
      {
	_data[i] = _missing;
      }
      else
      {
	_data[i] = dataValue;
      }
    }
  }
}

//-----------------------------------------------------------------
void RayxData::modifyWhenMaskLessThanOrEqual(const RayxData &mask, double thresh,
                                             double dataValue, bool replaceWithMissing)
{
  if (mask._npt != _npt)
  {
    cerr << "ERROR - RayxData::modifyWhenMaskLessThanOrEqual" << endl;
    cerr << "  input npt " << mask._npt << " not same as local " << _npt << endl;
    return;
  }

  for (int i=0; i<_npt; ++i)
  {
    if (mask._data[i] <= thresh && mask._data[i] != mask._missing)
    {
      if (replaceWithMissing)
      {
	_data[i] = _missing;
      }
      else
      {
	_data[i] = dataValue;
      }
    }
  }
}


//-----------------------------------------------------------------
void RayxData::modifyWhenMaskGreaterThan(const RayxData &mask, double thresh,
                                         double dataValue, bool replaceWithMissing)
{
  if (mask._npt != _npt)
  {
    cerr << "ERROR - RayxData::modifyWhenMaskGreaterThan" << endl;
    cerr << "  input npt " << mask._npt << " not same as local " << _npt << endl;
    return;
  }

  for (int i=0; i<_npt; ++i)
  {
    if (mask._data[i] > thresh && mask._data[i] != mask._missing)
    {
      if (replaceWithMissing)
      {
	_data[i] = _missing;
      }
      else
      {
	_data[i] = dataValue;
      }
    }
  }
}


//-----------------------------------------------------------------
void RayxData::modifyWhenMaskGreaterThanOrEqual(const RayxData &mask, double thresh,
                                                double dataValue, bool replaceWithMissing)
{
  if (mask._npt != _npt)
  {
    cerr << "ERROR - RayxData::modifyWhenMaskGreaterThanOrEqual" << endl;
    cerr << "  input npt " << mask._npt << " not same as local " << _npt << endl;
    return;
  }

  for (int i=0; i<_npt; ++i)
  {
    if (mask._data[i] >= thresh && mask._data[i] != mask._missing)
    {
      if (replaceWithMissing)
      {
	_data[i] = _missing;
      }
      else
      {
	_data[i] = dataValue;
      }
    }
  }
}

//-----------------------------------------------------------------
void RayxData::modifyWhenMaskMissing(const RayxData &mask, double dataValue,
                                     bool replaceWithMissing)
{
  if (mask._npt != _npt)
  {
    cerr << "ERROR - RayxData::modifyWhenMaskMissing" << endl;
    cerr << "  input npt " << mask._npt << " not same as local " << _npt << endl;
    return;
  }

  for (int i=0; i<_npt; ++i)
  {
    if (mask._data[i] == mask._missing)
    {
      if (replaceWithMissing)
      {
	_data[i] = _missing;
      }
      else
      {
	_data[i] = dataValue;
      }
    }
  }
}


//-----------------------------------------------------------------
void RayxData::fuzzyRemap(const RadxFuzzyF &f)
{
  for (int i=0; i<_npt; ++i)
  {
    if (_data[i] != _missing)
    {
      _data[i] = f.apply(_data[i]);
    }
  }
}

//-----------------------------------------------------------------
void RayxData::fuzzy2dRemap(const RadxFuzzy2d &f, const RayxData &y)
{
  if (y._npt != _npt)
  {
    cerr << "ERROR - RayxData::fuzzy2dRemap" << endl;
    cerr << "  input npt " << y._npt << " not same as local " << _npt << endl;
    return;
  }

  for (int i=0; i<_npt; ++i)
  {
    if (_data[i] != _missing && y._data[i] != y._missing)
    {
      _data[i] = f.apply(_data[i], y._data[i]);
    }
    else
    {
      _data[i] = _missing;
    }
  }
}

//-----------------------------------------------------------------
void RayxData::changeMissing(const double missing)
{
  if (_missing == missing)
  {
    return;
  }

  int count = 0;
  for (int i=0; i<_npt; ++i)
  {
    if (_data[i] == missing)
    {
      ++count;
    }
    else if (_data[i] == _missing)
    {
      _data[i] = missing;
    }
  }
  if (count != 0 && _debug)
  {
    cerr << "WARNING - RayxData::changeMissing" << endl;
    cerr << "  "  << count << " points incorectly have new missing " << missing << endl;
  }
  _missing = missing;
}


//-----------------------------------------------------------------
void RayxData::db2linear(void)
{
  for (int i=0; i<_npt; ++i)
  {
    if (_data[i] != _missing)
    {
      _data[i] = pow(10, _data[i]/10.0); 
    }
  }
}

//-----------------------------------------------------------------
void RayxData::linear2db(void)
{
  for (int i=0; i<_npt; ++i)
  {
    if (_data[i] != _missing)
    {
      if (_data[i] != 0.0)
      {
	_data[i] = 10.0*log10(_data[i]);
      }
      else
      {
	_data[i] = _missing;
      }
    }
  }
}

//-----------------------------------------------------------------
void RayxData::gaussian2dRemap(const RayxData &y, const double xfactor,
                               const double yfactor, const bool absX,
                               const bool absY, const double scale)
{
  if (y._npt != _npt)
  {
    cerr << "ERROR - RayxData::gaussian2dRemap" << endl;
    cerr << "  input npt " << y._npt << " not same as local " << _npt << endl;
    return;
  }

  for (int i=0; i<_npt; ++i)
  {
    if (_data[i] != _missing && y._data[i] != y._missing)
    {
      double xi, yi;
      if (absX)
      {
	xi = fabs(_data[i]);
      }
      else
      {
	xi = _data[i];
      }
      if (absY)
      {
	yi = fabs(y._data[i]);
      }
      else
      {
	yi = y._data[i];
      }
      _data[i] = 1.0 - exp(-scale*(xi*xfactor + yi*yfactor));
    }
    else
    {
      _data[i] = _missing;
    }
  }
}

//-----------------------------------------------------------------
void RayxData::qscale(double scale, double topv, double lowv, bool invert)
{
  for (int i=0; i<_npt; ++i)
  {
    if (_data[i] != _missing)
    {
      _data[i] = exp(-scale*pow((_data[i]/topv - lowv/topv), 2.0));
      if (invert)
      {
	_data[i] = 1.0 - _data[i];
      }
    }
  }
}

//-----------------------------------------------------------------
void RayxData::qscale1(double scale, bool invert)
{
  for (int i=0; i<_npt; ++i)
  {
    if (_data[i] != _missing)
    {
      _data[i] = exp(scale*_data[i]);
      if (invert)
      {
	_data[i] = 1.0 - _data[i];
      }
    }
  }
}

//-----------------------------------------------------------------
void RayxData::invert(void)
{
  for (int i=0; i<_npt; ++i)
  {
    if (_data[i] != _missing)
    {
      _data[i] = 1.0/_data[i];
    }
  }
}


//-----------------------------------------------------------------
void RayxData::squareRoot(void)
{
  for (int i=0; i<_npt; ++i)
  {
    if (_data[i] != _missing)
    {
      _data[i] = sqrt(_data[i]);
    }
  }
}


//-----------------------------------------------------------------
void RayxData::logBase10(void)
{
  for (int i=0; i<_npt; ++i)
  {
    if (_data[i] != _missing)
    {
      _data[i] = log10(_data[i]);
    }
  }
}


//-----------------------------------------------------------------
void RayxData::pPassthrough(const RayxData &inp, const int i, const bool missing_ok)
{
  if (!missing_ok)
  {
    _data[i] = _missing;
    return;
  }

  if (inp._data[i] != inp._missing && _data[i] == _missing)
  {
    _data[i] = inp._data[i];
  }
  else if (inp._data[i] == inp._missing && _data[i] != _missing)
  {
    // don't change value
  }
  else if (inp._data[i] == inp._missing && _data[i] == _missing)
  {
    // don't change value
  }
  else
  {
    cerr << "ERROR - RayxData::pPassthrough" << endl;
    cerr << "  Both values non-missing not expected" << endl;
  }
}

