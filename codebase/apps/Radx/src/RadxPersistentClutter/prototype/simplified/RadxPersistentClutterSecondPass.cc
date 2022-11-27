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
 * @file RadxPersistentClutterSecondPass.cc
 */
#include "RadxPersistentClutterSecondPass.hh"
#include <Radx/RadxRay.hh>
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
RadxPersistentClutterSecondPass::
RadxPersistentClutterSecondPass(const RadxPersistentClutter &p) :
  RadxPersistentClutter(p)
{
  // For each base class store element, create a RayHistoInfo element
  for (std::map<RadxAzElev, RayClutterInfo>::iterator ii = _store.begin();
       ii!=_store.end(); ++ii)
  {
    RayHistoInfo h(ii->second, _params);
    _histo[ii->first] = h;
  }

  // rewind for reprocessing
  RadxPersistentClutter::_rewind();

  // redo the threading
  _thread.reinit(_params.num_threads, _params.thread_debug);
}

//------------------------------------------------------------------
RadxPersistentClutterSecondPass::~RadxPersistentClutterSecondPass(void)
{
}

//------------------------------------------------------------------
void RadxPersistentClutterSecondPass::initFirstTime(const time_t &t,
						    const RadxVol &vol)
{
  // Save this volume, will use its time information later
  _templateVol = vol;
}

//------------------------------------------------------------------
void RadxPersistentClutterSecondPass::finishLastTimeGood(const time_t &t,
							 RadxVol &vol)
{
  // The input volume and time are what is to be written out, do so now
  RadxPersistentClutter::_write(vol, t, _params.clutter_stats_output_dir);
  LOG(LogStream::DEBUG) << "Successful second pass";
}

//------------------------------------------------------------------
void RadxPersistentClutterSecondPass::finishBad(void)
{
  LOG(LogStream::ERROR) << "Never matched last time. No output";
}

//------------------------------------------------------------------
bool RadxPersistentClutterSecondPass::processFinishVolume(const time_t &t,
							  RadxVol &vol)
{
  if (t == _final_t)
  {
    // replace vol with the first template volume when time matches final time
    // from first pass
    vol = _templateVol;

    // prepare this volume for output
    _processForOutput(vol);
    return true;
  }
  else
  {
    return false;
  }
}

//------------------------------------------------------------------
bool RadxPersistentClutterSecondPass::processRay(const RayxData &r,
						 RayClutterInfo *h) const
{
  // cast input pointer to what we want (a little dangerous perhaps!)
  RayHistoInfo *histo = dynamic_cast<RayHistoInfo *>(h);

  // call the method that takes this data and updates state
  return histo->updateSecondPass(r);
}

//------------------------------------------------------------------
bool RadxPersistentClutterSecondPass::preProcessRay(const RadxRay &ray)
{
  // nothing to do
  return true;
}

//------------------------------------------------------------------
bool RadxPersistentClutterSecondPass::setRayForOutput(const RayClutterInfo *h,
						      const RayxData &r,
						      RadxRay &ray)
{
  // make a copy of the ray
  RayxData clutter(r);

  // cast the input point to what we want (again, dangerous design)
  const RayHistoInfo *histo = dynamic_cast<const RayHistoInfo *>(h);

  bool stat;

  // put values into the clutter RayxData
  stat = histo->setClutter(clutter, _params.clutter_percentile,
			   _params.missing_clutter_value);

  // prepare for output
  modifyRayForOutput(clutter, _params.output_field_name);
  updateRay(clutter, ray);
  return stat;
}

//------------------------------------------------------------------
RayClutterInfo *
RadxPersistentClutterSecondPass::matchingClutterInfo(const double az,
						     const double elev)
{
  // we match off of _histo, not the base class _store.
  // note this is what makes processRay and setRayForOutput work o.k.
  return matchInfo(_histo, az, elev);
}

//------------------------------------------------------------------
const RayClutterInfo * RadxPersistentClutterSecondPass::
matchingClutterInfoConst(const double az,
			 const double elev) const
{
  // we match off of _histo, not the base class _store.
  // note this is what makes processRay and setRayForOutput work o.k.
  return matchInfoConst(_histo, az, elev);
}

