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
// CedricPrint.cc
//
// CedricPrint object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2012
//
///////////////////////////////////////////////////////////////

#include "CedricPrint.hh"
#include <didss/DsDataFile.hh>
using namespace std;

// Constructor

CedricPrint::CedricPrint(int argc, char **argv)
  
{

  OK = TRUE;
  
  // set programe name

  _progName = "CedricPrint";
  
  // parse command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args." << endl;
    OK = FALSE;
    return;
  }
  
  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list, &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = FALSE;
    return;
  }

  return;

}

// destructor

CedricPrint::~CedricPrint()

{

}

//////////////////////////////////////////////////
// Run

int CedricPrint::Run()
{

  // read in file

  Cedric ced;
  if (ced.readFromPath(_params.path)) {
    cerr << "ERROR - CedricPrint::Run()" << endl;
    cerr << "  Cannot read in path: " << _params.path << endl;
    return -1;
  }

  if (_params.print_native) {
    ced.printNative(cout, _params.print_data);
  } else {
    ced.print(cout, _params.print_data);
  }

  return 0;

}

#ifdef JUNK

//////////////////////////////////////////////////
// set up read for MDV

void CedricPrint::_setupRead(DsMdvx *mdvx)
{

  if (_params.set_latest_valid_mod_time) {
    mdvx->setCheckLatestValidModTime(_latestValidModTime);
  }
  
  if (_params.file_format == Params::FORMAT_XML) {
    mdvx->setReadFormat(Mdvx::FORMAT_XML);
    mdvx->setWriteFormat(Mdvx::FORMAT_XML);
    if (_params.debug) {
      cerr << "Using XML format" << endl;
    }    
  } else if (_params.file_format == Params::FORMAT_NCF) {
    mdvx->setReadFormat(Mdvx::FORMAT_NCF);
    mdvx->setWriteFormat(Mdvx::FORMAT_NCF);
    if (_params.debug) {
      cerr << "Using NCF format" << endl;
    }    
  }

  if (_params.get_mode != Params::GET_TIME_LIST &&
      _params.get_mode != Params::GET_TIME_HEIGHT) {

    if (_params.specify_file_by_time) {

      mdvx->setReadTime((Mdvx::read_search_mode_t)
			_params.read_search_mode,
			_params.url,
			_params.read_search_margin,
			_readSearchTime,
			_params.read_forecast_lead_time);
      
    } else {
      
      string path;
      if (strlen(_params.path) > 0 &&
	  (_params.path[0] == '/' || _params.path[0] == '.')) {
	path = _params.path;
      } else {
	path = "./";
	path += _params.path;
      }
      mdvx->setReadPath(path);
      
    } // if (_params.specify_file_by_time)

    mdvx->setReadFieldFileHeaders();
    
  } // if (_params.get_mode != Params::GET_TIME_LIST ...

  if (_params.set_valid_time_search_wt) {
    mdvx->setValidTimeSearchWt(_params.valid_time_search_wt);
  }

  if (_params.constrain_forecast_lead_times) {
    mdvx->setConstrainFcastLeadTimes
      (_params.forecast_constraints.min_lead_time,
       _params.forecast_constraints.max_lead_time,
       _params.forecast_constraints.request_by_gen_time);
  }
  
  if (_params.read_set_horiz_limits) {
    mdvx->setReadHorizLimits(_params.read_horiz_limits.min_lat,
			     _params.read_horiz_limits.min_lon,
			     _params.read_horiz_limits.max_lat,
			     _params.read_horiz_limits.max_lon);
  }
  
  if (_params.read_set_vlevel_limits) {
    mdvx->setReadVlevelLimits(_params.read_lower_vlevel,
			      _params.read_upper_vlevel);
  }

  if (_params.read_set_plane_num_limits) {
    mdvx->setReadPlaneNumLimits(_params.read_lower_plane_num,
				_params.read_upper_plane_num);
  }
  
  if (_params.read_set_vlevel_type) {
    switch(_params.read_vlevel_type) {
    case Params::VERT_TYPE_Z:
      mdvx->setReadVlevelType(Mdvx::VERT_TYPE_Z);
      break;
    case Params::VERT_TYPE_PRESSURE:
      mdvx->setReadVlevelType(Mdvx::VERT_TYPE_PRESSURE);
      break;
    case Params::VERT_FLIGHT_LEVEL:
      mdvx->setReadVlevelType(Mdvx::VERT_FLIGHT_LEVEL);
      break;
    }
  }

  mdvx->setReadEncodingType((Mdvx::encoding_type_t)
			    _params.read_encoding_type);

  mdvx->setReadCompressionType((Mdvx::compression_type_t)
			       _params.read_compression_type);

  mdvx->setReadScalingType((Mdvx::scaling_type_t) _params.read_scaling_type,
			   _params.read_scale,
			   _params.read_bias);

  if (_params.read_composite) {
    mdvx->setReadComposite();
  }

  if (_params.read_set_fill_missing) {
    mdvx->setReadFillMissing();
  }

  if (_params.read_set_field_names) {

    for (int i = 0; i < _params.read_field_names_n; i++) {
      mdvx->addReadField(_params._read_field_names[i]);
    }

  } else if (_params.read_set_field_nums) {

    for (int i = 0; i < _params.read_field_nums_n; i++) {
      mdvx->addReadField(_params._read_field_nums[i]);
    }

  }

  if (_params.read_set_decimation) {
    mdvx->setReadDecimate(_params.decimation_max_nxy);
  }

  if (_params.read_time_list_also || 
      _params.get_mode == Params::GET_TIME_LIST ||
      _params.get_mode == Params::GET_TIME_HEIGHT) {
    _setTimeListMode(mdvx);
  }
  
  if (_params.read_time_list_also) {
    mdvx->setReadTimeListAlso();
  }

  if (_params.read_as_single_part) {
    mdvx->setReadAsSinglePart();
  }

  if (_params.read_field_file_headers_also) {
    mdvx->setReadFieldFileHeaders();
  }

  if (_params.get_mode == Params::GET_VSECTION ||
      _params.get_mode == Params::GET_TIME_HEIGHT) {
    
    // waypoints
    
    for (int i = 0; i < _params.read_vsect_waypts_n; i++) {
      mdvx->addReadWayPt(_params._read_vsect_waypts[i].lat,
			 _params._read_vsect_waypts[i].lon);
    }

    if (_params.disable_vsection_interp) {
      mdvx->setReadVsectDisableInterp();
    }

  }
  
}

