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
 * @file RadxApp.cc
 */

//------------------------------------------------------------------
#include <radar/RadxApp.hh>
#include <radar/RadxAppParms.hh>
#include <radar/RadxAppVolume.hh>
#include <Radx/RadxRay.hh>
#include <rapmath/MathData.hh>
#include <rapmath/VolumeData.hh>
#include <toolsa/LogMsgStreamInit.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/pmu.h>
#include <toolsa/port.h>

//------------------------------------------------------------------
RadxApp::RadxApp(const MathData &sweepData, const MathData &rayData,
		 const VolumeData &vdata)
{
  _ok = false;
  _setupUserUnaryOps(sweepData, rayData, vdata);
}

//------------------------------------------------------------------
RadxApp::RadxApp(const RadxAppParms &parms,
		 const MathData &sweepData, const MathData &rayData,
		 const VolumeData &vdata)
{
  _ok = true;
  _setupUserUnaryOps(sweepData, rayData, vdata);

  for (size_t i=0; i<parms._volumeBeforeFilters.size(); ++i)
  {
    _p.parse(parms._volumeBeforeFilters[i], MathParser::VOLUME_BEFORE,
	     parms._fixedConstants, parms._userData);
  }

  for (size_t i=0; i<parms._sweepFilters.size(); ++i)
  {
    _p.parse(parms._sweepFilters[i], MathParser::LOOP2D_TO_2D,
	     parms._fixedConstants, parms._userData);
  }

  for (size_t i=0; i<parms._rayFilters.size(); ++i)
  {
    _p.parse(parms._rayFilters[i], MathParser::LOOP1D, parms._fixedConstants,
	     parms._userData);
  }

  for (size_t i=0; i<parms._volumeAfterFilters.size(); ++i)
  {
    _p.parse(parms._volumeAfterFilters[i], MathParser::VOLUME_AFTER,
	     parms._fixedConstants, parms._userData);
  }

}

//------------------------------------------------------------------
RadxApp::~RadxApp(void)
{
}

//------------------------------------------------------------------
void RadxApp::printOperators(void) const
{
  std::vector<FunctionDef> f = _p.allFunctionDefs();
  for (size_t i=0; i<f.size(); ++i)
  {
    printf("%s:\n%s\n\n", f[i]._name.c_str(), f[i]._description.c_str());
  }
  
  
  // std::string s = _p.sprintBinaryOperators();
  // printf("Binary operations:\n%s\n", s.c_str());

  // s = _p.sprintUnaryOperators();
  // printf("Unary operations:\n%s\n", s.c_str());
}

//------------------------------------------------------------------
bool RadxApp::algInit(const std::string &appName,
		      const RadxAppParams &p, 
		      void cleanup(int))
{
  PMU_auto_init(appName.c_str(), p.instance, 60);
  PORTsignal(SIGQUIT, cleanup);
  PORTsignal(SIGTERM, cleanup);
  PORTsignal(SIGINT, cleanup);
  PORTsignal(SIGPIPE, (PORTsigfunc)SIG_IGN);

  // set up debugging state for logging
  LogMsgStreamInit::init(p.debug_mode == RadxAppParams::DEBUG ||
			 p.debug_mode == RadxAppParams::DEBUG_VERBOSE,
			 p.debug_mode == RadxAppParams::DEBUG_VERBOSE,
			 true, true);
  if (p.debug_triggering)
  {
    LogMsgStreamInit::setThreading(true);
  }
  LOG(DEBUG) << "setup";

  // set the out of storage handler 
  // set_new_handler(outOfStore);
	      
  
  // check the inputs to make sure on the list somewhere
  // return _params.inputsAccountedFor(inputFields);
  return true;
}

//------------------------------------------------------------------
void RadxApp::algFinish(void)
{
  PMU_auto_unregister();
}

//------------------------------------------------------------------
bool RadxApp::update(const RadxAppParms &P, RadxAppVolume *volume)
{
  // do the volume commands first
  _p.processVolume(volume);

  // then the loop commands, 1d
  _p.clearOutputDebugAll();
  for (int ii=0; ii < volume->numProcessingNodes(false); ++ii)
  {
    _p.processOneItem1d(volume, ii);
  }

  // then the loop commands, 2d
  for (int ii=0; ii < volume->numProcessingNodes(true); ++ii)
  {
    _p.processOneItem2d(volume, ii);
  }
  _p.setOutputDebugAll();

  // do the final volume commands
  _p.processVolumeAfter(volume);

  // trim to wanted outputs
  volume->trim();

  return true;
}


