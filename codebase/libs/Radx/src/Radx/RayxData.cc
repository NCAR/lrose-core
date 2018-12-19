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
      cerr << "  Constructor npt=" << _npt << ", RadxField npt=" << npoints 
	   << endl;
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
bool RayxData::transferData(const RayxData &r)
{
  if (r._npt != _npt)
  {
    cerr << "ERROR - RayxData::transferdata" << endl;
    cerr << "  Npt input " << r._npt << " versus local " << _npt << endl;
    return false;
  }
  _missing = r._missing;
  for (int i=0; i<_npt; ++i)
  {
    _data[i] = r._data[i];
  }
  return true;
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
bool RayxData::retrieveSubsetData(Radx::fl32 *data, const int npt) const
{
  if (npt < _npt)
  {
    cerr << "ERROR - RayxData::retrieveSubsetData" << endl;
    cerr << "  Npt input " << npt << " versus local " << _npt << endl;
    return false;
  }
  for (int i=0; i<_npt; ++i)
  {
    data[i] = static_cast<Radx::fl32>(_data[i]);
  }
  for (int i=_npt; i<npt; ++i)
  {
    data[i] = static_cast<Radx::fl32>(_missing);
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
      _passthrough(inp, i, missing_ok);
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
      _passthrough(inp, i, missing_ok);
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
      _passthrough(inp, i, missing_ok);
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
void RayxData::FIRfilter(const std::vector<double> coeff, FirFilter_t type,
			 RayxData &quality)
{
  // create a quality ray with value=0 everywhere
  for (int i=0; i<quality._npt; ++i)
  {
    quality._data[i] = 0.0;
  }

  // find index of first and last valid data
  int i0 = _firstValidIndex();
  int i1 = _lastValidIndex();
  if (i0 < 0 || i1 < 0)
  {
    cerr << "FIRfilter  All the data is missing, no filtering" << endl;
    return;
  }

  int nCoeff = static_cast<int>(coeff.size());
  if (i1-i0+1 < nCoeff*2)
  {
    cerr << "FIRfilter data mostly missing only " << i1-i0+1
	 << " good values" << endl;
    return;
  }
  if (_debug)
  {
    cerr << "FIRfilter  I0,I1=" << i0 << "," << i1 << endl;
  }

  // get the center index value
  int centerCoeff = nCoeff/2;
  if (nCoeff % 2)
  {
    // odd # of coeffs...good
  }
  else
  {
    cerr << "WARNING FIRfilter even number of coeff, use n/2'th as center"
	 << endl;
  }

  // if interpolating at edges, do a linear regression to get coefficients
  double m0=0, int0=0, m1=0, int1=0;
  bool allbad0=true, allbad1=true;
  if (type == FIR_EDGE_INTERP)
  {
    allbad0 = !_linearRegression(i0, i1, 20, true, m0, int0);
    allbad1 = !_linearRegression(i0, i1, 20, false, m1, int1);
  }

  // create a vector that extends at each end
  vector<double> tmpData = _extendData(i0, i1, centerCoeff, nCoeff,
				       type, allbad0,
				       m0, int0, allbad1, m1, int1);

  // do gap filling on this data
  vector<double> gapFilledData = tmpData;
  _fillGaps(gapFilledData);

  // create a vector to store data with gaps filled, compute sum of coefficients
  double sumCoeff = 0.0;
  for (int i=0; i<nCoeff; ++i)
  {
    sumCoeff += coeff[i];
  }


  for (int j=0; j<_npt; ++j)
  {
    quality._data[j] = _applyFIR(j, i0, i1, centerCoeff, type, tmpData,
				 gapFilledData, coeff, sumCoeff);
  }
}

//-----------------------------------------------------------------
void RayxData::constrain(int minGateIndex, int maxGateIndex)
{
  for (int i=0; i<minGateIndex; ++i)
  {
    _data[i] = _missing;
  }
  for (int i=maxGateIndex+1; i<_npt; ++i)
  {
    _data[i] = _missing;
  }
}

//-----------------------------------------------------------------
void RayxData::variance(double npt, double maxPctMissing)
{
  vector<double> tmp = _data;
  int half = (int)(npt/2.0);
  int maxMissing = (int)(npt*maxPctMissing);

  // set tmp to mean values
  for (int i=0; i<_npt; ++i)
  {
    int i0 = i-half;
    int i1 = i+half;
    double mean = 0.0;
    int nmissing = 0;
    double ngood = 0;
    bool isSet = false;
    for (int j=i0; j<=i1; ++j)
    {
      if (j >= 0 && j < _npt)
      {
	if (_data[j] == _missing)
	{
	  ++nmissing;
	}
	else
	{
	  mean += _data[j];
	  ++ngood;
	}
      }
      else
      {
	++nmissing;
      }
      if (nmissing > maxMissing)
      {
	tmp[i] = _missing;
	isSet = true;
	break;
      }
    }
    if (!isSet)
    {
      tmp[i] = mean/ngood;
    }
  }

  // set tmp2 to data, then use tmp and tmp2 to write back variances
  vector<double> tmp2 = _data;
  for (int i=0; i<_npt; ++i)
  {
    if (tmp[i] == _missing)
    {
      _data[i] = _missing;
    }
    else
    {
      int i0 = i-half;
      int i1 = i+half;
      double var = 0.0;
      int nmissing = 0;
      double ngood = 0;
      bool isSet = false;
      for (int j=i0; j<=i1; ++j)
      {
	if (j >= 0 && j < _npt)
	{
	  if (tmp2[j] == _missing)
	  {
	    ++nmissing;
	  }
	  else
	  {
	    var += pow(tmp2[j] - tmp[i], 2);
	    ++ngood;
	  }
	}
	else
	{
	  ++nmissing;
	}
	if (nmissing > maxMissing)
	{
	  isSet = true;
	  _data[i] = _missing;
	  break;
	}
      }
      if (!isSet)
      {
	_data[i] = var/ngood;
      }
    }
  }
}

//-----------------------------------------------------------------
double RayxData::_applyFIR(int j, int i0, int i1, int centerCoeff, 
			   FirFilter_t type,
			   const std::vector<double> &tmpData,
			   const std::vector<double> &gapFilledData,
			   const std::vector<double> &coeff, double sumCoeff)
{
  int tIndex = j+centerCoeff-i0;
  if (tIndex < 0 || tIndex >= (int)tmpData.size())
  {
    _data[j] = _missing;
    return 0.0;
  }
  
  if (j < i0 || j > i1)
  {
    _data[j] = _missing;
    //_data[j] = tmpData[tIndex];
    return 0.0;
  }
  if (_data[j] == _missing)
  {
    return 0.0;
  }
  if (_debug)
  {
    printf("Interpolating data centered at %d\n", j);
  }

  double quality  = _FIRquality(centerCoeff, tmpData, gapFilledData, tIndex);
  if (quality > 0)
  {
    _data[j] = _sumProduct(coeff, sumCoeff, gapFilledData,
			   tIndex-centerCoeff);
  }
  else
  {
    _data[j] = _missing;
  }
  return quality;
}

//-----------------------------------------------------------------
void RayxData::_passthrough(const RayxData &inp, const int i,
			    const bool missing_ok)
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

//-----------------------------------------------------------------
double RayxData::_FIRquality(int centerCoeff, const vector<double> &tmpData,
			     const vector<double> &gapFilledData,
			     int tIndex)
{
  int n = 2*centerCoeff + 1;  // the FIR filter window size

  vector<double> qmeasure;
  qmeasure.reserve(n);
  qmeasure[0] = 1.0;
  qmeasure[1] = 0.95;
  qmeasure[2] = 0.90;
  qmeasure[3] = 0.85;
  qmeasure[4] = 0.80;
  qmeasure[5] = 0.75;
  for (int i=6; i<n/2; ++i)
  {
    qmeasure[i] = 0.5;
  }
  for (int i=n/2; i<n; ++i)
  {
    qmeasure[i] = 0.0;
  }

  int nbad = 0;
  for (int i=-centerCoeff; i<=centerCoeff; ++i)
  {
    int ind = tIndex + i;
    if (tmpData[ind] == _missing)
    {
      ++nbad;
    }
    if (gapFilledData[ind] == _missing)
    {
      // don't allow any missing data in gap filled data
      return 0.0;
    }
  }
  return qmeasure[nbad];
}

//-----------------------------------------------------------------
void RayxData::_fillGaps(std::vector<double> &data) const
{
  int n = static_cast<int>(data.size());

  bool inside = true;
  int o0 = -1, o1 = -1;
  for (int i=0; i<n; ++i)
  {
    if (data[i] == _missing)
    {
      if (inside)
      {
	inside = false;
	o0 = o1 = i;
	if (_debug)
	{
	  printf("First point missing index=%d\n", i);
	}
      }
      else
      {
	o1 = i;
      }
    }
    else
    {
      if (!inside)
      {
	// went from outside to inside, now can interp
	inside = true;
	int interp0 = o0 - 1;
	int interp1 = i;
	if (_debug)
	{
	  printf("First point leaving missing index=%d, i0=%d, i1=%d\n", i,
		 interp0, interp1);
	}
	if (interp0 >= 0)
	{
	  _interp(data[interp0], data[interp1], interp0, interp1, data);
	}
      }
    }
  }
}

//-----------------------------------------------------------------
void RayxData::_interp(double d0, double d1, int i0, int i1,
		       std::vector<double> &iData) const
{
  // i0 is index of first data, i1 of second data
  // so i0+1, i0+2,... i1-1 are the places to interpolate to

  int nx = i1-i0;
  for (int i=1; i<nx; ++i)
  {
    int index = i0 + i;
    double pct = (double)i/(double)nx;
    double v = (1.0-pct)*d0 + pct*d1;
    iData[index] = v;
    if (_debug)
    {
      printf("interp data[%d] = %lf\n", index, v);
    }
  }
}

//-----------------------------------------------------------------
std::vector<double> RayxData::_extendData(int i0, int i1, int centerCoeff, 
					  int nCoeff, FirFilter_t type,
					  bool allbad0,
					  double m0, double int0, bool allbad1,
					  double m1, double int1) const
{
  // copy the data, but expand at each end using one of the 4 algs
  vector<double> tmpData;
  int nTmp = i1-i0+1 + centerCoeff*2;

  tmpData.reserve(nTmp);

  for (int j=0; j<nTmp; ++j)
  {
    int eIndex = -centerCoeff+j;  // extension index
    int dIndex = eIndex + i0;     // data index,  crosses over to >=0 at i0
    if (j < centerCoeff)
    {
      tmpData.push_back(_extend(eIndex, eIndex, i0, i0+4,
				type, m0, int0, allbad0));
    }
    else if (dIndex <= i1)
    {
      tmpData.push_back(_data[dIndex]);
    }
    else
    {
      int eIndexUp = dIndex - i1 - 2;
      int eIndP = dIndex - i1;
      tmpData.push_back(_extend(eIndexUp, eIndP, i1, (i1-4),
				type, m1, int1, allbad1));
    }
  }
  return tmpData;
}
    
//-----------------------------------------------------------------
double RayxData::_extend(int mirrorIndex, int interpIndex,
			 int boundaryDataIndex, int otherAveIndex,
			 FirFilter_t type, double m,
			 double intercept, bool allbad) const
 {
   double d = _missing;
   switch (type)
   {
   case FIR_EDGE_CLOSEST:
     d = _data[boundaryDataIndex];
     break;
   case FIR_EDGE_MIRROR:
     d = _data[boundaryDataIndex - mirrorIndex - 1];
     break;
   case FIR_EDGE_INTERP:
     if (!allbad)
     {
       d = m*static_cast<double>(interpIndex) + intercept;
     }
     break;
   case FIR_EDGE_MEAN:
     if (boundaryDataIndex < otherAveIndex)
     {
       d = _mean(boundaryDataIndex, otherAveIndex);
     }
     else
     {
       d = _mean(otherAveIndex, boundaryDataIndex);
     }
     break;
   default:
     cerr << "ERROR no computation of extension";
     break;
   }
   return d;
 }

//-----------------------------------------------------------------
double RayxData::_sumProduct(const std::vector<double> &coeff, double sumCoeff,
			     const std::vector<double> &data, int i0) const
{
  double sumprod = 0.0;
  for (size_t i=0; i<coeff.size(); ++i)
  {
    if (data[i+i0] == _missing)
    {
      return _missing;
    }
    else
    {
      sumprod += coeff[i]*data[i+i0];
    }
  }
  return sumprod/sumCoeff;
}
  
//-----------------------------------------------------------------
int RayxData::_firstValidIndex(void) const
{
  for (int i=0; i<_npt; ++i)
  {
    if (_data[i] != _missing)
    {
      return i;
    }
  }
  return -1;
}


//-----------------------------------------------------------------
int RayxData::_lastValidIndex(void) const
{
  for (int i=_npt-1; i>=0; --i)
  {
    if (_data[i] != _missing)
    {
      return i;
    }
  }
  return -1;
}


//-----------------------------------------------------------------
bool RayxData::_linearRegression(int i0, int i1, int npt, bool up,
				 double &slope, double &intercept) const
{
  // build up nptLinearInterp at each end of the data, skipping missing
  // data
  double N = 0;
  double sumxy=0, sumx=0, sumy=0,sumx2=0;
  int dataOffset=0;

  int for0, for1, delta;
  if (up)
  {
    for0 = 0;
    for1 = i1-i0+1;
    delta = 1;
    dataOffset = i0;
  }
  else
  {
    for0 = 0;
    for1 = -(i1-i0+1);
    delta = -1;
    dataOffset = i1;
  }

  for (int i=for0; i!=for1; i += delta)
  {
    if (_data[i+dataOffset] != _missing)
    {
      N++;
      double x = static_cast<double>(i);
      sumx += x;
      sumy += _data[i+dataOffset];
      sumxy += x*_data[i+dataOffset];
      sumx2 += x*x;
      if (N >= npt)
      {
	break;
      }
    }
  }
  if (N == npt)
  {
    slope = (N*sumxy - sumx*sumy)/(N*sumx2 - sumx*sumx);
    intercept = (sumy - slope*sumx)/N;
    return true;
  }
  else
  {
    return false;
  }
}

//-----------------------------------------------------------------
double RayxData::_mean(int i0, int i1) const
{
  double N = 0.0, S = 0.0;
  for (int i=i0; i<=i1; ++i)
  {
    if (_data[i] != _missing)
    {
      S += _data[i];
      N ++;
    }
  }
  if (N > 0)
  {
    return S/N;
  }
  else
  {
    return _missing;
  }
}
