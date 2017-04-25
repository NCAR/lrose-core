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
////////////////////////////////////////////////////////////////////////////////
//
// HsrlSim.cc
// HsrlSim class
//
// Mike Dixon, EOL, NCAR, Boulder, CO, USA
// April 2017
//
////////////////////////////////////////////////////////////////////////////////
//
// HsrlSim listens for clients. When a client connects, it spawns a
// child to handle the client. The child opens an HSRL raw NetCDF
// file, and loops reading the file, creating raw rays and sending
// them to the client.";
// 
////////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <string>
#include <toolsa/pmu.h>
#include <toolsa/uusleep.h>
#include <toolsa/Socket.hh>
#include <toolsa/TaStr.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include "RawFile.hh"
#include "HsrlSim.hh"
using namespace std;

// constructor

HsrlSim::HsrlSim(const string& progName,
		     const Params& params) :
	ProcessServer(progName, 
		      params.instance,
		      params.port, 
		      params.max_clients,
		      params.debug >= Params::DEBUG_VERBOSE, 
		      params.debug >= Params::DEBUG_EXTRA),
	_progName(progName),
	_params(params)
  
{
  if (_params.no_threads) {
    setNoThreadDebug(true);
  }
}

// destructor

HsrlSim::~HsrlSim()

{

}

// handle clients which connect

int HsrlSim::handleClient(Socket* socket)
{

  PMU_auto_register("handleClient");

  // read HSRL raw file into RadxVol

  RadxVol vol;
  RawFile rawFile(_params);
  if (rawFile.readFromPath(_params.netcdf_file_path, vol)) {
    cerr << "ERROR - HsrlSim::handleClient" << endl;
    cerr << "  Cannot read in raw file: " << _params.netcdf_file_path << endl;
    cerr << rawFile.getErrStr() << endl;
    return -1;
  }

  // Loop forever, until client dies

  HsrlRawRay rawRay;

  while (true) {

    // loop through the rays in the volume
    
    vector<RadxRay *> &rays = vol.getRays();
    for (size_t iray = 0; iray < rays.size(); iray++) {
      
      const RadxRay *ray = rays[iray];

      size_t nGates = ray->getNGates();
      double elevDeg = ray->getElevationDeg();
      int telescopeDirn = 0;
      if (elevDeg > 0) {
        telescopeDirn = 1;
      }
      
      // get more info from ray here
      
      double totalEnergy = ray->getMeasXmitPowerDbmH();
      double polAngle = ray->getEstimatedNoiseDbmHc();
      
      //get raw data fields
      const RadxField *hiField = ray->getField(_params.combined_hi_field_name);
      const RadxField *loField = ray->getField(_params.combined_lo_field_name);
      const RadxField *crossField = ray->getField(_params.cross_field_name);
      const RadxField *molField = ray->getField(_params.molecular_field_name);

      if (hiField == NULL || loField == NULL
          || crossField == NULL || molField == NULL) {
        continue;
      }
      
      const Radx::fl32 *hiData = hiField->getDataFl32();
      const Radx::fl32 *loData = loField->getDataFl32();
      const Radx::fl32 *crossData = crossField->getDataFl32();
      const Radx::fl32 *molData = molField->getDataFl32();

      // get the current time

      RadxTime now(RadxTime::NOW);
      rawRay.setTime(now.utime(), now.getSubSec());

      // set other members
      
      rawRay.setTelescopeLocked(true);
      rawRay.setTelescopeDirn(telescopeDirn);
      rawRay.setTotalEnergy(totalEnergy);
      rawRay.setPolAngle(polAngle);
      
      // set the fields

      rawRay.setFields(nGates, hiData, loData, molData, crossData);

      // serialize into buffer

      rawRay.serialize();

      // send to client

      if (socket->writeBuffer(rawRay.getBufPtr(), rawRay.getBufLen())) {
        if (_params.debug) {
          cerr << "ERROR - HsrlSim::handleClient" << endl;
          cerr << " Writing to client" << endl;
          cerr << socket->getErrStr() << endl;
        }
        return -1;
      }
      
      // sleep a bit

      umsleep(_params.delay_secs_between_rays * 1000.0);
      
    } // iray

  } // while (true)

  return 0;

}
