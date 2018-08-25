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
// MdvRadarShear.cc
//
// MdvRadarShear object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2009
//
///////////////////////////////////////////////////////////////
//
// MdvRadarShear reads in radial velocity data in MDV polar
// radar format, computes the shear and writes out shear fields
// in MDV format.
//
///////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/MemBuf.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/toolsa_macros.h>
#include <rapmath/usvd.h>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxChunk.hh>
#include <Mdv/MdvxRadar.hh>
#include <radar/FilterUtils.hh>
#include "MdvRadarShear.hh"
using namespace std;

const double MdvRadarShear::missingDouble = -9999.0;
const fl32 MdvRadarShear::missingFloat = -9999.0;

// Constructor

MdvRadarShear::MdvRadarShear(int argc, char **argv)

{
  
  isOK = true;
  
  // pointers into MDV volume

  _dbzMdv = NULL;
  _snrMdv = NULL;
  _velMdv = NULL;
  _censorMdv = NULL;

  _b = NULL;
  _x = NULL;
  _w = NULL;

  _a = NULL;
  _u = NULL;
  _v = NULL;

  // geometry

  _startRange = 0.0;
  _gateSpacing = 0.0;
  _isRhi = false;

  // set programe name

  _progName = "MdvRadarShear";
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
    if (_input.setRealtime(_params.input_dir, 600,
			   PMU_auto_register)) {
      isOK = false;
    }
  } else if (_params.mode == Params::ARCHIVE) {
    if (_input.setArchive(_params.input_dir,
			  _args.startTime,
			  _args.endTime)) {
      isOK = false;
    }
  } else if (_params.mode == Params::FILELIST) {
    if (_input.setFilelist(_args.inputFileList)) {
      isOK = false;
    }
  }

  // set kernel size - make sure it is odd
  
  _nKernel = _params.shear_kernel_size;
  if (((_nKernel / 2) * 2) == _nKernel) {
    _nKernel++;
  }
  _nKernelHalf = _nKernel / 2;

  // set up vectors and matrices for SVD solving

  _b = new double[nCoeff];
  _x = new double[nCoeff];
  _w = new double[nCoeff];

  _a = (double **) umalloc2(nCoeff, nCoeff, sizeof(double));
  _u = (double **) umalloc2(nCoeff, nCoeff, sizeof(double));
  _v = (double **) umalloc2(nCoeff, nCoeff, sizeof(double));
  
  return;

}

// destructor

MdvRadarShear::~MdvRadarShear()

