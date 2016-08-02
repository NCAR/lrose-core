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
 * @file MdvFuzzyCombine.cc
 */

#include "MdvFuzzyCombine.hh"
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/GenPolyGrid.hh>
#include <euclid/GridAlgs.hh>
#include <dsdata/DsUrlTrigger.hh>
#include <toolsa/LogMsg.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <cstdio>

//----------------------------------------------------------------
MdvFuzzyCombine::MdvFuzzyCombine(const ParmsMdvFuzzyCombine &parms,
				 void tidyAndExit(int)) :  
  _parms(parms), _hasMask(false), _first(true),
  _distFill(parms._distFillMax, parms._distFillScale),
  _eroder(parms._erodeMax, parms._distFillScale)
{
}

//----------------------------------------------------------------
MdvFuzzyCombine::~MdvFuzzyCombine()
{
}

//----------------------------------------------------------------
void MdvFuzzyCombine::run(void)
{
  DsUrlTrigger *trigger=NULL;
  DsUrlTrigger::Trigger_t mode;
  if (_parms._isForecastData)
  {
    mode = DsUrlTrigger::FCST_LEAD;
  }
  else
  {
    mode = DsUrlTrigger::OBS;
  }

  if (_parms._isArchiveMode)
  {
    trigger = new DsUrlTrigger(_parms._archiveTime0, _parms._archiveTime1,
			       _parms._inputUrl, mode);
  }			   
  else
  {
    trigger = new DsUrlTrigger(_parms._inputUrl, mode);
  }
  if (_parms._isForecastData)
  {
    time_t gt;
    int lt;
    while (trigger->nextTime(gt, lt))
    {
      _process(gt, lt);
    }
  }
  else
  {
    time_t t;
    while (trigger->nextTime(t))
    {
      _process(t);
    }
  }

  delete trigger;
}

//----------------------------------------------------------------
void MdvFuzzyCombine::_process(const time_t &gt, const int lt)
{
  LOGF(LogMsg::DEBUG, "---Triggered %s+%d---", DateTime::strn(gt).c_str(), lt);
  _grid.clear();
  _hasMask = false;

  DsMdvx D;
  D.setReadTime(Mdvx::READ_SPECIFIED_FORECAST, _parms._inputUrl, 0, gt, lt);

  if (!_read(D))
  {
    LOGF(LogMsg::ERROR, "reading mdv volume from %s at %s+%d",
	 _parms._inputUrl.c_str(), DateTime::strn(gt).c_str(), lt);
    return;
  }

  if (!_parms._inputMaskUrl.empty() && !_parms._staticMask)
  {
    DsMdvx M;
    if (_parms._isForecastMaskData)
    {
      M.setReadTime(Mdvx::READ_SPECIFIED_FORECAST, _parms._inputMaskUrl,
		    0, gt, lt);
    }
    else
    {
      M.setReadTime(Mdvx::READ_FIRST_BEFORE, _parms._inputMaskUrl, 0, gt+lt);
    }
    _hasMask = _readMask(M, D);
    if (!_hasMask)
    {
      LOG(LogMsg::WARNING, "No mask data at trigger time");
    }
  }
  else if ((!_parms._inputMaskUrl.empty()) && _parms._staticMask && _first)
  {
    _first = false;
    _hasMask = _loadStaticMask(gt, D);
    if (!_hasMask)
    {
      LOG(LogMsg::WARNING, "No mask data");
    }
  }

  Grid2d g;
  _combine(g);
  _write(D, g, true);
}