////////////////////////
// handle volume

int CedricPrint::_handleVolume(DsMdvx *mdvx)

{

  if (!_needData()) {
    // just read all headers
    return _handleAllHeaders(mdvx);
  }

  // read the volume

  if (_getVolume(mdvx)) {
    return -1;
  }
  
  if (_params.save_to_file) {

    int iret = 0;
    const Mdvx::master_header_t &mhdr = mdvx->getMasterHeader();
    if (mhdr.data_collection_type == Mdvx::DATA_EXTEOLOLATED ||
        mhdr.data_collection_type == Mdvx::DATA_FORECAST ||
        mhdr.forecast_time != 0) {
      mdvx->setWriteAsForecast();
    }
    if (mdvx->writeToDir(_params.save_url)) {
      cerr << "ERROR - CedricPrint" << endl;
      cerr << "  Cannot save file to url: " << _params.save_url << endl;
      cerr << mdvx->getErrStr() << endl;
      iret = -1;
    }
    if (_params.debug) {
      cerr << "Wrote file: " << mdvx->getPathInUse() << endl;
    }
    if (_params.no_print) {
      return iret;
    }

  } else if (mdvx->getCurrentFormat() == Mdvx::FORMAT_NCF) {

    // save to tmp file, so it can be printed
    
    time_t now = time(NULL);
    DateTime dnow(now);
    pid_t pid = getpid();
    char tmpFilePath[FILENAME_MAX];
    sprintf(tmpFilePath,
            "/tmp/CedricPrint_vol_tmp_%.4d%.2d%.2d_%.2d%.2d%.2d_%.5d",
            dnow.getYear(), dnow.getMonth(), dnow.getDay(),
            dnow.getHour(), dnow.getMin(), dnow.getSec(), pid);
    if (mdvx->writeToPath(tmpFilePath)) {
      cerr << "ERROR - CedricPrint" << endl;
      cerr << "  Cannot save tmp file: " << tmpFilePath << endl;
      cerr << mdvx->getErrStr() << endl;
      return -1;
    }
    if (_params.debug) {
      cerr << "Wrote file: " << mdvx->getPathInUse() << endl;
    }

  }

  if (_params.get_mode == Params::GET_GIS) {
    _printVolGis(mdvx);
  } else if (_params.get_mode == Params::GET_TABLE) {
    _printVolTable(mdvx);
  } else if (_params.print_summary) {
    if (_params.read_time_list_also) {
      _printTimeList(mdvx);
    }
    _printVolSummary(mdvx);
  } else {
    if (_params.read_time_list_also) {
      _printTimeList(mdvx);
    }
    _printVolume(mdvx);
  }

  return 0;

}

