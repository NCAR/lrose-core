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
 * @file RadxQpeMgr.cc
 */
#include <toolsa/umisc.h>
#include "RadxQpeMgr.hh"
#include "RadxQpe.hh"
#include "Parms.hh"
#include "InputData.hh"
#include "OutputData.hh"
#include "BeamBlock.hh"
#include "PpiInterp.hh"
#include <Radx/RadxRay.hh>
#include <Radx/RadxSweep.hh>
#include <toolsa/LogMsg.hh>
#include <toolsa/pmu.h>

//------------------------------------------------------------------
RadxQpeMgr::RadxQpeMgr(int argc, char **argv, void tidyAndExit(int)) :
  _isOK(false),
  _params(NULL),
  _alg(NULL),
  _data(NULL),
  _out(NULL),
  _beamBlock(NULL)
{

  string progName("RadxQpe");
  Args args;

  ucopyright((char *) progName.c_str());
  
  // get command line args
  if (args.parse(argc, argv, progName))
  {
    cerr << "ERROR: " << progName << endl;
    cerr << "Problem with command line args" << endl;
    return;
  }

  // get TDRP params
  char *paramsPath = (char *) "unknown";
  Params p;
  if (p.loadFromArgs(argc, argv, args.override.list,
		     &paramsPath))
  {
    cerr << "ERROR: " << progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
  }

  _params = new Parms(p, progName);

  int maxSeconds = 60;
  PMU_auto_init(progName.c_str(), _params->instance, maxSeconds);
  PORTsignal(SIGQUIT, tidyAndExit);
  PORTsignal(SIGTERM, tidyAndExit);
  PORTsignal(SIGINT, tidyAndExit);
  PORTsignal(SIGPIPE, (PORTsigfunc)SIG_IGN);
  LOG_INIT(_params->debug_norm, _params->debug_verbose, 
	   _params->debug_show_realtime,
	   _params->debug_show_class_and_method);
  if (_params->debug_triggering)
  {
    LOG_ENABLE_SEVERITY(LogMsg::TRIGGER);
  }

  // use params to create the remaining objects
  _alg = new RadxQpe(*_params);
  _data = new InputData(*_params, args);
  _out = new OutputData(*_params);
  _beamBlock = new BeamBlock(*_params);

  _isOK = true;
  if (!_beamBlock->_dataIsOK)
  {
    _isOK = false;
  }
  if (!_data->_dataIsOK)
  {
    _isOK = false;
  }
  if (!_out->_dataIsOK)
  {
    _isOK = false;
  }
}

//------------------------------------------------------------------
RadxQpeMgr::~RadxQpeMgr(void)
{
  PMU_auto_unregister();
  if (_params != NULL)
  {
    delete _params;
  }
  if (_alg != NULL)
  {
    delete _alg;
  }
  if (_data != NULL)
  {
    delete _data;
  }
  if (_out != NULL)
  {
    delete _out;
  }
  if (_beamBlock != NULL)
  {
    delete _beamBlock;
  }
  _freeInterpRays();
}

//------------------------------------------------------------------
int RadxQpeMgr::Run(void)
{
  if (!_isOK)
  {
    return 1;
  }

  time_t t;
  int index = 0;
  LOG(LogMsg::DEBUG, "  Reading input volume ---");
  while (_data->nextVolume(t))
  {
    LOG(LogMsg::DEBUG, " Aligning beam blockage grid ---- ");
    if (!_beamBlock->align(*_data))
    {
      return 1;
    }
    LOG(LogMsg::DEBUG, " Creating Qpe output data grids ---- ");
    _alg->process(*_data, *_beamBlock, t, *_out);
    LOG(LogMsg::DEBUG, " Writing polar output ---- ");
    _out->writeVolume(t, *_data, index++);

    if (strlen(_params->output_cartesian_dir) != 0)
    {
      LOG(LogMsg::DEBUG, " Interpolating/writing cartesian output ---- ");
      _interpolate(_out->getOutputVol());
    }
    LOG(LogMsg::DEBUG, "  Reading input volume ---");
  }
  return 0;
}


//------------------------------------------------------------------
void RadxQpeMgr::_interpolate(const RadxVol &vol)
{
  // now take the out object and interpolate
  _initInterpFields(vol);
  
  // load up the input ray data vector
  _loadInterpRays(vol);

  PpiInterp *interp = new PpiInterp("RadxQpe", *_params, vol,
				    _interpFields, _interpRays);
  interp->interpVol();
  delete interp;
}

//------------------------------------------------------------------
// initialize the fields for interpolation
void RadxQpeMgr::_initInterpFields(const RadxVol &vol)
{
  _interpFields.clear();
  
  // get field name list from volume
  vector<string> fieldNames = vol.getUniqueFieldNameList();

  // find an example of each field, by search through the rays
  // use that field as the template

  for (size_t ifield = 0; ifield < fieldNames.size(); ifield++)
  {

    string radxName = fieldNames[ifield];

    const vector<RadxRay *> &rays = vol.getRays();
    for (size_t iray = 0; iray < rays.size(); iray++)
    {
      const RadxRay *ray = rays[iray];
      const RadxField *field = ray->getField(radxName);
      if (field != NULL)
      {
        Interp::Field interpField(*field);
        if (field->getFieldFolds())
	{
	  LOGF(LogMsg::ERROR, "Not handling folded fields, %s",
	       field->getName().c_str());
        }
	else
	{
	  _interpFields.push_back(interpField);
	}
        break;
      }
    }
  } // ifield

  // override discrete flag from the parameters
  
  if (_params->set_discrete_fields)
  {
    for (int ii = 0; ii < _params->discrete_fields_n; ii++)
    {
      string radxName = _params->_discrete_fields[ii].input_name;
      bool isDiscrete = _params->_discrete_fields[ii].is_discrete;
      for (size_t ifld = 0; ifld < _interpFields.size(); ifld++)
      {
        if (_interpFields[ifld].radxName == radxName)
	{
          _interpFields[ifld].isDiscrete = isDiscrete;
          break;
        }
      } // ifld
    } // ii
  }
}

//------------------------------------------------------------------
// load up the input ray data vector

void RadxQpeMgr::_loadInterpRays(const RadxVol &vol)
{
  // free up previous usage
  
  _freeInterpRays();

  // loop through the rays in the read volume,
  // making some checks and then adding the rays
  // to the interp rays array as appropriate
  const vector<RadxRay *> &rays = vol.getRays();
  for (size_t isweep = 0; isweep < vol.getNSweeps(); isweep++)
  {
    const RadxSweep *sweep = vol.getSweeps()[isweep];

    for (size_t iray = sweep->getStartRayIndex();
         iray <= sweep->getEndRayIndex(); iray++)
    {
      Interp::Ray *interpRay = 
        new Interp::Ray(rays[iray], isweep, _interpFields);
      _interpRays.push_back(interpRay);

    } // iray

  } // isweep

}
  
//------------------------------------------------------------------
// Free up input rays

void RadxQpeMgr::_freeInterpRays()
  
{
  for (size_t ii = 0; ii < _interpRays.size(); ii++)
  {
    delete _interpRays[ii];
  }
  _interpRays.clear();
}

