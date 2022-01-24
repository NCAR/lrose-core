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
// ResTestEcco.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2022
//
////////////////////////////////////////////////////////////////////
//
// ResTestEcco tests Ecco for different grid resolutions.
// It does so by degrading the resolution of the input data set and
// comparing TDBZ for the different grid resolutions.
//
/////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <dataport/bigend.h>
#include <toolsa/toolsa_macros.h>
#include "ResTestEcco.hh"
using namespace std;

// Constructor

ResTestEcco::ResTestEcco(int argc, char **argv)
  
{
  
  isOK = true;
  _resReducedField = NULL;
  _missingFloat = -9999.0;
  
  // set programe name

  _progName = "ResTestEcco";
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
  
  // initialize the data input object
  
  if (_params.mode == Params::ARCHIVE) {
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

  // set up sums for different echo types

  _sumCountStrat.resize(_params.resolutions_n, 0.0);
  _sumCountMixed.resize(_params.resolutions_n, 0.0);
  _sumCountConv.resize(_params.resolutions_n, 0.0);

}

// destructor

ResTestEcco::~ResTestEcco()

{

  if (_resReducedField != NULL) {
    delete _resReducedField;
  }

  _clearResultsMdvx();
  
}

//////////////////////////////////////////////////
// Run

int ResTestEcco::Run()
{
  
  int iret = 0;

  // loop until end of data
  
  _input.reset();
  while (!_input.endOfData()) {
    
    _clearResultsMdvx();

    // do the read
    
    if (_doRead()) {
      cerr << "ERROR - ResTestEcco::Run()" << endl;
      umsleep(1000);
      iret = -1;
      continue;
    }

    for (int ires = 0; ires < _params.resolutions_n; ires++) {
      
      // process the file for this resolution factor
      
      double resFactor = _params._resolutions[ires].res_reduction_factor;
      double textureRadiusKm = _params._resolutions[ires].texture_radius_km;
      double textureLimitHigh = _params._resolutions[ires].texture_limit_high;
      
      if (_params.debug) {
        cerr << "Processing file for res reduction factor: " << resFactor << endl;
      }

      if (_processResolution(ires, resFactor, textureRadiusKm, textureLimitHigh)) {
        cerr << "ERROR - ResTestEcco::Run()" << endl;
        cerr << "  Calling _processResolution" << endl;
        iret = -1;
      }
      
    } // ires

    // clear
    
    _inMdvx.clear();
    _finder.freeArrays();
    
  } // while

  cerr << "============= Totals for data set ============" << endl;

  for (int ires = 0; ires < _params.resolutions_n; ires++) {

    double totalCount =
      _sumCountConv[ires] + _sumCountMixed[ires] + _sumCountStrat[ires];
    double fracConv = _sumCountConv[ires] / totalCount;
    double fracMixed = _sumCountMixed[ires] / totalCount;
    double fracStrat = _sumCountStrat[ires] / totalCount;
    
    cerr << "resNum, fraction strat, mixed, conv: "
         << ires << ", "
         << fracConv << ", "
         << fracMixed << ", "
         << fracStrat << endl;
    
  } // ires

  cerr << "==============================================" << endl;

  return iret;

}

////////////////////////////////////////////////////////////
// Process the input data for the given resolution factor

int ResTestEcco::_processResolution(int resNum,
                                    double resFactor,
                                    double textureRadiusKm,
                                    double textureLimitHigh)
{

  int iret = 0;
  
  // get DBZ field from input file
  
  const MdvxField *dbzFieldIn = _inMdvx.getField(_params.dbz_field_name);
  if (dbzFieldIn == NULL) {
    cerr << "ERROR - ResTestEcco::_processResolution()" << endl;
    cerr << "  no dbz field found: " << _params.dbz_field_name << endl;
    return -1;
  }
  _missingFloat = dbzFieldIn->getFieldHeader().missing_data_value;

  // create Mdvx object for results, add to vector

  _outMdvx = new DsMdvx;
  _resultsMdvx.push_back(_outMdvx);
  
  // create dbz field with reduced resolution
  
  _dbzField = NULL;
  if (fabs(resFactor - 1.0) < 0.001) {
    // for unchanged res, use input field
    _dbzField = dbzFieldIn;
  } else {
    _dbzField = _createDbzReducedRes(dbzFieldIn, resFactor);
  }
  
  const Mdvx::field_header_t &fhdr = _dbzField->getFieldHeader();
  const Mdvx::vlevel_header_t &vhdr = _dbzField->getVlevelHeader();
  bool isLatLon = (fhdr.proj_type == Mdvx::PROJ_LATLON);
  vector<double> zLevels;
  for (int iz = 0; iz < fhdr.nz; iz++) {
    zLevels.push_back(vhdr.level[iz]);
  }
    
  // set up ConvStratFinder object
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _finder.setVerbose(true);
  } else if (_params.debug) {
    _finder.setDebug(true);
  }
  _finder.setUseMultipleThreads(_params.use_multiple_threads);
  _finder.setMinValidDbz(_params.min_valid_dbz);
  _finder.setBaseDbz(_params.base_dbz);
  _finder.setMinConvectivityForConvective(_params.min_convectivity_for_convective);
  _finder.setMaxConvectivityForStratiform(_params.max_convectivity_for_stratiform);
  // double textureRadiusKm = _params.texture_radius_km * (log10(resFactor) + 1.0);
  _finder.setTextureRadiusKm(textureRadiusKm);
  _finder.setMinValidFractionForTexture
    (_params.min_valid_fraction_for_texture);
  _finder.setMinValidFractionForFit
    (_params.min_valid_fraction_for_fit);
  _finder.setTextureLimitLow(_params.texture_limit_low);
  _finder.setTextureLimitHigh(textureLimitHigh);
  
  cerr << "-->> Using textureRadiusKm: " << textureRadiusKm << endl;
  cerr << "-->> Using textureLimitHigh: " << textureLimitHigh << endl;
  
  _finder.setGrid(fhdr.nx,
                  fhdr.ny,
                  fhdr.grid_dx, 
                  fhdr.grid_dy, 
                  fhdr.grid_minx, 
                  fhdr.grid_miny,
                  zLevels,
                  isLatLon);

  _finder.setConstantHtThresholds(4.5, 9.0);
  
  // compute the convective/stratiform partition

  const fl32 *dbz = (const fl32*) _dbzField->getVol();
  if (_finder.computeEchoType(dbz, _missingFloat)) {
    cerr << "ERROR - ResTestEcco::Run()" << endl;
    return -1;
  }

  // write out
  
  if (_doWrite(resNum, resFactor)) {
    cerr << "ERROR - ResTestEcco::Run()" << endl;
    iret = -1;
  }

  // count up the categories

  const ui08 *ecco = _finder.getEchoType3D();
  size_t nPts = fhdr.nx * fhdr.ny * zLevels.size();
  double nConv = 0;
  double nMixed = 0;
  double nStrat = 0;
  for (size_t ii = 0; ii < nPts; ii++) {
    int etype = ecco[ii];
    if (etype >= ConvStratFinder::CATEGORY_STRATIFORM_LOW &&
        etype <= ConvStratFinder::CATEGORY_STRATIFORM_HIGH) {
      nStrat++;
    } else if (etype == ConvStratFinder::CATEGORY_MIXED) {
      nMixed++;
    } else if (etype >= ConvStratFinder::CATEGORY_CONVECTIVE_ELEVATED &&
               etype <= ConvStratFinder::CATEGORY_CONVECTIVE_DEEP) {
      nConv++;
    }
  }

  _sumCountConv[resNum] += nConv;
  _sumCountMixed[resNum] += nMixed;
  _sumCountStrat[resNum] += nStrat;

  double total = nConv + nMixed + nStrat;
  cerr << "resNum, fraction strat, mixed, conv: "
       << resNum << ", "
       << nStrat / total << ", "
       << nMixed / total << ", "
       << nConv / total << endl;

  // clear
  
  _finder.freeArrays();
    
  return iret;

}