///////////////////////////////////
// check that we need to read data

bool CedricPrint::_needData()

{

  if (_params.save_to_file) {
    return true;
  }
  if (_params.get_mode == Params::GET_GIS) {
    return true;
  }
  if (_params.get_mode == Params::GET_TABLE) {
    return true;
  }
  if (_params.print_summary) {
    return true;
  }
  if (_params.print_data) {
    return true;
  }
  if (_params.print_canonical) {
    return true;
  }
  if (_params.print_chunks) {
    return true;
  }
  if (_params.read_as_single_part) {
    return true;
  }
  if (_params.read_set_decimation) {
    return true;
  }
  if (_params.read_set_field_nums) {
    return true;
  }
  if (_params.read_set_field_names) {
    return true;
  }
  if (_params.read_set_vlevel_type) {
    return true;
  }
  if (_params.read_set_vlevel_limits) {
    return true;
  }
  if (_params.read_transform_to_linear) {
    return true;
  }
  if (_params.read_composite) {
    return true;
  }
  return false;
}
  
////////////////////////
// print volume

void CedricPrint::_printVolume(const DsMdvx *mdvx) const
{

  cout << "=============== VOLUME ==============" << endl;
  _doPrintVol(mdvx);

}
  
///////////////////////////////////////////////////////
// print vert section
  
void CedricPrint::_printVsection(const DsMdvx *mdvx) const
  
{

  cout << "=============== VERTICAL SECION ==============" << endl;
  _doPrintVol(mdvx);

}

///////////////////////////////////////////////////////
// do print on volume
  
void CedricPrint::_doPrintVol(const DsMdvx *mdvx) const
  
{

  mdvx->printFormats(cout);
  
  if (mdvx->getCurrentFormat() != Mdvx::FORMAT_NCF) {
    
    _printVol(mdvx);

  } else {

    mdvx->printNcfInfo(cout);
    char command[2048];
    sprintf(command, "ncdump %s", mdvx->getPathInUse().c_str());
    system(command);
    if (!_params.save_to_file) {
      // remove tmp file
      unlink(mdvx->getPathInUse().c_str());
    }

  }

}

////////////////////////////////////
// print vol - volume and vsection

void CedricPrint::_printVol(const DsMdvx *mdvx) const
{

  cout << endl;
  cout << "File path: " << mdvx->getPathInUse() << endl;

  // master header

  const Mdvx::master_header_t &mhdr = mdvx->getMasterHeader();
  mdvx->printMasterHeader(mhdr, cout);
  
  // fields
  
  for (int i = 0; i < mdvx->getNFields(); i++) {

    MdvxField *field = mdvx->getField(i);

    const Mdvx::field_header_t &fhdr = field->getFieldHeader();
    mdvx->printFieldHeader(fhdr, cout);
    const Mdvx::vlevel_header_t &vhdr = field->getVlevelHeader();
    mdvx->printVlevelHeader(vhdr, fhdr.nz, fhdr.field_name, cout);

    if (_params.read_field_file_headers_also) {
      const Mdvx::field_header_t *fhdrFile = field->getFieldHeaderFile();
      const Mdvx::vlevel_header_t *vhdrFile = field->getVlevelHeaderFile();
      if (fhdrFile && vhdrFile) {
        cout << "============ Field header as in file =============" << endl;
        mdvx->printFieldHeader(*fhdrFile, cout);
        cout << "==================================================" << endl;
        cout << "=========== Vlevel header as in file =============" << endl;
        mdvx->printVlevelHeader(*vhdrFile,
                                fhdrFile->nz, fhdrFile->field_name, cout);
        cout << "==================================================" << endl;
      }
    }

    MdvxProj proj(field->getFieldHeader());
    MdvxRadar mdvxRadar;
    if (mdvxRadar.loadFromMdvx(*mdvx) == 0) {
      DsRadarParams radar = mdvxRadar.getRadarParams();
      proj.setSensorPosn(radar.latitude, radar.longitude, radar.altitude);
    }
    proj.print(cout);

    if (_params.print_data) {
      if (_params.read_transform_to_linear) {
	if (field->transform2Linear()){
	  cerr << field->getErrStr() << endl;
	}
      }
      field->printVoldata(cout, _params.print_native, true, true,
			  _params.print_canonical);
    }
  }

  // chunks

  mdvx->printChunks(cout);

}
  
