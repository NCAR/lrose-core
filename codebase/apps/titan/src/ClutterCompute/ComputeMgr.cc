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
//////////////////////////////////////////////////////////
// ComputeMgr.cc
//
// Manages the computations for median
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2002
//
//////////////////////////////////////////////////////////

#include <Mdv/MdvxField.hh>
#include <toolsa/pmu.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/TaArray.hh>
#include "ComputeMgr.hh"
using namespace std;

// constructor

ComputeMgr::ComputeMgr(const Params &params) :
  _params(params)
{
  _nPointsPerPass = _params.npoints_per_pass;
  _nPointsField = 0;
  _nVolumes = 0;
  _startTime = 0;
  _endTime = 0;
  _nFields = _params.field_names_n;
  for (int i = 0; i < _nFields; i++) {
    _fieldMedians.push_back(NULL);
  }
  for (int i = 0; i < _nFields; i++) {
    _fieldVolumes.push_back(NULL);
  }
}

// destructor

ComputeMgr::~ComputeMgr()
{
  for (size_t ii = 0; ii < _fieldMedians.size(); ii++) {
    if (_fieldMedians[ii]) {
      delete[] _fieldMedians[ii];
    };
  }
  for (size_t ii = 0; ii < _fieldVolumes.size(); ii++) {
    if (_fieldVolumes[ii]) {
      delete[] _fieldVolumes[ii];
    };
  }
}

//////////////////////////////////////////////////
// scan the files, make sure they are OK,
// set up constants etc.

int ComputeMgr::scanFiles(DsMdvxInput &input)

{

  // register with procmap
  
  PMU_auto_register("ComputeMgr::scanFiles");

  if (_params.debug) {
    cerr << "Scanning input files ... " << endl;
    cerr << "  Looking for fields:" << endl;
    for (int ii = 0; ii < _params.field_names_n; ii++) {
      cerr << "  Field: " << ii << ", name: "
	   << _params._field_names[ii] << endl;
    }
  }
  
  // create input DsMdvx object
  
  DsMdvx inMdvx;
  inMdvx.setDebug(_params.debug);
  
  // loop through files, checking them

  _nVolumes = 0;
  input.reset();
  while (!input.endOfData()) {
    
    PMU_auto_register("Scanning files");
    
    // set up the Mdvx read
    
    _setupRead(inMdvx);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Read request for MDV data" << endl;
      inMdvx.printReadRequest(cerr);
    }
    
    // read the volume
    
    PMU_auto_register("Before read");
    if (input.readVolumeNext(inMdvx)) {
      cerr << "ERROR - ComputeMgr::scanFiles" << endl;
      cerr << "  Cannot read in data." << endl;
      cerr << input.getErrStr() << endl;
      return -1;
    }
    
    if (_params.debug) {
      cerr << "  Read in file: " << inMdvx.getPathInUse() << endl;
    }

    const Mdvx::master_header_t &mhdr = inMdvx.getMasterHeader();
    
    if ((int) inMdvx.getNFields() != _params.field_names_n) {
      cerr << "ERROR - ComputeMgr::scanFiles" << endl;
      cerr << "  Wrong number of fields in input data" << endl;
      cerr << "  Expecting: " << _params.field_names_n << endl;
      cerr << "  Got: " << inMdvx.getNFields() << endl;
      return -1;
    }
    
    if (_nVolumes == 0) {
      _startTime = mhdr.time_centroid;
      const MdvxField *fld = inMdvx.getField(0);
      const Mdvx::field_header_t &fhdr = fld->getFieldHeader();
      _nPointsField = fhdr.nx * fhdr.ny * fhdr.nz;
      if (_params.debug) {
	cerr << "  First vol:" << endl;
	cerr << "    nx, ny, nz: " << fhdr.nx << ", "
	     << fhdr.ny << ", " << fhdr.nz << endl;
	cerr << "    nPointsField: " << _nPointsField << endl;
      }
    }
    
    // check sizes
    
    for (size_t i = 0; i < inMdvx.getNFields(); i++) {
      const MdvxField *fld = inMdvx.getField(i);
      const Mdvx::field_header_t &fhdr = fld->getFieldHeader();
      int nPoints = fhdr.nx * fhdr.ny * fhdr.nz;
      if (nPoints != _nPointsField) {
	cerr << "ERROR - ComputeMgr::scanFiles" << endl;
	cerr << "  Incorrect number of points in field: "
	     << fhdr.field_name << endl;
	cerr << "  Expecting: " << _nPointsField << endl;
	cerr << "  Found: " << nPoints << endl;
	cerr << "nx, ny, nz: " << fhdr.nx << ", "
	     << fhdr.ny << ", " << fhdr.nz << endl;
	return -1;
      }
    }

    if (_endTime == 0) {
      _endTime = mhdr.time_centroid;
    } else { 
      _endTime = MAX(_endTime, mhdr.time_centroid);
    }
    _nVolumes++;

  } // while

  return 0;

}

