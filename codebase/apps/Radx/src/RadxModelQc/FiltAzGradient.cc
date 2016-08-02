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
 * @file FiltAzGradient.cc
 */
#include "FiltAzGradient.hh"
#include <Radx/RadxSweep.hh>
#include <Radx/RadxVol.hh>
#include <toolsa/LogStream.hh>
#include <algorithm>

//------------------------------------------------------------------
FiltAzGradient::FiltAzGradient(const Params::data_filter_t f,
			       const Params &P) : Filter(f, P)
{
  if (!ok())
  {
    return;
  }
  _type = P._parm_math[f.filter_index].type;
  _value = P._parm_math[f.filter_index].value;
  _stateSet = false;
}

//------------------------------------------------------------------
FiltAzGradient::~FiltAzGradient()
{
}

//------------------------------------------------------------------
void FiltAzGradient::filter_print(void) const
{
  LOG(DEBUG_VERBOSE) << "filtering";
}

//------------------------------------------------------------------
bool FiltAzGradient::canThread(void) const
{
  return false;
}

//------------------------------------------------------------------
bool FiltAzGradient::filter(const time_t &t, const RadxRay *ray0,
			    const RadxRay &ray, const RadxRay *ray1,
			    std::vector<RayxData> &data) const
{
  if (!_stateSet)
  {
    LOG(ERROR) << "State not set";
    return false;
  }
  int nState = static_cast<int>(_state.size());
  if (nState < 1)
  {
    LOG(ERROR) << "State is empty";
    return false;
  }

  RayxData r, r0, r1;
  if (!RadxApp::retrieveRay(_f.input_field, ray, data, r))
  {
    return false;
  }

  const RadxRay *lray0 = NULL;
  const RadxRay *lray1 = NULL;


  // see if can proceed or not
  bool ok = _set2Rays(ray0, ray1, ray, &lray0, &lray1);

  if (ok)
  {
    if (!RadxApp::retrieveRay(_f.input_field, *lray0, data, r0))
    {
      return false;
    }
    if (!RadxApp::retrieveRay(_f.input_field, *lray1, data, r1))
    {
      return false;
    }
    int n = r.getNpoints();
    int n0 = r0.getNpoints();
    int n1 = r1.getNpoints();
    int nend = n;
    double a0 = r0.getAzimuth();
    double a1 = r1.getAzimuth();
    // double a = r.getAzimuth();
    if (n0 < nend) nend = n0;
    if (n1 < nend) nend = n1;

    switch (_type)
    {
    case Params::ADD:
      r.inc(_value);
      r0.inc(_value);
      r1.inc(_value);
      break;
    case Params::SUBTRACT:
      r.dec(_value);
      r0.dec(_value);
      r1.dec(_value);
      break;
    case Params::MULT:
      r.multiply(_value);
      r0.multiply(_value);
      r1.multiply(_value);
      break;
    case Params::NOOP:
    default:
      break;
    }

    for (int i=0; i<nend; ++i)
    {
      double v0, v1;
      if (r0.getV(i, v0) && r1.getV(i, v1))
      {
	r.setV(i, (v1 - v0)/(a1-a0)/2.0);
      }
      else
      {
	r.setV(i, r.getMissing());
      }
    }
    for (int i=nend; i<n; ++i)
    {
      r.setV(i, r.getMissing());
    }
  }
  else
  {
    r.setAllToValue(r.getMissing());
  }

  RadxApp::modifyRayForOutput(r, _f.output_field, _f.output_units,
                              _f.output_missing);
  data.push_back(r);
  return true;
}


//------------------------------------------------------------------
void FiltAzGradient::filterVolume(const RadxVol &vol)
{
  _state.clear();

  // get elev angle for each sweep
  const vector<RadxSweep *> s = vol.getSweeps();
  for (size_t i=0; i<s.size(); ++i)
  {
    _setState(*s[i], vol);
  }
  _stateSet = true;
}

