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
/////////////////////////////////////////////////////////////
// OpticalFlow.cc
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2005
//
///////////////////////////////////////////////////////////////
//
// OpticalFlow performs optical flow tracking on MDV fields
// separated in time.
//
// The objective is to estimate the 2D velocity of the field.
//
// Output is the original tracked field, plus the U,V components
// of the velocity.
//
// The optical flow code was provided courtesy of Alan Seed of the
// Australian Bureau or Meteorology.
//
////////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/MemBuf.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxRemapLut.hh>
#include "OpticalFlow.hh"
#include "optical_flow.h"
#include "array_utils.h"
using namespace std;
using namespace ancilla;

// Constructor

OpticalFlow::OpticalFlow(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "OpticalFlow";
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

OpticalFlow::~OpticalFlow()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int OpticalFlow::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");
  
  // create DsMdvx object
  
  DsMdvx current, previous;
  current.setDebug(_params.debug);
  previous.setDebug(_params.debug);

  // loop until end of data

  int count = 0;
  int iret = 0;
  while (!_input.endOfData()) {
    
    PMU_auto_register("In main loop");
    count++;
    
    // set up the Mdvx read for the current file

    _setupCurrRead(current);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Read for current file" << endl;
      current.printReadRequest(cerr);
    }

    // read the current file

    PMU_auto_register("Before read");
    if (_input.readVolumeNext(current)) {
      cerr << "ERROR - OpticalFlow::Run" << endl;
      cerr << "  Cannot read in current file." << endl;
      cerr << _input.getErrStr() << endl;
      continue;
    }
    time_t currentTime = current.getMasterHeader().time_centroid;
    
    // set up the Mdvx read for the previous file

    time_t searchTime = currentTime - _params.min_time_between_files;
    int searchMargin =
      _params.max_time_between_files - _params.min_time_between_files;

    _setupPrevRead(previous);
    previous.setReadTime(Mdvx::READ_FIRST_BEFORE,
                         _params.input_url,
                         searchMargin,
                         searchTime);
    
    // read the previous file

    PMU_auto_register("Before read");
    if (previous.readVolume()) {
      if (count > 1) {
        cerr << "ERROR - OpticalFlow::Run" << endl;
        cerr << "  Cannot read in previous file." << endl;
        cerr << previous.getErrStr() << endl;
      }
      continue;
    }
    time_t previousTime = previous.getMasterHeader().time_centroid;
    
    if (_params.debug) {
      cerr << "  Previous time: " << DateTime::strm(previousTime) << endl;
      cerr << "  Current  time: " << DateTime::strm(currentTime) << endl;
      cerr << "  Read previous file: " << previous.getPathInUse() << endl;
      cerr << "  Read current  file: " << current.getPathInUse() << endl;
    }

    // process files for this time step

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "  Performing tracking ......." << endl;
    }

    _processTimeStep(previous, current);

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "  ....... done with tracking" << endl;
    }

    // write out

    if (_writeResults(current)) {
      cerr << "ERROR - OpticalFlow::Run" << endl;
      cerr << "  Cannot write output data set." << endl;
      cerr << current.getErrStr() << endl;
      iret = -1;
    }

  } // while

  return iret;

}

//////////////////////////////////////////////////
// process files for this time step

int OpticalFlow::_processTimeStep(DsMdvx &previous, DsMdvx &current)

