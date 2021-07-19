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
// OdimHdf5ToMdv.cc
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2021
//
///////////////////////////////////////////////////////////////
//
// OdimHdf5ToMdv reads ODIM OPERA data in HDF5 format, and
// converts to MDV.
//
////////////////////////////////////////////////////////////////

#include <algorithm>
#include <toolsa/toolsa_macros.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/DateTime.hh>
#include <toolsa/Path.hh>
#include <toolsa/TaArray.hh>
#include <Mdv/MdvxField.hh>
#include <dsserver/DsLdataInfo.hh>
#include <euclid/search.h>
#include "OdimHdf5ToMdv.hh"
using namespace std;

const fl32 OdimHdf5ToMdv::_missingFloat = -9999.0;

// Constructor

OdimHdf5ToMdv::OdimHdf5ToMdv(int argc, char **argv)

{

  isOK = true;
  _input = NULL;

  // set programe name

  _progName = "OdimHdf5ToMdv";
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

  // check that file list set in archive mode
  
  if (_params.mode == Params::FILELIST && _args.inputFileList.size() == 0) {
    cerr << "ERROR: OdimHdf5ToMdv" << endl;
    cerr << "  Mode is ARCHIVE."; 
    cerr << "  You must use -f to specify files on the command line."
	 << endl;
    _args.usage(_progName, cerr);
    isOK = false;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		_params.reg_interval);

  // initialize the data input object
  
  if (_params.mode == Params::REALTIME) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _params.max_realtime_age,
			     PMU_auto_register,
			     _params.use_ldata_info_file,
			     _params.process_latest_file_only);
  } else if (_params.mode == Params::ARCHIVE) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _args.startTime, _args.endTime);
  } else if (_params.mode == Params::FILELIST) {
    if (_params.debug) {
      for (int ii = 0; ii < (int) _args.inputFileList.size(); ii++) {
        cerr << "Adding path: " << _args.inputFileList[ii] << endl;
      }
    }
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _args.inputFileList);
  }

  if (strlen(_params.file_name_ext) > 0) {
    _input->setSearchExt(_params.file_name_ext);
  }

  if (strlen(_params.file_name_substr) > 0) {
    _input->setSubString(_params.file_name_substr);
  }

}

// destructor

OdimHdf5ToMdv::~OdimHdf5ToMdv()

{

  // unregister process

  PMU_auto_unregister();

  // free up

  if (_input) {
    delete _input;
  }

}

//////////////////////////////////////////////////
// Run

int OdimHdf5ToMdv::Run ()
{
  
  int iret = 0;
  PMU_auto_register("Run");
  
  // create the output fields
  
  for (int ifield = 0; ifield < _params.output_fields_n; ifield++) {
    OutputField *fld = new OutputField(_params._output_fields[ifield]);
    _outputFields.push_back(fld);
  }

  // loop until end of data
  
  char *inputPath;
  while ((inputPath = _input->next()) != NULL) {
    
    PMU_auto_register("Reading file");
    ta_file_uncompress(inputPath);

    if (_processFile(inputPath)) {
      cerr << "ERROR = OdimHdf5ToMdv::Run" << endl;
      cerr << "  Processing file: " << inputPath << endl;
      iret = -1;
    }
    
    // clear the output fields
    
    for (size_t ii = 0; ii < _outputFields.size(); ii++) {
      _outputFields[ii]->clear();
    }
    
  }
  
  return iret;

}

///////////////////////////////
// process file

int OdimHdf5ToMdv::_processFile(const char *input_path)