////////////////////////////////////////////////////////////
// Create field at reduced resolution

MdvxField *ResTestEcco::_createDbzReducedRes(const MdvxField *dbzFieldIn,
                                             double resFactor)

{

  if (_resReducedField != NULL) {
    delete _resReducedField;
    _resReducedField = NULL;
  }
  
  // set headers
  
  const Mdvx::field_header_t &fhdrIn = dbzFieldIn->getFieldHeader();
  const Mdvx::vlevel_header_t &vhdrIn = dbzFieldIn->getVlevelHeader();

  // modify header for grid resolution
  
  Mdvx::field_header_t fhdrOut(fhdrIn);
  fhdrOut.grid_dx = fhdrIn.grid_dx * resFactor;
  fhdrOut.grid_dy = fhdrIn.grid_dy * resFactor;
  fhdrOut.nx = (int) floor(fhdrIn.nx / resFactor);
  fhdrOut.ny = (int) floor(fhdrIn.ny / resFactor);
  size_t nxyOut = fhdrOut.nx * fhdrOut.ny;
  size_t nxyzOut = nxyOut * fhdrOut.nz;
  fhdrOut.volume_size = nxyzOut * sizeof(fl32);

  // compute the kernel
  
  _computeKernel(fhdrIn, resFactor);
  
  // alloc data

  vector<fl32> dbzOut;
  dbzOut.resize(nxyzOut);
  for (size_t ii = 0; ii < nxyzOut; ii++) {
    dbzOut[ii] = _missingFloat;
  } // ii
  
  // compute the mean dbz data at this resolution
  
  const fl32 *dbzIn = (fl32 *) dbzFieldIn->getVol();
  size_t nxyIn = fhdrIn.nx * fhdrIn.ny;
  
  cerr << "==>> fhdrIn.nx: " << fhdrIn.nx << endl;
  cerr << "==>> fhdrIn.ny: " << fhdrIn.ny << endl;
  cerr << "==>> fhdrOut.nx: " << fhdrOut.nx << endl;
  cerr << "==>> fhdrOut.ny: " << fhdrOut.ny << endl;
  cerr << "==>> fhdrOut.nz: " << fhdrOut.nz << endl;
  
  for (int iz = 0; iz < fhdrIn.nz; iz++) {
    
    fl32 *dbzPlaneIn = (fl32 *) dbzIn + iz * nxyIn;
    fl32 *dbzPlaneOut = dbzOut.data() + iz * nxyOut;

    for (int iyOut = 0; iyOut < fhdrOut.ny; iyOut++) {
      for (int ixOut = 0; ixOut < fhdrOut.nx; ixOut++, dbzPlaneOut++) {
        int iyIn = (int) floor(iyOut * resFactor + 0.5);
        int ixIn = (int) floor(ixOut * resFactor + 0.5);
        if (iyIn >= fhdrIn.ny || ixIn >= fhdrIn.nx) {
          continue;
        }
        size_t xycenterIn = iyIn * fhdrIn.nx + ixIn;
        double count = 0.0;
        double sum = 0.0;
        for (size_t ii = 0; ii < _kernelOffsets.size(); ii++) {
          const kernel_t &kern = _kernelOffsets[ii];
          int jy = iyIn + kern.jy;
          int jx = ixIn + kern.jx;
          if (jy < 0 || jy > fhdrIn.ny - 1) {
            continue;
          }
          if (jx < 0 || jx > fhdrIn.nx - 1) {
            continue;
          }
          size_t jj = xycenterIn + kern.offset;
          if (jj < nxyIn) {
            fl32 val = dbzPlaneIn[jj];
            if (val != _missingFloat) {
              sum += val;
              count++;
            }
          }
        } // ii
        if (count > 0) {
          fl32 mean = sum / count;
          *dbzPlaneOut = mean;
          if (isnan(mean)) {
            cerr << "NAN" << endl;
          }
        }
      } // ix
    } // iy

    {
      // write the kernel into the original DBZ field
      // with a value of 80 at (100,100)
      int iyIn = 100;
      int ixIn = 100;
      size_t xycenterIn = iyIn * fhdrIn.nx + ixIn;
      for (size_t ii = 0; ii < _kernelOffsets.size(); ii++) {
        const kernel_t &kern = _kernelOffsets[ii];
        int jy = iyIn + kern.jy;
        int jx = ixIn + kern.jx;
        if (jy < 0 || jy > fhdrIn.ny - 1) {
          continue;
        }
        if (jx < 0 || jx > fhdrIn.nx - 1) {
          continue;
        }
        size_t jj = xycenterIn + kern.offset;
        if (jj < nxyIn) {
          dbzPlaneIn[jj] = 80;
        }
      } // ii
    }
    
  } // iz

  for (size_t ii = 0; ii < nxyzOut; ii++) {
    if (isnan(dbzOut[ii])) {
      cerr << "XXXXXXXXXXXXXX ii: " << endl;
    }
  } // ii

  _resReducedField = new MdvxField(fhdrOut, vhdrIn, dbzOut.data());
  return _resReducedField;

}