//---------------------------------------------------------------
bool RadxApp::retrieveRay(const std::string &name, const RadxRay &ray,
                          const std::vector<RayxData> &data, RayxData &r,
			  bool showError)
{
  // try to find the field in the ray first
  if (retrieveRay(name, ray, r, false))
  {
    return true;
  }

  // try to find the field in the data vector
  for (size_t i=0; i<data.size(); ++i)
  {
    if (data[i].getName() == name)
    {
      // return a copy of that ray
      r = data[i];
      return true;
    }
  }
    
  if (showError)
  {
    LOG(ERROR) << "Field " << name << " never found";
  }
  return false;
}  

//---------------------------------------------------------------
bool RadxApp::retrieveRay(const std::string &name, const RadxRay &ray,
                          RayxData &r, const bool showError)
{
  // try to find the field in the ray
  const vector<RadxField *> &fields = ray.getFields();
  for (size_t ifield = 0; ifield < fields.size(); ifield++)
  {
    if (fields[ifield]->getName() == name)
    {
      Radx::DataType_t t = fields[ifield]->getDataType();
      if (t != Radx::FL32)
      {
	// need this to pull out values
	fields[ifield]->convertToFl32();
      }
      r = RayxData(name, fields[ifield]->getUnits(),
		  fields[ifield]->getNPoints(), fields[ifield]->getMissing(),
		  ray.getAzimuthDeg(), ray.getElevationDeg(),
		  ray.getGateSpacingKm(), ray.getStartRangeKm(),
		  *fields[ifield]);
      return true;
    }
  }
  if (showError)
  {
    LOG(ERROR) << "Field " << name << " never found";
  }
  return false;
}

//---------------------------------------------------------------
bool RadxApp::retrieveAnyRay(const RadxRay &ray, RayxData &r)
{
  // try to find the field in the ray
  const vector<RadxField *> &fields = ray.getFields();
  for (size_t ifield = 0; ifield < fields.size(); ifield++)
  {
    Radx::DataType_t t = fields[ifield]->getDataType();
    if (t != Radx::FL32)
    {
      // need this to pull out values
      fields[ifield]->convertToFl32();
    }
    r = RayxData(fields[ifield]->getName(), fields[ifield]->getUnits(),
		 fields[ifield]->getNPoints(), fields[ifield]->getMissing(),
		 ray.getAzimuthDeg(), ray.getElevationDeg(),
		 ray.getGateSpacingKm(), ray.getStartRangeKm(),
		 *fields[ifield]);
    return true;
  }
  LOG(ERROR) << "No fields in ray";
  return false;
}

//---------------------------------------------------------------
RayxData *RadxApp::retrieveRayPtr(const std::string &name, const RadxRay &ray,
				  std::vector<RayxData> &data, 
				  bool &isNew, bool showError)
{
  // try to find the field in the ray first
  RayxData *r = retrieveRayPtr(name, ray, false);
  if (r !=NULL)
  {
    isNew = true;
    return r;
  }

  // try to find the field in the data vector
  for (size_t i=0; i<data.size(); ++i)
  {
    if (data[i].getName() == name)
    {
      isNew = false;
      return &(data[i]);
    }
  }
    
  if (showError)
  {
    LOG(ERROR) << "Field " << name << " never found";
  }
  return NULL;
}  

//---------------------------------------------------------------
RayxData *RadxApp::retrieveRayPtr(const std::string &name, const RadxRay &ray,
				  const bool showError)
{
  // try to find the field in the ray
  const vector<RadxField *> &fields = ray.getFields();
  for (size_t ifield = 0; ifield < fields.size(); ifield++)
  {
    if (fields[ifield]->getName() == name)
    {
      Radx::DataType_t t = fields[ifield]->getDataType();
      if (t != Radx::FL32)
      {
	// need this to pull out values
	fields[ifield]->convertToFl32();
      }
      RayxData *r = new
	RayxData(name, fields[ifield]->getUnits(),
		 fields[ifield]->getNPoints(), fields[ifield]->getMissing(),
		 ray.getAzimuthDeg(), ray.getElevationDeg(),
		 ray.getGateSpacingKm(), ray.getStartRangeKm(),
		 *fields[ifield]);
      return r;
    }
  }
  if (showError)
  {
    LOG(ERROR) << "Field " << name << " never found";
  }
  return NULL;
}