//----------------------------------------------------------------
void MdvFuzzyCombine::_process(const time_t &t)
{
  LOGF(LogMsg::DEBUG, "---Triggered %s---", DateTime::strn(t).c_str());
  _grid.clear();
  _hasMask = false;

  DsMdvx D;
  D.setReadTime(Mdvx::READ_FIRST_BEFORE, _parms._inputUrl, 0, t);

  if (!_read(D))
  {
    LOGF(LogMsg::ERROR, "reading mdv volume from %s at %s",
	 _parms._inputUrl.c_str(), DateTime::strn(t).c_str());
    return;
  }

  if (!_parms._inputMaskUrl.empty() && !_parms._staticMask)
  {
    DsMdvx M;
    M.setReadTime(Mdvx::READ_FIRST_BEFORE, _parms._inputMaskUrl, 0, t);
    _hasMask = _readMask(M, D);
    if (!_hasMask)
    {
      LOG(LogMsg::WARNING, "No mask data at trigger time");
    }
  }
  else if ((!_parms._inputMaskUrl.empty()) && _parms._staticMask && _first)
  {
    _first = false;
    _hasMask = _loadStaticMask(t, D);
    if (!_hasMask)
    {
      LOG(LogMsg::WARNING, "No mask data");
    }
  }

  Grid2d g;
  _combine(g);
  _write(D, g, false);
}

//----------------------------------------------------------------
bool MdvFuzzyCombine::_loadStaticMask(const time_t &t, DsMdvx &D)
{
  DsMdvx M;
  // look back 5 years from today
  time_t t0 = t - 5*365*24*3600;
  if (_parms._isForecastMaskData)
  {
    M.setTimeListModeGen(_parms._inputMaskUrl, t0, t);
    M.compileTimeList();
    vector<time_t> gt = M.getTimeList();
    if (gt.empty())
    {
      return false;
    }
    else
    {
      int ngt = static_cast<int>(gt.size());
      if (ngt > 1)
      {
	LOGF(LogMsg::WARNING, "%d static mask gen times, expect one",  ngt);
      }
      time_t gent = gt[ngt-1];
      M.setTimeListModeForecast(_parms._inputMaskUrl, gent);
      M.compileTimeList();
      vector<time_t> vt = M.getValidTimes();
      if (vt.empty())
      {
	LOGF(LogMsg::ERROR, "Mask data for gentime=%s, but no lead times",
	     DateTime::strn(gent).c_str());
	return false;
      }
      else
      {
	M.setReadTime(Mdvx::READ_SPECIFIED_FORECAST, _parms._inputMaskUrl,
		      0, gent, vt[0] - gent);
      }
    }
  }
  else
  {
    M.setTimeListModeValid(_parms._inputMaskUrl, t0, t);
    M.compileTimeList();
    vector<time_t> gt = M.getTimeList();
    if (gt.empty())
    {
      return false;
    }
    else
    {
      int ngt = static_cast<int>(gt.size());
      if (ngt > 1)
      {
	LOGF(LogMsg::WARNING, "%d static mask times, expect one", ngt);
      }
      time_t gent = gt[ngt-1];
      M.setReadTime(Mdvx::READ_FIRST_BEFORE, _parms._inputMaskUrl, 0, gent);
    }
  }
  return _readMask(M, D);
}

//----------------------------------------------------------------
bool MdvFuzzyCombine::_read(DsMdvx &D)
{
  _grid.clear();
  D.clearReadFields();
  for (size_t i=0; i<_parms._fields.size(); ++i)
  {
    D.addReadField(_parms._fields[i]);
  }
  if (D.readVolume())
  {
    return false;
  }

  for (size_t i=0; i<_parms._fields.size(); ++i)
  {
    Grid2d gr;
    if (GenPolyGrid::mdvToGrid2d(D, _parms._fields[i], gr))
    {
      _grid[_parms._fields[i]] = gr;
    }
    else
    {
      return false;
    }
  }
  return true;
}

//----------------------------------------------------------------
bool MdvFuzzyCombine::_readMask(DsMdvx &M, DsMdvx &D)
{
  M.addReadField(_parms._inputMaskFieldName);
  MdvxProj proj(D);
  M.setReadRemap(proj);
  if (M.readVolume())
  {
    return false;
  }

  if (!GenPolyGrid::mdvToGrid2d(M, _parms._inputMaskFieldName, _mask))
  {
    return false;
  }
  
  // take mask and invert it so missing is non-missing and vice versa
  GridAlgs a = GridAlgs::promote(_mask);
  a.invert(100);

  // now expand into missing in the inverted mask
  Grid2d imask;
  _eroder.replaceMissing(a, imask);

  
  // now set the _mask data to missing whereever the imask data is not missing
  GridAlgs a2 = GridAlgs::promote(_mask);
  a2.setMaskToMissing(imask);
  _erodedMask = a2;
  return true;
}

