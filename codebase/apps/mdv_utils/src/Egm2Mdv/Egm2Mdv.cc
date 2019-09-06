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

  // reorder the latitude rows from south to north

  _reorderGeoidLats();

  // reorder the longitude cols from -180 to +180
  // the data comes in as 0 to 360

  _reorderGeoidLons();

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

  // print out in verbose debug mode
  
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

}

/////////////////////////////////////////////////////////
// reorder the geoid latitude
// the original data is north to south
// we need the data south to north, for normal coordinates

void Egm2Mdv::_reorderGeoidLats()
  
{

  // create an array for a line of data at a time

  fl32 *rowData = new fl32[_nLon];

  // loop through the top half of the rows

  size_t nBytesRow = _nLon * sizeof(fl32);
  
  for (int irow = 0; irow < _nLat / 2; irow++) {

    // make a copy of row data in northern half
    
    memcpy(rowData,
           _geoidM + irow * _nLon,
           nBytesRow);
    
    // copy from southern half to northern half

    memcpy(_geoidM + irow * _nLon,
           _geoidM + (_nLat - 1 - irow) * _nLon,
           nBytesRow);

    // copy from northern half to southern half
    
    memcpy(_geoidM + (_nLat - 1 - irow) * _nLon,
           rowData,
           nBytesRow);

  }

  // clean up

  delete[] rowData;

}


///////////////////////////////////////////////////////////
// reorder the geoid longitude
// the original data is 0 to 360
// reorder from -180 to +180

void Egm2Mdv::_reorderGeoidLons()
  
{

  // create arrays

  size_t nLonHalf = _nLon / 2;
  fl32 *rowData = new fl32[_nLon];
  size_t nBytesRow = _nLon * sizeof(fl32);
  size_t nBytesHalf = nLonHalf * sizeof(fl32);
  
  // loop through the top half of the rows
  
  
  for (int irow = 0; irow < _nLat; irow++) {

    // make a copy of east half of row data
    
    memcpy(rowData,
           _geoidM + irow * _nLon + nLonHalf,
           nBytesHalf);
    
    // make a copy of west half of row data
    
    memcpy(rowData + nLonHalf,
           _geoidM + irow * _nLon,
           nBytesHalf);
    
    // copy from row data back into place

    memcpy(_geoidM + irow * _nLon,
           rowData,
           nBytesRow);

  }

  // clean up
  
  delete[] rowData;

}


///////////////////////////////
// write out the MDV file

int Egm2Mdv::_writeMdvFile()
  
{

  // create master header

  DsMdvx mdvx;
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
  fhdr.proj_origin_lat = 0;
  fhdr.proj_origin_lon = 0;
  
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

  mdvx.setWriteFormat(Mdvx::FORMAT_NCF);
  mdvx.setMdv2NcfOutput(false, false, false, false);
  mdvx.setMdv2NcfCompression(true, 9);

  if (mdvx.writeToPath(_params.output_file_path)) {
    cerr << "ERROR - Egm2Mdv::_writeMdvFile()" << endl;
    cerr << "  Cannot write output file" << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }

  cerr << "Wrote output file: " << mdvx.getPathInUse() << endl;

  return 0;

}