{

  // register with procmap
  
  PMU_auto_register("Processing time step");

  // get previous data field - this is already a composite, was converted on read

  MdvxField *prevComp = previous.getField(_params.tracking_field_name);
  if (prevComp == NULL) {
    cerr << "ERROR - OpticalFlow::_processTimeStep" << endl;
    cerr << "  Cannot find tracking field: "
         << _params.tracking_field_name << endl;
    cerr << "  file: " << previous.getPathInUse();
    return -1;
  }

  // get current field - not yet composite

  MdvxField *currField = current.getField(_params.tracking_field_name);
  if (currField == NULL) {
    cerr << "ERROR - OpticalFlow::_processTimeStep" << endl;
    cerr << "  Cannot find tracking field: "
         << _params.tracking_field_name << endl;
    cerr << "  file: " << current.getPathInUse();
    return -1;
  }

  // make copy of current field, create composite
  
  MdvxField *currComp = new MdvxField(*currField);
  if (_params.set_vlevel_limits) {
    currComp->convert2Composite(_params.lower_vlevel,
                               _params.upper_vlevel);
  } else {
    currComp->convert2Composite();
  }
  string compName("Comp_");
  compName.append(_params.tracking_field_name);
  currComp->setFieldName(compName.c_str());
  
  // get dimensions

  size_t nx = prevComp->getFieldHeader().nx;
  size_t ny = prevComp->getFieldHeader().ny;
  
  // check consistency

  if ((int) nx != currComp->getFieldHeader().nx) {
    cerr << "ERROR - OpticalFlow::_processTimeStep" << endl;
    cerr << "  nx values not consistent for files to be tracked" << endl;
    cerr << "  nx prev: " << nx << endl;
    cerr << "  for file: " << previous.getPathInUse() << endl;
    cerr << "  nx curr: " << currComp->getFieldHeader().nx << endl;
    cerr << "  for file: " << current.getPathInUse() << endl;
    delete currComp;
    return -1;
  }

  if ((int) ny != currComp->getFieldHeader().ny) {
    cerr << "ERROR - OpticalFlow::_processTimeStep" << endl;
    cerr << "  ny values not consistent for files to be tracked" << endl;
    cerr << "  ny prev: " << ny << endl;
    cerr << "  for file: " << previous.getPathInUse() << endl;
    cerr << "  ny curr: " << currComp->getFieldHeader().ny << endl;
    cerr << "  for file: " << current.getPathInUse() << endl;
    delete currComp;
    return -1;
  }

  // get array pointers
  
  fl32 *prevVol = (fl32 *) prevComp->getVol();
  fl32 *currVol = (fl32 *) currComp->getVol();

  // set up arrays for algorithm

  size_t dims[2];
  dims[0] = ny;
  dims[1] = nx;

  array2<fl32> prevArray(dims);
  array2<fl32> currArray(dims);

  fl32 *prevData = prevArray.data();
  fl32 *currData = currArray.data();
  
  memcpy(prevData, prevVol, ny * nx * sizeof(fl32));
  memcpy(currData, currVol, ny * nx * sizeof(fl32));

  // replace missing data with NaNs

  fl32 missingPrev = prevComp->getFieldHeader().missing_data_value;
  fl32 missingCurr = currComp->getFieldHeader().missing_data_value;

  for (size_t ii = 0; ii < ny * nx; ii++) {
    if (prevData[ii] == missingPrev) {
      prevData[ii] = NAN;
    }
    if (currData[ii] == missingCurr) {
      currData[ii] = NAN;
    }
  }

  // get min and max so we can scale between 1 and 255
  
  fl32 minVal = 1.0e99;
  fl32 maxVal = -1.0e99;
  
  for (size_t ii = 0; ii < ny * nx; ii++) {
    fl32 val = prevData[ii];
    if (std::isnan(val)) {
      continue;
    }
    if (val < minVal) {
      minVal = val;
    }
    if (val > maxVal) {
      maxVal = val;
    }
  }

  for (size_t ii = 0; ii < ny * nx; ii++) {
    fl32 val = currData[ii];
    if (std::isnan(val)) {
      continue;
    }
    if (val < minVal) {
      minVal = val;
    }
    if (val > maxVal) {
      maxVal = val;
    }
  }

  // perform the scaling between 2.5 and 252.5

  double scale = 250.0 / (maxVal - minVal);
  double offset = 2.5;

  for (size_t ii = 0; ii < ny * nx; ii++) {
    fl32 val = prevData[ii];
    if (std::isfinite(val)) {
      prevData[ii] = (val - minVal) * scale + offset;
    }
  }
  for (size_t ii = 0; ii < ny * nx; ii++) {
    fl32 val = currData[ii];
    if (std::isfinite(val)) {
      currData[ii] = (val - minVal) * scale + offset;
    }
  }
  double threshold = (_params.tracking_threshold - minVal) * scale + offset;

  // initialize arrays to hold our output vectors
  
  array2<fl32> adv_x(dims);
  array2<fl32> adv_y(dims);
  array_utils::zero(adv_x);
  array_utils::zero(adv_y);

  // set up optical flow tracking object

  optical_flow tracker(nx,
                       ny,
                       _params.scale_factor,
                       _params.max_levels,
                       _params.window_size,
                       _params.n_iterations,
                       _params.polygon_neighborhood,
                       _params.polygon_sigma);
  
  // perform the tracking

  double background = threshold / 2.0;
  double gain = 1.0;

  tracker.determine_velocities(prevArray,
                               currArray,
                               adv_x,
                               adv_y,
                               _params.seed_with_previous_vectors,
                               background,
                               threshold,
                               gain,
                               _params.interp_over_missing_areas,
                               _params.interp_spacing,
                               _params.min_frac_bins_for_avg,
                               _params.idw_low_res_pwr,
                               _params.idw_high_res_pwr);

  // scale the velocities into m/s
  // set missing value as appropriate

  time_t prevTime = previous.getMasterHeader().time_centroid;
  time_t currTime = current.getMasterHeader().time_centroid;
  double timeDelta = (double) (currTime - prevTime);

  double xscale = (prevComp->getFieldHeader().grid_dx * 1000.0) / timeDelta;
  double yscale = (prevComp->getFieldHeader().grid_dy * 1000.0) / timeDelta;
  if(prevComp->getFieldHeader().proj_type == Mdvx::PROJ_LATLON) {
    // Base units of grid are degrees
    yscale = (prevComp->getFieldHeader().grid_dy * 1000.0 * KM_PER_DEG_AT_EQ) / timeDelta;
  }
  
  fl32 missingVal = -9999.0;
  fl32 *uu = adv_x.data();
  fl32 *vv = adv_y.data();
  size_t ii = 0;
  for (size_t yindex = 0; yindex < ny; yindex++) { // Loop through y Dim - Rows

    // Compute proper scale if LAT lon proj
    if(prevComp->getFieldHeader().proj_type == Mdvx::PROJ_LATLON) {
      // Use approx distance per degree at each latitiude (row) to compute a new xscale
      double lat = (prevComp->getFieldHeader().grid_miny +
                    (prevComp->getFieldHeader().grid_dy * yindex));
      xscale = cos(lat * DEG_TO_RAD) * yscale;
    }

    for(size_t xindex = 0; xindex < nx; xindex++) { // each column.
      ii = yindex * nx + xindex;

      fl32 velx = uu[ii];
      if (std::isfinite(velx)) {
	velx *= xscale;
	if (fabs(velx) < 1.0e-3) {
	  velx = 0.0;
	}
	uu[ii] = velx;
      } else {
	uu[ii] = missingVal;
      }

      fl32 vely = vv[ii];
      if (std::isfinite(vely)) {
	vely *= yscale;
	if (fabs(vely) < 1.0e-3) {
	  vely = 0.0;
	}
	vv[ii] = vely;
      } else {
	vv[ii] = missingVal;
      }

    } // xindex
  } // yindex

  // add U and V fields to current MDV file

  _addVelocityFields(current, *currComp, uu, vv, missingVal);

  // add tracking field
  
  if (_params.write_composite_field_to_output) {
    current.addField(currComp);
  } else {
    delete currComp;
  }
  
  return 0;

}

