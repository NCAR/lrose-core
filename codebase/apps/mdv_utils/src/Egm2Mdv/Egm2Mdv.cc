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
// Egm2Mdv.cc
//
// Egm2Mdv object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2019
//
///////////////////////////////////////////////////////////////
//
// Egm2Mdv reads in an Egm2008 file,
// and converts into NetCDF MDV
//
////////////////////////////////////////////////////////////////

#include <toolsa/toolsa_macros.h>
#include <toolsa/umisc.h>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/NcfMdvx.hh>
#include <dataport/bigend.h>
#include <dataport/bigend.h>
#include "Egm2Mdv.hh"
using namespace std;

const fl32 Egm2Mdv::_missingFloat = -9999.0;

// Constructor

Egm2Mdv::Egm2Mdv(int argc, char **argv)

{

  isOK = true;
  _geoidM = NULL;
  
  // set programe name
  
  _progName = "Egm2Mdv";
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

}

// destructor

Egm2Mdv::~Egm2Mdv()

{

  if (_geoidM) {
    delete[] _geoidM;
  }

}

//////////////////////////////////////////////////
// Run

int Egm2Mdv::Run ()
{
  
  // read in the input file
  
  if (_readInputFile(_params.input_file_path)) {
    cerr << "ERROR = Egm2Mdv::Run" << endl;
    cerr << "  Reading file: " << _params.input_file_path << endl;
    return -1;
  }

  // write out the MDV file

  if (_writeMdvFile()) {
    cerr << "ERROR = Egm2Mdv::Run" << endl;
    cerr << "  Writing MDV file" << endl;
    return -1;
  }
    
  return 0;

}

///////////////////////////////
// read in the input file

int Egm2Mdv::_readInputFile(const char *input_path)
  
