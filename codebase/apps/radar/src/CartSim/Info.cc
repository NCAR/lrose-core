// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// © University Corporation for Atmospheric Research (UCAR) 2009-2010. 
// All rights reserved.  The Government's right to use this data and/or 
// software (the "Work") is restricted, per the terms of Cooperative 
// Agreement (ATM (AGS)-0753581 10/1/08) between UCAR and the National 
// Science Foundation, to a "nonexclusive, nontransferable, irrevocable, 
// royalty-free license to exercise or have exercised for or on behalf of 
// the U.S. throughout the world all the exclusive rights provided by 
// copyrights.  Such license, however, does not include the right to sell 
// copies or phonorecords of the copyrighted works to the public."   The 
// Work is provided "AS IS" and without warranty of any kind.  UCAR 
// EXPRESSLY DISCLAIMS ALL OTHER WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
// ANY IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
// PURPOSE.  
//  
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#include <toolsa/copyright.h>

/**
 * @file Info.cc
 */

#include "Info.hh"
#include "CartSimMgr.hh"
#include "DataHandler.hh"

//----------------------------------------------------------------
Info::Info(const int dt, const CartSim &alg, CartSimMgr *mgr) :
  _data(NULL),
  _alg(alg),    // copy the alg
  _mgr(mgr)     // save the pointer
{

  // make a clone so have a local copy
  _data = mgr->cloneData();

  // set time value internally
  _alg.setTime(dt);
}

//----------------------------------------------------------------
Info::~Info()
{
  // this must be deleted as it was cloned
  delete _data;
}

//----------------------------------------------------------------
void Info::process(void)
{
  // reset indices, etc.
  _data->reset();

  // get location of each point
  Xyz loc;
  while (_data->nextPoint(loc))
  {
    // process that point
    Data data;
    _alg.process(loc, data);

    // store results back to data
    _data->store(data);
  }

  // write data content out
  _mgr->lock();
  _data->write(_alg.currentTime());
  _mgr->unlock();
}



