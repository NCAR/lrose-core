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
///////////////////////////////////////////////////////////////
// Rhi2Spdb.cc
//
// Rhi2Spdb object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2004
//
///////////////////////////////////////////////////////////////
//
// Rhi2Spdb reads MDV files containing measured RHI data, creates
// a GenPt which describes the RHI and writes it to SPBD
//
///////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxRadar.hh>
#include <Spdb/DsSpdb.hh>
#include <rapformats/GenPt.hh>
#include "Rhi2Spdb.hh"
using namespace std;

// Constructor

Rhi2Spdb::Rhi2Spdb(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "Rhi2Spdb";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }

  // check that start and end time is set in archive mode

  if (_params.mode == Params::ARCHIVE) {
    if (_args.startTime == 0 || _args.endTime == 0) {
      cerr << "ERROR - must specify start and end dates." << endl << endl;
      _args.usage(_progName, cerr);
      isOK = false;
    }
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(), _params.instance,
		PROCMAP_REGISTER_INTERVAL);

  // initialize the data input object

  if (_params.mode == Params::REALTIME) {
    if (_input.setRealtime(_params.input_url,
                           _params.max_realtime_data_age_secs,
                           PMU_auto_register,
                           _params.time_between_checks_secs * 1000)) {
      isOK = false;
    }
  } else if (_params.mode == Params::ARCHIVE) {
    if (_input.setArchive(_params.input_url,
			  _args.startTime,
			  _args.endTime)) {
      isOK = false;
    }
  } else if (_params.mode == Params::FILELIST) {
    if (_input.setFilelist(_args.inputFileList)) {
      isOK = false;
    }
  }

  return;

}

// destructor

Rhi2Spdb::~Rhi2Spdb()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Rhi2Spdb::Run ()
{

  int iret = 0;

  // register with procmap
  
  PMU_auto_register("Run");
  
  // create DsMdvx object
  
  DsMdvx mdvx;
  mdvx.setDebug(_params.debug >= Params::DEBUG_VERBOSE);

  // loop until end of data

  while (!_input.endOfData()) {
    
    PMU_auto_register("In main loop");
    
    if (_input.readAllHeadersNext(mdvx)) {
      cerr << "ERROR - Rhi2Spdb::Run" << endl;
      cerr << "  Cannot read in data." << endl;
      cerr << _input.getErrStr() << endl;
      iret = -1;
      continue;
    }
    
    if (_params.debug) {
      cerr << "Working on file: " << mdvx.getPathInUse() << endl;
    }

    //    const Mdvx::master_header_t &mhdr = mdvx.getMasterHeader();
    const Mdvx::master_header_t &mhdr = mdvx.getMasterHeaderFile();
    if (mdvx.getNFieldsFile() < 1) {
      cerr << "ERROR - Rhi2Spdb::Run()" << endl;
      cerr << "  Working on file: " << //  mdvx.getPathInUse() << endl;
        cerr << "  No fields in file" << endl;
      continue;
    }

    const Mdvx::field_header_t &fhdr = mdvx.getFieldHeaderFile(0);
    const Mdvx::vlevel_header_t &vhdr = mdvx.getVlevelHeaderFile(0);
    
    // create a GenPt, load it up with the info
    
    MdvxRadar radar;
    bool haveRadarInfo = false;
    if (radar.loadFromMdvx(mdvx) == 0) {
      haveRadarInfo = true;
    }

    GenPt pt;
    pt.setName("RHIs");
    pt.setId(0);

    if (haveRadarInfo && radar.radarParamsAvail()) {
      string name("RHIs for radar ");
      name += radar.getRadarParams().radarName;
      pt.setName(name);
      pt.setId(radar.getRadarParams().radarId);
    }

    pt.setTime(mhdr.time_centroid);
    pt.setLat(mhdr.sensor_lat);
    pt.setLon(mhdr.sensor_lon);
    pt.setNLevels(fhdr.nz);

    pt.addFieldInfo("azimuth", "deg");
    pt.addFieldInfo("n_gates", "count");
    pt.addFieldInfo("start_range", "km");
    pt.addFieldInfo("gate_spacing", "km");
    pt.addFieldInfo("n_elevations", "count");
    pt.addFieldInfo("start_elevation", "deg");
    pt.addFieldInfo("delta_elevation", "deg");

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Time: " << utimstr(mhdr.time_centroid) << endl;
      cerr << "n azimuths: " << fhdr.nz << endl;
    }

    // add values

    for (int ii = 0; ii < fhdr.nz; ii++) {
      pt.addVal(vhdr.level[ii]);
      pt.addVal(fhdr.nx);
      pt.addVal(fhdr.grid_minx);
      pt.addVal(fhdr.grid_dx);
      pt.addVal(fhdr.ny);
      pt.addVal(fhdr.grid_miny);
      pt.addVal(fhdr.grid_dy);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "  azimuths: " << vhdr.level[ii] << endl;
	cerr << "  n gates: " << fhdr.nx << endl;
	cerr << "  n elev: " << fhdr.ny << endl;
      }
    }

    // assemble into buffer

    pt.assemble();

    // write to Spdb
    
    DsSpdb spdb;
    spdb.setAppName(_progName);
    si32 data_type = 0;
    if (haveRadarInfo && radar.radarParamsAvail()) {
      data_type = Spdb::hash4CharsToInt32
	(radar.getRadarParams().radarName.c_str());
    }
    spdb.addPutChunk(data_type, mhdr.time_centroid, mhdr.time_centroid,
		     pt.getBufLen(), pt.getBufPtr());
    if (spdb.put(_params.output_url,
		 SPDB_GENERIC_POINT_ID,
		 SPDB_GENERIC_POINT_LABEL)) {
      cerr << "ERROR - Rhi2Spdb::run()" << endl;
      cerr << "  Cannot put rhi data to URL: "
	   << _params.output_url << endl;
      cerr << "  " << spdb.getErrStr() << endl;
      iret = -1;
    }
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Wrote data to URL: " << _params.output_url << endl;
    }

  } // while
  
  return iret;

}