{

  // get file size
  
  struct stat fstat;
  if (ta_stat(input_path, &fstat)) {
    int errNum = errno;
    cerr << "ERROR - cannot stat egm file: " << input_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  off_t nbytes = fstat.st_size;

  // assume 2.5 minute file to start

  _nPtsPerDeg = 24;
  if (nbytes > 149368328) {
    // 1 minute file
    _nPtsPerDeg = 60;
  }
  
  _gridRes = 1.0 / (double) _nPtsPerDeg;
  _nLat = 180 * _nPtsPerDeg + 1;
  _nLon = 360 * _nPtsPerDeg;
  _nPoints = _nLat * _nLon;
  ui32 recLen = _nLon * sizeof(fl32);

  if (_params.debug) {
    cerr << "==>> _nPtsPerDeg: " << _nPtsPerDeg << endl;
    cerr << "==>> _nLat: " << _nLat << endl;
    cerr << "==>> _nLon: " << _nLon << endl;
    cerr << "==>> _nPoints: " << _nPoints << endl;
    cerr << "==>> recLen: " << recLen << endl;
  }
  
  int nexpected = _nPoints * sizeof(fl32) + _nLat * 2 * sizeof(ui32);
  if (nbytes != nexpected) {
    cerr << "ERROR - bad egm2008 file: " << input_path << endl;
    cerr << "  expected nbytes: " << nexpected << endl;
    cerr << "  file size nbytes: " << nbytes << endl;
    return -1;
  }
  
  // open elevation data file
  
  FILE *egmFile;
  if ((egmFile = fopen(input_path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - cannot open egm file: " << input_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // geoid data in meters
  
  _geoidM = new fl32[_nPoints];
  int npts = 0;
  bool mustSwap = false;
  
  // read through the file, a fortran record at a time
  
  while (!feof(egmFile)) {
    
    // read starting fortran record len
    
    ui32 recLenStart;
    if (fread(&recLenStart, sizeof(recLenStart), 1, egmFile) != 1) {
      if (feof(egmFile)) {
        break;
      }
      int errNum = errno;
      cerr << "ERROR - cannot read start fort rec len from egm file: " 
           << input_path << endl;
      cerr << "  " << strerror(errNum) << endl;
      fclose(egmFile);
      return -1;
    }
    if (recLenStart != recLen) {
      recLenStart = BE_from_ui32(recLenStart);
      mustSwap = true;
    }
    if (recLenStart != recLen) {
      cerr << "ERROR - bad egm2008 file: " << input_path << endl;
      cerr << "  bad recLenStart: " << recLenStart << endl;
      fclose(egmFile);
      return -1;
    }
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Start fort rec len: " << recLenStart << endl;
    }
    
    // read in data
    
    int nFl32InRec = recLenStart / sizeof(fl32);
    fl32 *buf = new fl32[nFl32InRec];
    if (fread(buf, sizeof(fl32), nFl32InRec, egmFile) != (size_t) nFl32InRec) {
      int errNum = errno;
      cerr << "ERROR - cannot read data from egm file: " << input_path << endl;
      cerr << "  " << strerror(errNum) << endl;
      fclose(egmFile);
      delete[] buf;
      return -1;
    }
    
    // swap as needed

    if (mustSwap) {
      BE_from_array_32(buf, nFl32InRec * sizeof(fl32));
    }

    // copy data into array
    
    for (off_t ii = 0; ii < nFl32InRec; ii++) {
      _geoidM[npts] = buf[ii];
      npts++;
    }
    delete[] buf;
    
    // read ending fortran record len
    
    ui32 recLenEnd;
    if (fread(&recLenEnd, sizeof(recLenEnd), 1, egmFile) != 1) {
      int errNum = errno;
      cerr << "ERROR - cannot read end fort rec len from egm file: " 
           << input_path << endl;
      cerr << "  " << strerror(errNum) << endl;
      fclose(egmFile);
      return -1;
    }
    if (mustSwap) {
      recLenEnd = BE_from_ui32(recLenEnd);
    }
    if (recLenEnd != recLen) {
      cerr << "ERROR - bad egm2008 file: " << input_path << endl;
      cerr << "  bad recLenEnd: " << recLenEnd << endl;
      fclose(egmFile);
      return -1;
    }
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "End fort rec len: " << recLenEnd << endl;
    }
    
  } // while
  
  // close file

  fclose(egmFile);

  if (_params.debug) {
    cerr << "Read npts: " << npts << endl;
  }

  // print out in debug mode
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    int ipt = 0;
    for (int ilat = 0; ilat < _nLat; ilat++) {
      double lat = 90.0 - ilat / (double) _nPtsPerDeg;
      for (int ilon = 0; ilon < _nLon; ilon++, ipt++) {
        double lon = ilon / (double) _nPtsPerDeg;
        if (lon > 180.0) {
          lon -= 360.0;
        }
        fprintf(stderr, "lat, lon, geoidM: %10.5f  %10.5f  %10.5f\n", 
                lat, lon, _geoidM[ipt]);
      }
    } // ilat
  }

  return 0;

#ifdef NOTNOW

  if (_params.debug) {
    cerr << "Processing file: " << input_path << endl;
  }

  // open file
  
  if (_openNc3File(input_path)) {
    cerr << "ERROR - Egm2Mdv::_processFile" << endl;
    cerr << "  File path: " << input_path << endl;
    return -1;
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    _printFile(*_ncFile);
  }

  // check that this is a valid file

  if (_loadMetaData()) {
    cerr << "ERROR - Egm2Mdv::_processFile" << endl;
    cerr << "  File has invalid data" << endl;
    cerr << "  File: " << input_path << endl;
    return -1;
  }

  // loop through times

  for (int itime = 0; itime < _nTimes; itime++) {

    // create output Mdvx file object
    
    NcfMdvx mdvx;
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      mdvx.setDebug(true);
    }
    
    // set master header
    
    if (_setMasterHeader(mdvx, itime)) {
      return -1;
    }

    // add the data fields

    if (_addDataFields(mdvx, itime)) {
      return -1;
    }
    
    // sun angle correction?

    if (_params.perform_sun_angle_correction) {
      _sunAngle.initForTime(_validTime);
      for (int ii = 0; ii < mdvx.getNFields(); ii++) {
        MdvxField *field = mdvx.getField(ii);
        bool applyCorrection = false;
        for (int jj = 0; jj < _params.sun_correction_fields_n; jj++) {
          if (strcmp(field->getFieldName(),
                     _params._sun_correction_fields[jj]) == 0) {
            applyCorrection = true;
            break;
          }
        } // jj
        if (applyCorrection) {
          _correctForSunAngle(field);
        }
      } // ii
    } // if (_params.perform_sun_angle_correction)
    
    // remap if required
    
    if (_params.remap_output_projection) {
      _remapOutput(mdvx);
    }
    
    // write output file
    
    if (_params.debug) {
      cerr << "Writing file to url: " << _params.output_url << endl;
    }

    cerr << "1111111111111111111111111" << endl;

    if (mdvx.convertMdv2Ncf(_params.output_url)) {
      cerr << "ERROR - Egm2Mdv" << endl;
      cerr << "  Cannot write file to url: " << _params.output_url << endl;
      cerr << mdvx.getErrStr() << endl;
      return -1;
    }

    // if (mdvx.writeToDir(_params.output_url)) {
    //   cerr << "ERROR - Egm2Mdv" << endl;
    //   cerr << "  Cannot write file to url: " << _params.output_url << endl;
    //   cerr << mdvx.getErrStr() << endl;
    //   return -1;
    // }
    
    if (_params.debug) {
      cerr << "  Wrote output file: " << mdvx.getPathInUse() << endl;
    }

  } // itime

  return 0;

#endif

}