////////////////////////
// print volume summary

void CedricPrint::_printVolSummary(const DsMdvx *mdvx) const
{

  cout << endl;
  cout << "File path: " << mdvx->getPathInUse() << endl;

  // master header

  const Mdvx::master_header_t &mhdr = mdvx->getMasterHeader();
  mdvx->printFormats(cout);
  mdvx->printMasterHeaderSummary(mhdr, cout);
  
  // fields
  
  for (int i = 0; i < mdvx->getNFields(); i++) {
    MdvxField *field = mdvx->getField(i);
    MdvxProj proj(field->getFieldHeader());
    const Mdvx::field_header_t &fhdr = field->getFieldHeader();
    mdvx->printFieldHeaderSummary(fhdr, cout);
  }

}
  
/////////////////////////////
// print volume in GIS format

void CedricPrint::_printVolGis(const DsMdvx *mdvx) const
{
  
  // some sanity checks

  if (mdvx->getNFields() < 1) {
    cerr << "ERROR - CedricPrint, GIS mode" << endl;
    cerr << "  No fields found" << endl;
    return;
  }
  
  MdvxField *field = mdvx->getFieldByNum(0);
  const Mdvx::field_header_t &fhdr = field->getFieldHeader();
  MdvxProj proj(fhdr);
  
  if (mdvx->getNFields() > 1) {
    cerr << "WARNING - CedricPrint, GIS mode" << endl;
    cerr << "  More than one field found" << endl;
    cerr << "  Only the first field will be output" << endl;
    cerr << "  Output field name: " << fhdr.field_name << endl;
  }
  if (fhdr.nz > 1) {
    cerr << "WARNING - CedricPrint, GIS mode" << endl;
    cerr << "  More than one plane found" << endl;
    cerr << "  Only the first plane will be output" << endl;
  }
  bool cellSizesDiffer = false;
  if (fabs(fhdr.grid_dx - fhdr.grid_dy) > 1.0e-5) {
    cellSizesDiffer = true;
    cerr << "WARNING - CedricPrint, GIS mode" << endl;
    cerr << "  Grid cell size differs in X and Y" << endl;
    cerr << "    dx: " << fhdr.grid_dx << endl;
    cerr << "    dy: " << fhdr.grid_dy << endl;
    cerr << "  Will output dx and dy" << endl;
  }

  // convert to floats, uncompressed, linear
  
  field->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);

  if (field->transform2Linear()){
    cerr << field->getErrStr() << endl;
  }

  // ESRI header

  cout << "nrows " << fhdr.ny << endl;
  cout << "ncols " << fhdr.nx << endl;
  cout << "xllcorner " << fhdr.grid_minx << endl;
  cout << "yllcorner " << fhdr.grid_miny << endl;
  if (cellSizesDiffer) {
    cout << "dx " << fhdr.grid_dx << endl;
    cout << "dy " << fhdr.grid_dy << endl;
  } else {
    cout << "cellsize " << fhdr.grid_dx << endl;
  }
  cout << "NODATA_value " << fhdr.missing_data_value << endl;

  // print out starting at top row

  int iy_begin, iy_end, iy_increment;
  if (_params.start_at_top) {
    iy_begin = fhdr.ny - 1;
    iy_end = -1;
    iy_increment = -1;
  } else {
    cout << "Bottom row first" << endl;
    iy_begin = 0;
    iy_end = fhdr.ny;
    iy_increment = 1;
  }

  for (int iy = iy_begin; iy != iy_end; iy += iy_increment) {
    
    fl32 *data = ((fl32 *) field->getVol()) + (iy * fhdr.nx);

    for (int ix = 0; ix < fhdr.nx; ix++, data++) {
      
      cout << *data;
      if (ix == fhdr.nx - 1) {
	cout << endl;
      } else {
	cout << " ";
      }

    } // ix

  } // iy

}
  
/////////////////////////////////
// print volume in tabular format

