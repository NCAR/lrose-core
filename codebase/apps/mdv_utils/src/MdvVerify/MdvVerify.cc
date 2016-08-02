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
// MdvVerify.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2006
//
///////////////////////////////////////////////////////////////
//
// MdvVerify performs statistical verification of gridded data
// in MDV files.
//
////////////////////////////////////////////////////////////////

#include <toolsa/pmu.h>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxRemapLut.hh>
#include "MdvVerify.hh"
#include "Contingency.hh"
#include "Histogram.hh"
#include "Regression.hh"
using namespace std;

// Constructor

MdvVerify::MdvVerify(int argc, char **argv)

{
  
  isOK = true;

  // set programe name

  _progName = "MdvVerify";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = "unknown";
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

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  // initialize the data input object

  if (_params.mode == Params::REALTIME) {
    if (_input.setRealtime(_params.target.url, 600,
			   PMU_auto_register)) {
      isOK = false;
    }
  } else if (_params.mode == Params::ARCHIVE) {
    if (_input.setArchive(_params.target.url,
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

MdvVerify::~MdvVerify()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int MdvVerify::Run ()
{

  int iret = 0;

  // register with procmap
  
  PMU_auto_register("Run");

  // create computation object

  Comps *comps = NULL;
  if (_params.method == Params::CONTINGENCY) {
    comps = new Contingency(_progName, _params);
  } else if (_params.method == Params::HISTOGRAM) {
    comps = new Histogram(_progName, _params);
  } else if (_params.method == Params::REGRESSION) {
    comps = new Regression(_progName, _params);
  }
  
  // create target DsMdvx object
  
  DsMdvx target;

  // loop until end of data

  while (!_input.endOfData()) {

    PMU_auto_register("In main loop");
    
    // set up the Mdvx read

    _setupTargetRead(target);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Read details for target file" << endl;
      target.printReadRequest(cerr);
    }
    
    // read the volume
    
    PMU_auto_register("Before read");
    if (_input.readVolumeNext(target)) {
      cerr << "ERROR - MdvVerify::Run" << endl;
      cerr << "  Cannot read in data." << endl;
      cerr << _input.getErrStr() << endl;
      iret = -1;
      continue;
    }
    
    if (_params.debug) {
      cerr << "Got target data from file:" << endl;
      cerr << "  " << target.getPathInUse() << endl;
      cerr << "  Field: " << _params.target.field_name << endl;
    }

    // set up read for truth data

    DsMdvx truth;
    _setupTruthRead(target, truth);
    
    // read truth data

    if (truth.readVolume()) {
      if (_params.debug) {
        cerr << "ERROR - MdvVerify::Run" << endl;
        cerr << "  Cannot read in truth data." << endl;
        cerr << _input.getErrStr() << endl;
        truth.printReadRequest(cerr);
        continue;
      }
    }

    if (_params.debug) {
      cerr << "Got truth data from file:" << endl;
      cerr << "  " << truth.getPathInUse() << endl;
      cerr << "  Field: " << _params.truth.field_name << endl;
    }

    if (_verify(target, truth, *comps)) {
      cerr << "ERROR - MdvVerify::Run" << endl;
      cerr << "  Verification failed." << endl;
      iret = -1;
      continue;
    }

  } // while

  // print out

  comps->print(cout);
  delete comps;

  return iret;

}

//////////////////////////////////
// set up read for target data set

void MdvVerify::_setupTargetRead(DsMdvx &target)
  
{
  
  target.clearRead();
  
  target.addReadField(_params.target.field_name);

  if (_params.target.set_vlevel_limits) {
    target.setReadVlevelLimits(_params.target.lower_vlevel,
                               _params.target.upper_vlevel);
  }

  if (_params.target.compute_composite) {
    target.setReadComposite();
  }

  target.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  target.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  if (_params.remap_xy) {
    if (_params.remap_projection == Params::PROJ_LATLON) {
      target.setReadRemapLatlon(_params.remap_grid.nx,
                                _params.remap_grid.ny,
                                _params.remap_grid.minx,
                                _params.remap_grid.miny,
                                _params.remap_grid.dx,
                                _params.remap_grid.dy);
    } else if (_params.remap_projection == Params::PROJ_LAMBERT_CONF) {
      target.setReadRemapLc2(_params.remap_grid.nx,
                             _params.remap_grid.ny,
                             _params.remap_grid.minx,
                             _params.remap_grid.miny,
                             _params.remap_grid.dx,
                             _params.remap_grid.dy,
                             _params.remap_origin_lat,
                             _params.remap_origin_lon,
                             _params.remap_lambert_lat1,
                             _params.remap_lambert_lat2);
    } else if (_params.remap_projection == Params::PROJ_OBLIQUE_STEREO) {
      target.setReadRemapObliqueStereo(_params.remap_grid.nx,
                                       _params.remap_grid.ny,
                                       _params.remap_grid.minx,
                                       _params.remap_grid.miny,
                                       _params.remap_grid.dx,
                                       _params.remap_grid.dy,
                                       _params.remap_origin_lat,
                                       _params.remap_origin_lon,
                                       _params.remap_stereo_tangent_lat,
                                       _params.remap_stereo_tangent_lon,
                                       _params.remap_stereo_central_scale);
    } else if (_params.remap_projection == Params::PROJ_FLAT) {
      target.setReadRemapFlat(_params.remap_grid.nx,
                              _params.remap_grid.ny,
                              _params.remap_grid.minx,
                              _params.remap_grid.miny,
                              _params.remap_grid.dx,
                              _params.remap_grid.dy,
                              _params.remap_origin_lat,
                              _params.remap_origin_lon,
                              0.0);
    }
  } // if (_params.remap_xy)
  
}

/////////////////////////////////
// set up read for truth data set

void MdvVerify::_setupTruthRead(const DsMdvx &target,
                                DsMdvx &truth)
  
{

  truth.clearRead();
  
  time_t targetTime = target.getMasterHeader().time_centroid;
  time_t truthSearchTime = targetTime + _params.truth_offset_secs;

  truth.setReadTime(Mdvx::READ_CLOSEST,
                    _params.truth.url,
                    _params.truth_margin_secs,
                    truthSearchTime);
  
  truth.addReadField(_params.truth.field_name);
  
  if (_params.truth.set_vlevel_limits) {
    truth.setReadVlevelLimits(_params.truth.lower_vlevel,
                              _params.truth.upper_vlevel);
  }
  
  if (_params.truth.compute_composite) {
    truth.setReadComposite();
  }

  truth.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  truth.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  if (_params.remap_xy) {
    if (_params.remap_projection == Params::PROJ_LATLON) {
      truth.setReadRemapLatlon(_params.remap_grid.nx,
                               _params.remap_grid.ny,
                               _params.remap_grid.minx,
                               _params.remap_grid.miny,
                               _params.remap_grid.dx,
                               _params.remap_grid.dy);
    } else if (_params.remap_projection == Params::PROJ_LAMBERT_CONF) {
      truth.setReadRemapLc2(_params.remap_grid.nx,
                            _params.remap_grid.ny,
                            _params.remap_grid.minx,
                            _params.remap_grid.miny,
                            _params.remap_grid.dx,
                            _params.remap_grid.dy,
                            _params.remap_origin_lat,
                            _params.remap_origin_lon,
                            _params.remap_lambert_lat1,
                            _params.remap_lambert_lat2);
    } else if (_params.remap_projection == Params::PROJ_OBLIQUE_STEREO) {
      truth.setReadRemapObliqueStereo(_params.remap_grid.nx,
                                      _params.remap_grid.ny,
                                      _params.remap_grid.minx,
                                      _params.remap_grid.miny,
                                      _params.remap_grid.dx,
                                      _params.remap_grid.dy,
                                      _params.remap_origin_lat,
                                      _params.remap_origin_lon,
                                      _params.remap_stereo_tangent_lat,
                                      _params.remap_stereo_tangent_lon,
                                      _params.remap_stereo_central_scale);
    } else if (_params.remap_projection == Params::PROJ_FLAT) {
      truth.setReadRemapFlat(_params.remap_grid.nx,
                             _params.remap_grid.ny,
                             _params.remap_grid.minx,
                             _params.remap_grid.miny,
                             _params.remap_grid.dx,
                             _params.remap_grid.dy,
                             _params.remap_origin_lat,
                             _params.remap_origin_lon,
                             0.0);
    }
  } // if (_params.remap_xy)
  
}

int MdvVerify::_verify(const DsMdvx &target,
                       const DsMdvx &truth,
                       Comps &comps)
  
{


  // check we have the same basic geometry

  const MdvxField *targetFld = target.getField(_params.target.field_name);
  if (targetFld == NULL) {
    cerr << "ERROR - MdvVerify::_verify" << endl;
    cerr << "  Cannot find target field: " << _params.target.field_name << endl;
    cerr << "  File: " << target.getPathInUse() << endl;
    return -1;
  }

  const MdvxField *truthFld = truth.getField(_params.truth.field_name);
  if (truthFld == NULL) {
    cerr << "ERROR - MdvVerify::_verify" << endl;
    cerr << "  Cannot find truth field: " << _params.truth.field_name << endl;
    cerr << "  File: " << truth.getPathInUse() << endl;
    return -1;
  }

  const Mdvx::field_header_t &targetFhdr = targetFld->getFieldHeader();
  const Mdvx::field_header_t &truthFhdr = truthFld->getFieldHeader();

  if (targetFhdr.nx != truthFhdr.nx ||
      targetFhdr.ny != truthFhdr.ny ||
      targetFhdr.nz != truthFhdr.nz) {
    cerr << "ERROR - MdvVerify::_verify" << endl;
    cerr << "  Target and truth grids not the same size" << endl;
    cerr << "---->> Target field:" << endl;
    Mdvx::printFieldHeaderSummary(targetFhdr, cerr);
    cerr << "---->> Truth field:" << endl;
    Mdvx::printFieldHeaderSummary(truthFhdr, cerr);
    return -1;
  }

  comps.update(*targetFld, *truthFld);

  return 0;

}