{

  PMU_auto_register("Processing file");
  
  if (_params.debug) {
    cerr << "Processing file: " << input_path << endl;
  }

  // use try block to catch any exceptions
  
  try {
    
    // open file

    H5File file(input_path, H5F_ACC_RDONLY);
    if (_params.debug) {
      cerr << "==>> File size: " << file.getFileSize() << endl;
    }
    
    // get the root group
    
    Group root(file.openGroup("/"));
    if (_params.debug >= Params::DEBUG_EXTRA) {
      Hdf5xx::printFileStructure(root, 0, cerr);
    }

    // root attributes

    _conventions = Hdf5xx::getStringAttribute(root, "Conventions");

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Conventions: " << _conventions << endl;
    }

    // read the metadata from the main what and how groups
    
    _readMetadata(root);

    // read the where group - grid location etc.
    
    _readWhere(root);

    // read in the fields

    if (_readFields(root)) {
      cerr << "ERROR processing file: " << input_path << endl;
      cerr << "  Cannot read in the fields" << endl;
    }
    
  } // try
  
  catch (H5x::Exception &e) {
    // _addErrStr("ERROR - reading GPM HDF5 file");
    // _addErrStr(e.getDetailMsg());
    return -1;
  }

  // create output Mdvx file object
  
  DsMdvx mdvx;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    mdvx.setDebug(true);
  }
    
  // set master header
  
  _setMasterHeader(mdvx);
  
  // interpolate fields onto latlon grid, and add to the mdvx object

  for (size_t ifield = 0; ifield < _outputFields.size(); ifield++) {
    
    // get field as it was read in
    
    OutputField *fld = _outputFields[ifield];
    if (!fld->valid) {
      delete fld;
      continue;
    }
    
    // add to mdvx object
    
    _addFieldToMdvx(mdvx, fld);

    // delete the field

    delete fld;

  } // ifield
    
  // clean up

  _outputFields.clear();

  // write output file
  
  if (_params.debug) {
    cerr << "Writing file to url: " << _params.output_url << endl;
  }
  
  if (mdvx.writeToDir(_params.output_url)) {
    cerr << "ERROR - OdimHdf5ToMdv" << endl;
    cerr << "  Cannot write file to url: " << _params.output_url << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "  Wrote output file: " << mdvx.getPathInUse() << endl;
  }

  return 0;

}

//////////////////////////////////////////////
// read the main what and how group

void OdimHdf5ToMdv::_readMetadata(Group &root)
  
{

  Group what(root.openGroup("what"));

  string dateStr = Hdf5xx::getStringAttribute(what, "date");
  string timeStr = Hdf5xx::getStringAttribute(what, "time");
  _midTime.set(dateStr + timeStr);
  _version = Hdf5xx::getStringAttribute(what, "version");
  _source = Hdf5xx::getStringAttribute(what, "source");
  
  Group how(root.openGroup("how"));

  _history = Hdf5xx::getStringAttribute(how, "nodes");
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Reading main what and how groups" << endl;
    cerr << "  midTime: " << _midTime.asString() << endl;
    cerr << "  version: " << _version << endl;
    cerr << "  source: " << _source << endl;
    cerr << "  history: " << _history << endl;
  }

}

//////////////////////////////////////////////
// read the where group - grid location etc.

void OdimHdf5ToMdv::_readWhere(Group &root)
  
{

  Group where(root.openGroup("where"));

  _projStr = Hdf5xx::getStringAttribute(where, "projdef");

  _nx = Hdf5xx::getIntAttribute(where, "xsize");
  _ny = Hdf5xx::getIntAttribute(where, "ysize");

  _dxM = Hdf5xx::getDoubleAttribute(where, "xscale");
  _dyM = Hdf5xx::getDoubleAttribute(where, "yscale");

  _dxKm = _dxM / 1000.0;
  _dyKm = _dyM / 1000.0;

  _minxKm = -(_nx / 2.0) * _dxKm + _dxKm / 2.0;
  _minyKm = -(_ny / 2.0) * _dyKm + _dyKm / 2.0;

  _nz = 1; // always 2D for now
  _minzKm = _params.radar_min_z_km;
  _dzKm = _params.radar_delta_z_km;
  
  _llLon = Hdf5xx::getDoubleAttribute(where, "LL_lon");
  _llLat = Hdf5xx::getDoubleAttribute(where, "LL_lat");
  _ulLon = Hdf5xx::getDoubleAttribute(where, "UL_lon");
  _ulLat = Hdf5xx::getDoubleAttribute(where, "UL_lat");
  _urLon = Hdf5xx::getDoubleAttribute(where, "UR_lon");
  _urLat = Hdf5xx::getDoubleAttribute(where, "UR_lat");
  _lrLon = Hdf5xx::getDoubleAttribute(where, "LR_lon");
  _lrLat = Hdf5xx::getDoubleAttribute(where, "LR_lat");
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Reading where group" << endl;
    cerr << "  projStr: " << _projStr << endl;
    cerr << "  nx: " << _nx << endl;
    cerr << "  ny: " << _ny << endl;
    cerr << "  dxM: " << _dxM << endl;
    cerr << "  dyM: " << _dyM << endl;
    cerr << "  llLon: " << _llLon << endl;
    cerr << "  llLat: " << _llLat << endl;
    cerr << "  ulLon: " << _ulLon << endl;
    cerr << "  ulLat: " << _ulLat << endl;
    cerr << "  urLon: " << _urLon << endl;
    cerr << "  urLat: " << _urLat << endl;
    cerr << "  lrLon: " << _lrLon << endl;
    cerr << "  lrLat: " << _lrLat << endl;
  }
  
}