//////////////////////////////////////////////////
// Allocate the arrays
//
// scanFiles must be called first

int ComputeMgr::allocArrays()

{

  if (_nVolumes == 0) {
    cerr << "ERROR - ComputeMgr::allocArrays()" << endl;
    cerr << "  No volumes" << endl;
    return -1;
  }

  if (_nPointsField == 0) {
    cerr << "ERROR - ComputeMgr::allocArrays()" << endl;
    cerr << "  No points in vol" << endl;
    return -1;
  }
  
  for (size_t ii = 0; ii < _fieldMedians.size(); ii++) {
    if (_fieldMedians[ii]) {
      delete[] _fieldMedians[ii];
    };
    _fieldMedians[ii] = new fl32[_nPointsField];
  }

  for (size_t ii = 0; ii < _fieldVolumes.size(); ii++) {
    if (_fieldVolumes[ii]) {
      delete[] _fieldVolumes[ii];
    };
    _fieldVolumes[ii] = new fl32[_nVolumes * _nPointsPerPass];
  }

  return 0;

}

//////////////////////////////////////////////////
// Compute the medians, load up the field arrays

int ComputeMgr::computeMedian(DsMdvxInput &input)
  
{

  // register with procmap
  
  PMU_auto_register("ComputeMgr::computeMedian");
  
  // create input DsMdvx object
  
  DsMdvx inMdvx;
  inMdvx.setDebug(_params.debug);

  // compute number of passes

  int nPasses = ((_nPointsField  - 1 ) / _nPointsPerPass) + 1;

  for (int ipass = 0; ipass < nPasses; ipass++) {

    // compute the start and end point for this pass

    int startPoint = ipass * _nPointsPerPass;
    int endPoint = startPoint + _nPointsPerPass - 1;
    if (endPoint > _nPointsField - 1) {
      endPoint = _nPointsField - 1;
    }
    int nPointsThisPass = endPoint - startPoint + 1;

    if (_params.debug) {
      cerr << "Pass number: " << ipass + 1 << " of " << nPasses << endl;
      cerr << "  Start point: " << startPoint << endl;
      cerr << "  End point: " << endPoint << endl;
      cerr << "  nPointsThisPass: " << nPointsThisPass << endl;
    }
    
    // loop through files

    int ivol = 0;
    input.reset();
    while (!input.endOfData()) {
      
      PMU_auto_register("computeMedian");
      
      // set up the Mdvx read
      
      _setupRead(inMdvx);
      
      // read the volume
      
      PMU_auto_register("Before read");
      if (input.readVolumeNext(inMdvx)) {
	cerr << "ERROR - ComputeMgr::computeMedian" << endl;
	cerr << "  Cannot read in data." << endl;
	cerr << input.getErrStr() << endl;
	return -1;
      }
      
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "  Read in file: " << inMdvx.getPathInUse() << endl;
      }

      // add this file to the vol data
      
      for (int ii = 0; ii < _nFields; ii++) {
	const MdvxField *fld = inMdvx.getField(ii);
	fl32 *fdata = (fl32 *) fld->getVol() + startPoint;
	fl32 *vdata = _fieldVolumes[ii] + ivol * _nPointsPerPass;
	memcpy(vdata, fdata, nPointsThisPass * sizeof(fl32));
      }

      ivol++;
      
    } // while

    // compute median values into field arrays
    
    int medianPos = _nVolumes / 2;
    
    for (size_t ii = 0; ii < _fieldMedians.size(); ii++) {
    
      fl32 *med = _fieldMedians[ii] + startPoint;
      fl32 *vdata = _fieldVolumes[ii];
      TaArray<fl32> point_;
      fl32 *point = point_.alloc(_nVolumes);
      
      for (int ip = 0; ip < nPointsThisPass; ip++) {
	
	// load up 1D array for this point

	fl32 *vptr = vdata + ip;
	for (int iv = 0; iv < _nVolumes; iv++, vptr += _nPointsPerPass) {
	  point[iv] = *vptr;
	} // iv

	// sort the array

	qsort(point, _nVolumes, sizeof(fl32), _fl32Compare);
	med[ip] = point[medianPos];

      }

    } // ii

  } // ipass

  return 0;

}

