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
 * @file RadxQpe.cc
 */

#include "RadxQpe.hh"
#include "Data.hh"
#include "OutputData.hh"
#include "BeamBlock.hh"
#include "VertData.hh"
#include "VertPrecipData.hh"
#include "Out.hh"
#include "QpeInfo.hh"
#include <toolsa/LogMsg.hh>
#include <toolsa/TaThreadSimple.hh>

//------------------------------------------------------------------
RadxQpe::RadxQpe(const Parms &params) : _params(params)
{
  _thread.init(params.n_compute_threads, params.threads_debug);
}

//------------------------------------------------------------------
RadxQpe::~RadxQpe()
{
}

//------------------------------------------------------------------
void RadxQpe::process(Data &data, const BeamBlock &block, const time_t &t,
		      OutputData &out)
{
  // copy the geometry over to output, and create the grids
  out = OutputData(_params, data);

  // make sure block data has same elevations as data
  if (data.size() != block.size())
  {
    LOG(LogMsg::ERROR, "Setting up beam block data, wrong elevations");
    return;
  }

  // loop through the geometry
  LOG(LogMsg::DEBUG, "Loop now");
  for (int iaz=0; iaz < data.nOutputAz(); ++iaz)
  {
    QpeInfo *info = new QpeInfo(&data, &block, iaz, &out, this);

    int index = 0;
    _thread.thread(index, info);
    // Note that info is freed within the thread
  }

  // wait for all active threads to complete
  _thread.waitForThreads();
  LOG(LogMsg::DEBUG, "End loop");
}

//------------------------------------------------------------------
void RadxQpe::_computeInThread(void *thread_data)
  
{
  QpeInfo *info = static_cast<QpeInfo *>(thread_data);
  RadxQpe *alg = info->_alg;
  assert(alg);

  alg->_processAzimuth(*info);


  // delete is done here
  delete info;

}

//------------------------------------------------------------------
void RadxQpe::_processAzimuth(QpeInfo &info)
{
  for (int igt=0; igt < info._data->nGate(); ++igt)
  {
    // create the fixed vertical data
    VertData vdata(*info._data, *info._block, igt, info._iaz, _params);

    // figure out height at which precip can be used, and set diagnostic
    // values
    Out outdata(vdata, _params);

    // write out the diagnostic values into out.
    outdata.store(igt, info._iaz, _params, (*info._out)[0]);

    // process each precip field
    for (int i=0; i<_params.numRainRate(); ++i)
    {
      string name = _params.ithInputPrecipName(i);
      VertPrecipData pdata(*info._data, igt, info._iaz, name, i);

      // set and store output using this precip data
      outdata.storePrecip(name, pdata, igt, info._iaz, _params,
			  (*info._out)[0]);
    }
  }
}

//------------------------------------------------------------------
TaThread *RadxQpe::QpeThreads::clone(int index)
{
  TaThreadSimple *t = new TaThreadSimple(index);
  t->setThreadContext(this);
  t->setThreadMethod(RadxQpe::_computeInThread);
  return dynamic_cast<TaThread *>(t);
}