/////////////////////////////////////////////
// Read in the data fields

int OdimHdf5ToMdv::_readFields(Group &root)
  
{
  
  for (int ii = 1; ii < 999; ii++) {

    char datasetGrpName[1024];
    snprintf(datasetGrpName, 1024, "dataset%d", ii);
    try {
      H5x::Exception::dontPrint();
      if (root.nameExists(datasetGrpName)) {
        H5x::Exception::defaultPrint();
        Group datasetGrp(root.openGroup(datasetGrpName));
        _readField(datasetGrp);
      } else {
        H5x::Exception::defaultPrint();
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "N dataset groups found: " << ii - 1 << endl;
          cerr << "  End of data" << endl;
        }
        return 0;
      }
    } catch (H5x::Exception &e) {
      H5x::Exception::defaultPrint();
      return -1;
    }

  } // ii

  return 0;

}
    
//////////////////////////////////////////////
// read in a field, if required

void OdimHdf5ToMdv::_readField(Group &dataGrp)
  
{

  // get field attributes from the what group

  Group what(dataGrp.openGroup("what"));
  string fieldName = Hdf5xx::getStringAttribute(what, "quantity");
  
  // do we want this field
  
  OutputField *fld = NULL;
  bool fieldWanted = false;
  for (size_t ifield = 0; ifield < _outputFields.size(); ifield++) {
    fld = _outputFields[ifield];
    if (fieldName == string(fld->params.hdf5Quantity)) {
      fieldWanted = true;
      break;
    }
  }

  if (fieldWanted) {
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Found field: " << fieldName << endl;
    }
    
    // read attributes
    
    _readFieldAttributes(what, fieldName, fld);
    
    // read data
    
    _readFieldData(dataGrp, fieldName, fld);
    
  }
  
}

//////////////////////////////////////////////
// read in field attributes

int OdimHdf5ToMdv::_readFieldAttributes(Group &what,
                                        const string &fieldName,
                                        OutputField *fld)
  
{

  string startDateStr = Hdf5xx::getStringAttribute(what, "startdate");
  string startTimeStr = Hdf5xx::getStringAttribute(what, "starttime");
  string endDateStr = Hdf5xx::getStringAttribute(what, "enddate");
  string endTimeStr = Hdf5xx::getStringAttribute(what, "endtime");

  fld->startTime.set(startDateStr + startTimeStr);
  fld->endTime.set(endDateStr + endTimeStr);

  fld->gain = Hdf5xx::getDoubleAttribute(what, "gain");
  fld->offset = Hdf5xx::getDoubleAttribute(what, "offset");
  fld->nodata = Hdf5xx::getDoubleAttribute(what, "nodata");
  fld->fl32Missing = fld->nodata;
  fld->undetect = Hdf5xx::getDoubleAttribute(what, "undetect");
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Reading attributes, field: " << fieldName << endl;
    cerr << "  start time: " << fld->startTime.asString() << endl;
    cerr << "  end time: " << fld->endTime.asString() << endl;
    cerr << "  gain: " << fld->gain << endl;
    cerr << "  offset: " << fld->offset << endl;
    cerr << "  nodata: " << fld->nodata << endl;
    cerr << "  undetect: " << fld->undetect << endl;
  }

  _startTime = fld->startTime;
  _endTime = fld->endTime;
  
  return 0;
  
}

//////////////////////////////////////////////
// read in field data

int OdimHdf5ToMdv::_readFieldData(Group &dataGrp,
                                  const string &fieldName,
                                  OutputField *fld)
  