//------------------------------------------------------------------
void FiltAzGradient::finish(void)
{
  _stateSet = false;
  _state.clear();
}

//------------------------------------------------------------------
void FiltAzGradient::_setState(const RadxSweep &s, const RadxVol &vol)
{
  double fixedAngle = s.getFixedAngleDeg();
  int sweepNumber = s.getSweepNumber();
  int r0 = s.getStartRayIndex();
  int r1 = s.getEndRayIndex();
  int n = s.getNRays();
  
  double angle0, angle1, da;
    
  // figure out delta azimuth
  da = _deltaAngleDegrees(s, vol, angle0, angle1);

  // check for full 360
  bool is360 = static_cast<int>(da*static_cast<double>(n)) == 360;
  if (is360)
  {
    const RadxRay *ray0=NULL, *ray1=NULL;

    // expect the last and first to be one da apart
    double daCross = fabs(angle1 - angle0);
    while (daCross >= 360)  daCross -=360;
    if (daCross != da)
    {
      LOG(WARNING) << "Unexpected da at crossover " << daCross << " expect " << da;
    }
    // here is where we store pointers to the 0th and last azimuths in the sweep
    const vector<RadxRay *> rays = vol.getRays();
    ray0 = rays[r0];
    ray1 = rays[r1];
    _state.push_back(AzGradientState(fixedAngle, sweepNumber, r0, r1, ray0, ray1));
    LOG(DEBUG_VERBOSE) << "sweep " << sweepNumber << " 360, fixedAngle:" 
		       << fixedAngle << "rayInd:[" << r0 << "," << r1 
		       << "], numRay=" << s.getNRays();
  }
  else
  {
    _state.push_back(AzGradientState(fixedAngle, sweepNumber));
    LOG(DEBUG_VERBOSE) << "sweep " << sweepNumber << " sector, fixedAngle:" 
		       << fixedAngle << "rayInd:[" << r0 << "," 
		       << r1 << "], numRay=" << s.getNRays();
  }
}

//------------------------------------------------------------------
double FiltAzGradient::_deltaAngleDegrees(const RadxSweep &s,
					  const RadxVol &vol,
					  double &angle0,
					  double &angle1) const
{
  const vector<RadxRay *> rays = vol.getRays();

  vector<double> angles;
  for (int i = s.getStartRayIndex(); 
       i <= static_cast<int>(s.getEndRayIndex()); ++i)
  {
    double a = rays[i]->getAzimuthDeg();
    angles.push_back(a);
  }

  vector<double> sangles(angles);
  sort(sangles.begin(), sangles.end());
  double da = 0;
  for (size_t ia=0; ia<sangles.size()-1; ++ia)
  {
    double dai = sangles[ia+1] - sangles[ia];
    if (ia == 0)
    {
      da = dai;
    }
    else
    {
      if (da != dai)
      {
	LOG(WARNING) << "Uneven angle spacing " << dai << " going with " << da;
      }
    }
  }

  angle0 = angles[0];
  angle1 = *(angles.rbegin());
  return da;
}    

//------------------------------------------------------------------
bool FiltAzGradient::_set2Rays(const RadxRay *ray0, const RadxRay *ray1,
			       const RadxRay &ray, const RadxRay **lray0,
			       const RadxRay **lray1) const
{
  if (ray0 == NULL && ray1 == NULL)
  {
    // don't know what this is
    LOG(ERROR) << "Missing two of 3 inputs, not expected";
    return false;
  }
  else if (ray0 == NULL && ray1 != NULL)
  {
    return _veryFirstRay(ray, ray1, lray0, lray1);
  }
  else if (ray0 != NULL && ray1 == NULL)
  {
    return _veryLastRay(ray0, ray, lray0, lray1);
  }
  else 
  {
    *lray0 = ray0;
    *lray1 = ray1;
    // check for sweep boundary 
    int s0 = (*lray0)->getSweepNumber();
    int s1 = (*lray1)->getSweepNumber();
    int s = ray.getSweepNumber();
    if (s0 != s && s1 == s)
    {
      // current starts a new sweep
      return _setPreviousRayWhenNewSweep(s0, s, lray0, lray1);
    }
    else if (s0 == s && s1 != s)
    {
      // current ends a new sweep
      return _setNextRayWhenEndSweep(s, s1, lray0, lray1);
    }
    else
    {
      return true;
    }
  }
}

