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
// Mdv2SweepNetCDF.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2003
//
///////////////////////////////////////////////////////////////
//
// Mdv2SweepNetCDF reads MDV files and converts them to
// NetCDF sweep files.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <toolsa/os_config.h>
#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxRadar.hh>
#include "Mdv2SweepNetCDF.hh"
#include "SweepFile.hh"
using namespace std;

// Constructor

Mdv2SweepNetCDF::Mdv2SweepNetCDF(int argc, char **argv)

{

  isOK = true;
  _volNum = 0;

  // set programe name

  _progName = "Mdv2SweepNetCDF";
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

  if (_args.outputFilePath.size() > 0) {
    if (_args.inputFileList.size() != 1) {
      cerr << "ERROR: -of specified, for one output file." << endl;
      cerr << "  Therefore, specify one input file with -if." << endl;
      _args.usage(_progName, cerr);
      isOK = false;
    }
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
  // initialize the data input object

  if (_params.mode == Params::REALTIME) {
    if (_input.setRealtime(_params.input_url,
			   _params.max_realtime_valid_age,
			   PMU_auto_register)) {
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

Mdv2SweepNetCDF::~Mdv2SweepNetCDF()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Mdv2SweepNetCDF::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");
  
  // create DsMdvx object
  
  DsMdvx mdvx;
  mdvx.setDebug(_params.debug);

  // loop until end of data
  
  while (!_input.endOfData()) {
    
    PMU_auto_register("In main loop");
    
    // set up the Mdvx read

    mdvx.clearRead();

    if (_params.specify_output_field_names) {
      for (int ii = 0; ii < _params.output_field_names_n; ii++) {
        if (_params.debug) {
          cerr << "Adding read field name: " << _params._output_field_names[ii] << endl;
        }
        mdvx.addReadField(_params._output_field_names[ii]);
      }
    }
    
    if (_params.output_encoding == Params::ENCODING_INT8) {
      mdvx.setReadEncodingType(Mdvx::ENCODING_INT8);
    } else if (_params.output_encoding == Params::ENCODING_INT16) {
      mdvx.setReadEncodingType(Mdvx::ENCODING_INT16);
    } else if (_params.output_encoding == Params::ENCODING_FLOAT32) {
      mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
    }

    mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
    if (_input.readVolumeNext(mdvx)) {
      cerr << "ERROR - Mdv2SweepNetCDF::Run" << endl;
      cerr << "  Cannot read in data." << endl;
      cerr << _input.getErrStr() << endl;
      return -1;
    }
    
    if (_params.debug) {
      cerr << "Working on file: " << mdvx.getPathInUse() << endl;
    }

    // check that this file is suitable

    if (_checkMdvFile(mdvx)) {
      continue;
    }
    
    // write out

    PMU_auto_register("Before write");
    
    if (_writeSweepFiles(mdvx)) {
      cerr << "ERROR - Mdv2SweepNetCDF::Run" << endl;
      cerr << "  Cannot write output sweep files." << endl;
      return -1;
    }

    _volNum++;
    
  } // while

  return 0;

}

/////////////////////////////////////////
// check that this is a suitable MDV file
//
// Returns 0 on success, -1 on failure

int Mdv2SweepNetCDF::_checkMdvFile(const DsMdvx &mdvx)
  
{

  if (mdvx.getNFields() < 1) {
    cerr << "ERROR - Mdv2SweepNetCDF::_checkMdvFile" << endl;
    cerr << "  No fields in file" << endl;
    return -1;
  }
  
  int nx = 0, ny = 0, nz = 0;
  
  for (size_t ifield = 0; ifield < mdvx.getNFields(); ifield++) {
    
    MdvxField *fld = mdvx.getField(ifield);
    const Mdvx::field_header_t &fhdr = fld->getFieldHeader();
    const Mdvx::vlevel_header_t &vhdr = fld->getVlevelHeader();
    
    if (fhdr.proj_type != Mdvx::PROJ_POLAR_RADAR) {
      cerr << "ERROR - Mdv2SweepNetCDF::_checkMdvFile" << endl;
      cerr << "  Incorrect projection type" << endl;
      cerr << "  Expecting PROJ_POLAR_RADAR" << endl;
      cerr << "  Found " << Mdvx::projType2Str(fhdr.proj_type) << endl;
      return -1;
    }

    if (ifield == 0) {
      nx = fhdr.nx;
      ny = fhdr.ny;
      nz = fhdr.nz;
      _elevations.clear();
      for (int ii = 0; ii < nz; ii++) {
	_elevations.push_back((double) vhdr.level[ii]);
      }
    } else {
      if (nx != fhdr.nx || ny != fhdr.ny || nz != fhdr.nz) {
	cerr << "ERROR - Mdv2SweepNetCDF::_checkMdvFile" << endl;
	cerr << "  Variable geometry between fields" << endl;
	cerr << "  Expecting nx, ny, nz: "
	     << nx << ", " << ny << ", " << nz << endl;
	cerr << "  Found nx, ny, nz: "
	     << fhdr.nx << ", " << fhdr.ny << ", " << fhdr.nz << endl;
	cerr << "  Field: " << fld->getFieldName() << endl;
	return -1;
      }
    }
    
  } // ifield
  
  _nGates = nx;
  _nAz = ny;
  _nTilts = nz;

  //
  // Load the radar parameters from the mdv file
  //

  MdvxRadar mdvxRadar;

  if( mdvxRadar.loadFromMdvx(mdvx) && mdvxRadar.radarParamsAvail()) {

    _radarParams = mdvxRadar.getRadarParams();
    
  } else {
    
    MdvxField *fld0 = mdvx.getField(0);
    const Mdvx::field_header_t &fhdr0 = fld0->getFieldHeader();

    _radarParams.numFields = mdvx.getNFields();
    _radarParams.numGates = _nGates;
    _radarParams.samplesPerBeam = 64;
    _radarParams.scanType = 0;
    _radarParams.scanMode = DS_RADAR_SURVEILLANCE_MODE;
    _radarParams.polarization = DS_POLARIZATION_HORIZ_TYPE;
    _radarParams.radarType = DS_RADAR_GROUND_TYPE;
    
    _radarParams.radarConstant = -100;
    _radarParams.altitude = fhdr0.grid_minz;
    _radarParams.latitude = fhdr0.proj_origin_lat;
    _radarParams.longitude = fhdr0.proj_origin_lon;
    _radarParams.gateSpacing = fhdr0.grid_dx;
    _radarParams.startRange = fhdr0.grid_minx;
    _radarParams.horizBeamWidth = 1.0;
    _radarParams.vertBeamWidth = 1.0;
    _radarParams.pulseWidth = 1.0;
    _radarParams.pulseRepFreq = 1000;
    _radarParams.wavelength = 10.0;
    _radarParams.xmitPeakPower = 1.e6;
    _radarParams.receiverMds = -108.0;
    _radarParams.receiverGain = 70.0;
    _radarParams.antennaGain = 40.0;
    _radarParams.systemGain = 80.0;
    _radarParams.unambigVelocity = 32.0;
    _radarParams.unambigRange = 150.0;
    
  }
  
  return 0;

}

  
//////////////////////////////////
// write the output to sweep files

int Mdv2SweepNetCDF::_writeSweepFiles(const DsMdvx &mdvx)
  
{
  
  // write out a sweep file for each tilt num

  for (int ii = 0; ii < _nTilts; ii++) {
    
    SweepFile sfile(_params, mdvx,
		    _nTilts, ii, _elevations[ii],
		    _volNum, _nAz, _nGates, _radarParams);
    if (sfile.write()) {
      cerr << "ERROR = Mdv2SweepNetCDF::_writeSweepFile" << endl;
      cerr << "  Cannot write sweep files" << endl;
      return -1;
    }
    
  } // ii

  return 0;

}