{

  // open the data set subgroup

  Group data1Grp(dataGrp.openGroup("data1"));

  // get the field properties
  
  if (fld->hdf5.getVarProps(data1Grp, "data",
                            fld->dims, fld->units, 
                            fld->h5class, fld->h5sign, fld->h5order, fld->h5size)) {
    cerr << fld->hdf5.getErrStr() << endl;
    fld->valid = false;
    return -1;
  }
  
  // set interp type
  
  if (fld->h5class == H5T_INTEGER) {
    fld->nearestNeighbor = true;
  } else {
    fld->nearestNeighbor = _params.interp_using_nearest_neighbor;
  }
  
  // debug print
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Reading fields" << endl;
    cerr << "  hdf5Quantity: " << fld->params.hdf5Quantity << endl;
    cerr << "  dims: ";
    for (size_t ii = 0; ii < fld->dims.size(); ii++) {
      cerr << fld->dims[ii];
      if (ii == fld->dims.size() - 1) {
        cerr << endl;
      } else {
        cerr << ", ";
      }
    }
    if (fld->units.size() > 0) {
      cerr << "  units: " << fld->units << endl;
    }
    if (fld->h5class == H5T_INTEGER) {
      if (fld->h5sign == H5T_SGN_NONE) {
        cerr << "  Is unsigned integer" << endl;
      } else {
        cerr << "  Is signed integer" << endl;
      }
    } else {
      cerr << "  Is float" << endl;
    }
    if (fld->h5order == H5T_ORDER_LE) {
      cerr << "  Byte order: little-endian" << endl;
    } else {
      cerr << "  Byte order: big-endian" << endl;
    }
    cerr << "  Byte len: " << fld->h5size << endl;
  }
  
  // set number of levels
    
  if (fld->dims.size() == 3) {
    _nz = fld->dims[2];
  } else {
    _nz = 1;
  }
  _zLevels.resize(_nz);
  for (size_t iz = 0; iz < _nz; iz++) {
    _zLevels[iz] = _minzKm + iz * _dzKm;
  }

  // read in the field data as floats
  
  if (fld->dims.size() == 2) {
    // 2D field
    if (_readField2D(data1Grp, "data",
                     fld->fl32Input,
                     fld->fl32Missing,
                     fld->units) == 0) {
      fld->valid = true;
    }
  } else {
    // 3D field
    if (_readField3D(data1Grp, "data",
                     fld->fl32Input,
                     fld->fl32Missing,
                     fld->units) == 0) {
      fld->valid = true;
    }
  } // if (fld->dims.size() == 2)
    
  return 0;

}

//////////////////////////////////////////////
// read 3D float field

int OdimHdf5ToMdv::_readField3D(Group &grp,
                                const string &dsetName,
                                vector<NcxxPort::fl32> &vals,
                                NcxxPort::fl32 &missingVal,
                                string &units)
  
{
  
  Hdf5xx hdf5;
  
  // read Latitude
  
  vector<size_t> dims;
  if (hdf5.readFl32Array(grp, dsetName,
                         dims, missingVal, vals, units)) {
    cerr << "ERROR - OdimHdf5ToMdv::_readField3D()" << endl;
    cerr << "  Cannot read group/dataset: " << grp.getObjName() << "/" << dsetName << endl;
    return -1;
  }

  // check dimensions for consistency
  
  if (dims.size() != 3) {
    cerr << "ERROR - OdimHdf5ToMdv::_readField3D()" << endl;
    cerr << "  Cannot read group/dataset: " << grp.getObjName() << "/" << dsetName << endl;
    cerr << "  zFactorCorrected must have 3 dimensions" << endl;
    cerr << "  dims.size(): " << dims.size() << endl;
    return -1;
  }
  if (dims[0] != _nz || dims[1] != _ny || dims[2] != _nx) {
    cerr << "ERROR - OdimHdf5ToMdv::_readField3D()" << endl;
    cerr << "  Cannot read group/dataset: " << grp.getObjName() << "/" << dsetName << endl;
    cerr << "  dimensions must match nz, ny and nx" << endl;
    cerr << "  dims[0]: " << dims[0] << endl;
    cerr << "  dims[1]: " << dims[1] << endl;
    cerr << "  dims[2]: " << dims[2] << endl;
    cerr << "  _nz: " << _nz << endl;
    cerr << "  _ny: " << _ny << endl;
    cerr << "  _nx: " << _nx << endl;
    return -1;
  }

  _nz = dims[0];

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "====>> Read 3D fl32 group/dataset: " 
         << grp.getObjName() << "/" << dsetName << " <<====" << endl;
    cerr << "  nz, ny, nx: " 
         << _nz << ", " << _ny << ", " << _nx << endl;
    cerr << "  missingVal: " << missingVal << endl;
    if (_params.debug >= Params::DEBUG_EXTRA) {
      for (size_t iz = 0; iz < _nz; iz++) {
        for (size_t iy = 0; iy < _ny; iy++) {
          for (size_t ix = 0; ix < _nx; ix++) {
            size_t ipt = iz * _ny * _nx + iy * _nx + ix;
            if (vals[ipt] != missingVal) {
              cerr << "  iz, iy, ix, val: "
                   << iz << ", "
                   << iy << ", "
                   << ix << ", "
                   << vals[ipt] << endl;
            }
          } // ix
        } // iy
      } // iz
    } // extra
  } // verbose
  
  return 0;

}