{

  if (_b) {
    delete[] _b;
  }
  if (_x) {
    delete[] _x;
  }
  if (_w) {
    delete[] _w;
  }

  if (_a) {
    ufree2((void **) _a);
  }
  if (_u) {
    ufree2((void **) _u);
  }
  if (_v) {
    ufree2((void **) _v);
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int MdvRadarShear::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");
  
  // loop until end of data
  
  while (!_input.endOfData()) {
    
    PMU_auto_register("In main loop");

    // read the next MDV volume
    
    if (_readNextVolume()) {
      continue;
    }
    
    // check for uniform field geometry
    // Sets _nGates, _nBeams, _nTilts
    
    if (!_checkFields()) {
      cerr << "ERROR - MdvRadarShear::Run" << endl;
      cerr << "  Ignoring file because fields do not have same geometry." << endl;
      cerr << "  File: " << _inMdvx.getPathInUse() << endl;
      continue;
    }
    
    // set array pointers to the input field data
    
    if (_setInputPointers()) {
      cerr << "ERROR - MdvRadarShear::Run()" << endl;
      cerr << "  Cannot set input pointers" << endl;
      continue;
    }
    
    // compute KDP

    if (_isRhi) {
      if (_params.debug >= Params::DEBUG_NORM) {
        cerr << "RHI - ignoring ..." << endl;
      }
      continue;
    }

    _processVolume();
    
    // create the output MDV file and set the data fields

    if (_params.debug >= Params::DEBUG_NORM) {
      cerr << "Writing file ..." << endl;
    }

    DsMdvx outMdvx;
    _fillOutput(outMdvx);

    // write output

    if (_writeOutput(outMdvx)) {
      cerr << "ERROR - MdvRadarShear::Run()" << endl;
      cerr << "  Cannot write output file" << endl;
    }
    
  } // while

  return 0;

}

///////////////////////////////////////////
// set up the read for the next MDV file

void MdvRadarShear::_setupRead()
  
{

  _inMdvx.setDebug(_params.debug);
  _inMdvx.clearRead();
  
  if (_params.set_elev_limits) {
    _inMdvx.setReadVlevelLimits(_params.lower_elev, _params.upper_elev);
  }

  _inMdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _inMdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  
  _inMdvx.addReadField(_params.DBZ_field_name);
  _inMdvx.addReadField(_params.SNR_field_name);
  _inMdvx.addReadField(_params.VEL_field_name);
  
  if (_params.censor_using_thresholds) {
    _inMdvx.addReadField(_params.censor_field_name);
  }

  for (int ii = 0; ii < _params.echo_field_names_n; ii++) {
    _inMdvx.addReadField(_params._echo_field_names[ii]);
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Reading input MDV file" << endl;
    _inMdvx.printReadRequest(cerr);
  }

}

///////////////////////////
// read in next volume file

int MdvRadarShear::_readNextVolume()
  
{

  PMU_auto_register("Before read");

  // set up read
  
  _setupRead();

  // read the volume
  
  if (_input.readVolumeNext(_inMdvx)) {
    cerr << "ERROR - MdvRadarShear::_readInput" << endl;
    cerr << "  Cannot read in MDV data." << endl;
    cerr << _input.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Working on file: " << _inMdvx.getPathInUse() << endl;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Input file fields:" << endl;
    for (int ifield = 0;
	 ifield < _inMdvx.getMasterHeader().n_fields; ifield++) {
      MdvxField *fld = _inMdvx.getField(ifield);
      const Mdvx::field_header_t &fhdr = fld->getFieldHeader();
      cerr << "Field " << ifield << ", name " << fhdr.field_name << endl;
    }
  }

  return 0;

}

///////////////////////////////////////////
// check that the fields are uniform in size

bool MdvRadarShear::_checkFields()
  
{

  const Mdvx::master_header_t &mhdr = _inMdvx.getMasterHeader();
  const MdvxField *fld0 = _inMdvx.getField(0);
  const Mdvx::field_header_t &fhdr0 = fld0->getFieldHeader();
  const Mdvx::vlevel_header_t &vhdr0 = fld0->getVlevelHeader();
  
  _nGates = fhdr0.nx;
  _nBeams = fhdr0.ny;
  _nTilts = fhdr0.nz;
  _nPointsTilt = _nGates * _nBeams;
  _nPointsVol = _nPointsTilt * _nTilts;

  for (int i = 1; i < mhdr.n_fields; i++) {
    const MdvxField *fld = _inMdvx.getField(i);
    const Mdvx::field_header_t &fhdr = fld->getFieldHeader();
    int nGates = fhdr.nx;
    int nBeams = fhdr.ny;
    int nTilts = fhdr.nz;
    if (nGates != _nGates ||
	nBeams != _nBeams ||
	nTilts != _nTilts) {
      cerr << "ERROR - MdvRadarShear::_uniformFields()" << endl;
      cerr << "  Fields are not uniform in geometry" << endl;
      cerr << "  Geometry in first field, name, nGates, nBeams, nTilts: "
	   << fhdr0.field_name << ", "
	   << _nGates << ", " << _nBeams << ", " << _nTilts << endl;
      cerr << "  Geometry in this field, name, nGates, nBeams, nTilts: "
	   << fhdr.field_name << ", "
	   << nGates << ", " << nBeams << ", " << nTilts << endl;
      cerr << "  File: " << _inMdvx.getPathInUse() << endl;
      return false;
    }
  }

  _startRange = fhdr0.grid_minx;
  _gateSpacing = fhdr0.grid_dx;
  _startAz = fhdr0.grid_miny;
  _deltaAz = fhdr0.grid_dy;

  _elevs.clear();
  for (int ii = 0; ii < _nTilts; ii++) {
    _elevs.push_back(vhdr0.level[ii]);
  }
  
  if (fhdr0.proj_type == Mdvx::PROJ_RHI_RADAR) {
    _isRhi = true;
    _rhiMinEl = fhdr0.grid_miny;
    _rhiDeltaEl = fhdr0.grid_dy;
  }

  return true;

}

///////////////////////////////////////////
// set up the input pointers

int MdvRadarShear::_setInputPointers()
  
{

  const MdvxField *dbzFld = _inMdvx.getField(_params.DBZ_field_name);
  if (dbzFld == NULL) {
    cerr << "ERROR - MdvRadarShear::_setInputPointers()" << endl;
    cerr << "  Cannot find DBZ field, name: " << _params.DBZ_field_name << endl;
    return -1;
  }
  _dbzMdv = (fl32 *) dbzFld->getVol();
  _dbzMiss = dbzFld->getFieldHeader().missing_data_value;

  const MdvxField *snrFld = _inMdvx.getField(_params.SNR_field_name);
  if (snrFld == NULL) {
    cerr << "ERROR - MdvRadarShear::_setInputPointers()" << endl;
    cerr << "  Cannot find SNR field, name: " << _params.SNR_field_name << endl;
    return -1;
  }
  _snrMdv = (fl32 *) snrFld->getVol();
  _snrMiss = snrFld->getFieldHeader().missing_data_value;
  
  const MdvxField *velFld = _inMdvx.getField(_params.VEL_field_name);
  if (velFld == NULL) {
    cerr << "ERROR - MdvRadarShear::_setInputPointers()" << endl;
    cerr << "  Cannot find VEL field, name: " << _params.VEL_field_name << endl;
    return -1;
  }
  _velMdv = (fl32 *) velFld->getVol();
  _velMiss = velFld->getFieldHeader().missing_data_value;
  
  if (_params.censor_using_thresholds) {
    const MdvxField *censorFld = _inMdvx.getField(_params.censor_field_name);
    if (censorFld == NULL) {
      cerr << "ERROR - MdvRadarShear::_setInputPointers()" << endl;
      cerr << "  Cannot find censor field, name: " << _params.censor_field_name << endl;
      return -1;
    }
    _censorMdv = (fl32 *) censorFld->getVol();
    _censorMiss = censorFld->getFieldHeader().missing_data_value;
  }

  return 0;

}

///////////////////////////////////////////
// process the volume

void MdvRadarShear::_processVolume()
  
{

  if (_params.debug >= Params::DEBUG_NORM) {
    cerr << "Processing volume ..." << endl;
  }

  // allocate arrays for computed fields, set to missing
  
  _radial_shear = _radial_shear_.alloc(_nPointsVol);
  _azimuthal_shear = _azimuthal_shear_.alloc(_nPointsVol);
  _max_shear = _max_shear_.alloc(_nPointsVol);
  _mean_vel = _mean_vel_.alloc(_nPointsVol);

  for (int ii = 0; ii < _nPointsVol; ii++) {
    _radial_shear[ii] = missingFloat;
    _azimuthal_shear[ii] = missingFloat;
    _max_shear[ii] = missingFloat;
    _mean_vel[ii] = missingFloat;
  }

  // compute range and ss - which is the distance between gates
  // at adjacent azimuths

  double deltaAzRad = _deltaAz * DEG_TO_RAD;
  _range = _range_.alloc(_nGates);
  _ss = _ss_.alloc(_nGates);
  for (int ii = 0; ii < _nGates; ii++) {
    _range[ii] = _startRange + ii * _gateSpacing;
    _ss[ii] = _range[ii] * deltaAzRad;
  }
  
  // alloc local arrays which include extra space in azimuths
  // so that our kernel computations work correctly
  
  int nBeamsExtended = _nBeams + 2 * _nKernelHalf;
  
  fl32 **dbz_ = (fl32 **) umalloc2(nBeamsExtended, _nGates, sizeof(fl32));
  fl32 **dbz = dbz_ + _nKernelHalf;
  
  fl32 **vel_ = (fl32 **) umalloc2(nBeamsExtended, _nGates, sizeof(fl32));
  fl32 **vel = vel_ + _nKernelHalf;
  
  fl32 **snr_ = (fl32 **) umalloc2(nBeamsExtended, _nGates, sizeof(fl32));
  fl32 **snr = snr_ + _nKernelHalf;
  
  // deal with a tilt at a time
  
  for (int itilt = 0; itilt < _nTilts; itilt++) {
    
    // load up field data

    int mdvPos = itilt * _nPointsTilt;
    for (int ibeam = 0; ibeam < _nBeams; ibeam++) {
      for (int igate = 0; igate < _nGates; igate++, mdvPos++) {
        dbz[ibeam][igate] = _dbzMdv[mdvPos];
        vel[ibeam][igate] = _velMdv[mdvPos];
        snr[ibeam][igate] = _snrMdv[mdvPos];
      }
    }

    // populate the overlap region as appropriate

    double azExtent = _nBeams * _deltaAz;
    double full360 = false;
    if (fabs(azExtent - 360) < 0.01) {
      full360 = true;
    }
    
    // copy data into the overlap region as appropriate

    for (int ii = -_nKernelHalf; ii < 0; ii++) {
      if (full360) {
        int jj = ii + _nBeams;
        for (int igate = 0; igate < _nGates; igate++) {
          dbz[ii][igate] = dbz[jj][igate];
          vel[ii][igate] = vel[jj][igate];
          snr[ii][igate] = snr[jj][igate];
        }
      } else {
        for (int igate = 0; igate < _nGates; igate++) {
          dbz[ii][igate] = _dbzMiss;
          vel[ii][igate] = _velMiss;
          snr[ii][igate] = _snrMiss;
        }
      }
    } // ii

    for (int ii = _nBeams; ii < _nBeams + _nKernelHalf; ii++) {
      if (full360) {
        int jj = ii - _nBeams;
        for (int igate = 0; igate < _nGates; igate++) {
          dbz[ii][igate] = dbz[jj][igate];
          vel[ii][igate] = vel[jj][igate];
          snr[ii][igate] = snr[jj][igate];
        }
      } else {
        for (int igate = 0; igate < _nGates; igate++) {
          dbz[ii][igate] = _dbzMiss;
          vel[ii][igate] = _velMiss;
          snr[ii][igate] = _snrMiss;
        }
      }
    } // ii

    // apply median filter as appropriate
    
    if (_params.apply_median_filter_to_DBZ) {
      int filterLen = _params.DBZ_median_filter_len;
      for (int ibeam = -_nKernelHalf; ibeam < _nBeams + _nKernelHalf; ibeam++) {
        _applyMedianFilter(dbz[ibeam], _nGates, filterLen);
      }
    }

    if (_params.apply_median_filter_to_VEL) {
      int filterLen = _params.VEL_median_filter_len;
      for (int ibeam = -_nKernelHalf; ibeam < _nBeams + _nKernelHalf; ibeam++) {
        _applyMedianFilter(vel[ibeam], _nGates, filterLen);
      }
    }

    if (_params.apply_median_filter_to_SNR) {
      int filterLen = _params.SNR_median_filter_len;
      for (int ibeam = -_nKernelHalf; ibeam < _nBeams + _nKernelHalf; ibeam++) {
        _applyMedianFilter(snr[ibeam], _nGates, filterLen);
      }
    }

    // compute shear for each suitable point in the tilt

    for (int ibeam = 0; ibeam < _nBeams; ibeam++) {
      for (int igate = _nKernelHalf; igate < _nGates - _nKernelHalf; igate++) {
        double radialShear, azimuthalShear, meanVel;
        _computeShear(ibeam, igate, dbz, vel, snr,
                      radialShear, azimuthalShear, meanVel);
        int mdvPos = itilt * _nPointsTilt + ibeam * _nGates + igate;
        _radial_shear[mdvPos] = radialShear;
        _azimuthal_shear[mdvPos] = azimuthalShear;
        _max_shear[mdvPos] = MAX(radialShear, azimuthalShear);
        _mean_vel[mdvPos] = meanVel;
      } // igate
    }
    
  } // itilt

  // free up

  ufree2 ((void **) dbz_);
  ufree2 ((void **) vel_);
  ufree2 ((void **) snr_);

}

//////////////////////////////////////////
// compute shear at a gate

void MdvRadarShear::_computeShear(int ibeam, int igate,
                                  fl32 **dbz, fl32 **vel, fl32 **snr,
                                  double &radialShear, double &azimuthalShear,
                                  double &meanVel)

{

  radialShear = missingDouble;
  azimuthalShear = missingDouble;
  meanVel = missingDouble;
  
  // we compuet shear by fitting a 2-D plane to the local (kernel)
  // region of the radial velocity.

  // initialize matrices

  memset(_b, 0, nCoeff * sizeof(double));
  memset(_x, 0, nCoeff * sizeof(double));
  memset(_w, 0, nCoeff * sizeof(double));

  for (int ii = 0; ii < nCoeff; ii++) {
    for (int jj = 0; jj < nCoeff; jj++) {
      _a[ii][jj] = 0.0;
      _u[ii][jj] = 0.0;
      _v[ii][jj] = 0.0;
    } // jj
  } // ii

  // load up matrices

  double ss = _ss[igate];
  double maxDeltaS = _params.max_arc_width / 2.0;

  int count = 0;

  for (int ii = - _nKernelHalf; ii <= _nKernelHalf; ii++) {
    for (int jj = - _nKernelHalf; jj <= _nKernelHalf; jj++) {

      double deltaS = ii * ss;
      double deltaR = jj * _gateSpacing;
      
      fl32 sn = snr[ibeam + ii][igate + jj];
      fl32 uu = vel[ibeam + ii][igate + jj];
      
      if (sn == _snrMiss || uu == _velMiss) {
        continue;
      }

      if (fabs(deltaS) > maxDeltaS) {
        continue;
      }

      double weight = 1.0;

      _a[0][0] += deltaR * deltaR * weight;
      _a[0][1] += deltaR * deltaS * weight;
      _a[0][2] += deltaR * weight;

      _a[1][0] += deltaR * deltaS * weight;
      _a[1][1] += deltaS * deltaS * weight;
      _a[1][2] += deltaS * weight;

      _a[2][0] += deltaR * weight;
      _a[2][1] += deltaS * weight;
      _a[2][2] += weight;

      _b[0] += deltaR * uu * weight;
      _b[1] += deltaS * uu * weight;
      _b[2] += uu * weight;

      count++;

    }
  }

  // check count

  if (count < _nKernel) {
    // too little valid data
    return;
  }
  
  // compute SVD
  
  int iret = usvd(_a, nCoeff, nCoeff, _u, _v, _w);
  if (iret) {
    cerr << "WARNING - MdvRadarShear::_computeShear()" << endl;
    cerr << "  SVD returns error: " << iret << endl;
    cerr << "  ibeam, igate: " << ibeam << ", " << igate << endl;
    return;
  }
  
  // apply SVD to solve for x

  usvd_apply(_u, _w, _v, nCoeff, nCoeff, _b, _x);
  
  // set return values

  if (fabs(_x[0]) < 100 && fabs(_x[1]) < 100 && fabs(_x[2]) < 250) {
    radialShear = _x[0];
    azimuthalShear = _x[1];
    meanVel = _x[2];
  }

}

//////////////////////////////////////////
// fill output MDV object

void MdvRadarShear::_fillOutput(DsMdvx &outMdvx)
 
{

  // fields

  outMdvx.clearFields();

  if (_params.RADIAL_SHEAR_output_field.write) {
    _addField(outMdvx,
	      _params.RADIAL_SHEAR_output_field.name, 
	      _params.RADIAL_SHEAR_output_field.long_name, 
	      _params.RADIAL_SHEAR_output_field.units,
	      _params.RADIAL_SHEAR_output_field.encoding,
	      _radial_shear);
  }
  
  if (_params.AZIMUTHAL_SHEAR_output_field.write) {
    _addField(outMdvx,
              _params.AZIMUTHAL_SHEAR_output_field.name, 
              _params.AZIMUTHAL_SHEAR_output_field.long_name, 
              _params.AZIMUTHAL_SHEAR_output_field.units,
              _params.AZIMUTHAL_SHEAR_output_field.encoding,
              _azimuthal_shear);
  }
  
  if (_params.MAX_SHEAR_output_field.write) {
    _addField(outMdvx,
              _params.MAX_SHEAR_output_field.name, 
              _params.MAX_SHEAR_output_field.long_name, 
              _params.MAX_SHEAR_output_field.units,
              _params.MAX_SHEAR_output_field.encoding,
              _max_shear);
  }
  
  if (_params.MEAN_VEL_output_field.write) {
    _addField(outMdvx,
              _params.MEAN_VEL_output_field.name, 
              _params.MEAN_VEL_output_field.long_name, 
              _params.MEAN_VEL_output_field.units,
              _params.MEAN_VEL_output_field.encoding,
              _mean_vel);
  }
  
  // optionally echo input fields into output file
  
  _echoInputFields(outMdvx);
  
  // copy chunks
  
  outMdvx.clearChunks();
  for (int ii = 0; ii < _inMdvx.getNChunks(); ii++) {
    MdvxChunk *chunk = new MdvxChunk(*_inMdvx.getChunkByNum(ii));
    outMdvx.addChunk(chunk);
  }

  // master header
  
  Mdvx::master_header_t mhdr = _inMdvx.getMasterHeader();
  mhdr.n_fields = outMdvx.getNFields();
  mhdr.n_chunks = outMdvx.getNChunks();
  outMdvx.setMasterHeader(mhdr);
  
  string info = "Shear fields computed by MdvRadarShear\n";
  info += mhdr.data_set_info;
  info += "\n";
  outMdvx.setDataSetInfo(info.c_str());

}
  
//////////////////////////////////////////
// add output field to output MDVX object

void MdvRadarShear::_addField(DsMdvx &outMdvx,
			    const string &field_name,
			    const string &long_field_name,
			    const string &units,
			    int encoding_type,
			    const fl32 *data)
  
{
  
  if (_params.debug) {
    cerr << "  Adding field: " << field_name << endl;
  }

  const MdvxField *fld0 = _inMdvx.getField(0);
  
  // copy field header, set members which change
  
  Mdvx::field_header_t fhdr = fld0->getFieldHeader();

  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = 4;
  fhdr.volume_size = _nPointsVol * sizeof(fl32);
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;
  
  fhdr.scale = 1.0;
  fhdr.bias = 0.0;
  fhdr.bad_data_value = missingFloat;
  fhdr.missing_data_value = missingFloat;
  
  fhdr.min_value = 0;
  fhdr.max_value = 0;
  fhdr.min_value_orig_vol = 0;
  fhdr.max_value_orig_vol = 0;
  
  STRncopy(fhdr.field_name_long, long_field_name.c_str(), MDV_LONG_FIELD_LEN);
  STRncopy(fhdr.field_name, field_name.c_str(), MDV_SHORT_FIELD_LEN);
  STRncopy(fhdr.units, units.c_str(), MDV_UNITS_LEN);
  STRncopy(fhdr.transform, "none", MDV_TRANSFORM_LEN);
  
  // copy vlevel header
  
  Mdvx::vlevel_header_t vhdr = fld0->getVlevelHeader();

  // create field
  
  MdvxField *fld = new MdvxField(fhdr, vhdr, data);

  // convert to output encoding type, and compress

  fld->convertDynamic((Mdvx::encoding_type_t) encoding_type, Mdvx::COMPRESSION_GZIP);

  // add to object
  
  outMdvx.addField(fld);

}
  
////////////////////////////////////
// echo input fields to output file

int MdvRadarShear::_echoInputFields(DsMdvx &outMdvx)
  
{

  if (!_params.echo_input_fields_in_output_file) {
    return 0;
  }

  int iret = 0;

  for (int ii = 0; ii < _params.echo_field_names_n; ii++) {
    char *fname = _params._echo_field_names[ii];
    MdvxField *fld = _inMdvx.getField(fname);
    if (_params.debug) {
      cerr << "  Echoing input field: " << fname << endl;
    }
    if (fld == NULL) {
      cerr << "WARNING - void MdvRadarShear::_fillOutput" << endl;
      cerr << "  Trying to echo input field to output file: "
           << fname << endl;
      cerr << "  Cannot find field in input file" << endl;
      iret = -1;
    } else {
      MdvxField *echoFld = new MdvxField(*fld);
      outMdvx.addField(echoFld);
    }
  } // ii

  return iret;

}

///////////////////////////
// write Output file

int MdvRadarShear::_writeOutput(DsMdvx &outMdvx)
  
{

  PMU_auto_register("Before write");
  outMdvx.setWriteLdataInfo();
  outMdvx.setAppName(_progName);
  if(outMdvx.writeToDir(_params.output_url)) {
    cerr << "ERROR - MdvRadarShear::_writeOutput" << endl;
    cerr << "  Cannot write data set." << endl;
    cerr << outMdvx.getErrStr() << endl;
    return -1;
  }
  if (_params.debug) {
    cerr << "Wrote file: " << outMdvx.getPathInUse() << endl;
  }

  return 0;

}


///////////////////////////////////
// apply a median filter to a field

void MdvRadarShear::_applyMedianFilter(fl32 *field,
                                       int fieldLen,
                                       int filterLen)
  
{
  
  // make sure filter len is odd
  
  int halfFilt = filterLen / 2;
  int len = halfFilt * 2 + 1;
  if (len < 3) {
    return;
  }
  
  TaArray<fl32> buf_, copy_;
  fl32 *buf = buf_.alloc(len);
  fl32 *copy = copy_.alloc(fieldLen);
  memcpy(copy, field, fieldLen * sizeof(fl32));
  
  for (int ii = halfFilt; ii < fieldLen - halfFilt; ii++) {
    
    memcpy(buf, copy + ii - halfFilt, len * sizeof(fl32));
    qsort(buf, len, sizeof(fl32), _fl32Compare);
    field[ii] = buf[halfFilt];

  }

}

/////////////////////////////////////////////////////
// define functions to be used for sorting

int MdvRadarShear::_fl32Compare(const void *i, const void *j)
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