///////////////////////////////
// write out the MDV file

int Egm2Mdv::_writeMdvFile()
  
{

  // create master header

  NcfMdvx mdvx;
  mdvx.clearMasterHeader();
  DateTime dtime(2008, 1, 1, 0, 0, 0);
  time_t validTime = dtime.utime();
  mdvx.setValidTime(validTime);
  mdvx.setDataCollectionType(Mdvx::DATA_MEASURED);
  mdvx.setDataSetName(_params.data_set_name);
  mdvx.setDataSetSource(_params.data_set_source);
  mdvx.setDataSetInfo(_params.data_set_info);
  
  if (_params.debug) {
    cerr << "===========================================" << endl;
    cerr << "Created data set for time: " << DateTime::strm(validTime) << endl;
  }

  // add field
  
  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);

  fhdr.proj_type = Mdvx::PROJ_LATLON;
  fhdr.proj_origin_lat = 0.0;
  fhdr.proj_origin_lon = 0.0;
  
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;
  
  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.dz_constant = true;
  fhdr.data_dimension = 2;

  fhdr.bad_data_value = _missingFloat;
  fhdr.missing_data_value = _missingFloat;
  
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = sizeof(fl32);
  fhdr.volume_size = _nLon * _nLat * sizeof(fl32);
  
  fhdr.nx = _nLon;
  fhdr.ny = _nLat;
  fhdr.nz = 1;

  fhdr.grid_minx = -180;
  fhdr.grid_miny = -90;
  fhdr.grid_minz = 0.0;

  fhdr.grid_dx = _gridRes;
  fhdr.grid_dy = _gridRes;
  fhdr.grid_dz = 1.0;
  
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  
  // create MdvxField object
  // converting data to encoding and compression types
  
  MdvxField *field = new MdvxField(fhdr, vhdr, _geoidM);

  // set names etc
  
  field->setFieldName("GeoidHt");
  field->setFieldNameLong("Ht of geoid above ellipsoid");
  field->setUnits("m");
  field->setTransform("");

  mdvx.addField(field);

  // write it out

  cerr << "1111111111111111111111111" << endl;
  
  // if (mdvx.convertMdv2Ncf(_params.output_dir)) {
  //   cerr << "ERROR - Egm2Mdv" << endl;
  //   cerr << "  Cannot write file to dir: " << _params.output_dir << endl;
  //   cerr << mdvx.getErrStr() << endl;
  //   return -1;
  // }
  
  mdvx.setWriteFormat(Mdvx::FORMAT_NCF);
  if (mdvx.writeToDir(_params.output_dir)) {
    cerr << "ERROR - Egm2Mdv::_writeMdvFile()" << endl;
    cerr << "  Cannot write output file" << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }

  cerr << "Wrote output file: " << mdvx.getPathInUse() << endl;

  return 0;

}