/////////////////////////////////////
// set up MDV read for previous data
// get composite

void OpticalFlow::_setupPrevRead(DsMdvx &mdvx)

{

  mdvx.clearRead();
  if (_params.set_vlevel_limits) {
    mdvx.setReadVlevelLimits(_params.lower_vlevel,
         		     _params.upper_vlevel);
  }
  mdvx.setReadComposite();
  mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  mdvx.addReadField(_params.tracking_field_name);

}

/////////////////////////////////////
// set up MDV read for current data
// keep as 3D

void OpticalFlow::_setupCurrRead(DsMdvx &mdvx)

{

  mdvx.clearRead();
  mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  mdvx.addReadField(_params.tracking_field_name);

  // add fields to be copied to output

  if (_params.copy_input_fields_to_output) {
    for (int ii = 0; ii < _params.copy_fields_n; ii++) {
      if (strcmp(_params._copy_fields[ii].input_name,
                 _params.tracking_field_name) != 0) {
        mdvx.addReadField(_params._copy_fields[ii].input_name);
      }
    }
  }

}

//////////////////////////////////
// add U and V fields to MDV file

void OpticalFlow::_addVelocityFields(DsMdvx &mdvx,
                                     MdvxField &trackingField,
                                     fl32 *uu, fl32 *vv,
                                     fl32 missingVal)

