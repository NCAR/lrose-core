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
 * @file CartSimMgr.cc
 */

#include "CartSimMgr.hh"
#include "DataHandlerMdv.hh"
#include "DataHandlerCfRadial.hh"
#include "Info.hh"
#include <toolsa/TaThreadSimple.hh>

//------------------------------------------------------------------------
// clone a simple thread, which has this manager and this objects compute
TaThread *CartSimMgr::CartSimThreads::clone(int index)
{
  TaThreadSimple *t = new TaThreadSimple(index);
  t->setThreadContext(this);
  t->setThreadMethod(CartSimMgr::compute);
  return dynamic_cast<TaThread *>(t);
}

//----------------------------------------------------------------
CartSimMgr::CartSimMgr(const Params &parms) : 
  _parms(parms), 
  _alg(parms),
  _data(NULL), 
  _step_seconds(static_cast<int>(parms.simulation_step_minutes*60.0)),
  _max_seconds(static_cast<int>(parms.simulation_length_minutes*60.0))

					      
{
  // create the correct data handler
  switch (parms.format)
  {
  case Params::MDV:
    _data = new DataHandlerMdv(parms);
    break;
  case Params::CFRADIAL:
    _data = new DataHandlerCfRadial(parms);
    break;
  default:
    printf("ERROR in value for format %d\n", (int)parms.format);
    exit(1);
  }

  // initialize threading 
  _thread.init(_parms.num_threads, _parms.thread_debug);
}

//----------------------------------------------------------------
CartSimMgr::~CartSimMgr()
{
  if (_data != NULL)
  {
    delete _data;
  }
}

//----------------------------------------------------------------
int CartSimMgr::run(void)
{
  if (_data == NULL)
  {
    // no can do without data
    return 1;
  }
  // read in a template to use in data handling
  if (!_data->init())
  {
    return 1;
  }

  printf("Begin processing now\n");
  int j = 0;
  for (int dt = 0; dt <= _max_seconds; dt += _step_seconds, ++j)
  {
    // create info
    Info *info = new Info(dt, _alg, this);
    // thread it (the compute method will delete the info pointer)a
    _thread.thread(j, info);
  }
  _thread.waitForThreads();
  return 0;
}

//------------------------------------------------------------------------
void CartSimMgr::compute(void *ti)
{
  // cast
  Info *info = static_cast<Info *>(ti);

  // go for it
  info->process();

  // clean up
  delete info;
}

//------------------------------------------------------------------------
void CartSimMgr::lock()
{
  _thread.lockForIO();
}

//------------------------------------------------------------------------
void CartSimMgr::unlock()
{
  _thread.unlockAfterIO();
}

//------------------------------------------------------------------------
DataHandler *CartSimMgr::cloneData(void) const
{
  return _data->clone();
}