/////////////////////////////////////////////////
// Set the master header from the NCF file
//
// Returns 0 on success, -1 on failure

int Egm2Mdv::_setMasterHeader(DsMdvx &mdvx)

{

  return 0;

}

/////////////////////////////////////////////////
// Add the data field
//
// Returns 0 on success, -1 on failure

int Egm2Mdv::_addDataField(DsMdvx &mdvx)

{

#ifdef JUNK

  // set npoints, allocate float values array
  
  int npts = _nz * _ny * _nx;

  TaArray<float> vals_;
  float *vals = vals_.alloc(npts);

  // set current location based on time

  if (_zDim) {
    var->set_cur(itime, 0, 0, 0);
  } else {
    var->set_cur(itime, 0, 0);
  }

  if (var->type() == NC_FLOAT) {

    TaArray<float> fvals_;
    float *fvals = fvals_.alloc(npts);
    
    // read data

    int iret = 0;
    if (_zDim) {
      iret = var->get(fvals, 1, _nz, _ny, _nx);
    } else {
      iret = var->get(fvals, 1, _ny, _nx);
    }

    if (iret == 0) {
      cerr << "ERROR - Egm2Mdv::_addDataField" << endl;
      cerr << "  Cannot get data from input netcdf variable" << endl;
      cerr << "  field name: " << var->name() << endl;
      cerr << _ncErr->get_errmsg() << endl;
      return -1;
    }

    // save data
    
    float missing = missingAtt->as_float(0);
    for (int ii = 0; ii < npts; ii++) {
      if (fvals[ii] == missing) {
        vals[ii] = _missingFloat;
      } else {
        vals[ii] = fvals[ii];
      }
    }

  } else if (var->type() == NC_DOUBLE) {
    
    TaArray<double> dvals_;
    double *dvals = dvals_.alloc(npts);

    // read data

    int iret = 0;
    if (_zDim) {
      iret = var->get(dvals, 1, _nz, _ny, _nx);
    } else {
      iret = var->get(dvals, 1, _ny, _nx);
    }
    if (iret == 0) {
      cerr << "ERROR - Egm2Mdv::_addDataField" << endl;
      cerr << "  Cannot get data from input netcdf variable" << endl;
      cerr << "  field name: " << var->name() << endl;
      cerr << _ncErr->get_errmsg() << endl;
      return -1;
    }

    // save data
    
    double missing = missingAtt->as_double(0);
    for (int ii = 0; ii < npts; ii++) {
      if (dvals[ii] == missing) {
        vals[ii] = _missingFloat;
      } else {
        vals[ii] = dvals[ii];
      }
    }

  } else {

    // for int fields, we need scale and offset

    double scale = 1.0;
    Nc3Att *scaleAtt = var->get_att("scale");
    if (scaleAtt == NULL) {
      scaleAtt = var->get_att("scale_factor");
    }
    if (scaleAtt == NULL) {
      cerr << "WARNING - Egm2Mdv::_addDataField" << endl;
      cerr << "  Cannot get scale for integer variable" << endl;
      cerr << "  field name: " << var->name() << endl;
      cerr << "  Setting scale to 1.0" << endl;
    } else {
      scale = scaleAtt->as_double(0);
      delete scaleAtt;
    }
      
    double offset = 0.0;
    Nc3Att *offsetAtt = var->get_att("offset");
    if (offsetAtt == NULL) {
      if (_params.debug) {
        cerr << "WARNING - Egm2Mdv::_addDataField" << endl;
        cerr << "  Cannot get offset for integer variable" << endl;
        cerr << "  field name: " << var->name() << endl;
        cerr << "  setting to 0" << endl;
      }
    } else {
      offset = offsetAtt->as_double(0);
      delete offsetAtt;
    }
    
    if (var->type() == NC_INT) {
      
      TaArray<int> ivals_;
      int *ivals = ivals_.alloc(npts);
      
      // read data
      
      int iret = 0;
      if (_zDim) {
        iret = var->get(ivals, 1, _nz, _ny, _nx);
      } else {
        iret = var->get(ivals, 1, _ny, _nx);
      }
      if (iret == 0) {
        cerr << "ERROR - Egm2Mdv::_addDataField" << endl;
        cerr << "  Cannot get data from input netcdf variable" << endl;
        cerr << "  field name: " << var->name() << endl;
        cerr << _ncErr->get_errmsg() << endl;
        return -1;
      }

      // save data

      int missing = missingAtt->as_int(0);
      for (int ii = 0; ii < npts; ii++) {
        if (ivals[ii] == missing) {
          vals[ii] = _missingFloat;
        } else {
          vals[ii] = ivals[ii] * scale + offset;
        }
      }

    } else if (var->type() == NC_SHORT) {
      
      TaArray<short> svals_;
      short *svals = svals_.alloc(npts);
      
      // read data
      
      int iret = 0;
      if (_zDim) {
        iret = var->get(svals, 1, _nz, _ny, _nx);
      } else {
        iret = var->get(svals, 1, _ny, _nx);
      }
      if (iret == 0) {
        cerr << "ERROR - Egm2Mdv::_addDataField" << endl;
        cerr << "  Cannot get data from input netcdf variable" << endl;
        cerr << "  field name: " << var->name() << endl;
        cerr << _ncErr->get_errmsg() << endl;
        return -1;
      }

      // save data

      short missing = missingAtt->as_short(0);
      for (int ii = 0; ii < npts; ii++) {
        if (svals[ii] == missing) {
          vals[ii] = _missingFloat;
        } else {
          vals[ii] = svals[ii] * scale + offset;
        }
      }

    } else if (var->type() == NC_BYTE) {
      

      TaArray<ncbyte> bvals_;
      ncbyte *bvals = bvals_.alloc(npts);
      TaArray<unsigned char> uvals_;
      unsigned char *uvals = uvals_.alloc(npts);
      
      // read data
      
      int iret = 0;
      if (_zDim) {
        iret = var->get(bvals, 1, _nz, _ny, _nx);
      } else {
        iret = var->get(bvals, 1, _ny, _nx);
      }
      if (iret == 0) {
        cerr << "ERROR - Egm2Mdv::_addDataField" << endl;
        cerr << "  Cannot get data from input netcdf variable" << endl;
        cerr << "  field name: " << var->name() << endl;
        cerr << _ncErr->get_errmsg() << endl;
        return -1;
      }
      memcpy(uvals, bvals, npts);

      // save data
      
      ncbyte missing = missingAtt->as_ncbyte(0);
      for (int ii = 0; ii < npts; ii++) {
        if (bvals[ii] == missing) {
          vals[ii] = _missingFloat;
        } else {
          if (_params.treat_ncbyte_as_unsigned) {
            vals[ii] = (int) uvals[ii] * scale + offset;
          } else {
            vals[ii] = (int) bvals[ii] * scale + offset;
          }
        }
      }

    } // if (var->type() == NC_INT)

  } // if (var->type() == NC_FLOAT)

  // free up attribute

  delete missingAtt;

  // reverse y order if it was in reverse order in the file

  if (_yIsReversed) {

    TaArray<float> tmpVals_;
    float *tmpVals = tmpVals_.alloc(npts);
    memcpy(tmpVals, vals, npts * sizeof(float));

    int nptsPlane = _ny * _nx;

    for (int iz = 0; iz < _nz; iz ++) {
      int planeOffset = iz * nptsPlane;
      for (int iy = 0; iy < _ny; iy ++) {
        float *src = tmpVals + planeOffset + _nx * (_ny - 1 - iy);
        float *dest = vals + planeOffset + _nx * iy;
        memcpy(dest, src, _nx * sizeof(float));
      } // iy
    } // iz
    
  }
  
  // get field name and units

  string fieldName(var->name());

  string units;
  Nc3Att *unitsAtt = var->get_att("units");
  if (unitsAtt != NULL) {
    units = unitsAtt->as_string(0);
    delete unitsAtt;
  }

  string longName(fieldName);
  Nc3Att *longNameAtt = var->get_att("long_name");
  if (longNameAtt != NULL) {
    longName = longNameAtt->as_string(0);
    delete longNameAtt;
  }

  // create fields and add to mdvx object

  if (_params.input_xy_is_latlon &&
      _params.resample_latlon_onto_regular_grid &&
      (!_dxIsConstant || !_dyIsConstant)) {
    
    // create the field from the remapped
    
    MdvxField *field =
      _createRegularLatlonField(fieldName, longName, units, vals);
    // add to Mdvx, which takes over ownership
    mdvx.addField(field);
    
  } else {
  
    // create the field from the netcdf array
    MdvxField *field = _createMdvxField(fieldName, longName, units,
                                        _nx, _ny, _nz,
                                        _minx, _miny, _minz,
                                        _dx, _dy, _dz,
                                        vals);
    // add to Mdvx, which takes over ownership
    mdvx.addField(field);
    
  }

#endif
  
  return 0;

}