//////////////////////////////////////////////
// read a 2D field - floats

int OdimHdf5ToMdv::_readField2D(Group &grp,
                                const string &dsetName,
                                vector<NcxxPort::fl32> &vals,
                                NcxxPort::fl32 &missingVal,
                                string &units)
  
{

  Hdf5xx hdf5;

  // read Latitude
  
  vector<size_t> dims;
  
  if (hdf5.readFl32Array(grp, dsetName,
                         dims, missingVal, vals, units)) {
    cerr << "ERROR - OdimHdf5ToMdv::_readField2D()" << endl;
    cerr << "  Cannot read group/dataset: " << grp.getObjName() << "/" << dsetName << endl;
    return -1;
  }

  // check dimensions for consistency
  
  if (dims.size() != 2) {
    cerr << "ERROR - OdimHdf5ToMdv::_readField2D()" << endl;
    cerr << "  Cannot read group/dataset: " << grp.getObjName() << "/" << dsetName << endl;
    cerr << "  2D fields must have 2 dimensions" << endl;
    cerr << "  dims.size(): " << dims.size() << endl;
    return -1;
  }
  if (dims[0] != _ny || dims[1] != _nx) {
    cerr << "ERROR - OdimHdf5ToMdv::_readField2D()" << endl;
    cerr << "  Cannot read group/dataset: " << grp.getObjName() << "/" << dsetName << endl;
    cerr << "  dimensions must match ny and nx" << endl;
    cerr << "  dims[0]: " << dims[0] << endl;
    cerr << "  dims[1]: " << dims[1] << endl;
    cerr << "  _ny: " << _ny << endl;
    cerr << "  _nx: " << _nx << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "====>> Read 2D fl32 group/dataset: "
         << grp.getObjName() << "/" << dsetName << " <<====" << endl;
    cerr << "  ny, nx: " << _ny << ", " << _nx << endl;
    cerr << "  missingVal: " << missingVal << endl;
    if (_params.debug >= Params::DEBUG_EXTRA) {
      for (size_t iy = 0; iy < _ny; iy++) {
        for (size_t ix = 0; ix < _nx; ix++) {
          size_t ipt = iy * _nx + ix;
          if (vals[ipt] != missingVal) {
            cerr << "  iy, ix, val: "
                 << iy << ", "
                 << ix << ", "
                 << vals[ipt] << endl;
          }
        } // ix
      } // iy
    } // extra
  } // verbose
  
  return 0;

}

/////////////////////////////////////////////////
// Set the master header from the NCF file
//
// Returns 0 on success, -1 on failure

void OdimHdf5ToMdv::_setMasterHeader(DsMdvx &mdvx)