//////////////////////////////////////
// compute the kernels for this grid

void ResTestEcco::_computeKernel(const Mdvx::field_header_t &fhdrIn,
                                 double resFactor)

{

  // texture kernel

  _kernelOffsets.clear();

  _nyKernel = (size_t) floor(resFactor + 0.5) - 1;
  if (_nyKernel < 1) {
    _nyKernel = 1;
  }
  _nxKernel = _nyKernel;

  kernel_t entry;
  for (int jdy = -_nyKernel; jdy <= _nyKernel; jdy++) {
    double yy = jdy;
    for (int jdx = -_nxKernel; jdx <= _nxKernel; jdx++) {
      double xx = jdx;
      double radius = sqrt(yy * yy + xx * xx);
      if (radius <= (resFactor - 1.0) * 1.1) {
        entry.jx = jdx;
        entry.jy = jdy;
        entry.offset = jdx + jdy * fhdrIn.nx;
        _kernelOffsets.push_back(entry);
      }
    }
  }

}

/////////////////////////////////////////////////////////
// perform the read
//
// Returns 0 on success, -1 on failure.

int ResTestEcco::_doRead()
  
{
  
  _inMdvx.clear();
  if (_params.debug >= Params::DEBUG_EXTRA) {
    _inMdvx.setDebug(true);
  }
  _inMdvx.addReadField(_params.dbz_field_name);
  _inMdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _inMdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
 
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _inMdvx.printReadRequest(cerr);
  }
  
  // read in
  
  if (_input.readVolumeNext(_inMdvx)) {
    cerr << "ERROR - ResTestEcco::_doRead" << endl;
    cerr << "  Cannot read in data." << endl;
    cerr << _input.getErrStr() << endl;
    return -1;
  }
  
  if (_inMdvx.getField(_params.dbz_field_name) == NULL) {
    cerr << "ERROR - ResTestEcco::_doRead()" << endl;
    cerr << "  no dbz field found: " << _params.dbz_field_name << endl;
    cerr << "  file path: " << _inMdvx.getPathInUse() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Read in file: " << _inMdvx.getPathInUse() << endl;
  }

  return 0;

}

