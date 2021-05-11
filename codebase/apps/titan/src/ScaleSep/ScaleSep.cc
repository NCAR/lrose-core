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
// ScaleSep.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// AUGUST 2014
//
////////////////////////////////////////////////////////////////////
//
// ScaleSep separates a radar image scene into different spatial
// scales, by applying a filter in the 2D FFT frequency domain
//
/////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <dataport/bigend.h>
#include <toolsa/pmu.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/TaArray.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxChunk.hh>
#include <physics/vil.h>
#include "ScaleSep.hh"
#include "WorkingField.hh"
#include "Fft2D.hh"
using namespace std;

const fl32 ScaleSep::_missing = -9999.0;

// Constructor

ScaleSep::ScaleSep(int argc, char **argv)

{

  isOK = true;
  gettimeofday(&_timeA, NULL);

  _baseField = NULL;
  _basePadded = NULL;
  _spectrum = NULL;
  _specFilt = NULL;
  _filtPadded = NULL;
  _filtered = NULL;
  _filter = NULL;

  _prevNx = 0;
  _prevNy = 0;
  _fft = NULL;

  // set programe name

  _progName = "ScaleSep";
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
    return;
  }

  // check that start and end time is set in archive mode

  if (_params.mode == Params::ARCHIVE) {
    if (_args.startTime == 0 || _args.endTime == 0) {
      cerr << "ERROR - must specify start and end dates." << endl << endl;
      _args.usage(_progName, cerr);
      isOK = false;
      return;
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

ScaleSep::~ScaleSep()

{

  // unregister process

  PMU_auto_unregister();

  // free up

  _deleteFields();

  if (_fft) {
    delete _fft;
  }

}

//////////////////////////////////////////////////
// Run

int ScaleSep::Run()
{
  
  int iret = 0;

  // register with procmap
  
  PMU_auto_register("Run");

  // loop until end of data
  
  _input.reset();
  while (!_input.endOfData()) {
    
    PMU_auto_register("In main loop");
    
    // do the read

    PMU_auto_register("Before read");
    if (_doRead()) {
      cerr << "ERROR - ScaleSep::Run()" << endl;
      umsleep(1000);
      iret = -1;
      continue;
    }

    // innitialize the fields and FFT

    _init();

    // process the data set

    if (_processDataSet()) {
      iret = -1;
      continue;
    }

    // write out
    
    if (_doWrite()) {
      cerr << "ERROR - ScaleSep::Run()" << endl;
      umsleep(1000);
      iret = -1;
      continue;
    }

    // clear

    _inMdvx.clear();
    _outMdvx.clear();
    
  } // while
  
  return iret;

}

/////////////////////////////////////////////////////////
// perform the read
//
// Returns 0 on success, -1 on failure.

int ScaleSep::_doRead()
  
{
  
  _inMdvx.clear();
  if (_params.debug) {
    _inMdvx.setDebug(true);
  }
  _inMdvx.addReadField(_params.dbz_field_name);
  _inMdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _inMdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  _inMdvx.setReadVlevelLimits(_params.min_valid_height, _params.max_valid_height);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _inMdvx.printReadRequest(cerr);
  }
  
  // read in
  
  PMU_auto_register("Before read");
  
  if (_input.readVolumeNext(_inMdvx)) {
    cerr << "ERROR - ScaleSep::_doRead" << endl;
    cerr << "  Cannot read in data." << endl;
    cerr << _input.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Read in file: " << _inMdvx.getPathInUse() << endl;
  }

  // set sizes

  MdvxField *dbzFld = _inMdvx.getField(_params.dbz_field_name);
  const Mdvx::field_header_t &fhdr = dbzFld->getFieldHeader();
  _nz = fhdr.nz;
  _ny = fhdr.ny;
  _nx = fhdr.nx;
  _nxy = _nx * _ny;
  _minx = fhdr.grid_minx;
  _miny = fhdr.grid_miny;
  _dx = fhdr.grid_dx;
  _dy = fhdr.grid_dy;

  if (fhdr.proj_type == Mdvx::PROJ_LATLON) {
    double meanLat = (_miny + _ny * _dy / 2.0);
    double cosLat = cos(meanLat * DEG_TO_RAD);
    _dyKm = _dy * KM_PER_DEG_AT_EQ;
    _dxKm = _dx * KM_PER_DEG_AT_EQ * cosLat;
  } else {
    _dxKm = _dx;
    _dyKm = _dy;
  }

  // compute the number of points needed for padding around the
  // data to prevent wrapping - use half the wavelength

  _nxPad = (int) (_params.spatial_filter_wavelength_km / _dxKm + 0.5);
  _nyPad = (int) (_params.spatial_filter_wavelength_km / _dyKm + 0.5);
  _nxPadded = _nx + 2 * _nxPad;
  _nyPadded = _ny + 2 * _nyPad;
  _nxyPadded = _nxPadded * _nyPadded;
  _minxPadded = _minx - _nxPad * _dx;
  _minyPadded = _miny - _nyPad * _dy;

  _printRunTime("ScaleSep::_doRead");

  return 0;

}

/////////////////////////////////////////////////////////
// process the data set
// Returns 0 on success, -1 on failure.

int ScaleSep::_processDataSet()
  
{
  
  // load up input field data

  _loadBaseField();

  // filter the field

  _applyFilter();

  return 0;
  
}

/////////////////////////////////////////////////////////
// load up the base field

void ScaleSep::_loadBaseField()
  
{

  MdvxField *dbzFld = _inMdvx.getField(_params.dbz_field_name);
  const Mdvx::field_header_t &fhdr = dbzFld->getFieldHeader();
  fl32 *dbz = (fl32 *) dbzFld->getVol();
  fl32 missingDbz = fhdr.missing_data_value;

  if (_params.analysis_method == Params::COMPUTE_COLUMN_MAX) {

    fl32 *colMax = _baseField->getData();
    _dbzMin = 9999;

    for (int ii = 0; ii < _nxy; ii++) {
      int zOffset = 0;
      colMax[ii] = _missing;
      fl32 dbzMax = -9999;
      for (int iz = 0; iz < _nz; iz++, zOffset += _nxy) {
        fl32 dbzVal = dbz[zOffset + ii];
        if (dbzVal != missingDbz) {
          if (dbzVal < _params.min_valid_dbz) {
            dbzVal = _params.min_valid_dbz;
          }
          if (dbzVal < _dbzMin) {
            _dbzMin = dbzVal;
          }
          if (dbzVal > dbzMax) {
            dbzMax = dbzVal;
          }
        }
      } // iz
      if (dbzMax > -9990) {
        colMax[ii] = dbzMax;
      }
    } // ii

    // add the min value to make all values positive
    
    for (int ii = 0; ii < _nxy; ii++) {
      if (colMax[ii] == _missing) {
        colMax[ii] = 0;
      } else {
        colMax[ii] -= _dbzMin;
      }
    }

    // copy this to padded field

    _copyToPadded(colMax, _basePadded->getData());

  } else {

    // VIL

    vector<double> deltaHt;
    for (int iz = 0; iz < _nz; iz++) {
      deltaHt.push_back(_getDeltaHt(iz, *dbzFld));
    }

    fl32 *vil = _baseField->getData();
    _baseField->setToZero();
    for (int ii = 0; ii < _nxy; ii++) {
      int zOffset = 0;
      double vilSum;
      VIL_INIT(vilSum);
      for (int iz = 0; iz < _nz; iz++, zOffset += _nxy) {
        fl32 dbzVal = dbz[zOffset + ii];
        if (dbzVal != missingDbz) {
          VIL_ADD(vilSum, dbzVal, deltaHt[iz]);
        }
      }
      vil[ii] = VIL_COMPUTE(vilSum);
    }

    // copy this to padded field

    _copyToPadded(vil, _basePadded->getData());

  }

  _printRunTime("ScaleSep::_loadInputField");

}

  
/////////////////////////////////////////////////////////
// Apply fft-based filter to the field

void ScaleSep::_applyFilter()
  
{

  // compute complex spectrum - forward fft
  
  TaArray<fftw_complex> specComplex_;
  fftw_complex *specComplex = specComplex_.alloc(_nxyPadded);
  _fft->fwd(_basePadded->getData(), specComplex);
  _printRunTime("ScaleSep::_applyFilter - fwd fft");
  
  // load the unfiltered spectrum for output
  
  fl32 *spec = _spectrum->getData();
  for (int ii = 0; ii < _nxyPadded; ii++) {
    double re = specComplex[ii][0];
    double im = specComplex[ii][1];
    spec[ii] = sqrt(re * re + im * im);
  }

  // filter the spectrum

  const fl32 *filter = _filter->getData();
  for (int iy = 0, ii = 0; iy < _nyPadded; iy++) {
    for (int ix = 0; ix < _nxPadded; ix++, ii++) {
      specComplex[ii][0] *= filter[ii];
      specComplex[ii][1] *= filter[ii];
    }
  }

  // load the filtered spectra for output
  
  fl32 *specFilt = _specFilt->getData();
  for (int ii = 0; ii < _nxyPadded; ii++) {
    double re = specComplex[ii][0];
    double im = specComplex[ii][1];
    specFilt[ii] = sqrt(re * re + im * im);
  }

  // invert to get filtered spatial domain product
  
  _fft->inv(specComplex, _filtPadded->getData());

  // copy to unpadded fitered field

  _copyFromPadded(_filtPadded->getData(), _filtered->getData());
  
  // adjust col max dbz if needed

  if (_params.analysis_method == Params::COMPUTE_COLUMN_MAX) {
    // adjust using dbzMin
    fl32 *colMax = _baseField->getData();
    fl32 *filt = _filtered->getData();
    for (int ii = 0; ii < _nxy; ii++) {
      colMax[ii] += _dbzMin;
      filt[ii] += _dbzMin;
    }
  }

  _printRunTime("ScaleSep::_applyFilter - inv fft");

}


/////////////////////////////////////////////////////////
// add fields to the output object

void ScaleSep::_addFields()
  
{

  _outMdvx.clearFields();

  _addField(_baseField);
  if (_params.write_debug_fields) {
    _addField(_basePadded);
    _addField(_spectrum);
    _addField(_specFilt);
    _addField(_filtPadded);
    _addField(_filter);
  }
  _addField(_filtered);

}

/////////////////////////////////////////////////////////
// add field to the output object

void ScaleSep::_addField(const WorkingField *fld)
  
{

  if (fld == NULL) {
    return;
  }

  MdvxField *dbzField = _inMdvx.getField(_params.dbz_field_name);
  Mdvx::field_header_t fhdr = dbzField->getFieldHeader();
  fhdr.nz = 1;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.nx = fld->getNx();
  fhdr.ny = fld->getNy();
  fhdr.grid_minx = fld->getMinx();
  fhdr.grid_miny = fld->getMiny();
  fhdr.grid_dx = fld->getDx();
  fhdr.grid_dy = fld->getDy();
  int volSize32 = fld->getNxy() * sizeof(fl32);
  fhdr.volume_size = volSize32;
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = 4;
  fhdr.missing_data_value = _missing;
  fhdr.bad_data_value = _missing;
  
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  vhdr.level[0] = 0;
  vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  
  MdvxField *mdvxFld = new MdvxField(fhdr, vhdr);
  mdvxFld->setFieldName(fld->getName().c_str());
  mdvxFld->setFieldNameLong(fld->getLongName().c_str());
  mdvxFld->setUnits(fld->getUnits().c_str());

  mdvxFld->setVolData(fld->getData(), volSize32,
                      Mdvx::ENCODING_FLOAT32);
  mdvxFld->convertType(Mdvx::ENCODING_FLOAT32,
                       Mdvx::COMPRESSION_GZIP);

  if (_params.debug) {
    cerr << "Adding field: " << mdvxFld->getFieldName() << endl;
  }

  _outMdvx.addField(mdvxFld);
  
}

/////////////////////////////////////////////////////////
// perform the write
//
// Returns 0 on success, -1 on failure.

int ScaleSep::_doWrite()
  
{
  
  // create output DsMdvx object
  // copying master header from input object
  
  _outMdvx.clear();
  if (_params.debug) {
    _outMdvx.setDebug(true);
  }
  _outMdvx.setWriteLdataInfo();
  Mdvx::master_header_t mhdr = _inMdvx.getMasterHeader();
  _outMdvx.setMasterHeader(mhdr);
  string info = _inMdvx.getMasterHeader().data_set_info;
  info += " : ScaleSep filters 2D data for scale separation";
  _outMdvx.setDataSetInfo(info.c_str());
  
  // copy any chunks
  
  for (size_t i = 0; i < _inMdvx.getNChunks(); i++) {
    MdvxChunk *chunk = new MdvxChunk(*_inMdvx.getChunkByNum(i));
    _outMdvx.addChunk(chunk);
  }
  
  // add fields
  
  _addFields();
  
  // write out
  
  PMU_auto_register("Before write");
  if(_outMdvx.writeToDir(_params.output_url)) {
    cerr << "ERROR - ScaleSep::Run" << endl;
    cerr << "  Cannot write data set." << endl;
    cerr << _outMdvx.getErrStr() << endl;
    return -1;
  }
  if (_params.debug) {
    cerr << "Wrote file: " << _outMdvx.getPathInUse() << endl;
  }

  _printRunTime("ScaleSep::_doWrite");

  return 0;

}
  
/////////////////////////////
// get height of level
//

double ScaleSep::_getHeight(int iz, const MdvxField &field)                   
{
  const Mdvx::vlevel_header_t &vhdr = field.getVlevelHeader();
  return vhdr.level[iz];
}

////////////////////////////////////////////
// compute delta height from vertical levels

double ScaleSep::_getDeltaHt(int iz, const MdvxField &field)
{
  
  const Mdvx::field_header_t &fhdr = field.getFieldHeader();
  const Mdvx::vlevel_header_t &vhdr = field.getVlevelHeader();
  int nz = fhdr.nz;

  double base_ht = 0, top_ht = 0;

  if (nz == 1) {
    base_ht = vhdr.level[0] - fhdr.grid_dz / 2.0;
    top_ht  = vhdr.level[0] + fhdr.grid_dz / 2.0;
  } else {
    if (iz == 0) {
      base_ht =  vhdr.level[0] - (vhdr.level[1] - vhdr.level[0]) / 2.0;
    } else {
      base_ht = (vhdr.level[iz] + vhdr.level[iz - 1]) / 2.0;
    }
    if (iz == nz - 1) {
      top_ht  =  vhdr.level[nz - 1] + (vhdr.level[nz - 1] - vhdr.level[nz - 2]) / 2.0;
    } else {
      top_ht  = (vhdr.level[iz] + vhdr.level[iz + 1]) / 2.0;
    }
  }

  return(top_ht - base_ht);

}

//////////////////////////////////////////////////////////////////
// Print the elapsed run time since the previous call, in seconds.

void ScaleSep::_printRunTime(const string& str, bool verbose /* = false */)
{
  if (verbose) {
    if (_params.debug < Params::DEBUG_VERBOSE) {
      return;
    }
  } else if (!_params.debug) {
    return;
  }
  struct timeval tvb;
  gettimeofday(&tvb, NULL);
  double deltaSec = tvb.tv_sec - _timeA.tv_sec
    + 1.e-6 * (tvb.tv_usec - _timeA.tv_usec);
  cerr << "TIMING, task: " << str << ", secs used: " << deltaSec << endl;
  _timeA.tv_sec = tvb.tv_sec;
  _timeA.tv_usec = tvb.tv_usec;
}

//////////////////////////////////////////////////////////////////
// copy to/from padded array

void ScaleSep::_copyToPadded(const fl32 *normal, fl32 *padded)
{
  memset(padded, 0, _nxyPadded * sizeof(fl32));
  int ii = 0;
  int ipad = _nyPad * _nxPadded + _nxPad;
  for (int iy = 0; iy < _ny; iy++, ii += _nx, ipad += _nxPadded) {
    memcpy(padded + ipad, normal + ii, _nx * sizeof(fl32));
  }
}

void ScaleSep::_copyFromPadded(const fl32 *padded, fl32 *normal)
{
  int ii = 0;
  int ipad = _nyPad * _nxPadded + _nxPad;
  for (int iy = 0; iy < _ny; iy++, ii += _nx, ipad += _nxPadded) {
    memcpy(normal + ii, padded + ipad, _nx * sizeof(fl32));
  }
}

//////////////////////////////////////
// initialize

void ScaleSep::_init()

{

  // check if grid has changed
  
  if (_nx == _prevNx &&
      _ny == _prevNy &&
      _minx == _prevMinx &&
      _miny == _prevMiny &&
      _dx == _prevDx &&
      _dy == _prevDy) {

    // no change
    
    return;
    
  }

  // initialize fields

  _deleteFields();
  _allocFields();

  // initalize fft

  if (_fft) {
    delete _fft;
  }
  _fft = new Fft2D(_nyPadded, _nxPadded);

  // initialize filter

  _computeFilter();
  
}
    
//////////////////////////////////////
// allocate working fields

void ScaleSep::_allocFields()

{

  string units;

  if (_params.analysis_method == Params::COMPUTE_COLUMN_MAX) {
    
    // COLUMN-MAX DBZ
    
    units = "dBZ";
    _baseField = new WorkingField("DBZ",
                                  "MaxDbzOverVertColumn",
                                  units,
                                  _ny, _nx,
                                  _minx, _miny,
                                  _dx, _dy,
                                  true);
    
    _basePadded = new WorkingField("DBZPadded",
                                   "MaxDbzOverVertColumnPadded",
                                   units,
                                   _nyPadded, _nxPadded,
                                   _minxPadded, _minyPadded,
                                   _dx, _dy,
                                   true);
    
  } else {
    
    // VIL
    
    units = "kg/m2";
    _baseField = new WorkingField("VIL",
                                  "VerticallyIntegratedLiquid",
                                  units,
                                  _ny, _nx,
                                  _minx, _miny,
                                  _dx, _dy,
                                  true);
    
    _basePadded = new WorkingField("VILPadded",
                                   "VerticallyIntegratedLiquidPadded",
                                   units,
                                   _nyPadded, _nxPadded,
                                   _minxPadded, _minyPadded,
                                   _dx, _dy,
                                   true);
    
  }
  
  
  _spectrum = new WorkingField("Spectrum",
                               "Spectrum_of_base_field",
                               units,
                               _nyPadded, _nxPadded,
                               _minxPadded, _minyPadded,
                               _dx, _dy,
                               true);
  
  _specFilt = new WorkingField("SpecFilt",
                               "Filtered_spectrum_of_base_field",
                               units,
                               _nyPadded, _nxPadded,
                               _minxPadded, _minyPadded,
                               _dx, _dy,
                               true);
  
  _filtPadded = new WorkingField("FiltPadded",
                                 "SpatialDomainFilteredPadded",
                                 units,
                                 _nyPadded, _nxPadded,
                                 _minxPadded, _minyPadded,
                                 _dx, _dy,
                                 true);

  _filtered = new WorkingField(_params.filtered_field_name,
                               "SpatiallyFilteredResult",
                               units,
                               _ny, _nx,
                               _minx, _miny,
                               _dx, _dy,
                               true);
  
  _filter = new WorkingField("Filter",
                             "FilterCoefficients",
                             "",
                             _nyPadded, _nxPadded,
                             _minxPadded, _minyPadded,
                             _dx, _dy,
                             true);

}


//////////////////////////////////////
// delete fields

void ScaleSep::_deleteFields()

{

  // free up

  if (_baseField) {
    delete _baseField;
    _baseField = NULL;
  }

  if (_basePadded) {
    delete _basePadded;
    _basePadded = NULL;
  }

  if (_spectrum) {
    delete _spectrum;
    _spectrum = NULL;
  }

  if (_specFilt) {
    delete _specFilt;
    _specFilt = NULL;
  }

  if (_filtPadded) {
    delete _filtPadded;
    _filtPadded = NULL;
  }

  if (_filtered) {
    delete _filtered;
    _filtered = NULL;
  }

  if (_filter) {
    delete _filter;
    _filter = NULL;
  }

}

//////////////////////////////////////
// create the filter

void ScaleSep::_computeFilter()

{
  // create filter kernel
  
  double wavelengthKm = _params.spatial_filter_wavelength_km;
  int filtNx = (int) ((_nx * _dxKm) / (1.0 * wavelengthKm) + 0.5);
  int filtNy = (int) ((_ny * _dyKm) / (1.0 * wavelengthKm) + 0.5);
  double filtLen = sqrt((double) filtNx * filtNx + (double) filtNy * filtNy);

  if (_params.debug) {
    cerr << "Note: filtNx, filtNy, filtLen: "
         << filtNx << ", "
         << filtNy << ", "
         << filtLen << endl;
  }

  fl32 *filter = _filter->getData();

  double frac = 0.5;
  int nyHalf = _nyPadded / 2;
  int nxHalf = _nxPadded / 2;
  for (int iy = 0, ii = 0; iy < _nyPadded; iy++) {
    for (int ix = 0; ix < _nxPadded; ix++, ii++) {
      int ky = iy - nyHalf;
      int kx = ix - nxHalf;
      double dist = sqrt(kx * kx + ky * ky);
      if (dist <= filtLen * frac) {
        filter[ii] = 1.0;
      } else if (dist >= (filtLen * (1.0 + frac))) {
        filter[ii] = 0.0;
      } else {
        double arg = ((dist - (frac * filtLen)) / filtLen) * M_PI_2;
        filter[ii] = cos(arg);
      }
    } // ix
  } // iy

}