void CedricPrint::_printVolTable(const DsMdvx *mdvx) const
{
  
  // some sanity checks

  if (mdvx->getNFields() < 1) {
    cerr << "ERROR - CedricPrint, TABLE mode" << endl;
    cerr << "  No fields found" << endl;
    return;
  }

  const vector<MdvxField *> fields = mdvx->getFields();

  const Mdvx::field_header_t &fhdr0 = fields[0]->getFieldHeader();
  const Mdvx::vlevel_header_t &vhdr0 = fields[0]->getVlevelHeader();

  int nxPrint = fhdr0.nx;
  int nyPrint = fhdr0.ny;
  int nzPrint = fhdr0.nz;
  for (size_t ii = 1; ii < fields.size(); ii++) {
    const Mdvx::field_header_t &fhdr = fields[ii]->getFieldHeader();
    if (fhdr.nx != fhdr0.nx || fhdr.ny != fhdr0.ny || fhdr.nz != fhdr0.nz) {
      cerr << "WARNING - CedricPrint, TABLE mode" << endl;
      cerr << "  Field sizes differ" << endl;
      cerr << "  Will use smallest dimensions for printing table" << endl;
    }
    nxPrint = MIN(nxPrint, fhdr.nx);
    nyPrint = MIN(nyPrint, fhdr.ny);
    nzPrint = MIN(nzPrint, fhdr.nz);
  }
  
  // convert to floats, uncompressed, linear
  
  for (size_t ii = 0; ii < fields.size(); ii++) {
    fields[ii]->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);

    if (fields[ii]->transform2Linear()){
      cerr << fields[ii]->getErrStr() << endl;
    }

    fields[ii]->setPlanePtrs();
  }

  // print headers

  const Mdvx::master_header_t &mhdr = mdvx->getMasterHeader();

  cout << "# MDV file - tabular output" << endl;
  cout << "# File: " << mdvx->getPathInUse() << endl;
  cout << "# Time: " << DateTime::strn(mhdr.time_centroid) << endl;

  cout << "# Fields: z y x";
  for (size_t ii = 0; ii < fields.size(); ii++) {
    cout << " " << fields[ii]->getFieldHeader().field_name;
  }
  cout << endl;
  
  cout << "# Units:";
  int projType = fields[0]->getFieldHeader().proj_type;
  int vlevelType = fields[0]->getFieldHeader().vlevel_type;
  cout << " " << Mdvx::vertTypeZUnits(vlevelType);
  cout << " " << Mdvx::projType2YUnits(projType);
  cout << " " << Mdvx::projType2XUnits(projType);
  for (size_t ii = 0; ii < fields.size(); ii++) {
    cout << " " << fields[ii]->getFieldHeader().units;
  }
  cout << endl;

  // print values

  double minx = fhdr0.grid_minx;
  double miny = fhdr0.grid_miny;
  double dx = fhdr0.grid_dx;
  double dy = fhdr0.grid_dy;

  for (int iz = 0; iz < nzPrint; iz++) {
    for (int iy = 0; iy < nyPrint; iy++) {
      for (int ix = 0; ix < nxPrint; ix++) {
	
	cout << vhdr0.level[iz];
	cout << " " << miny + iy * dy;
	cout << " " << minx + ix * dx;

	for (size_t ii = 0; ii < fields.size(); ii++) {
	  fl32 *plane = (fl32 *) fields[ii]->getPlane(iz);
	  const Mdvx::field_header_t &fhdr = fields[ii]->getFieldHeader();
	  int offset = iy * fhdr.nx + ix;
	  fl32 val = plane[offset];
	  cout << " " << val;
	}
	cout << endl;

      } // ix
    } // iy
  } // iz

}

////////////////////////
// print all headers

void CedricPrint::_printAllHeaders(const DsMdvx *mdvx) const
{

  cout << endl;
  cout << "File path: " << mdvx->getPathInUse() << endl;

  // master header

  const Mdvx::master_header_t &mhdr = mdvx->getMasterHeaderFile();
  mdvx->printFormats(cout);
  mdvx->printMasterHeader(mhdr, cout);
  
  // fields
  
  for (int i = 0; i < mhdr.n_fields; i++) {
    const Mdvx::field_header_t &fhdr = mdvx->getFieldHeaderFile(i);
    mdvx->printFieldHeader(fhdr, cout);
    const Mdvx::vlevel_header_t &vhdr = mdvx->getVlevelHeaderFile(i);
    mdvx->printVlevelHeader(vhdr, fhdr.nz, fhdr.field_name, cout);
    MdvxProj proj(fhdr);
    proj.print(cout);
  }

  for (int i = 0; i < mhdr.n_chunks; i++) {
    const Mdvx::chunk_header_t &chdr = mdvx->getChunkHeaderFile(i);
    mdvx->printChunkHeader(chdr, cout);
  }

}
  
/////////////////////////////////////////////////
// print the time list