///////////////////////////////
// Create an Mdvx field

MdvxField *Egm2Mdv::_createMdvxField
  (const string &fieldName,
   const string &longName,
   const string &units,
   int nx, int ny, int nz,
   double minx, double miny, double minz,
   double dx, double dy, double dz,
   const float *vals)

{

#ifdef JUNK

  // check max levels

  if (nz > MDV_MAX_VLEVELS) {
    nz = MDV_MAX_VLEVELS;
  }

  // set up MdvxField headers

  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);
  
  _inputProj.syncToFieldHdr(fhdr);

  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;
  
  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  fhdr.dz_constant = false;
  fhdr.data_dimension = 3;

  fhdr.bad_data_value = _missingFloat;
  fhdr.missing_data_value = _missingFloat;
  
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = sizeof(fl32);
  fhdr.volume_size = nx * ny * nz * sizeof(fl32);
  
  fhdr.nx = nx;
  fhdr.ny = ny;
  fhdr.nz = nz;

  fhdr.grid_minx = minx;
  fhdr.grid_miny = miny;
  fhdr.grid_minz = minz;

  fhdr.grid_dx = dx;
  fhdr.grid_dy = dy;
  fhdr.grid_dz = dz;
  
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  
  for (int ii = 0; ii < nz; ii++) {
    vhdr.type[ii] = Mdvx::VERT_TYPE_Z;
    vhdr.level[ii] = _zArray[ii];
  }

  // create MdvxField object
  // converting data to encoding and compression types

  MdvxField *field = new MdvxField(fhdr, vhdr, vals);
  field->convertType
    ((Mdvx::encoding_type_t) _params.output_encoding_type,
     (Mdvx::compression_type_t) _params.output_compression_type);

  // set names etc
  
  field->setFieldName(fieldName.c_str());
  field->setFieldNameLong(longName.c_str());
  field->setUnits(units.c_str());
  field->setTransform("");

  return field;

