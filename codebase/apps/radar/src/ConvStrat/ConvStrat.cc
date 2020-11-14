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
// ConvStrat.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// MAY 2014
//
////////////////////////////////////////////////////////////////////
//
// ConvStrat finds stratiform/convective regions in a Cartesian
// radar volume
//
/////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <dataport/bigend.h>
#include <toolsa/pmu.h>
#include <toolsa/toolsa_macros.h>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxChunk.hh>
#include "ConvStrat.hh"
using namespace std;

const fl32 ConvStrat::_missing = -9999.0;

// Constructor

ConvStrat::ConvStrat(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "ConvStrat";
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

  // set up ConvStratFinder object

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _finder.setVerbose(true);
  } else if (_params.debug) {
    _finder.setDebug(true);
  }
  _finder.setMinValidHtKm(_params.min_valid_height);
  _finder.setMaxValidHtKm(_params.max_valid_height);
  _finder.setMinValidDbz(_params.min_valid_dbz);
  _finder.setDbzForDefiniteConvection
    (_params.dbz_threshold_for_definite_convection);
  _finder.setComputeConvRadius(_params.conv_radius_function.min_dbz,
                               _params.conv_radius_function.max_dbz,
                               _params.conv_radius_function.min_radius_km,
                               _params.conv_radius_function.max_radius_km,
                               _params.background_dbz_radius_km);
  _finder.setTextureRadiusKm(_params.texture_radius_km);
  _finder.setMinValidFractionForTexture
    (_params.min_valid_fraction_for_texture);
  _finder.setMinTextureForConvection
    (_params.min_texture_for_convection);

  return;

}

// destructor