//----------------------------------------------------------------
void MdvFuzzyCombine::_combine(Grid2d &g)
{
  // pull out a grid as a template

  g = _grid[_parms._fields[0]];
  g.setName(_parms._outputFieldName);
  g.setAllMissing();

  int nx = g.getNx();
  int ny = g.getNy();

  for (int i=0; i<nx*ny; ++i)
  {
    double w = 0.0;
    double wsum = 0.0;
    bool first = true;
    for (size_t j=0; j<_parms._input.size(); ++j)
    {
      double v;
      if (_grid[_parms._input[j]._name].getValue(i, v))
      {
	first = false;
	w += _parms._input[j]._mapping.apply(v)*_parms._input[j]._weight;
	wsum += _parms._input[j]._weight;
      }
    }
    if (!first)
    {
      if (_hasMask)
      {
	if (!_erodedMask.isMissing(i))
	{
	  g.setValue(i, w);
	}
      }
      else
      {
	g.setValue(i, w);
      }
    }
  }

  if (_hasMask && _parms._distFillMax > 0)
  {
    // smooth the data
    GridAlgs g2 = GridAlgs::promote(g);
    _unSmoothed = g;
    g2.smooth(_parms._smoothNpt, _parms._smoothNpt);

    // mask out smoothing that smoothed back into eroded area
    g2.maskMissingToMissing(_unSmoothed);
    _smoothed = g2;

    GridAlgs g3;
    _distFill.replaceMissing(g2, g3);

    // go back in and re-set places where data was not missing in eroded
    // grid (which is now g) to original non-smoothed values
    g3.fillInMask(g);

    _nonTapered = g3;

    // now taper based on distances to data
    int nx = _nonTapered.getNx();
    int ny = _nonTapered.getNy();
    
    Grid2d dist("dist", nx, ny, -1);
    Grid2d val("val", nx, ny, -1);
    _distFill.distanceToNonMissing(_smoothed, dist, val);
    
    for (int i=0; i<dist.getNdata(); ++i)
    {
      double d;
      if (dist.getValue(i, d))
      {
	d = _parms._taper.apply(d);
	double v;
	if (_nonTapered.getValue(i, v))
	{
	  g.setValue(i, v*d);
	}
	else
	{
	  g.setMissing(i);
	}
      }
      else
      {
	g.setMissing(i);
      }
    }
  }
}

//----------------------------------------------------------------
void MdvFuzzyCombine::_write(DsMdvx &D, const Grid2d &g, const bool isFcst)
{
  vector<pair<string,string> > nu;
  vector<Grid2d> vg;
  nu.push_back(pair<string,string>(_parms._outputFieldName,
				   _parms._outputFieldUnits));
  vg.push_back(g);

  _mask.setName("mask");
  nu.push_back(pair<string,string>("mask", "none"));
  vg.push_back(_mask);

  _erodedMask.setName("erodedMask");
  nu.push_back(pair<string,string>("erodedMask", "none"));
  vg.push_back(_erodedMask);

  _smoothed.setName("smoothed");
  nu.push_back(pair<string,string>("smoothed", "none"));
  vg.push_back(_smoothed);

  _unSmoothed.setName("unSmoothed");
  nu.push_back(pair<string,string>("unSmoothed", "none"));
  vg.push_back(_unSmoothed);

  _nonTapered.setName("unTapered");
  nu.push_back(pair<string,string>("unTapered", "none"));
  vg.push_back(_nonTapered);

  if (GenPolyGrid::grid2dToDsMdvx(D, vg, nu))
  {
    D.setWriteLdataInfo();
    if (isFcst)
    {
      D.setWriteAsForecast();
    }
    if (D.writeToDir(_parms._outputUrl.c_str()))
    {
      LOG(LogMsg::ERROR, "Unable to write mdv");
    }
  }
}