//---------------------------------------------------------------
RayxData *RadxApp::retrieveAnyRayPtr(const RadxRay &ray)
{
  // try to find the field in the ray
  const vector<RadxField *> &fields = ray.getFields();
  for (size_t ifield = 0; ifield < fields.size(); ifield++)
  {
    Radx::DataType_t t = fields[ifield]->getDataType();
    if (t != Radx::FL32)
    {
      // need this to pull out values
      fields[ifield]->convertToFl32();
    }
    RayxData *r =
      new RayxData(fields[ifield]->getName(), fields[ifield]->getUnits(),
		   fields[ifield]->getNPoints(), fields[ifield]->getMissing(),
		   ray.getAzimuthDeg(), ray.getElevationDeg(),
		   ray.getGateSpacingKm(), ray.getStartRangeKm(),
		   *fields[ifield]);
    return r;
  }
  LOG(ERROR) << "No fields in ray";
  return NULL;
}

//---------------------------------------------------------------
void RadxApp::modifyRayForOutput(RayxData &r, const std::string &name,
                                 const std::string &units,
                                 const double missing)
{
  r.changeName(name);
  if (units.empty())
  {
    return;
  }
  r.changeUnits(units);
  r.changeMissing(missing);
}

//---------------------------------------------------------------
void RadxApp::updateRay(const RayxData &r, RadxRay &ray)
{
  // add in the one RayxData, then clear out everything else
    
  int nGatesPrimary = ray.getNGates();
  Radx::fl32 *data = new Radx::fl32[nGatesPrimary];

  string name = r.getName();
  r.retrieveData(data, nGatesPrimary);
  ray.addField(name, r.getUnits(), r.getNpoints(), r.getMissing(), data, true);
  vector<string> wanted;
  wanted.push_back(name);
  ray.trimToWantedFields(wanted);
  delete [] data;
}

//---------------------------------------------------------------
void RadxApp::updateRay(const vector<RayxData> &raydata, RadxRay &ray)
{
  // take all the data and add to the ray.
  // note that here should make sure not deleting a result (check names)
  if (raydata.empty())
  {
    return;
  }
  int nGatesPrimary = ray.getNGates();
  Radx::fl32 *data = new Radx::fl32[nGatesPrimary];

  string name = raydata[0].getName();
  raydata[0].retrieveData(data, nGatesPrimary);
  ray.addField(name, raydata[0].getUnits(), raydata[0].getNpoints(),
	       raydata[0].getMissing(), data, true);
  vector<string> wanted;
  wanted.push_back(name);
  ray.trimToWantedFields(wanted);

  // now add in all the other ones
  for (int i=1; i<(int)raydata.size(); ++i)
  {
    raydata[i].retrieveData(data, nGatesPrimary);
    ray.addField(raydata[i].getName(), raydata[i].getUnits(),
		 raydata[i].getNpoints(),
		 raydata[i].getMissing(), data, true);
  }
  delete [] data;
}

//---------------------------------------------------------------
bool RadxApp::write(RadxAppVolume *vol)
{
  return vol->write();
}

//---------------------------------------------------------------
bool RadxApp::write(RadxAppVolume *vol, const std::string &url)
{
  return vol->write(url);
}

//------------------------------------------------------------------
void RadxApp::_setupUserUnaryOps(const MathData &sweepData,
				 const MathData &rayData,
				 const VolumeData &vdata)
{
  std::vector<FunctionDef> userUops = sweepData.userUnaryOperators();
  for (size_t i=0; i<userUops.size(); ++i)
  {
    _p.addUserUnaryOperator(userUops[i]);
  }
  userUops = rayData.userUnaryOperators();
  for (size_t i=0; i<userUops.size(); ++i)
  {
    _p.addUserUnaryOperator(userUops[i]);
  }
  userUops = vdata.userUnaryOperators();
  for (size_t i=0; i<userUops.size(); ++i)
  {
    _p.addUserUnaryOperator(userUops[i]);
  }
}
