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
// DataSet.cc
//
// DataSet object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2008
//
///////////////////////////////////////////////////////////////

#include <toolsa/toolsa_macros.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <toolsa/TaArray.hh>
#include <dataport/port_types.h>
#include <dsserver/DsLdataInfo.hh>
#include <Mdv/MdvxField.hh>
#include "DataSet.hh"
using namespace std;

const fl32 DataSet::_missingFloat = -9999.0;

// Constructor

DataSet::DataSet(const Params &params,
		 const string &fileNameSubString,
		 const string &fieldName,
		 const string &fieldNameLong,
		 const string &units,
		 double dataScale,
		 double dataOffset,
		 int gridNx,
		 int gridNy,
		 double gridMinx,
		 double gridMiny,
		 double gridDx,
		 double gridDy,
		 const string &outputUrl) :
  _params(params),
  _fileNameSubString(fileNameSubString),
  _fieldName(fieldName),
  _fieldNameLong(fieldNameLong),
  _units(units),
  _dataScale(dataScale),
  _dataOffset(dataOffset),
  _nx(gridNx),
  _ny(gridNy),
  _minx(gridMinx),
  _miny(gridMiny),
  _dx(gridDx),
  _dy(gridDy),
  _outputUrl(outputUrl)

{

  // initialize output projection and grid
  
  _proj.setGrid(_nx, _ny, _dx, _dy, _minx, _miny);
  
  if (_params.output_projection == Params::PROJ_LATLON) {
    double midLon = _minx + _nx * _dx / 2.0;
    _proj.initLatlon(midLon);
  } else if (_params.output_projection == Params::PROJ_FLAT) {
    _proj.initFlat(_params.proj_origin_lat,
		   _params.proj_origin_lon,
		   _params.proj_rotation);
  } else if (_params.output_projection == Params::PROJ_LAMBERT_CONF) {
    _proj.initLambertConf(_params.proj_origin_lat,
			  _params.proj_origin_lon,
			  _params.proj_lat1,
			  _params.proj_lat2);
  } else if (_params.output_projection == Params::PROJ_POLAR_STEREO) {
    Mdvx::pole_type_t poleType = Mdvx::POLE_NORTH;
    if (!_params.proj_pole_is_north) {
      poleType = Mdvx::POLE_SOUTH;
    }
    _proj.initPolarStereo(_params.proj_origin_lat,
			  _params.proj_origin_lon,
			  _params.proj_tangent_lon,
			  poleType,
			  _params.proj_central_scale);
  } else if (_params.output_projection == Params::PROJ_OBLIQUE_STEREO) {
    _proj.initObliqueStereo(_params.proj_origin_lat,
			    _params.proj_origin_lon,
			    _params.proj_tangent_lat,
			    _params.proj_tangent_lon);
  } else if (_params.output_projection == Params::PROJ_MERCATOR) {
    _proj.initMercator(_params.proj_origin_lat,
		       _params.proj_origin_lon);
  } else if (_params.output_projection == Params::PROJ_TRANS_MERCATOR) {
    _proj.initTransMercator(_params.proj_origin_lat,
			    _params.proj_origin_lon,
			    _params.proj_central_scale);
  } else if (_params.output_projection == Params::PROJ_ALBERS) {
    _proj.initAlbers(_params.proj_origin_lat,
		     _params.proj_origin_lon,
		     _params.proj_lat1,
		     _params.proj_lat2);
  } else if (_params.output_projection == Params::PROJ_LAMBERT_AZIM) {
    _proj.initLambertAzim(_params.proj_origin_lat,
			  _params.proj_origin_lon);
  } else if (_params.output_projection == Params::PROJ_VERT_PERSP) {
    _proj.initVertPersp(_params.proj_origin_lat,
			_params.proj_origin_lon,
			_params.proj_persp_radius);
  }
  
  return;

}

// destructor

DataSet::~DataSet()

{

}

////////////////////////////////////////////
// check if appropriate for given file path
//
// Is considered appropriate if the file name
// substring is contained in the file name, or if
// the substring is zero-length.

bool DataSet::appropriateForFile(const char *input_path)

{

  if (_fileNameSubString.size() == 0) {
    return true;
  }
  
  Path inputPath(input_path);
  string fileName = inputPath.getFile();
  if (fileName.find(_fileNameSubString) != string::npos) {
    return true;
  }

  return false;

}

///////////////////////////////
// process file

int DataSet::processFile(const char *input_path)

{

  PMU_auto_register("Processing file");

  // open file

  Nc3File ncf(input_path);
  if (!ncf.is_valid()) {
    cerr << "ERROR - DataSet::_processFile" << endl;
    cerr << "  File: " << input_path << endl;
    return -1;
  }

  // declare an error object

  Nc3Error err(Nc3Error::silent_nonfatal);

  if (_params.debug >= Params::DEBUG_EXTRA) {
    _printFile(ncf);
  }

  // check that this is a valid file

  if (_checkFile(ncf)) {
    cerr << "ERROR - DataSet::_processFile" << endl;
    cerr << "  Not a valid McIdas file" << endl;
    cerr << "  File: " << input_path << endl;
    return -1;
  }

  // create output Mdvx file object
  
  DsMdvx mdvx;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    mdvx.setDebug(true);
  }

  // set master header
  
  if (_setMasterHeader(ncf, mdvx)) {
    return -1;
  }

  // add the data field
  
  if (_addDataField(ncf, mdvx)) {
    return -1;
  }

  // write output file

  if (mdvx.writeToDir(_outputUrl)) {
    cerr << "ERROR - DataSet" << endl;
    cerr << "  Cannot write file to url: " << _outputUrl << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "  Wrote output file: " << mdvx.getPathInUse() << endl;
  }

  return 0;

}