{

  Mdvx::field_header_t fhdr = trackingField.getFieldHeader();
  fhdr.missing_data_value = missingVal;
  fhdr.bad_data_value = missingVal;
  Mdvx::vlevel_header_t vhdr = trackingField.getVlevelHeader();
  
  MdvxField *uuField = _createField(mdvx,
                                    fhdr,
                                    vhdr,
                                    uu,
                                    "U",
                                    "X-component of velocity",
                                    "m/s");
  mdvx.addField(uuField);

  MdvxField *vvField = _createField(mdvx,
                                    fhdr,
                                    vhdr,
                                    vv,
                                    "V",
                                    "Y-component of velocity",
                                    "m/s");
  mdvx.addField(vvField);

}

//////////////////////////////////
// create MDV field

MdvxField *OpticalFlow::_createField(DsMdvx &mdvx,
                                     Mdvx::field_header_t fhdr,
                                     Mdvx::vlevel_header_t vhdr,
                                     fl32 *data,
                                     const char *name,
                                     const char *longName,
                                     const char *units)

{

  MdvxField *field = new MdvxField(fhdr, vhdr, data);
  field->setFieldName(name);
  field->setFieldNameLong(longName);
  field->setUnits(units);
  field->computeMinAndMax();

  return field;

}

////////////////////////////////////////
// write out results

int OpticalFlow::_writeResults(DsMdvx &out)

{
 
  PMU_auto_register("Before write");

  out.setWriteLdataInfo();
  out.setAppName(_progName);

  // delete input field if not required in output

  bool inputNeeded = false;
  if (_params.copy_input_fields_to_output) {
    for (int ii = 0; ii < _params.copy_fields_n; ii++) {
      if (strcmp(_params.tracking_field_name,
                 _params._copy_fields[ii].input_name) == 0) {
        inputNeeded = true;
        break;
      }
    } // ii
  }
  if (!inputNeeded) {
    MdvxField *trackingInput = out.getField(_params.tracking_field_name);
    out.deleteField(trackingInput);
  }

  // convert and compress the fields
  
  for (size_t ii = 0; ii < out.getNFields(); ii++) {
    MdvxField *field = out.getField(ii);
    if (_params.output_encoding_type == Params::ENCODING_INT8) {
      field->convertDynamic(Mdvx::ENCODING_INT8,
                            Mdvx::COMPRESSION_GZIP);
    } else if (_params.output_encoding_type == Params::ENCODING_INT16) {
      field->convertDynamic(Mdvx::ENCODING_INT16,
                            Mdvx::COMPRESSION_GZIP);
    } else {
      field->convertDynamic(Mdvx::ENCODING_FLOAT32,
                            Mdvx::COMPRESSION_GZIP);
    }
  }

  // write the file

  if(out.writeToDir(_params.output_url)) {
    cerr << "ERROR - OpticalFlow::Run" << endl;
    cerr << "  Cannot write data set." << endl;
    cerr << out.getErrStr() << endl;
    return -1;
  }
  if (_params.debug) {
    cerr << "Wrote file: " << out.getPathInUse() << endl;
  }

  return 0;

}

    