/////////////////////////////////////////////////////////
// add fields to the output object

void ResTestEcco::_addFields()
  
{

  _outMdvx->clearFields();

  // initial fields are float32

  Mdvx::field_header_t fhdr2d = _dbzField->getFieldHeader();
  fhdr2d.nz = 1;
  fhdr2d.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  size_t planeSize32 = fhdr2d.nx * fhdr2d.ny * sizeof(fl32);
  fhdr2d.volume_size = planeSize32;
  fhdr2d.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr2d.data_element_nbytes = 4;
  fhdr2d.missing_data_value = _missingFloat;
  fhdr2d.bad_data_value = _missingFloat;
  fhdr2d.scale = 1.0;
  fhdr2d.bias = 0.0;
  
  Mdvx::vlevel_header_t vhdr2d;
  MEM_zero(vhdr2d);
  vhdr2d.level[0] = 0;
  vhdr2d.type[0] = Mdvx::VERT_TYPE_SURFACE;

  // add texture field composite
  _outMdvx->addField(_makeField(fhdr2d, vhdr2d,
                                _finder.getTexture2D(),
                                Mdvx::ENCODING_INT16,
                                "DbzTextureComp",
                                "reflectivity_texture_composite",
                                "dBZ"));

  // convectivity composite
  _outMdvx->addField(_makeField(fhdr2d, vhdr2d,
                                _finder.getConvectivity2D(),
                                Mdvx::ENCODING_INT16,
                                "ConvectivityComp",
                                "likelihood_of_convection_composite",
                                ""));
  // dbz composite
  _outMdvx->addField(_makeField(fhdr2d, vhdr2d,
                                _finder.getDbzColMax(),
                                Mdvx::ENCODING_INT16,
                                "DbzComp",
                                "dbz_composite",
                                "dBZ"));

  // load up fraction of texture kernel covered
  _outMdvx->addField(_makeField(fhdr2d, vhdr2d,
                                _finder.getFractionActive(),
                                Mdvx::ENCODING_INT16,
                                "FractionActive",
                                "fraction_of_texture_kernel_active",
                                ""));

  // the following 2d fields are unsigned bytes
  
  size_t planeSize08 = fhdr2d.nx * fhdr2d.ny * sizeof(ui08);
  fhdr2d.volume_size = planeSize08;
  fhdr2d.encoding_type = Mdvx::ENCODING_INT8;
  fhdr2d.data_element_nbytes = 1;
  fhdr2d.missing_data_value = ConvStratFinder::CATEGORY_MISSING;
  fhdr2d.bad_data_value = ConvStratFinder::CATEGORY_MISSING;
  
  // echoType field
  
  _outMdvx->addField(_makeField(fhdr2d, vhdr2d,
                                _finder.getEchoType2D(),
                                Mdvx::ENCODING_INT8,
                                "EchoTypeComp",
                                "convective_stratiform_echo_type_composite",
                                ""));

  // the following 3d fields are floats
  
  Mdvx::field_header_t fhdr3d = _dbzField->getFieldHeader();
  Mdvx::vlevel_header_t vhdr3d = _dbzField->getVlevelHeader();
  fhdr3d.missing_data_value = _missingFloat;
  fhdr3d.bad_data_value = _missingFloat;

  // texture in 3D
  _outMdvx->addField(_makeField(fhdr3d, vhdr3d,
                                _finder.getTexture3D(),
                                Mdvx::ENCODING_INT16,
                                "DbzTexture3D",
                                "reflectivity_texture_3D",
                                "dBZ"));
  // convectivity in 3D
  _outMdvx->addField(_makeField(fhdr3d, vhdr3d,
                                _finder.getConvectivity3D(),
                                Mdvx::ENCODING_INT16,
                                "Convectivity3D",
                                "likelihood_of_convection_3D",
                                ""));

  // echo the input DBZ field
  _outMdvx->addField(_makeField(fhdr3d, vhdr3d,
                                _finder.getDbz3D(),
                                Mdvx::ENCODING_INT16,
                                "Dbz3D",
                                "reflectivity_3D",
                                "dBZ"));
  
  // echoType for full volume
  size_t volSize08 = fhdr3d.nx * fhdr3d.ny * fhdr3d.nz * sizeof(ui08);
  fhdr3d.volume_size = volSize08;
  fhdr3d.encoding_type = Mdvx::ENCODING_INT8;
  fhdr3d.data_element_nbytes = 1;
  fhdr3d.missing_data_value = ConvStratFinder::CATEGORY_MISSING;
  fhdr3d.bad_data_value = ConvStratFinder::CATEGORY_MISSING;
  _outMdvx->addField(_makeField(fhdr3d, vhdr3d,
                                _finder.getEchoType3D(),
                                Mdvx::ENCODING_INT8,
                                "EchoType3D",
                                "convective_stratiform_echo_type_3D",
                                ""));

  MdvxField *dbzFieldIn = new MdvxField(*_inMdvx.getField(_params.dbz_field_name));
  _outMdvx->addField(dbzFieldIn);
  
}