//////////////////////////////////
// Check that this is a valid file
//
// Returns 0 on success, -1 on failure

int DataSet::_checkFile(Nc3File &ncf)

{

  if (ncf.get_dim(_params.netcdf_dim_n_lines) == NULL) {
    cerr << "ERROR - DataSet::_checkFile" << endl;
    cerr << "  dimension missing: " << _params.netcdf_dim_n_lines << endl;
    return -1;
  }
  
  if (ncf.get_dim(_params.netcdf_dim_n_elems) == NULL) {
    cerr << "ERROR - DataSet::_checkFile" << endl;
    cerr << "  dimension missing: " << _params.netcdf_dim_n_elems << endl;
    return -1;
  }
  
  if (ncf.get_var(_params.netcdf_var_image_date) == NULL) {
    cerr << "ERROR - DataSet::_checkFile" << endl;
    cerr << "  variable missing" << _params.netcdf_var_image_date << endl;
    return -1;
  }
  
  if (ncf.get_var(_params.netcdf_var_image_time) == NULL) {
    cerr << "ERROR - DataSet::_checkFile" << endl;
    cerr << "  variable missing" << _params.netcdf_var_image_time << endl;
    return -1;
  }

  if (ncf.get_var(_params.netcdf_var_line_res) == NULL) {
    cerr << "ERROR - DataSet::_checkFile" << endl;
    cerr << "  variable missing" << _params.netcdf_var_line_res << endl;
    return -1;
  }

  if (ncf.get_var(_params.netcdf_var_elem_res) == NULL) {
    cerr << "ERROR - DataSet::_checkFile" << endl;
    cerr << "  variable missing" << _params.netcdf_var_elem_res << endl;
    return -1;
  }

  if (ncf.get_var(_params.netcdf_var_image_data) == NULL) {
    cerr << "ERROR - DataSet::_checkFile" << endl;
    cerr << "  variable missing" << _params.netcdf_var_image_data << endl;
    return -1;
  }

  if (ncf.get_var(_params.netcdf_var_latitude) == NULL) {
    cerr << "ERROR - DataSet::_checkFile" << endl;
    cerr << "  variable missing" << _params.netcdf_var_latitude << endl;
    return -1;
  }

  if (ncf.get_var(_params.netcdf_var_longitude) == NULL) {
    cerr << "ERROR - DataSet::_checkFile" << endl;
    cerr << "  variable missing" << _params.netcdf_var_longitude << endl;
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////
// Set the master header from the NCF file
//
// Returns 0 on success, -1 on failure

int DataSet::_setMasterHeader(Nc3File &ncf, DsMdvx &mdvx)

{

  mdvx.clearMasterHeader();

  // image time

  Nc3Var *imageDateVar = ncf.get_var(_params.netcdf_var_image_date);
  if (imageDateVar == NULL) {
    cerr << "ERROR - DataSet::_setMasterHeader" << endl;
    cerr << "  variable missing" << _params.netcdf_var_image_date << endl;
    return -1;
  }
  int idate = imageDateVar->as_int(0);
  int iyear = idate / 1000;
  if (iyear < 100) {
    iyear += 1900;
  }
  int iday = idate % 1000;

  Nc3Var *imageTimeVar = ncf.get_var(_params.netcdf_var_image_time);
  if (imageTimeVar == NULL) {
    cerr << "ERROR - DataSet::_setMasterHeader" << endl;
    cerr << "  variable missing" << _params.netcdf_var_image_time << endl;
    return -1;
  }
  int itime = imageTimeVar->as_int(0);
  int ihour = itime / 10000;
  int ileft = itime % 10000;
  int imin = ileft / 100;
  int isec = ileft % 100;
  
  DateTime imageTime(iyear, 1, iday, ihour, imin, isec);
  Nc3Var *timeVar = ncf.get_var(_params.netcdf_var_time);
  if (timeVar != NULL) {
    int itime = timeVar->as_int(0);
    imageTime.set(itime);
  }

  _imageTime = imageTime.utime();
  mdvx.setBeginTime(imageTime.utime());
  _validTime = imageTime.utime() + _params.valid_time_offset;
  mdvx.setValidTime(_validTime);
  
  // CR time

  Nc3Var *crDateVar = ncf.get_var(_params.netcdf_var_creation_date);
  Nc3Var *crTimeVar = ncf.get_var(_params.netcdf_var_creation_time);
  if (crDateVar != NULL && crTimeVar != NULL) {
    int jdate = crDateVar->as_int(0);
    int jyear = jdate / 1000;
    int jday = jdate % 1000;
    int jtime = crTimeVar->as_int(0);
    int jhour = jtime / 10000;
    int jleft = jtime % 10000;
    int jmin = jleft / 100;
    int jsec = jleft % 100;
    DateTime crTime(jyear, 1, jday, jhour, jmin, jsec);
    if (crTime.utime() - imageTime.utime() < 2700) {
      mdvx.setEndTime(crTime.utime());
    } else {
      mdvx.setEndTime(_validTime);
    }
  }

  // data collection type

  mdvx.setDataCollectionType(Mdvx::DATA_MEASURED);

  // data set name and source

  mdvx.setDataSetName(_params.data_set_name);
  mdvx.setDataSetSource(_params.data_set_source);

  // data set info

  string dataSetInfo;

  Nc3Var *sensorIdVar = ncf.get_var(_params.netcdf_var_sensor_id);
  _sensorId = 0;
  if (sensorIdVar != NULL) {
    _sensorId = sensorIdVar->as_int(0);
    char text[128];
    sprintf(text, "sensorId: %d\n", _sensorId);
    dataSetInfo += text;
  }

  Nc3Var *auditTrailVar = ncf.get_var(_params.netcdf_var_audit_trail);
  if (auditTrailVar != NULL) {
    dataSetInfo += auditTrailVar->as_string(0);
    dataSetInfo += "\n";
  }

  mdvx.setDataSetInfo(dataSetInfo.c_str());

  return 0;

}

/////////////////////////////////////////////////
// Add the data field
//
// Returns 0 on success, -1 on failure

int DataSet::_addDataField(Nc3File &ncf,
			   DsMdvx &mdvx)

{

  // init

  _inputIsRegularLatlon = false;
  _preserveLatlonGrid = false;
  
  // get dimensions of input data field

  Nc3Dim *linesDim = ncf.get_dim(_params.netcdf_dim_n_lines);
  if (linesDim == NULL) {
    cerr << "ERROR - DataSet::_addDataField" << endl;
    cerr << "  dimension missing: " << _params.netcdf_dim_n_lines << endl;
    return -1;
  }
  int nLines = linesDim->size();
  
  Nc3Dim *elemsDim = ncf.get_dim(_params.netcdf_dim_n_elems);
  if (elemsDim == NULL) {
    cerr << "ERROR - DataSet::_addDataField" << endl;
    cerr << "  dimension missing" << _params.netcdf_dim_n_elems << endl;
    return -1;
  }
  int nElems = elemsDim->size();

  Nc3Dim *bandsDim = ncf.get_dim(_params.netcdf_dim_n_bands);
  int nBands = 1;
  if (bandsDim != NULL) {
    nBands = bandsDim->size();
  } else {
    cerr << "NOTE - DataSet::_addDataField" << endl;
    cerr << "  no bands dim found: " << _params.netcdf_dim_n_bands << endl;
    cerr << "  setting nBands = 1" << endl;
  }

  // lat / lon arrays

  Nc3Var *latVar = ncf.get_var(_params.netcdf_var_latitude);
  if (latVar == NULL) {
    cerr << "ERROR - DataSet::_addDataField" << endl;
    cerr << "  variable missing" << _params.netcdf_var_latitude << endl;
    return -1;
  }

  Nc3Var *lonVar = ncf.get_var(_params.netcdf_var_longitude);
  if (lonVar == NULL) {
    cerr << "ERROR - DataSet::_addDataField" << endl;
    cerr << "  variable missing" << _params.netcdf_var_longitude << endl;
    return -1;
  }

  TaArray<float> latArray_;
  float *latArray = (float *) latArray_.alloc(nLines * nElems);
  if (latVar->get(latArray, nLines, nElems) == 0) {
    cerr << "ERROR - DataSet::_addDataField" << endl;
    cerr << "  Cannot get latitudes from input netcdf variable" << endl;
    return -1;
  }
  
  TaArray<float> lonArray_;
  float *lonArray = (float *) lonArray_.alloc(nLines * nElems);
  if (lonVar->get(lonArray, nLines, nElems) == 0) {
    cerr << "ERROR - DataSet::_addDataField" << endl;
    cerr << "  Cannot get longitudes from input netcdf variable" << endl;
    return -1;
  }

  // check for latlon input grid by examining the latitudes in each line

  _checkForRegularLatLonInput(nLines, nElems, latArray, lonArray);

  // get grid input resolution

  Nc3Var *lineResVar = ncf.get_var(_params.netcdf_var_line_res);
  if (lineResVar != NULL) {
    _lineRes = lineResVar->as_double(0);
  } else {
    _lineRes = 1.0;
  }
  
  Nc3Var *elemResVar = ncf.get_var(_params.netcdf_var_elem_res);
  if (elemResVar != NULL) {
    _elemRes = elemResVar->as_double(0);
  } else {
    _elemRes = 1.0;
  }

  // compute the oversampling values, so that we know how far
  // to copy the input data around the input lat/lon

  if (_params.output_projection == Params::PROJ_LATLON) {
    
    double dx =
      _dx * KM_PER_DEG_AT_EQ / cos(_params.proj_origin_lat * DEG_TO_RAD);
    double dy = _dy * KM_PER_DEG_AT_EQ;

    _oversampleX = (int) ((_elemRes / dx) * 2.5 + 0.5);
    _oversampleY = (int) ((_lineRes / dy) * 2.5 + 0.5);
    
    if (_inputIsRegularLatlon) {
      dx = _dx;
      dy = _dy;
      _oversampleX = 1;
      _oversampleY = 1;
    }

  } else {

    _oversampleX = (int) ((_elemRes / _dx) * 2.5 + 0.5);
    _oversampleY = (int) ((_lineRes / _dy) * 2.5 + 0.5);

  }
  
  if (_params.debug) {
    cerr << "dy, dx: " << _dy << ", " << _dx << endl;
    cerr << "lineRes, elemRes: " << _lineRes << ", " << _elemRes << endl;
    cerr << "_oversampleX, _oversampleY: " 
         << _oversampleX << ", " << _oversampleY << endl;
  }

  // get data in netCDF file

  Nc3Var *dataVar = ncf.get_var(_params.netcdf_var_image_data);
  if (dataVar == NULL) {
    cerr << "ERROR - DataSet::_addDataField" << endl;
    cerr << "  variable missing" << _params.netcdf_var_image_data << endl;
    return -1;
  }
  
  // get input data, and associated lat/lon location
  
  Nc3Var *dataWidthVar = ncf.get_var(_params.netcdf_var_data_width);
  if (dataWidthVar != NULL) {
    _dataWidth = dataWidthVar->as_int(0);
  } else {
    _dataWidth = 4;
  }
  
  TaArray<float> dataArrayFloat_;
  TaArray<short> dataArrayShort_;
  TaArray<ncbyte> dataArrayNcbyte_;
  
  float *dataArrayFloat = NULL;
  short *dataArrayShort = NULL;
  ncbyte *dataArrayNcbyte = NULL;
  
  if (_dataWidth == 1) {
    
    dataArrayNcbyte = dataArrayNcbyte_.alloc(nBands * nLines * nElems);
    if (dataVar->get(dataArrayNcbyte, nBands, nLines, nElems) == 0) {
      cerr << "ERROR - DataSet::_addDataField" << endl;
      cerr << "  Cannot get data from input netcdf variable" << endl;
      cerr << "  _dataWidth: " << _dataWidth << endl;
      return -1;
    }

  } else if (_dataWidth == 2) {

    dataArrayShort = dataArrayShort_.alloc(nBands * nLines * nElems);
    if (dataVar->get(dataArrayShort, nBands, nLines, nElems) == 0) {
      cerr << "ERROR - DataSet::_addDataField" << endl;
      cerr << "  Cannot get data from input netcdf variable" << endl;
      cerr << "  _dataWidth: " << _dataWidth << endl;
      return -1;
    }

  } else {

    dataArrayFloat = dataArrayFloat_.alloc(nBands * nLines * nElems);
    if (dataVar->get(dataArrayFloat, nBands, nLines, nElems) == 0) {
      cerr << "ERROR - DataSet::_addDataField" << endl;
      cerr << "  Cannot get data from input netcdf variable" << endl;
      cerr << "  _dataWidth: " << _dataWidth << endl;
      return -1;
    }

  }
  
  // set up output data grids, distance error and lat/lon arrays

  fl32 **outputData = (fl32 **) umalloc2(_ny, _nx, sizeof(fl32));
  fl32 **distanceError = (fl32 **) umalloc2(_ny, _nx, sizeof(fl32));
  fl32 **outputLat = (fl32 **) umalloc2(_ny, _nx, sizeof(fl32));
  fl32 **outputLon = (fl32 **) umalloc2(_ny, _nx, sizeof(fl32));
  
  double yy = _miny;
  for (int iy = 0; iy < _ny; iy++, yy += _dy) {
    double xx = _minx;
    for (int ix = 0; ix < _nx; ix++, xx += _dx) {
      outputData[iy][ix] = _missingFloat;
      distanceError[iy][ix] = 1.0e10;
      double llat, llon;
      _proj.xy2latlon(xx, yy, llat, llon);
      outputLat[iy][ix] = llat;
      outputLon[iy][ix] = llon;
    }
  }

  // loop through the input data, navigating and inserting in output array
  
  int count = 0;
  for (int iline = 0; iline < nLines; iline++) {
    for (int ielem = 0; ielem < nElems; ielem++, count++) {
      float dataVal = 0;
      if (_dataWidth == 1) {
	dataVal = dataArrayNcbyte[count] * _dataScale + _dataOffset;
      } else if (_dataWidth == 2) {
	dataVal = dataArrayShort[count] * _dataScale + _dataOffset;
      } else {
	dataVal = dataArrayFloat[count] * _dataScale + _dataOffset;
      }
      if (_preserveLatlonGrid) {
        outputData[nLines - iline - 1][ielem] = dataVal;
      } else {
        double lat = latArray[count];
        double lon = lonArray[count];
        int ix, iy;
        if (_proj.latlon2xyIndex(lat, lon, ix, iy) == 0) {
          _insertDataVal(dataVal, lat, lon, ix, iy,
                         outputData, distanceError,
                         outputLat, outputLon);
        }
      }
    }
  }

  // scale into albedo if required

  if (_params.convert_counts_to_albedo) {

    double scale = _params.counts_to_albedo_scale;

    _sunAngle.initForTime(_imageTime);

    // scale the values
    for (int iy = 0; iy < _ny; iy++) {
      for (int ix = 0; ix < _nx; ix++) {
        double sinAlt =
          _sunAngle.computeSinAlt(outputLat[iy][ix], outputLon[iy][ix]);
        if (sinAlt < 0.15) {
          sinAlt = 0.15;
        }
        double radiance = outputData[iy][ix] * scale;
        double reflectance = radiance / sinAlt;
        if (reflectance > 100.0) {
          reflectance = 100.0;
        }
        outputData[iy][ix] = reflectance;
      }
    }

  } // if (_params.scale_counts_to_albedo)

  // set up MdvxField headers

  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);

  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;
  
  fhdr.native_vlevel_type = Mdvx::VERT_SATELLITE_IMAGE;
  fhdr.vlevel_type = Mdvx::VERT_SATELLITE_IMAGE;
  fhdr.dz_constant = true;

  fhdr.bad_data_value = _missingFloat;
  fhdr.missing_data_value = _missingFloat;
  
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = sizeof(fl32);
  
  _proj.syncToFieldHdr(fhdr);
  
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  
  vhdr.type[0] = Mdvx::VERT_SATELLITE_IMAGE;
  vhdr.level[0] = 0.0;

  // create MdvxField object, converting data to encoding and compression types

  MdvxField *field = new MdvxField(fhdr, vhdr, *outputData);
  field->convertType((Mdvx::encoding_type_t) _params.output_encoding_type,
		     (Mdvx::compression_type_t) _params.output_compression_type);

  // set names etc
  
  field->setFieldName(_fieldName.c_str());
  field->setFieldNameLong(_fieldNameLong.c_str());
  field->setUnits(_units.c_str());
  field->setTransform("");
  
  // add to Mdvx

  mdvx.addField(field);

  // write out Zebra data set if applicable

  if (_params.write_zebra_netcdf && fhdr.proj_type == Mdvx::PROJ_LATLON) {
    for (int ii = 0; ii < _params.zebra_datasets_n; ii++) {
      Params::zebra_dataset_t &zebdata = _params._zebra_datasets[ii];
      if (_fileNameSubString == zebdata.file_name_sub_string) {
        _writeZebraNetCDF(zebdata.zebra_data_set_name,
                          zebdata.zebra_field_name,
                          _units,
                          outputData);
      }
    } // ii
  }

  // free up 2D arrays

  ufree2((void **) outputData);
  ufree2((void **) distanceError);
  ufree2((void **) outputLat);
  ufree2((void **) outputLon);

  return 0;

}

///////////////////////////////
// insert data value into array

void DataSet::_insertDataVal(float dataVal,
                             float lat, float lon,
                             int ix, int iy,
                             fl32 **outputData,
                             fl32 **distanceError,
                             fl32 **outputLat,
                             fl32 **outputLon)
  
{
	
  // loop through the points surrounding and including the
  // selected point, setting the dataVal for points at which
  // the distanceError is less than that already there

  for (int deltay = -_oversampleY; deltay <= _oversampleY; deltay++) {
    for (int deltax = -_oversampleX; deltax <= _oversampleX; deltax++) {

      int jy = iy + deltay;
      int jx = ix + deltax;
      
      if (jy >= 0 && jy < _ny && jx >= 0 && jx < _nx) {
	
	double dlat = outputLat[jy][jx] - lat;
	double dlon = outputLon[jy][jx] - lon;
	double dist = sqrt(dlat * dlat + dlon * dlon);
	if (dist < distanceError[jy][jx]) {
	  outputData[jy][jx] = dataVal;
	  distanceError[jy][jx] = dist;
	}
	
      } // if (jy >= 0 ...
      
    } // deltax
  } // deltay
  
}

////////////////////////////////////////////////////
// check for regular latlon input grid

void DataSet::_checkForRegularLatLonInput(int nLines, int nElems,
                                          const float *latArray,
                                          const float *lonArray)

{

  _inputIsRegularLatlon = true;
  
  // check that rows have constant lat

  _meanLat.clear();
  for (int ii = 0; ii < nLines; ii++) {
    double minLat = 90.0;
    double maxLat = -90.0;
    double sum = 0.0;
    int index = ii * nElems;
    for (int jj = 0; jj < nElems; jj++, index++) {
      double lat = latArray[index];
      sum += lat;
      if (lat < minLat) minLat = lat;
      if (lat > maxLat) maxLat = lat;
    } // jj
    if (maxLat - minLat > 0.001) {
      _inputIsRegularLatlon = false;
      return;
    }
    double mean = sum / nElems;
    _meanLat.push_back(mean);
  } // ii

  // check for constant delta lat

  double meanDeltaLat =
    (_meanLat[0] - _meanLat[_meanLat.size()-1]) / (_meanLat.size() - 1.0);
  for (size_t ii = 1; ii < _meanLat.size(); ii++) {
    double deltaLat = _meanLat[ii-1] - _meanLat[ii];
    if (fabs(meanDeltaLat - deltaLat) > 0.001) {
      _inputIsRegularLatlon = false;
      return;
    }
  }

  // check that rows have constant lon

  _meanLon.clear();
  for (int ii = 0; ii < nElems; ii++) {
    double minLon = 720.0;
    double maxLon = -720.0;
    double sum = 0.0;
    int index = ii;
    for (int jj = 0; jj < nLines; jj++, index += nElems) {
      double lon = lonArray[index];
      sum += lon;
      if (lon < minLon) minLon = lon;
      if (lon > maxLon) maxLon = lon;
    } // jj
    if (maxLon - minLon > 0.001) {
      _inputIsRegularLatlon = false;
      return;
    }
    double mean = sum / nLines;
    _meanLon.push_back(mean);
  } // ii

  // check for constant delta lon
  
  double meanDeltaLon =
    (_meanLon[_meanLon.size()-1] - _meanLon[0]) / (_meanLon.size() - 1.0);
  if (meanDeltaLon < 0) {
    meanDeltaLon += 360.0 / (_meanLon.size() - 1.0);
  }
  for (size_t ii = 1; ii < _meanLon.size(); ii++) {
    double deltaLon = _meanLon[ii] - _meanLon[ii - 1];
    if (deltaLon < 0) {
      deltaLon += 360.0;
    }
    if (fabs(meanDeltaLon - deltaLon) > 0.001) {
      _inputIsRegularLatlon = false;
      return;
    }
  }

  _inputNx = nElems;
  _inputNy = nLines;
  _inputMinx = _meanLon[0];
  _inputMiny = _meanLat[nLines-1];
  _inputDx = meanDeltaLon;
  _inputDy = meanDeltaLat;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Identified regular lat/lon grid" << endl;
    cerr << "  nx, ny: " << _inputNx << ", " << _inputNy << endl;
    cerr << "  dx, dy: " << _inputDx << ", " << _inputDy << endl;
    cerr << "  minx, miny: " << _inputMinx << ", " << _inputMiny << endl;
  }

  // if the output projection is latlon, the optionally use
  // the input grid as the output grid

  if (_params.output_projection == Params::PROJ_LATLON &&
      _params.preserve_input_geom_for_regular_latlon) {
    _preserveLatlonGrid = true;
    _nx = nElems;
    _ny = nLines;
    _minx = _meanLon[0];
    _miny = _meanLat[nLines-1];
    _dx = meanDeltaLon;
    _dy = meanDeltaLat;
    double midLon = _minx + _nx * _dx / 2.0;
    _proj.setGrid(_nx, _ny, _dx, _dy, _minx, _miny);
    _proj.initLatlon(midLon);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "NOTE - preserving latlon grid from input data" << endl;
    }
  }

}

