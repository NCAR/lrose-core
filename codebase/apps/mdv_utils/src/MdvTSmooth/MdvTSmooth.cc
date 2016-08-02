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
// MdvTSmooth.cc
//
// MdvTSmooth object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2000
//
///////////////////////////////////////////////////////////////
//
// Performs temporal smoothing on Mdv file data
//
///////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/MemBuf.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxRemapLut.hh>
#include "MdvTSmooth.hh"
using namespace std;

// Constructor

MdvTSmooth::MdvTSmooth(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "MdvTSmooth";
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

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  // initialize the data input object

  if (_params.mode == Params::REALTIME) {
    if (_input.setRealtime(_params.input_url, 600,
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

MdvTSmooth::~MdvTSmooth()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int MdvTSmooth::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");
  
  // create DsMdvx object
  
  DsMdvx inMdvx;
  inMdvx.setDebug(_params.debug);

  // loop until end of data

  while (!_input.endOfData()) {

    PMU_auto_register("In main loop");
    
    // set up the Mdvx read

    _setupRead(inMdvx, true);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Read for main file" << endl;
      inMdvx.printReadRequest(cerr);
    }

    // read the volume

    PMU_auto_register("Before read");
    if (_input.readVolumeNext(inMdvx)) {
      cerr << "ERROR - MdvTSmooth::Run" << endl;
      cerr << "  Cannot read in data." << endl;
      cerr << _input.getErrStr() << endl;
      continue;
    }

    if (inMdvx.getMasterHeader().n_fields == 0) {
      cerr << "ERROR - MdvTSmooth::Run" << endl;
      cerr << "  No fields in input data, cannot proceed." << endl;
      continue;
    }

    if (_params.debug) {
      cerr << "Working on file: " << inMdvx.getPathInUse() << endl;
    }

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Input file fields:" << endl;
      for (int ifield = 0;
	   ifield < inMdvx.getMasterHeader().n_fields; ifield++) {
	MdvxField *fld = inMdvx.getField(ifield);
	const Mdvx::field_header_t &fhdr = fld->getFieldHeader();
	cerr << "Field " << ifield << ", name " << fhdr.field_name << endl;
      }
    }

    // copy to the working Mdvx

    DsMdvx working(inMdvx);

    // smooth

    if (_smooth(working)) {
      cerr << "ERROR - MdvTSmooth::Run" << endl;
      cerr << "  Cannot perform smoothing." << endl;
      continue;
    }

    // write out

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Output file fields:" << endl;
      for (int ifield = 0;
	   ifield < working.getMasterHeader().n_fields; ifield++) {
	MdvxField *fld = working.getField(ifield);
	const Mdvx::field_header_t &fhdr = fld->getFieldHeader();
	cerr << "Field " << ifield << ", name " << fhdr.field_name << endl;
      }
    }

    PMU_auto_register("Before write");
    working.setWriteLdataInfo();
    working.setAppName(_progName);
    if(working.writeToDir(_params.output_url)) {
      cerr << "ERROR - MdvTSmooth::Run" << endl;
      cerr << "  Cannot write data set." << endl;
      cerr << working.getErrStr() << endl;
      continue;
    }

  } // while

  return 0;

}

void MdvTSmooth::_setupRead(DsMdvx &mdvx,
			    bool read_unsmoothed)

{

  mdvx.clearRead();
  
  if (_params.set_vlevel_limits) {
    mdvx.setReadVlevelLimits(_params.lower_vlevel,
			     _params.upper_vlevel);
  } else if (_params.composite) {
    mdvx.setReadComposite();
  }
  mdvx.setReadEncodingType(Mdvx::ENCODING_ASIS);
  mdvx.setReadCompressionType(Mdvx::COMPRESSION_ASIS);

  if (read_unsmoothed) {
    for (int i = 0; i < _params.field_names_unsmoothed_n; i++) {
      mdvx.addReadField(_params._field_names_unsmoothed[i]);
    }
  }
  for (int i = 0; i < _params.field_names_smoothed_n; i++) {
    mdvx.addReadField(_params._field_names_smoothed[i]);
  }

  if (_params.remap_xy) {
    if (_params.remap_projection == Params::PROJ_LATLON) {
      mdvx.setReadRemapLatlon(_params.remap_grid.nx,
			      _params.remap_grid.ny,
			      _params.remap_grid.minx,
			      _params.remap_grid.miny,
			      _params.remap_grid.dx,
			      _params.remap_grid.dy);
    } else if (_params.remap_projection == Params::PROJ_LAMBERT_CONF) {
      mdvx.setReadRemapLc2(_params.remap_grid.nx,
			   _params.remap_grid.ny,
			   _params.remap_grid.minx,
			   _params.remap_grid.miny,
			   _params.remap_grid.dx,
			   _params.remap_grid.dy,
			   _params.remap_origin_lat,
			   _params.remap_origin_lon,
			   _params.remap_lat1,
			   _params.remap_lat2);
    } else if (_params.remap_projection == Params::PROJ_FLAT) {
      mdvx.setReadRemapFlat(_params.remap_grid.nx,
			    _params.remap_grid.ny,
			    _params.remap_grid.minx,
			    _params.remap_grid.miny,
			    _params.remap_grid.dx,
			    _params.remap_grid.dy,
			    _params.remap_origin_lat,
			    _params.remap_origin_lon,
			    _params.remap_rotation);
    }
  } // if (_params.remap_xy)
  
}