//////////////////////////////////////////////////
// Write the medians to the output file

int ComputeMgr::writeOutput(DsMdvxInput &input)

{

  // register with procmap
  
  PMU_auto_register("ComputeMgr::writeOutput");
  
  // create input DsMdvx object
  
  DsMdvx inMdvx;
  inMdvx.setDebug(_params.debug);

  // reread the first file

  input.reset();
  _setupRead(inMdvx);
  if (input.readVolumeNext(inMdvx)) {
    cerr << "ERROR - ComputeMgr::writeOutput" << endl;
    cerr << "  Cannot read in data." << endl;
    cerr << input.getErrStr() << endl;
    return -1;
  }
  
  // copy the first file to an output object
  
  DsMdvx outMdvx = inMdvx;
  
  // copy the median data into the fields of the output object
  
  for (size_t ii = 0; ii < _fieldMedians.size(); ii++) {
    
    MdvxField *fld = outMdvx.getField(ii);
    const Mdvx::field_header_t &fhdr = fld->getFieldHeader();
    
    string name = fhdr.field_name;
    name += "Med";
    fld->setFieldName(name.c_str());
    
    string long_name = fhdr.field_name_long;
    long_name += "_median";
    fld->setFieldNameLong(long_name.c_str());
    
    fl32 *fptr = (fl32 *) fld->getVol();
    fl32 *med = _fieldMedians[ii];
    memcpy(fptr, med, _nPointsField * sizeof(fl32));
    
  } // ii

  // set the master header
  
  Mdvx::master_header_t mhdr = outMdvx.getMasterHeader();
  mhdr.time_begin = _startTime;
  mhdr.time_centroid = _endTime;
  mhdr.time_end = _endTime;
  string info = mhdr.data_set_info;
  info += "\nMedian values computed using ClutterCompute.";

  outMdvx.setMasterHeader(mhdr);
  outMdvx.setDataSetInfo(info.c_str());

  // write out the file

  PMU_auto_register("Before write");
  outMdvx.setWriteLdataInfo();
  outMdvx.setDebug(_params.debug);
  if (_params.write_to_path) {
    if(outMdvx.writeToPath(_params.output_path)) {
      cerr << "ERROR - ComputeMgr::writeOutput" << endl;
      cerr << "  Cannot write median file, url:"
	   << _params.output_path << endl;
      cerr << outMdvx.getErrStr() << endl;
      return -1;
    }
  }
  if (_params.write_to_dir) {
    if(outMdvx.writeToDir(_params.output_dir)) {
      cerr << "ERROR - ComputeMgr::writeOutput" << endl;
      cerr << "  Cannot write median file, url:"
	   << _params.output_dir << endl;
      cerr << outMdvx.getErrStr() << endl;
      return -1;
    }
  }

  return 0;

}

void ComputeMgr::_setupRead(DsMdvx &mdvx)
  
{
  
  mdvx.clearRead();
  
  if (_params.set_vlevel_limits) {
    mdvx.setReadVlevelLimits(_params.lower_vlevel,
			     _params.upper_vlevel);
  }
  if (_params.composite) {
    mdvx.setReadComposite();
  }
  mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  
  for (int i = 0; i < _params.field_names_n; i++) {
    mdvx.addReadField(_params._field_names[i]);
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

/*****************************************************************************
 * define function to be used for sorting
 */

int ComputeMgr::_fl32Compare(const void *i, const void *j)
{
  fl32 *f1 = (fl32 *) i;
  fl32 *f2 = (fl32 *) j;
  if (*f1 < *f2) {
    return -1;
  } else if (*f1 > *f2) {
    return 1;
  } else {
    return 0;
  }
}