//------------------------------------------------------------------
bool FiltAzGradient::_setPreviousRayWhenNewSweep(int oldSweep, int sweep,
						 const RadxRay **lray0,
						 const RadxRay **lray1) const
{
  if (oldSweep >= sweep)
  {
    LOG(ERROR) << "Sweep indices did not increase";
    return false;
  }

  // crossed the boundary with s, if 360 can use last ray
  int nState = static_cast<int>(_state.size());
  for (int j=0; j<nState; ++j)
  {
    if (_state[j]._sweepNumber == sweep && _state[j]._is360)
    {
      *lray0 = _state[j]._ray1;
      LOG(DEBUG_VERBOSE) << "new sweep starts now " << sweep
			 << "full 360 so prev=last in sweep";
      return true;
    }
  }
  LOG(WARNING) << "new sweep starts now " <<  sweep
	       << " not full 360, leave gap here";
  return false;
}


//------------------------------------------------------------------
bool FiltAzGradient::_setNextRayWhenEndSweep(int sweep, int nextSweep,
					     const RadxRay **lray0,
					     const RadxRay **lray1) const
{
  if (sweep >= nextSweep)
  {
    LOG(ERROR) << "Sweep indices did not increase";
    return false;
  }

  // crossed boundary when look to next ray, of 360 can use 0th ray as next one
  int nState = static_cast<int>(_state.size());
  for (int j=0; j<nState; ++j)
  {
    if (_state[j]._sweepNumber == sweep && _state[j]._is360)
    {
      *lray1 = _state[j]._ray0;
      LOG(DEBUG_VERBOSE) << "last ray in sweep "<<  sweep
			 << " full 360, so next = 0th in sweep";
      return true;
    }
  }
  LOG(WARNING) << "last ray in sweep" <<  sweep
	       << " not full 360, leave gap";
  return false;
}


//------------------------------------------------------------------
bool FiltAzGradient::_veryFirstRay(const RadxRay &ray, const RadxRay *ray1,
				   const RadxRay **lray0,
				   const RadxRay **lray1) const
{
  // case of no previous ray, should be the 0th ray in the 0th sweep.  
  // if so, if 360, can use the last ray in the 0th sweep as ray0
  int s = ray.getSweepNumber();
  if (s == _state[0]._sweepNumber && _state[0]._is360)
  {
    LOG(DEBUG_VERBOSE) << 
      "First ray in vol, 360, set previous to last in sweep";
    *lray0 = _state[0]._ray1;
    *lray1 = ray1;
    return true;
  }
  else
  {
    LOG(DEBUG_VERBOSE) << "First ray and not 360, leave a gap at 0";
    return false;
  }
}

//------------------------------------------------------------------
bool FiltAzGradient::_veryLastRay(const RadxRay *ray0, const RadxRay &ray,
				  const RadxRay **lray0,
				  const RadxRay **lray1) const
{
  // case of no next ray, should be last ray in last sweep.  If so, if 360,
  // can use first ray in last weep as ray1
  int s = ray.getSweepNumber();
  int nState = static_cast<int>(_state.size());
  if (s == _state[nState-1]._sweepNumber && _state[nState-1]._is360)
  {
    LOG(DEBUG_VERBOSE) << "Last ray in vol, 360, set next to 0th in sweep";
    *lray0 = ray0;
    *lray1 = _state[nState-1]._ray0;
    return true;
  }
  else
  {
    LOG(DEBUG_VERBOSE) << "Last ray and not 360, leave a gap at last position";
    return false;
  }
}