void CedricPrint::_printTimeList(const DsMdvx *mdvx) const

{

  switch(_params.time_list_mode) {
  
  case Params::TIME_LIST_VALID: {

    cout << "TIME LIST - MODE VALID" << endl;
    cout << "  URL: " << _params.url << endl;
    cout << "  Start time: "
	 << DateTime::strn(_timeListStartTime) << endl;
    cout << "  End time: " << DateTime::strn(_timeListEndTime) << endl;

    const vector<time_t> &valid = mdvx->getValidTimes();
    const vector<time_t> &gen = mdvx->getGenTimes();
    bool hasForecasts = mdvx->timeListHasForecasts();
    if (valid.size() == 0) {
      cout << "    No times returned." << endl;
    } else {
      if (hasForecasts) {
	cout << "  Valid/gen times:" << endl;
      } else {
	cout << "  Valid times:" << endl;
      }
      for (size_t ii = 0; ii < valid.size(); ii++) {
	if (hasForecasts) {
	  cout << "    "
	       << DateTime::strn(valid[ii]) << " / "
	       << DateTime::strn(gen[ii]) << endl;
	} else {
	  cout << "    " << DateTime::strn(valid[ii]) << endl;
	}
      } // ii
    }

    break;
  }

  case Params::TIME_LIST_GEN: {

    cout << "TIME LIST - MODE GEN" << endl;
    cout << "  URL: " << _params.url << endl;
    cout << "  Start time: "
	 << DateTime::strn(_timeListStartTime) << endl;
    cout << "  End time: " << DateTime::strn(_timeListEndTime) << endl;

    const vector<time_t> &gen = mdvx->getGenTimes();
    if (gen.size() == 0) {
      cout << "    No times returned." << endl;
    } else {
      cout << "  Gen times:" << endl;
      for (size_t ii = 0; ii < gen.size(); ii++) {
	cout << "    " << DateTime::strn(gen[ii]) << endl;
      } // ii
    }

    break;
  }

  case Params::TIME_LIST_FORECAST: {

    cout << "TIME LIST - MODE FORECAST" << endl;
    cout << "  URL: " << _params.url << endl;
    cout << "  Gen time: " << DateTime::strn(_timeListGenTime) << endl;
    
    const vector<time_t> &valid = mdvx->getValidTimes();
    if (valid.size() == 0) {
      cout << "    No times returned." << endl;
    } else {
      cout << "  Forecast times:" << endl;
      for (size_t ii = 0; ii < valid.size(); ii++) {
	cout << "    " << DateTime::strn(valid[ii]) << endl;
      } // ii
    }

    break;
  }

  case Params::TIME_LIST_GEN_PLUS_FORECASTS: {

    cout << "TIME LIST - MODE GEN_PLUS_FORECASTS" << endl;
    cout << "  URL: " << _params.url << endl;
    cout << "  Start time: "
	 << DateTime::strn(_timeListStartTime) << endl;
    cout << "  End time: "
	 << DateTime::strn(_timeListEndTime) << endl;

    const vector<time_t> &gen = mdvx->getGenTimes();
    const vector<vector<time_t> > &ftArray =
      mdvx->getForecastTimesArray();

    if (gen.size() == 0) {
      cout << "    No times returned." << endl;
    } else {
      cout << "  Gen times:" << endl;
      for (size_t ii = 0; ii < gen.size(); ii++) {
	cout << "    " << DateTime::strn(gen[ii]) << endl;
      } // ii
      for (size_t ii = 0; ii < ftArray.size(); ii++) {
	cout << "Gen time: " << DateTime::strn(gen[ii]) << endl;
	cout << "  Forecast times:" << endl;
	if (ii < ftArray.size()) {
	  const vector<time_t> &forecastTimes = ftArray[ii];
	  for (size_t jj = 0; jj < forecastTimes.size(); jj++) {
	    cout << "    "
		 << DateTime::strn(forecastTimes[jj]) << endl;
	  }
	}
      } // ii
    }
  break;
  }
  
  case Params::TIME_LIST_VALID_MULT_GEN: {

    cout << "TIME LIST - MODE VALID_MULT_GEN" << endl;
    cout << "  URL: " << _params.url << endl;
    cout << "  Start time: "
	 << DateTime::strn(_timeListStartTime) << endl;
    cout << "  End time: " << DateTime::strn(_timeListEndTime) << endl;

    const vector<time_t> &valid = mdvx->getValidTimes();
    const vector<time_t> &gen = mdvx->getGenTimes();
    if (valid.size() == 0) {
      cout << "    No times returned." << endl;
    } else {
      cout << "  Valid/gen times:" << endl;
      for (size_t ii = 0; ii < valid.size(); ii++) {
	cout << "    "
	     << DateTime::strn(valid[ii]) << " / "
	     << DateTime::strn(gen[ii]) << endl;
      } // ii
    }

    break;
  }

  case Params::TIME_LIST_FIRST: {
    cout << "TIME LIST - MODE FIRST" << endl;
    cout << "  URL: " << _params.url << endl;
    const vector<time_t> &valid = mdvx->getValidTimes();
    const vector<time_t> &gen = mdvx->getGenTimes();
    bool hasForecasts = mdvx->timeListHasForecasts();
    if (valid.size() == 0) {
      cout << "    No time returned." << endl;
    } else {
      if (hasForecasts) {
	cout << "  First valid/gen time:" << endl;
	cout << "    "
	     << DateTime::strn(valid[0]) << " / "
	     << DateTime::strn(gen[0]) << endl;
      } else {
	cout << "  First time:" << endl;
	cout << "    " << DateTime::strn(valid[0]) << endl;
      }
    }
    break;
  }

  case Params::TIME_LIST_LAST: {
    cout << "TIME LIST - MODE LAST" << endl;
    cout << "  URL: " << _params.url << endl;
    const vector<time_t> &valid = mdvx->getValidTimes();
    const vector<time_t> &gen = mdvx->getGenTimes();
    bool hasForecasts = mdvx->timeListHasForecasts();
    if (valid.size() == 0) {
      cout << "    No time returned." << endl;
    } else {
      if (hasForecasts) {
	cout << "  Last valid/gen time:" << endl;
	cout << "    "
	     << DateTime::strn(valid[0]) << " / "
	     << DateTime::strn(gen[0]) << endl;
      } else {
	cout << "  Last time:" << endl;
	cout << "    " << DateTime::strn(valid[0]) << endl;
      }
    }
    break;
  }

  case Params::TIME_LIST_CLOSEST: {
    cout << "TIME LIST - MODE CLOSEST" << endl;
    cout << "  URL: " << _params.url << endl;
    cout << "  Search time: "
	 << DateTime::strn(_timeListSearchTime) << endl;
    cout << "  Time margin: " << _params.time_list_margin << endl;
    const vector<time_t> &valid = mdvx->getValidTimes();
    const vector<time_t> &gen = mdvx->getGenTimes();
    bool hasForecasts = mdvx->timeListHasForecasts();
    if (valid.size() == 0) {
      cout << "    No time returned." << endl;
    } else {
      if (hasForecasts) {
	cout << "  Closest valid/gen time:" << endl;
	cout << "    "
	     << DateTime::strn(valid[0]) << " / "
	     << DateTime::strn(gen[0]) << endl;
      } else {
	cout << "  Closest time:" << endl;
	cout << "    " << DateTime::strn(valid[0]) << endl;
      }
    }
    break;
  }

  case Params::TIME_LIST_FIRST_BEFORE: {
    cout << "TIME LIST - MODE FIRST_BEFORE" << endl;
    cout << "  URL: " << _params.url << endl;
    cout << "  Search time: "
	 << DateTime::strn(_timeListSearchTime) << endl;
    cout << "  Time margin: " << _params.time_list_margin << endl;
    const vector<time_t> &valid = mdvx->getValidTimes();
    const vector<time_t> &gen = mdvx->getGenTimes();
    bool hasForecasts = mdvx->timeListHasForecasts();
    if (valid.size() == 0) {
      cout << "    No time returned." << endl;
    } else {
      if (hasForecasts) {
	cout << "  First before valid/gen time:" << endl;
	cout << "    "
	     << DateTime::strn(valid[0]) << " / "
	     << DateTime::strn(gen[0]) << endl;
      } else {
	cout << "  First before time:" << endl;
	cout << "    " << DateTime::strn(valid[0]) << endl;
      }
    }
    break;
  }

  case Params::TIME_LIST_FIRST_AFTER: {
    cout << "TIME LIST - MODE FIRST_AFTER" << endl;
    cout << "  URL: " << _params.url << endl;
    cout << "  Search time: "
	 << DateTime::strn(_timeListSearchTime) << endl;
    cout << "  Time margin: " << _params.time_list_margin << endl;
    const vector<time_t> &valid = mdvx->getValidTimes();
    const vector<time_t> &gen = mdvx->getGenTimes();
    bool hasForecasts = mdvx->timeListHasForecasts();
    if (valid.size() == 0) {
      cout << "    No time returned." << endl;
    } else {
      if (hasForecasts) {
	cout << "  First after valid/gen time:" << endl;
	cout << "    "
	     << DateTime::strn(valid[0]) << " / "
	     << DateTime::strn(gen[0]) << endl;
      } else {
	cout << "  First after time:" << endl;
	cout << "    " << DateTime::strn(valid[0]) << endl;
      }
    }
    break;
  }

  case Params::TIME_LIST_BEST_FORECAST: {
    cout << "TIME LIST - MODE BEST_FORECAST" << endl;
    cout << "  URL: " << _params.url << endl;
    cout << "  Search time: "
	 << DateTime::strn(_timeListSearchTime) << endl;
    cout << "  Time margin: " << _params.time_list_margin << endl;
    const vector<time_t> &valid = mdvx->getValidTimes();
    const vector<time_t> &gen = mdvx->getGenTimes();
    if (valid.size() == 0) {
      cout << "    No time returned." << endl;
    } else {
      cout << "  Best forecast valid/gen time:" << endl;
      cout << "    "
	   << DateTime::strn(valid[0]) << " / "
	   << DateTime::strn(gen[0]) << endl;
    }
    break;
  }

  case Params::TIME_LIST_SPECIFIED_FORECAST: {
    cout << "TIME LIST - MODE SPECIFIED_FORECAST" << endl;
    cout << "  URL: " << _params.url << endl;
    cout << "  Generate time: "
	 << DateTime::strn(_timeListGenTime) << endl;
    cout << "  Search time: "
	 << DateTime::strn(_timeListSearchTime) << endl;
    cout << "  Time margin: " << _params.time_list_margin << endl;
    const vector<time_t> &valid = mdvx->getValidTimes();
    const vector<time_t> &gen = mdvx->getGenTimes();
    if (valid.size() == 0) {
      cout << "    No time returned." << endl;
    } else {
      cout << "  Specified forecast valid/gen time:" << endl;
      cout << "    "
	   << DateTime::strn(valid[0]) << " / "
	   << DateTime::strn(gen[0]) << endl;
    }
    break;
  }

  } // switch

}
  