///////////////////////////////
// print data in file

void DataSet::_printFile(Nc3File &ncf)

{

  cout << "ndims: " << ncf.num_dims() << endl;
  cout << "nvars: " << ncf.num_vars() << endl;
  cout << "ngatts: " << ncf.num_atts() << endl;
  Nc3Dim *unlimd = ncf.rec_dim();
  if (unlimd != NULL) {
    cout << "unlimdimid: " << unlimd->size() << endl;
  }
  
  // dimensions

  Nc3Dim *dims[ncf.num_dims()];
  for (int idim = 0; idim < ncf.num_dims(); idim++) {
    dims[idim] = ncf.get_dim(idim);

    cout << endl;
    cout << "Dim #: " << idim << endl;
    cout << "  Name: " << dims[idim]->name() << endl;
    cout << "  Length: " << dims[idim]->size() << endl;
    cout << "  Is valid: " << dims[idim]->is_valid() << endl;
    cout << "  Is unlimited: " << dims[idim]->is_unlimited() << endl;
    
  } // idim
  
  cout << endl;

  // global attributes

  cout << "Global attributes:" << endl;

  for (int iatt = 0; iatt < ncf.num_atts(); iatt++) {
    cout << "  Att num: " << iatt << endl;
    Nc3Att *att = ncf.get_att(iatt);
    _printAtt(att);
    delete att;
  }

  // loop through variables

  Nc3Var *vars[ncf.num_vars()];
  for (int ivar = 0; ivar < ncf.num_vars(); ivar++) {

    vars[ivar] = ncf.get_var(ivar);
    cout << endl;
    cout << "Var #: " << ivar << endl;
    cout << "  Name: " << vars[ivar]->name() << endl;
    cout << "  Is valid: " << vars[ivar]->is_valid() << endl;
    cout << "  N dims: " << vars[ivar]->num_dims();
    Nc3Dim *vdims[vars[ivar]->num_dims()];
    if (vars[ivar]->num_dims() > 0) {
      cout << ": (";
      for (int ii = 0; ii < vars[ivar]->num_dims(); ii++) {
	vdims[ii] = vars[ivar]->get_dim(ii);
	cout << " " << vdims[ii]->name();
	if (ii != vars[ivar]->num_dims() - 1) {
	  cout << ", ";
	}
      }
      cout << " )";
    }
    cout << endl;
    cout << "  N atts: " << vars[ivar]->num_atts() << endl;
    
    for (int iatt = 0; iatt < vars[ivar]->num_atts(); iatt++) {

      cout << "  Att num: " << iatt << endl;
      Nc3Att *att = vars[ivar]->get_att(iatt);
      _printAtt(att);
      delete att;

    } // iatt

    cout << endl;
    _printVarVals(vars[ivar]);
    
  } // ivar
  
}