{

  mdvx.clearMasterHeader();

  // time

  if (_params.debug) {
    cerr << "===========================================" << endl;
    cerr << "Found data set at start time: " << _startTime.asString(3) << endl;
  }

  mdvx.setBeginTime(_startTime.utime());
  mdvx.setEndTime(_endTime.utime());
  mdvx.setValidTime(_endTime.utime());
  
  // data collection type
  
  mdvx.setDataCollectionType(Mdvx::DATA_MEASURED);

  // data set name, source and info

  mdvx.setDataSetName(_params.data_set_name);
  mdvx.setDataSetSource(_params.data_set_source);
  
  string info;
  info += "Conventions: ";
  info += _conventions;
  info += "/n";
  info += "Version: ";
  info += _version;
  info += "/n";
  info += "Source: ";
  info += _source;
  info += "/n";
  info += "History: ";
  info += _history;
  info += "/n";

  mdvx.setDataSetInfo(info.c_str());

}

///////////////////////////////////
// Add the Mdvx fields

void OdimHdf5ToMdv::_addFieldToMdvx(DsMdvx &mdvx,
                                    OutputField *fld)

{

  // set the un-detect values to missing
  
  size_t npts = _nz * _ny * _nx;
  NcxxPort::fl32 undetect = fld->undetect;
  NcxxPort::fl32 missing = fld->fl32Missing;
  NcxxPort::fl32 *vals = fld->fl32Input.data();
  for (size_t ii = 0; ii < npts; ii++) {
    if (vals[ii] == undetect) {
      vals[ii] = missing;
    }
  }
  
  // create the field

  MdvxField *field = _createMdvxField(fld->params.outputName,
                                      fld->params.longName,
                                      fld->units,
                                      _nx, _ny, _nz,
                                      _minxKm, _minyKm, _minzKm,
                                      _dxKm, _dyKm, _dzKm,
                                      fld->fl32Missing,
                                      fld->fl32Input.data());

  // convert to output representation

  field->convertType((Mdvx::encoding_type_t) fld->params.encoding,
                     Mdvx::COMPRESSION_GZIP);

  // add to the object

  mdvx.addField(field);
    
}
      
///////////////////////////////////
// Create an Mdvx field - floats

MdvxField *OdimHdf5ToMdv::_createMdvxField(const string &fieldName,
                                           const string &longName,
                                           const string &units,
                                           size_t nx, size_t ny, size_t nz,
                                           double minx, double miny, double minz,
                                           double dx, double dy, double dz,
                                           NcxxPort::fl32 missingVal,
                                           NcxxPort::fl32 *vals)

{

  // check max levels

  if (nz > MDV_MAX_VLEVELS) {
    nz = MDV_MAX_VLEVELS;
  }

  // unitialize the projection

  MdvxProj proj;
  proj.initFromProjStr(_projStr);
  proj.setGrid(nx, ny, dx, dy, minx, miny);

  // set up MdvxField headers

  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);
  STRncopy(fhdr.field_name, fieldName.c_str(), MDV_SHORT_FIELD_LEN);
  
  // _inputProj.syncToFieldHdr(fhdr);

  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;
  
  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  fhdr.dz_constant = true;
  if (nz == 1) {
    fhdr.data_dimension = 2;
  } else {
    fhdr.data_dimension = 3;
  }
  
  fhdr.scale = 1.0;
  fhdr.bad_data_value = missingVal;
  fhdr.missing_data_value = missingVal;
  
  fhdr.proj_type = Mdvx::PROJ_LATLON;
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = sizeof(fl32);
  
  fhdr.nx = nx;
  fhdr.ny = ny;
  fhdr.nz = nz;

  fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * sizeof(fl32);

  fhdr.grid_minx = minx;
  fhdr.grid_miny = miny;
  fhdr.grid_minz = minz;

  fhdr.grid_dx = dx;
  fhdr.grid_dy = dy;
  fhdr.grid_dz = dz;
  
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  
  for (size_t ii = 0; ii < nz; ii++) {
    vhdr.type[ii] = Mdvx::VERT_TYPE_Z;
    vhdr.level[ii] = _zLevels[ii];
  }

  proj.syncXyToFieldHdr(fhdr);

  // create MdvxField object
  // converting data to encoding and compression types
  
  MdvxField *field = new MdvxField(fhdr, vhdr, vals);

  // set names etc
  
  field->setFieldName(fieldName);
  field->setFieldNameLong(longName);
  field->setUnits(units);
  field->setTransform("");

  return field;

}
  