/////////////////////////////////////////////////////////
// write out results
// Returns 0 on success, -1 on failure.

int ResTestEcco::_doWrite(int resNum, double resFactor)
  
{
  
  // create output DsMdvx object
  // copying master header from input object
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _outMdvx->setDebug(true);
  }
  Mdvx::master_header_t mhdr = _inMdvx.getMasterHeader();
  _outMdvx->setMasterHeader(mhdr);
  string info = _inMdvx.getMasterHeader().data_set_info;
  info += " : Stratfinder used to identify stratiform regions";
  _outMdvx->setDataSetInfo(info.c_str());
  
  // copy any chunks
  
  for (size_t i = 0; i < _inMdvx.getNChunks(); i++) {
    MdvxChunk *chunk = new MdvxChunk(*_inMdvx.getChunkByNum(i));
    _outMdvx->addChunk(chunk);
  }
  
  // add fields
  
  _addFields();
  
  // write out

  char outDir[1024];
  snprintf(outDir, 1024, "%s/res_factor_%.2f", _params.output_dir, resFactor);
  if(_outMdvx->writeToDir(outDir)) {
    cerr << "ERROR - ResTestEcco::Run" << endl;
    cerr << "  Cannot write data set." << endl;
    cerr << _outMdvx->getErrStr() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Wrote file: " << _outMdvx->getPathInUse() << endl;
  }

  return 0;

}

