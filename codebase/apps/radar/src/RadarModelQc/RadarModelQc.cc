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
 * @file RadarModelQc.cc
 */
#include "RadarModelQc.hh"
#include "Params.hh"
#include "FiltCreate.hh"
#include "Filter.hh"
#include "Info.hh"
#include <RadxFiltAlg/ParmApp.hh>
#include <toolsa/TaThreadSimple.hh>
#include <algorithm>

//------------------------------------------------------------------------
TaThread *RadarModelQc::RadarThreads::clone(int index)
{
  TaThreadSimple *t = new TaThreadSimple(index);
  t->setThreadContext(this);
  t->setThreadMethod(RadarModelQc::compute);
  return dynamic_cast<TaThread *>(t);
}

//------------------------------------------------------------------------
// Constructor

RadarModelQc::RadarModelQc(int argc, char **argv, void cleanup(int),
			   void outOfStore(void))
{
  OK = TRUE;
  OK = parmAppInit(_params, _alg, argc, argv);

  vector<string> input;
  vector<string> nonInput;

  // look at all input/output field names, and build a list of pure
  // inputs (the non nonInputs)
  for (int i=0; i<_params.filter_n; ++i)
  {
    string name = _params._filter[i].output_field;
    if (find(nonInput.begin(), nonInput.end(), name) == nonInput.end())
    {
      nonInput.push_back(name);
    }
  }

  for (int i=0; i<_params.filter_n; ++i)
  {
    string name = _params._filter[i].input_field;
    if (find(nonInput.begin(), nonInput.end(), name) == nonInput.end())
    {
      if (find(input.begin(), input.end(), name) == input.end())
      {
	input.push_back(name);
      }
    }
  }
    
  if (!_alg.init(cleanup, outOfStore, input))
  {
    OK = false;
  }
  else
  {
    for (int i=0; i<_params.filter_n; ++i)
    {
      if (!_create_filter(_params._filter[i]))
      {
	OK = false;
      }
    }
  }
    
  // initialize threading 
  pThread.init(_alg.parmRef().num_threads, _alg.parmRef().thread_debug);
  return;
}

// destructor

RadarModelQc::~RadarModelQc()
{
  _alg.finish();
}

//------------------------------------------------------------------------
//////////////////////////////////////////////////
// Run

int RadarModelQc::Run()
{
  RadxVol vol;
  time_t t;
  bool last;
  while (_alg.trigger(vol, t, last))
  {
    if (_process(t, vol))
    {
      pThread.waitForThreads();
      printf("Writing\n");
      _alg.write(vol, t);
    }
  }
  return 0;
}

//------------------------------------------------------------------------
void RadarModelQc::compute(void *ti)
{
  Info *info = static_cast<Info *>(ti);
  RadarModelQc *alg = info->_alg;

  vector<RayData> data;

  bool stat = true;

  // start from front and do filters.

  for (int i=0; i<static_cast<int>(alg->_filters.size()); ++i)
  {
    if (!alg->_filters[i]->filter(info->_time, *info->_ray, data))
    {
      alg->pThread.lockForIO();
      alg->pBeamStatus = false;
      alg->pThread.unlockAfterIO();
    }
  }
  if (stat)
  {
    RadxFiltAlg::updateRay(data, *info->_ray);
  }

  delete info;
}

//------------------------------------------------------------------
bool RadarModelQc::_process(const time_t t, RadxVol &vol)
{
  // break the vol into rays and process each one
  const vector<RadxRay *> &rays = vol.getRays();

  for (int i=0; i<static_cast<int>(_filters.size()); ++i)
  {
    _filters[i]->printInputOutput();
  }

  pBeamStatus = true;
  for (size_t ii = 0; ii < rays.size(); ii++)
  {
    // good that this is an array of pointers, for threading 
    RadxRay *ray = rays[ii];
    _processRay(t, ray);
  }
  pThread.waitForThreads();
  return pBeamStatus;
}

//------------------------------------------------------------------
bool RadarModelQc::_create_filter(const Params::data_filter_t &p)
{
  Filter *f = modelQc::filtCreate(p, _params);
  if (f == NULL)
  {
    return false;
  }
  if (!f->ok())
  {
    delete f;
    return false;
  }

  _filters.push_back(f);
  return true;
}

//------------------------------------------------------------------
void RadarModelQc::_processRay(const time_t &t, RadxRay *ray)
{
  Info *info = new Info(t, ray, this);
  int index = 0;
  pThread.thread(index, info);
}


