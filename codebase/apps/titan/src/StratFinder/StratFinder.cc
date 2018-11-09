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
// StratFinder.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// MAY 2014
//
////////////////////////////////////////////////////////////////////
//
// StratFinder finds stratiform/convective regions in a Cartesian
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
#include "StratFinder.hh"
using namespace std;

const fl32 StratFinder::_missing = -9999.0;

// Constructor

StratFinder::StratFinder(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "StratFinder";
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

  // set up ConvStrat object

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _convStrat.setVerbose(true);
  } else if (_params.debug) {
    _convStrat.setDebug(true);
  }
  _convStrat.setMinValidHtKm(_params.min_valid_height);
  _convStrat.setMaxValidHtKm(_params.max_valid_height);
  _convStrat.setMinValidDbz(_params.min_valid_dbz);
  _convStrat.setDbzForDefiniteConvection
    (_params.dbz_threshold_for_definite_convection);
  _convStrat.setConvectiveRadiusKm(_params.convective_radius_km);
  _convStrat.setTextureRadiusKm(_params.texture_radius_km);
  _convStrat.setMinValidFractionForTexture
    (_params.min_valid_fraction_for_texture);
  _convStrat.setMinTextureForConvection
    (_params.min_texture_for_convection);

  return;

}

// destructor

StratFinder::~StratFinder()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int StratFinder::Run()
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
      cerr << "ERROR - StratFinder::Run()" << endl;
      umsleep(1000);
      iret = -1;
      continue;
    }

    // set grid in ConvStrat object

    MdvxField *dbzField = _inMdvx.getField(_params.dbz_field_name);
    const Mdvx::field_header_t &fhdr = dbzField->getFieldHeader();
    const Mdvx::vlevel_header_t &vhdr = dbzField->getVlevelHeader();
    bool isLatLon = (fhdr.proj_type == Mdvx::PROJ_LATLON);
    vector<double> zLevels;
    for (int iz = 0; iz < fhdr.nz; iz++) {
      zLevels.push_back(vhdr.level[iz]);
    }
    
    _convStrat.setGrid(fhdr.nx,
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
    if (_convStrat.computePartition(dbz, missingDbz)) {
      cerr << "ERROR - StratFinder::Run()" << endl;
      umsleep(1000);
      iret = -1;
      continue;
    }
    
    // write out
    
    if (_doWrite()) {
      cerr << "ERROR - StratFinder::Run()" << endl;
      umsleep(1000);
      iret = -1;
      continue;
    }
    
    // clear
    
    _inMdvx.clear();
    _outMdvx.clear();
    _convStrat.freeArrays();
    
  } // while
  
  return iret;

}

/////////////////////////////////////////////////////////
// perform the read
//
// Returns 0 on success, -1 on failure.

int StratFinder::_doRead()
  
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
    cerr << "ERROR - StratFinder::_doRead" << endl;
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

void StratFinder::_addFields()
  