/////////////////////////////////////////////////////////
// create a float field

MdvxField *ResTestEcco::_makeField(Mdvx::field_header_t &fhdrTemplate,
                                   Mdvx::vlevel_header_t &vhdr,
                                   const fl32 *data,
                                   Mdvx::encoding_type_t outputEncoding,
                                   string fieldName,
                                   string longName,
                                   string units)
                                 
{

  Mdvx::field_header_t fhdr = fhdrTemplate;
  MdvxField::setFieldName(fieldName, fhdr);
  MdvxField::setFieldNameLong(longName, fhdr);
  MdvxField::setUnits(units, fhdr);
  MdvxField *newField =
    new MdvxField(fhdr, vhdr, NULL, false, false, false);
  size_t nPts = fhdr.nx * fhdr.ny * fhdr.nz;
  size_t volSize = nPts * sizeof(fl32);
  newField->setVolData(data, volSize, Mdvx::ENCODING_FLOAT32);
  newField->convertType(outputEncoding, Mdvx::COMPRESSION_GZIP);

  return newField;

}

/////////////////////////////////////////////////////////
// create a byte field

MdvxField *ResTestEcco::_makeField(Mdvx::field_header_t &fhdrTemplate,
                                   Mdvx::vlevel_header_t &vhdr,
                                   const ui08 *data,
                                   Mdvx::encoding_type_t outputEncoding,
                                   string fieldName,
                                   string longName,
                                   string units)
                                 
{
  
  Mdvx::field_header_t fhdr = fhdrTemplate;
  MdvxField::setFieldName(fieldName, fhdr);
  MdvxField::setFieldNameLong(longName, fhdr);
  MdvxField::setUnits(units, fhdr);
  MdvxField *newField =
    new MdvxField(fhdr, vhdr, NULL, false, false, false);
  size_t nPts = fhdr.nx * fhdr.ny * fhdr.nz;
  size_t volSize = nPts * sizeof(ui08);
  newField->setVolData(data, volSize, Mdvx::ENCODING_INT8);
  newField->convertType(outputEncoding, Mdvx::COMPRESSION_GZIP);
  
  return newField;

}

/////////////////////////////////////////////////////////
// clear the results

void ResTestEcco::_clearResultsMdvx()

{

  for (size_t ii = 0; ii < _resultsMdvx.size(); ii++) {
    delete _resultsMdvx[ii];
  }
  _resultsMdvx.clear();

}
