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
#include <toolsa/pmu.h>
#include <toolsa/toolsa_macros.h>
#include "ResTestEcco.hh"
using namespace std;

const fl32 ResTestEcco::_missing = -9999.0;

// Constructor

ResTestEcco::ResTestEcco(int argc, char **argv)
  
{
  
  isOK = true;
  _tempField = NULL;
  
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

  // set up ConvStratFinder object

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _finder.setVerbose(true);
  } else if (_params.debug) {
    _finder.setDebug(true);
  }
  _finder.setMinValidDbz(_params.min_valid_dbz);
  _finder.setMinConvectivityForConvective(_params.min_convectivity_for_convective);
  _finder.setMaxConvectivityForStratiform(_params.max_convectivity_for_stratiform);
  _finder.setTextureRadiusKm(_params.texture_radius_km);
  _finder.setMinValidFractionForTexture
    (_params.min_valid_fraction_for_texture);
  _finder.setMinValidFractionForFit
    (_params.min_valid_fraction_for_fit);
  _finder.setTextureLimitLow(_params.texture_limit_low);
  _finder.setTextureLimitHigh(_params.texture_limit_high);
  
}

// destructor

ResTestEcco::~ResTestEcco()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int ResTestEcco::Run()
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
      cerr << "ERROR - ResTestEcco::Run()" << endl;
      umsleep(1000);
      iret = -1;
      continue;
    }

    // set grid in ResTestEccoFinder object

    MdvxField *dbzField = _inMdvx.getField(_params.dbz_field_name);
    if (dbzField == NULL) {
      cerr << "ERROR - ResTestEcco::Run()" << endl;
      cerr << "  no dbz field found: " << _params.dbz_field_name << endl;
      return -1;
    }
    const Mdvx::field_header_t &fhdr = dbzField->getFieldHeader();
    const Mdvx::vlevel_header_t &vhdr = dbzField->getVlevelHeader();
    bool isLatLon = (fhdr.proj_type == Mdvx::PROJ_LATLON);
    vector<double> zLevels;
    for (int iz = 0; iz < fhdr.nz; iz++) {
      zLevels.push_back(vhdr.level[iz]);
    }
    
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
    
    const fl32 *dbz = (const fl32*) dbzField->getVol();
    fl32 missingDbz = fhdr.missing_data_value;
    if (_finder.computeEchoType(dbz, missingDbz)) {
      cerr << "ERROR - ResTestEcco::Run()" << endl;
      umsleep(1000);
      iret = -1;
      continue;
    }
    
    // write out
    
    if (_doWrite()) {
      cerr << "ERROR - ResTestEcco::Run()" << endl;
      umsleep(1000);
      iret = -1;
      continue;
    }
    
    // clear
    
    _inMdvx.clear();
    _outMdvx.clear();
    _finder.freeArrays();
    
  } // while
  
  return iret;

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
  
  PMU_auto_register("Before read");
  
  if (_input.readVolumeNext(_inMdvx)) {
    cerr << "ERROR - ResTestEcco::_doRead" << endl;
    cerr << "  Cannot read in data." << endl;
    cerr << _input.getErrStr() << endl;
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

  _outMdvx.clearFields();

  // initial fields are float32

  MdvxField *dbzField = _inMdvx.getField(_params.dbz_field_name);
  Mdvx::field_header_t fhdr2d = dbzField->getFieldHeader();
  fhdr2d.nz = 1;
  fhdr2d.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  size_t planeSize32 = fhdr2d.nx * fhdr2d.ny * sizeof(fl32);
  fhdr2d.volume_size = planeSize32;
  fhdr2d.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr2d.data_element_nbytes = 4;
  fhdr2d.missing_data_value = _missing;
  fhdr2d.bad_data_value = _missing;
  fhdr2d.scale = 1.0;
  fhdr2d.bias = 0.0;
  
  Mdvx::vlevel_header_t vhdr2d;
  MEM_zero(vhdr2d);
  vhdr2d.level[0] = 0;
  vhdr2d.type[0] = Mdvx::VERT_TYPE_SURFACE;

  // add 2D max texture field
  _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                               _finder.getTexture2D(),
                               Mdvx::ENCODING_INT16,
                               "DbzTextureComp",
                               "reflectivity_texture_composite",
                               "dBZ"));

  // convectivity max in 2D
  _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                               _finder.getConvectivity2D(),
                               Mdvx::ENCODING_INT16,
                               "ConvectivityComp",
                               "likelihood_of_convection_composite",
                               ""));
  // col max dbz
  _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                               _finder.getDbzColMax(),
                               Mdvx::ENCODING_INT16,
                               "DbzComp",
                               "dbz_composite",
                               "dBZ"));

  // load up fraction of texture kernel covered
  _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
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
  
  _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                               _finder.getEchoType2D(),
                               Mdvx::ENCODING_INT8,
                               "EchoTypeComp",
                               "convective_stratiform_echo_type_composite",
                               ""));

  // the following 3d fields are floats
  
  Mdvx::field_header_t fhdr3d = dbzField->getFieldHeader();
  Mdvx::vlevel_header_t vhdr3d = dbzField->getVlevelHeader();
  fhdr3d.missing_data_value = _missing;
  fhdr3d.bad_data_value = _missing;

  // texture in 3D
  _outMdvx.addField(_makeField(fhdr3d, vhdr3d,
                               _finder.getTexture3D(),
                               Mdvx::ENCODING_INT16,
                               "DbzTexture3D",
                               "reflectivity_texture_3D",
                               "dBZ"));
  // convectivity in 3D
  _outMdvx.addField(_makeField(fhdr3d, vhdr3d,
                               _finder.getConvectivity3D(),
                               Mdvx::ENCODING_INT16,
                               "Convectivity3D",
                               "likelihood_of_convection_3D",
                               ""));

  // echo the input DBZ field
  _outMdvx.addField(_makeField(fhdr3d, vhdr3d,
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
  _outMdvx.addField(_makeField(fhdr3d, vhdr3d,
                               _finder.getEchoType3D(),
                               Mdvx::ENCODING_INT8,
                               "EchoType3D",
                               "convective_stratiform_echo_type_3D",
                               ""));

}

/////////////////////////////////////////////////////////
// perform the write
//
// Returns 0 on success, -1 on failure.

int ResTestEcco::_doWrite()
  
{
  
  // create output DsMdvx object
  // copying master header from input object
  
  _outMdvx.clear();
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _outMdvx.setDebug(true);
  }
  _outMdvx.setWriteLdataInfo();
  Mdvx::master_header_t mhdr = _inMdvx.getMasterHeader();
  _outMdvx.setMasterHeader(mhdr);
  string info = _inMdvx.getMasterHeader().data_set_info;
  info += " : Stratfinder used to identify stratiform regions";
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
  if(_outMdvx.writeToDir(_params.output_dir)) {
    cerr << "ERROR - ResTestEcco::Run" << endl;
    cerr << "  Cannot write data set." << endl;
    cerr << _outMdvx.getErrStr() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Wrote file: " << _outMdvx.getPathInUse() << endl;
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
  size_t npts = fhdr.nx * fhdr.ny * fhdr.nz;
  size_t volSize = npts * sizeof(fl32);
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
  size_t npts = fhdr.nx * fhdr.ny * fhdr.nz;
  size_t volSize = npts * sizeof(ui08);
  newField->setVolData(data, volSize, Mdvx::ENCODING_INT8);
  newField->convertType(outputEncoding, Mdvx::COMPRESSION_GZIP);
  
  return newField;

}