{

  _outMdvx.clearFields();

  // initial fields are float32

  MdvxField *dbzField = _inMdvx.getField(_params.dbz_field_name);
  Mdvx::field_header_t fhdr = dbzField->getFieldHeader();
  fhdr.nz = 1;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  int volSize32 = fhdr.nx * fhdr.ny * sizeof(fl32);
  fhdr.volume_size = volSize32;
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = 4;
  fhdr.missing_data_value = _missing;
  fhdr.bad_data_value = _missing;
  fhdr.scale = 1.0;
  fhdr.bias = 0.0;
  
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  vhdr.level[0] = 0;
  vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;

  if (_params.write_debug_fields) {

    // load up fraction of texture kernel covered
    
    Mdvx::field_header_t fractionFhdr = fhdr;
    MdvxField *fractionField = new MdvxField(fractionFhdr, vhdr);
    fractionField->setVolData(_convStrat.getFractionActive(),
                              volSize32,
                              Mdvx::ENCODING_FLOAT32);
    fractionField->convertType(Mdvx::ENCODING_FLOAT32,
                               Mdvx::COMPRESSION_GZIP);
    fractionField->setFieldName("FractionActive");
    fractionField->setFieldNameLong("Fraction of texture kernel active");
    fractionField->setUnits("");
    _outMdvx.addField(fractionField);
    
    // load up mean texture field, add to output object
    
    Mdvx::field_header_t textureFhdr = fhdr;
    MdvxField *textureField = new MdvxField(textureFhdr, vhdr);
    textureField->setVolData(_convStrat.getMeanTexture(), 
                             volSize32,
                             Mdvx::ENCODING_FLOAT32);
    textureField->convertType(Mdvx::ENCODING_FLOAT32,
                              Mdvx::COMPRESSION_GZIP);
    textureField->setFieldName("DbzTexture");
    textureField->setFieldNameLong("Mean texture of dbz");
    textureField->setUnits("dBZ");
    _outMdvx.addField(textureField);
    
    // max dbz
    
    Mdvx::field_header_t maxFhdr = fhdr;
    MdvxField *maxField = new MdvxField(maxFhdr, vhdr);
    maxField->setVolData(_convStrat.getColMaxDbz(),
                         volSize32,
                         Mdvx::ENCODING_FLOAT32);
    maxField->convertType(Mdvx::ENCODING_FLOAT32,
                          Mdvx::COMPRESSION_GZIP);
    maxField->setFieldName("ColMaxDbz");
    maxField->setFieldNameLong("Column max dbz");
    maxField->setUnits("dBZ");
    _outMdvx.addField(maxField);

  }
  
  // the following fields are unsigned bytes
  
  int volSize08 = fhdr.nx * fhdr.ny * sizeof(ui08);
  fhdr.volume_size = volSize08;
  fhdr.encoding_type = Mdvx::ENCODING_INT8;
  fhdr.data_element_nbytes = 1;
  fhdr.missing_data_value = ConvStrat::CATEGORY_MISSING;
  fhdr.bad_data_value = ConvStrat::CATEGORY_MISSING;
  
  // convective flag from max dbz
  
  if (_params.write_debug_fields) {

    Mdvx::field_header_t convFromMaxFhdr = fhdr;
    MdvxField *convFromMaxField = new MdvxField(convFromMaxFhdr, vhdr);
    convFromMaxField->setVolData(_convStrat.getConvFromColMax(),
                                 volSize08,
                                 Mdvx::ENCODING_INT8);
    convFromMaxField->convertType(Mdvx::ENCODING_INT8,
                                  Mdvx::COMPRESSION_GZIP);
    convFromMaxField->setFieldName("ConvFromColMax");
    convFromMaxField->setFieldNameLong("Flag for convection from column max dbz");
    convFromMaxField->setUnits("");
    _outMdvx.addField(convFromMaxField);
    
    // convective flag from texture
    
    Mdvx::field_header_t convFromTextureFhdr = fhdr;
    MdvxField *convFromTextureField = new MdvxField(convFromTextureFhdr, vhdr);
    convFromTextureField->setVolData(_convStrat.getConvFromTexture(),
                                     volSize08,
                                     Mdvx::ENCODING_INT8);
    convFromTextureField->convertType(Mdvx::ENCODING_INT8,
                                      Mdvx::COMPRESSION_GZIP);
    convFromTextureField->setFieldName("ConvFromTexture");
    convFromTextureField->setFieldNameLong
      ("Flag for convection from texture field");
    convFromTextureField->setUnits("");
    _outMdvx.addField(convFromTextureField);

  }
  
  // partition field
  
  if (_params.write_partition_field) {

    Mdvx::field_header_t partitionFhdr = fhdr;
    MdvxField *partitionField = new MdvxField(partitionFhdr, vhdr);
    partitionField->setVolData(_convStrat.getPartition(),
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
    convDbzField->setVolData(_convStrat.getConvectiveDbz(),
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
    stratDbzField->setVolData(_convStrat.getStratiformDbz(),
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
  
  
}

/////////////////////////////////////////////////////////
// perform the write
//
// Returns 0 on success, -1 on failure.

int StratFinder::_doWrite()
  
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
  
  for (int i = 0; i < _inMdvx.getNChunks(); i++) {
    MdvxChunk *chunk = new MdvxChunk(*_inMdvx.getChunkByNum(i));
    _outMdvx.addChunk(chunk);
  }
  
  // add fields
  
  _addFields();
  
  // write out
  
  PMU_auto_register("Before write");
  if(_outMdvx.writeToDir(_params.output_url)) {
    cerr << "ERROR - StratFinder::Run" << endl;
    cerr << "  Cannot write data set." << endl;
    cerr << _outMdvx.getErrStr() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Wrote file: " << _outMdvx.getPathInUse() << endl;
  }

  return 0;

}
  