/////////////////////////////////////////////////
// print the time height profile

void CedricPrint::_printTimeHeight(const DsMdvx *mdvx) const

{

  // master header
  
  const Mdvx::master_header_t &mhdr = mdvx->getMasterHeader();
  mdvx->printFormats(cout);
  mdvx->printMasterHeader(mhdr, cout);
  
  // fields
  
  for (int i = 0; i < mdvx->getNFields(); i++) {

    MdvxField *field = mdvx->getField(i);
    const Mdvx::field_header_t &fhdr = field->getFieldHeader();
    mdvx->printFieldHeader(fhdr, cout);

    const Mdvx::vlevel_header_t &vhdr = field->getVlevelHeader();
    mdvx->printVlevelHeader(vhdr, fhdr.nz, fhdr.field_name, cout);

    if (_params.print_data) {
      if (_params.read_transform_to_linear) {
	if (field->transform2Linear()){
	  cerr << field->getErrStr() << endl;
	}
      }
      field->printTimeHeightData(cout,
				 mdvx->getValidTimes(),
				 _params.print_native);
    }

  } // i

}

//////////////////////////////
// do test for retrieval speed

int CedricPrint::_doTest(DsMdvx *mdvx)

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    mdvx->setDebug();
    mdvx->printReadRequest(cerr);
  }

  if (_params.debug) {
    cerr << "Testing MDV retrieval" << endl;
    cerr << "  n retreivals: " << _params.test_n_retrievals << endl;
  }
  
  for (int i = 0; i < _params.test_n_retrievals; i++) {

    if (_params.get_mode == Params::GET_VOLUME ||
	_params.get_mode == Params::GET_GIS ||
	_params.get_mode == Params::GET_TABLE) {
      if (mdvx->readVolume()) {
	cerr << mdvx->getErrStr();
	cerr << "******* ABORTING TEST!!!! *******" << endl;
	return -1;
      }
    } else if (_params.get_mode == Params::GET_VSECTION) {
      if (mdvx->readVsection()) {
	cerr << mdvx->getErrStr();
	cerr << "******* ABORTING TEST!!!! *******" << endl;
	return -1;
      }
    }

  }
  
  return 0;

}

#endif