ConvStrat::~ConvStrat()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int ConvStrat::Run()
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
      cerr << "ERROR - ConvStrat::Run()" << endl;
      umsleep(1000);
      iret = -1;
      continue;
    }

    // set grid in ConvStratFinder object

    MdvxField *dbzField = _inMdvx.getField(_params.dbz_field_name);
    if (dbzField == NULL) {
      cerr << "ERROR - ConvStrat::Run()" << endl;
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

    // compute the convective/stratiform partition

    const fl32 *dbz = (const fl32*) dbzField->getVol();
    fl32 missingDbz = fhdr.missing_data_value;
    if (_finder.computePartition(dbz, missingDbz)) {
      cerr << "ERROR - ConvStrat::Run()" << endl;
      umsleep(1000);
      iret = -1;
      continue;
    }
    
    // write out
    
    if (_doWrite()) {
      cerr << "ERROR - ConvStrat::Run()" << endl;
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

int ConvStrat::_doRead()
  
{
  
  _inMdvx.clear();
  if (_params.debug) {
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
    cerr << "ERROR - ConvStrat::_doRead" << endl;
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

void ConvStrat::_addFields()
  
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

  if (_params.write_debug_fields) {

    // load up fraction of texture kernel covered
    
    Mdvx::field_header_t fractionFhdr = fhdr2d;
    MdvxField *fractionField = new MdvxField(fractionFhdr, vhdr2d);
    fractionField->setVolData(_finder.getFractionActive(),
                              planeSize32,
                              Mdvx::ENCODING_FLOAT32);
    fractionField->convertType(Mdvx::ENCODING_FLOAT32,
                               Mdvx::COMPRESSION_GZIP);
    fractionField->setFieldName("FractionActive");
    fractionField->setFieldNameLong("Fraction of texture kernel active");
    fractionField->setUnits("");
    _outMdvx.addField(fractionField);
    
    // add mean texture field
    
    Mdvx::field_header_t meanTextFhdr = fhdr2d;
    MdvxField *meanTextField = new MdvxField(meanTextFhdr, vhdr2d);
    meanTextField->setVolData(_finder.getMeanTexture(), 
                             planeSize32,
                             Mdvx::ENCODING_FLOAT32);
    meanTextField->convertType(Mdvx::ENCODING_FLOAT32,
                              Mdvx::COMPRESSION_GZIP);
    meanTextField->setFieldName("DbzTextureMean");
    meanTextField->setFieldNameLong("Mean texture of dbz");
    meanTextField->setUnits("dBZ");
    _outMdvx.addField(meanTextField);
    
    // add max texture field
    
    Mdvx::field_header_t maxTextFhdr = fhdr2d;
    MdvxField *maxTextField = new MdvxField(maxTextFhdr, vhdr2d);
    maxTextField->setVolData(_finder.getMaxTexture(), 
                             planeSize32,
                             Mdvx::ENCODING_FLOAT32);
    maxTextField->convertType(Mdvx::ENCODING_FLOAT32,
                              Mdvx::COMPRESSION_GZIP);
    maxTextField->setFieldName("DbzTextureMax");
    maxTextField->setFieldNameLong("Max texture of dbz");
    maxTextField->setUnits("dBZ");
    _outMdvx.addField(maxTextField);
    
    // max dbz
    
    Mdvx::field_header_t maxFhdr = fhdr2d;
    MdvxField *maxField = new MdvxField(maxFhdr, vhdr2d);
    maxField->setVolData(_finder.getColMaxDbz(),
                         planeSize32,
                         Mdvx::ENCODING_FLOAT32);
    maxField->convertType(Mdvx::ENCODING_FLOAT32,
                          Mdvx::COMPRESSION_GZIP);
    maxField->setFieldName("DbzColMax");
    maxField->setFieldNameLong("Column max dbz");
    maxField->setUnits("dBZ");
    _outMdvx.addField(maxField);

    // background dbz
    
    Mdvx::field_header_t backgrFhdr = fhdr2d;
    MdvxField *backgrField = new MdvxField(backgrFhdr, vhdr2d);
    backgrField->setVolData(_finder.getBackgroundDbz(),
                            planeSize32,
                            Mdvx::ENCODING_FLOAT32);
    backgrField->convertType(Mdvx::ENCODING_FLOAT32,
                             Mdvx::COMPRESSION_GZIP);
    backgrField->setFieldName("DbzBackground");
    backgrField->setFieldNameLong("Background dbz");
    backgrField->setUnits("dBZ");
    _outMdvx.addField(backgrField);

    // convective radius in km
    
    Mdvx::field_header_t convRadFhdr = fhdr2d;
    MdvxField *convRadField = new MdvxField(convRadFhdr, vhdr2d);
    convRadField->setVolData(_finder.getConvRadiusKm(),
                             planeSize32,
                             Mdvx::ENCODING_FLOAT32);
    convRadField->convertType(Mdvx::ENCODING_FLOAT32,
                              Mdvx::COMPRESSION_GZIP);
    convRadField->setFieldName("ConvRadius");
    convRadField->setFieldNameLong("ConvectiveRadius");
    convRadField->setUnits("km");
    _outMdvx.addField(convRadField);

  }
  
  // the following fields are unsigned bytes
  
  int volSize08 = fhdr2d.nx * fhdr2d.ny * sizeof(ui08);
  fhdr2d.volume_size = volSize08;
  fhdr2d.encoding_type = Mdvx::ENCODING_INT8;
  fhdr2d.data_element_nbytes = 1;
  fhdr2d.missing_data_value = ConvStratFinder::CATEGORY_MISSING;
  fhdr2d.bad_data_value = ConvStratFinder::CATEGORY_MISSING;
  
  // partition field
  
  if (_params.write_partition_field) {

    Mdvx::field_header_t partitionFhdr = fhdr2d;
    MdvxField *partitionField = new MdvxField(partitionFhdr, vhdr2d);
    partitionField->setVolData(_finder.getPartition(),
                               volSize08,
                               Mdvx::ENCODING_INT8);
    partitionField->convertType(Mdvx::ENCODING_INT8,
                                Mdvx::COMPRESSION_GZIP);
    partitionField->setFieldName(_params.partition_field_name);
    partitionField->setFieldNameLong("1 = stratiform, 2 = convective");
    partitionField->setUnits("");
    _outMdvx.addField(partitionField);
    
  }
  
  // the following 3d fields are floats
  
  Mdvx::field_header_t fhdr3d = dbzField->getFieldHeader();
  Mdvx::vlevel_header_t vhdr3d = dbzField->getVlevelHeader();
  fhdr3d.missing_data_value = _missing;
  fhdr3d.bad_data_value = _missing;

  if (_params.write_convective_dbz) {

    MdvxField *convDbzField = new MdvxField(fhdr3d, vhdr3d);
    convDbzField->setVolData(_finder.getConvectiveDbz(),
                             fhdr3d.volume_size,
                             Mdvx::ENCODING_FLOAT32);
    if (_params.convert_convective_dbz_to_column_max) {
      convDbzField->convert2Composite();
    }
    convDbzField->convertType(Mdvx::ENCODING_INT16,
                              Mdvx::COMPRESSION_GZIP);
    convDbzField->setFieldName(_params.convective_dbz_field_name);
    convDbzField->setFieldNameLong("Convective reflectivity");
    convDbzField->setUnits("dBZ");
    _outMdvx.addField(convDbzField);

  }
  
  if (_params.write_stratiform_dbz) {

    MdvxField *stratDbzField = new MdvxField(fhdr3d, vhdr3d);
    stratDbzField->setVolData(_finder.getStratiformDbz(),
                              fhdr3d.volume_size,
                              Mdvx::ENCODING_FLOAT32);
    if (_params.convert_stratiform_dbz_to_column_max) {
      stratDbzField->convert2Composite();
    }
    stratDbzField->convertType(Mdvx::ENCODING_FLOAT32,
                               Mdvx::COMPRESSION_GZIP);
    stratDbzField->setFieldName(_params.stratiform_dbz_field_name);
    stratDbzField->setFieldNameLong("Stratiform reflectivity");
    stratDbzField->setUnits("dBZ");
    _outMdvx.addField(stratDbzField);

  }
  
  if (_params.write_debug_fields) {
    
    // texture for full volume
    
    MdvxField *volTextureField = new MdvxField(fhdr3d, vhdr3d);
    volTextureField->setVolData(_finder.getVolTexture(),
                                fhdr3d.volume_size,
                                Mdvx::ENCODING_FLOAT32);
    volTextureField->convertType(Mdvx::ENCODING_INT16,
                                 Mdvx::COMPRESSION_GZIP);
    volTextureField->setFieldName("DbzTexture3D");
    volTextureField->setFieldNameLong("ReflectivityTexture3D");
    volTextureField->setUnits("dBZ");
    _outMdvx.addField(volTextureField);

  }
  
}

/////////////////////////////////////////////////////////
// perform the write
//
// Returns 0 on success, -1 on failure.

int ConvStrat::_doWrite()
  
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
  if(_outMdvx.writeToDir(_params.output_url)) {
    cerr << "ERROR - ConvStrat::Run" << endl;
    cerr << "  Cannot write data set." << endl;
    cerr << _outMdvx.getErrStr() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Wrote file: " << _outMdvx.getPathInUse() << endl;
  }

  return 0;

}
  