int MdvTSmooth::_smooth(DsMdvx &working)
  
{

  // get the available data times

  DsMdvx times;
  times.clearTimeListMode();
  const Mdvx::master_header_t &mhdr = working.getMasterHeader();
  times.setTimeListModeValid(_params.input_url,
			     mhdr.time_centroid - _params.smoothing_period,
			     mhdr.time_centroid - 1);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    times.printTimeListRequest(cerr);
  }

  if (times.compileTimeList()) {
    cerr << "ERROR - MdvTSmooth::_smooth()" << endl;
    cerr << "  Cannot compile time list" << endl;
    cerr << times.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    for (int i = 0; i < times.getNTimesInList(); i++) {
      cerr << "  Time " << i << ": " <<
	DateTime::str(times.getTimeFromList(i)) << endl;
    }
  }

  if (times.getNTimesInList() == 0) {
    if (_params.debug) {
      cerr << "  No times found for smoothing" << endl;
    }
    return -1;
  }

  // check for uniform length fields
  
  int nPointsField = 0;
  if (!_uniformFields(working, nPointsField)) {
    return -1;
  }

  // set up the array buffers for summing up the field
  // data and weights

  int nBytesField = nPointsField * sizeof(fl32);
  int nFieldsSmooth = _params.field_names_smoothed_n;
  int nBytesWork = 2 * nFieldsSmooth * nBytesField;
  
  MemBuf ptrBuf;
  ptrBuf.reserve(2 * nFieldsSmooth * sizeof(fl32 **));

  fl32 **sum = (fl32 **) ptrBuf.getPtr();
  fl32 **weights = sum + nFieldsSmooth;

  MemBuf workBuf;
  workBuf.reserve(nBytesWork);
  memset(workBuf.getPtr(), 0, nBytesWork);
  
  for (int i = 0; i < nFieldsSmooth; i++) {
    sum[i] = (fl32 *) workBuf.getPtr() + i * nPointsField;
    weights[i] = sum[i] + nFieldsSmooth * nPointsField;
  }

  // initialize with current data

  fl32 currentWt = _params.current_weighting_factor;

  for (int ifield = 0; ifield < nFieldsSmooth; ifield++) {
    
    MdvxField *fld =
      working.getField(_params._field_names_smoothed[ifield]);
    const Mdvx::field_header_t &fhdr = fld->getFieldHeader();

    fld->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
    
    fl32 *ss = sum[ifield];
    fl32 *ww = weights[ifield];
    fl32 *ff = (fl32 *) fld->getVol();
    fl32 missing = fhdr.missing_data_value;
    fl32 bad = fhdr.bad_data_value;
    
    for (int j = 0; j < nPointsField; j++, ss++, ww++, ff++) {
      fl32 fff = *ff;
      if (fff != missing && fff != bad) {
	*ss += fff * currentWt;
	*ww += currentWt;
      }
    } // j
    
  } // ifield
  
  // read the past files, summing with weights

  DsMdvx past;

  for (int itime = 0; itime < times.getNTimesInList(); itime++) {

    time_t pastTime = times.getTimeFromList(itime);

    // set up the read
    
    _setupRead(past, false);
    past.setReadTime(Mdvx::READ_CLOSEST, _params.input_url, 0, pastTime);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Read for past file, time: "
	   << DateTime::str(pastTime) << endl;
      past.printReadRequest(cerr);
    }
    
    // read the volume
    
    PMU_auto_register("Before read past");
    if (past.readVolume()) {
      cerr << "ERROR - MdvTSmooth::_smooth()" << endl;
      cerr << "  Cannot read volume" << endl;
      cerr << past.getErrStr() << endl;
      return -1;
    }
    
    const Mdvx::master_header_t &pmhdr = past.getMasterHeader();

    int lagTime = mhdr.time_centroid - pmhdr.time_centroid;
    double wt = _params.current_weighting_factor +
      (lagTime / 3600.0) * _params.past_weight_slope;

    if (_params.debug) {
      cerr << "Read past file, time: "
	   << DateTime::str(pmhdr.time_centroid) << endl;
      cerr << "  lag time (secs): " << lagTime << endl;
      cerr << "  weight: " << wt << endl;
    }

    // go through the fields, summing

    for (int ifield = 0; ifield < nFieldsSmooth; ifield++) {

      MdvxField *pfld =
	past.getField(_params._field_names_smoothed[ifield]);
      pfld->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);

      const Mdvx::field_header_t &pfhdr = pfld->getFieldHeader();
      int nPoints = pfhdr.nx * pfhdr.ny * pfhdr.nz;

      if (nPoints != nPointsField) {
	cerr << "ERROR - MdvTSmooth::_smooth()" << endl;
	cerr << "  Fields are not uniform in length" << endl;
	cerr << "  Npoints in first field: " << nPointsField << endl;
	cerr << "  Npoints in field " << ifield << ": " << nPoints << endl;
	cerr << "  File: " << past.getPathInUse() << endl;
	return -1;
      }

      fl32 *ss = sum[ifield];
      fl32 *ww = weights[ifield];
      fl32 *ff = (fl32 *) pfld->getVol();
      fl32 missing = pfhdr.missing_data_value;
      fl32 bad = pfhdr.bad_data_value;
      
      for (int j = 0; j < nPointsField; j++, ss++, ww++, ff++) {
	fl32 fff = *ff;
	if (fff != missing && fff != bad) {
	  *ss += fff * wt;
	  *ww += wt;
	}
      } // j

    } // ifield
    
  } // itime

  // normalize with respect to the weights
  // If the weight is not at least as large as the current wt,
  // the field is set to missing at that point

  for (int ifield = 0; ifield < nFieldsSmooth; ifield++) {

    MdvxField *fld =
      working.getField(_params._field_names_smoothed[ifield]);
    const Mdvx::field_header_t &fhdr = fld->getFieldHeader();
    
    fl32 *ss = sum[ifield];
    fl32 *ww = weights[ifield];
    fl32 *ff = (fl32 *) fld->getVol();
    fl32 missing = fhdr.missing_data_value;

    for (int j = 0; j < nPointsField; j++, ss++, ww++, ff++) {
      if (*ww < currentWt) {
	*ff = missing;
      } else {
	*ff = *ss / *ww;
      }
    } // j
    
    // convert output

    fld->computeMinAndMax(true);
    fld->convertRounded((Mdvx::encoding_type_t) _params.output_encoding_type,
			Mdvx::COMPRESSION_ZLIB);

  } // ifield

  return 0;

}

///////////////////////////////////////////
// check that the fields are uniform in size

bool MdvTSmooth::_uniformFields(DsMdvx &working, int &n_points)
  
{

  n_points = 0;

  const Mdvx::master_header_t &mhdr = working.getMasterHeader();
  const MdvxField *fld0 = working.getField(0);
  const Mdvx::field_header_t &fhdr0 = fld0->getFieldHeader();
  int nPoints0 = fhdr0.nx * fhdr0.ny * fhdr0.nz;

  for (int i = 1; i < mhdr.n_fields; i++) {
    const MdvxField *fld = working.getField(i);
    const Mdvx::field_header_t &fhdr = fld->getFieldHeader();
    int nPoints = fhdr.nx * fhdr.ny * fhdr.nz;
    if (nPoints != nPoints0) {
      cerr << "ERROR - MdvTSmooth::_uniformFields()" << endl;
      cerr << "  Fields are not uniform in length" << endl;
      cerr << "  Npoints in first field: " << nPoints0 << endl;
      cerr << "  Npoints in field " << i << ": " << nPoints0 << endl;
      cerr << "  File: " << working.getPathInUse() << endl;
      return false;
    }
  }

  n_points = nPoints0;
  return true;

}