/////////////////////
// print an attribute

void DataSet::_printAtt(Nc3Att *att)

{

  cout << "    Name: " << att->name() << endl;
  cout << "    Num vals: " << att->num_vals() << endl;
  cout << "    Type: ";
  
  Nc3Values *values = att->values();

  switch(att->type()) {
    
  case nc3NoType: {
    cout << "No type: ";
  }
  break;
  
  case nc3Byte: {
    cout << "BYTE: ";
    unsigned char *vals = (unsigned char *) values->base();
    for (long ii = 0; ii < att->num_vals(); ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case nc3Char: {
    cout << "CHAR: ";
    char vals[att->num_vals() + 1];
    MEM_zero(vals);
    memcpy(vals, values->base(), att->num_vals());
    cout << vals;
  }
  break;
  
  case nc3Short: {
    cout << "SHORT: ";
    short *vals = (short *) values->base();
    for (long ii = 0; ii < att->num_vals(); ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case nc3Int: {
    cout << "INT: ";
    int *vals = (int *) values->base();
    for (long ii = 0; ii < att->num_vals(); ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case nc3Float: {
    cout << "FLOAT: ";
    float *vals = (float *) values->base();
    for (long ii = 0; ii < att->num_vals(); ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case nc3Double: {
    cout << "DOUBLE: ";
    double *vals = (double *) values->base();
    for (long ii = 0; ii < att->num_vals(); ii++) {
      cout << " " << vals[ii];
    }
  }
  break;

  default: {}
  
  }
  
  cout << endl;

  delete values;

}

    
///////////////////////////////
// print variable values

void DataSet::_printVarVals(Nc3Var *var)

{

  int nprint = var->num_vals();
  if (nprint > 100) {
    nprint = 100;
  }

  Nc3Values *values = var->values();

  cout << "  Variable vals:";
  
  switch(var->type()) {
    
  case nc3NoType: {
  }
  break;
  
  case nc3Byte: {
    cout << "(byte)";
    unsigned char *vals = (unsigned char *) values->base();
    for (long ii = 0; ii < nprint; ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case nc3Char: {
    cout << "(char)";
    char str[nprint + 1];
    MEM_zero(str);
    memcpy(str, values->base(), nprint);
    cout << " " << str;
  }
  break;
  
  case nc3Short: {
    cout << "(short)";
    short *vals = (short *) values->base();
    for (long ii = 0; ii < nprint; ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case nc3Int: {
    cout << "(int)";
    int *vals = (int *) values->base();
    for (long ii = 0; ii < nprint; ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case nc3Int64: {
    cout << "(int64)";
    int64_t *vals = (int64_t *) values->base();
    for (long ii = 0; ii < nprint; ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case nc3Float: {
    cout << "(float)";
    float *vals = (float *) values->base();
    for (long ii = 0; ii < nprint; ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case nc3Double: {
    cout << "(double)";
    double *vals = (double *) values->base();
    for (long ii = 0; ii < nprint; ii++) {
      cout << " " << vals[ii];
    }
  }
  break;

  }
  
  cout << endl;

  delete values;

}

/////////////////////////////////
// write Zebra-style netCDF file

int DataSet::_writeZebraNetCDF(const string &dataSetName,
                               const string &fieldName,
                               const string &units,
                               fl32 **floatData)

{

  DateTime outputTime(_validTime);

  // compute dir path

  string zebDir;
  zebDir += _params.zebra_netcdf_dir;

  if (_params.zebra_write_to_day_dir) {
    char dayDir[32];
    sprintf(dayDir, "%.4d%.2d%.2d",
            outputTime.getYear(), outputTime.getMonth(), outputTime.getDay());
    zebDir += PATH_DELIM;
    zebDir += dayDir;
  }

  // make dir

  if (ta_makedir_recurse(zebDir.c_str())) {
    int errNum = errno;
    cerr << "ERROR - DataSet::_writeZebraNetCDF" << endl;
    cerr << "  Cannot make zebra output dir: " << zebDir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // compute file name
  
  char fileName[1024];
  sprintf(fileName, "%s.%.4d%.2d%.2d.%.2d%.2d%.2d.%s.nc",
          dataSetName.c_str(),
          outputTime.getYear(), outputTime.getMonth(), outputTime.getDay(),
          outputTime.getHour(), outputTime.getMin(), outputTime.getSec(),
          fieldName.c_str());

  // compute file path
          
  string zebPath(zebDir);
  zebPath += PATH_DELIM;
  zebPath += fileName;

  if (_params.debug) {
    cerr << "Writing zebra file: " << zebPath << endl;
  }

  // open file

  Nc3File zebFile(zebPath.c_str(), Nc3File::Replace, NULL, 0, Nc3File::Classic);
  if (!zebFile.is_valid()) {
    cerr << "ERROR - DataSet::_writeZebraNetCDF" << endl;
    cerr << "  Cannot open netCDF file: " << zebPath << endl;
    return -1;
  }

  // create error object

  Nc3Error zebErr(Nc3Error::silent_nonfatal);

  // add global attributes

  int iret = 0;
  iret |= !zebFile.add_att("projection" , (int) 2);
  iret |= !zebFile.add_att("projection_name" , "rectangular");
  iret |= !zebFile.add_att("sensor" , _sensorId);
  iret |= !zebFile.add_att("data_set_name", _params.data_set_name);
  iret |= !zebFile.add_att("data_set_source", _params.data_set_source);

  if (iret) {
    cerr << "ERROR - DataSet::_writeZebraNetCDF" << endl;
    cerr << "  Cannot add attributes to  file: " << zebPath << endl;
    return -1;
  }

  // add time dimension

  Nc3Dim *timeDim = zebFile.add_dim("time", 1);
  if (timeDim == NULL) {
    cerr << "ERROR - DataSet::_writeZebraNetCDF" << endl;
    cerr << "  Cannot add time dim to  file: " << zebPath << endl;
    return -1;
  }

  // add latitude and longitude dimensions

  Nc3Dim *latDim = zebFile.add_dim("latitude",  _ny);
  if (latDim == NULL) {
    cerr << "ERROR - DataSet::_writeZebraNetCDF" << endl;
    cerr << "  Cannot add latitude dim to  file: " << zebPath << endl;
    return -1;
  }

  Nc3Dim *lonDim = zebFile.add_dim("longitude",  _nx);
  if (lonDim == NULL) {
    cerr << "ERROR - DataSet::_writeZebraNetCDF" << endl;
    cerr << "  Cannot add longitude dim to  file: " << zebPath << endl;
    return -1;
  }

  // add variables

  Nc3Var *timeVar = zebFile.add_var("time", nc3Int, timeDim);
  timeVar->add_att("units", "seconds since 1970-01-01 00:00:00 +0000");
  timeVar->add_att("missing_value", -2147483647);
  timeVar->add_att("valid_min", -2147483646);
  timeVar->add_att("valid_max", 2147483647);

  Nc3Var *latVar = zebFile.add_var("latitude", nc3Float, latDim);
  latVar->add_att("units", "degrees_north");
  latVar->add_att("missing_value", -9999.0);
  latVar->add_att("valid_min", -90.0);
  latVar->add_att("valid_max", 90.0);

  Nc3Var *lonVar = zebFile.add_var("longitude", nc3Float, lonDim);
  lonVar->add_att("units", "degrees_east");
  lonVar->add_att("missing_value", -9999.0);
  lonVar->add_att("valid_min", -360.0);
  lonVar->add_att("valid_max", 360.0);

  Nc3Var *dataVar = zebFile.add_var(fieldName.c_str(), nc3Short, timeDim, latDim, lonDim);
  dataVar->add_att("units", units.c_str());
  dataVar->add_att("missing_value", (short) -32768);
  dataVar->add_att("valid_min", (short) -32767);
  dataVar->add_att("valid_max", (short) 32767);
  dataVar->add_att("scale_factor", 0.01);

  // load up lat and lon arrays

  TaArray<float> lats_;
  float *lats = lats_.alloc(_meanLat.size());
  for (size_t ii = 0; ii < _meanLat.size(); ii++) {
    lats[ii] = _meanLat[ii];
  }

  TaArray<float> lons_;
  float *lons = lons_.alloc(_meanLon.size());
  for (size_t ii = 0; ii < _meanLon.size(); ii++) {
    lons[ii] = _meanLon[ii];
  }

  // load up time

  int times[1];
  times[0] = (int) _validTime;

  // load up short data, reversing the row order and scaling by 100.0

  int npts = _ny * _nx;
  TaArray<short> shortData_;
  short *shortData = shortData_.alloc(npts);
  short *sptr = shortData;
  for (int ii = 0; ii < _ny; ii++) {
    int jj = _ny - ii - 1;
    fl32 *fptr = floatData[jj];
    for (int kk = 0; kk < _nx; kk++, fptr++, sptr++) {
      int ival = (int) floor(*fptr * 100.0 + 0.5);
      if (ival < -32767) ival = -32767;
      if (ival > 32767) ival = 32767;
      short sval = (short) ival;
      *sptr = sval;
    }
  }

  // write meta data variables

  iret |= !latVar->put(lats, _ny);
  iret |= !lonVar->put(lons, _nx);
  iret |= !timeVar->put(times, 1);

  // write the data variable
  
  iret |= !dataVar->put(shortData, 1, _ny, _nx);

  // close file

  zebFile.close();

  // check

  if (iret) {
    cerr << "ERROR - DataSet::_writeZebraNetCDF" << endl;
    cerr << "  Cannot write netCDF file: " << zebPath << endl;
    cerr << zebErr.get_errmsg() << endl;
    return -1;
  } 

  // write latest data info file
  
  DsLdataInfo ldata(_params.zebra_netcdf_dir, _params.debug);
  ldata.setWriter("McidasNcf2Mdv");
  ldata.setDataFileExt("nc");
  ldata.setDataType("nc");

  string relPath;
  Path::stripDir(_params.zebra_netcdf_dir, zebPath, relPath);
  ldata.setRelDataPath(relPath);
  ldata.write(_validTime);

  return 0;

}