#endif

  return NULL;

}
  
////////////////////////////////////////////////////////////////
// Create a field remapped from lat/lon onto regular grid

MdvxField *Egm2Mdv::_createRegularLatlonField
  (const string &fieldName,
   const string &longName,
   const string &units,
   const float *vals)
  
{
  
#ifdef JUNK

  if (_params.debug) {
    cerr << "==== Creating regular lat/lon grid ====" << endl;
  }

  // compute min dlat and dlon

  double dLon = 1.0e6;
  for (int ix = _ixValidStart; ix < _ixValidEnd; ix++) {
    double lonVal = fabs(_xArray[ix+1] - _xArray[ix]);
    if (lonVal < dLon) {
      dLon = lonVal;
    }
  }

  double dLat = 1.0e6;
  for (int iy = _iyValidStart; iy < _iyValidEnd; iy++) {
    double latVal = fabs(_yArray[iy+1] - _yArray[iy]);
    if (latVal < dLat) {
      dLat = latVal;
    }
  }

  double minLon = _xArray[_ixValidStart];
  double maxLon = _xArray[_ixValidEnd];
  double rangeLon = maxLon - minLon;

  double minLat = _yArray[_iyValidStart];
  double maxLat = _yArray[_iyValidEnd];
  double rangeLat = maxLat - minLat;

  int nLon = (int) (rangeLon / dLon) + 1;
  int nLat = (int) (rangeLat / dLat) + 1;

  if (_params.debug) {
    cerr << "==>> minLon, maxLon, rangeLon, dLon, nLon: "
         << minLon << ", " << maxLon << ", "
         << rangeLon << ", " << dLon << ", " << nLon << endl;
    cerr << "==>> minLat, maxLat, rangeLat, dLat, nLat: "
         << minLat << ", " << maxLat << ", "
         << rangeLat << ", " << dLat << ", " << nLat << endl;
  }

  // create resampled grid

  int nResamp = nLon * nLat;
  TaArray<fl32> resampled_;
  fl32 *resampled = resampled_.alloc(nResamp);

  // compute resampling indices

  vector<int> latIndex;
  double latitude = minLat;
  for (int ilat = 0; ilat < nLat; ilat++, latitude += dLat) {
    int index = _getClosestLatIndex(latitude, dLat / 2.0);
    latIndex.push_back(index);
  }

  vector<int> lonIndex;
  double longitude = minLon;
  for (int ilon = 0; ilon < nLon; ilon++, longitude += dLon) {
    int index = _getClosestLonIndex(longitude, dLon / 2.0);
    lonIndex.push_back(index);
  }
  
  // load the resampled grid

  fl32 *resamp = resampled;
  for (int ilat = 0; ilat < nLat; ilat++) {
    int latIx = latIndex[ilat];
    for (int ilon = 0; ilon < nLon; ilon++, resamp++) {
      int lonIx = lonIndex[ilon];
      if (latIx < 0 || lonIx < 0) {
        *resamp = _missingFloat;
      } else {
        int valIndex = latIx * _nx + lonIx;
        *resamp = vals[valIndex];
      }
    } // ilon
  } // ilat

  // create the field

  // set up MdvxField headers

  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);

  fhdr.proj_type = Mdvx::PROJ_LATLON;
  fhdr.proj_origin_lat = minLat + rangeLat / 2.0;
  fhdr.proj_origin_lon = minLon + rangeLon / 2.0;

  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;
  
  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  fhdr.dz_constant = false;
  fhdr.data_dimension = 3;

  fhdr.bad_data_value = _missingFloat;
  fhdr.missing_data_value = _missingFloat;
  
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = sizeof(fl32);
  fhdr.volume_size = nLon * nLat * _nz * sizeof(fl32);
  
  fhdr.nx = nLon;
  fhdr.ny = nLat;
  fhdr.nz = _nz;

  fhdr.grid_minx = minLon;
  fhdr.grid_miny = minLat;
  fhdr.grid_minz = _minz;

  fhdr.grid_dx = dLon;
  fhdr.grid_dy = dLat;
  fhdr.grid_dz = _dz;
  
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  
  for (int ii = 0; ii < _nz; ii++) {
    vhdr.type[ii] = Mdvx::VERT_TYPE_Z;
    vhdr.level[ii] = _zArray[ii];
  }

  // create MdvxField object
  // converting data to encoding and compression types
  
  MdvxField *field = new MdvxField(fhdr, vhdr, resampled);
  field->convertType
    ((Mdvx::encoding_type_t) _params.output_encoding_type,
     (Mdvx::compression_type_t) _params.output_compression_type);

  // set names etc
  
  field->setFieldName(fieldName.c_str());
  field->setFieldNameLong(longName.c_str());
  field->setUnits(units.c_str());
  field->setTransform("");

  return field;

#endif

  return NULL;

}
  
