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

    // optionally read in temperature profile

    _finder.setLevelHtValues(_params.freezing_level_ht,
                             _params.divergence_level_ht);
    
    // optionally read in temperature profile from a model
    // and set the freezing and divergence level ht grids

    if (_params.vert_levels_type == Params::VERT_LEVELS_BY_TEMP) {
      if (_readTempProfile(_inMdvx.getValidTime(), dbzField)) {
        cerr << "WARNING - ConvStrat::Run()" << endl;
        cerr << "  Cannot read in temperature profile, url: "
             << _params.temp_profile_url << endl;
        cerr << "  Not using temp profile" << endl;
      }
      _finder.setLevelHtGrids((fl32 *) _fzHtField.getVol(),
                              (fl32 *) _divHtField.getVol(),
                              fhdr.nx * fhdr.ny);
                              
    }

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
    MdvxField::setFieldName("FractionActive", fractionFhdr);
    MdvxField::setFieldNameLong("Fraction of texture kernel active", fractionFhdr);
    MdvxField::setUnits("", fractionFhdr);
    MdvxField *fractionField =
      new MdvxField(fractionFhdr, vhdr2d, NULL, false, false, false);
    fractionField->setVolData(_finder.getFractionActive(),
                              planeSize32,
                              Mdvx::ENCODING_FLOAT32);
    fractionField->convertType(Mdvx::ENCODING_FLOAT32,
                               Mdvx::COMPRESSION_GZIP);
    _outMdvx.addField(fractionField);
    
    // add mean texture fields
    
    Mdvx::field_header_t meanTextFhdr = fhdr2d;
    MdvxField::setFieldName("DbzTextureMean", meanTextFhdr);
    MdvxField::setFieldNameLong("Mean texture of dbz", meanTextFhdr);
    MdvxField::setUnits("dBZ", meanTextFhdr);
    MdvxField *meanTextField =
      new MdvxField(meanTextFhdr, vhdr2d, NULL, false, false, false);
    meanTextField->setVolData(_finder.getMeanTexture(), 
                             planeSize32,
                             Mdvx::ENCODING_FLOAT32);
    meanTextField->convertType(Mdvx::ENCODING_FLOAT32,
                              Mdvx::COMPRESSION_GZIP);
    _outMdvx.addField(meanTextField);
    
    Mdvx::field_header_t meanTextLowFhdr = fhdr2d;
    MdvxField::setFieldName("DbzTextureLow", meanTextLowFhdr);
    MdvxField::setFieldNameLong("Mean texture low", meanTextLowFhdr);
    MdvxField::setUnits("dBZ", meanTextLowFhdr);
    MdvxField *meanTextLowField =
      new MdvxField(meanTextLowFhdr, vhdr2d, NULL, false, false, false);
    meanTextLowField->setVolData(_finder.getMeanTextureLow(), 
                                 planeSize32,
                                 Mdvx::ENCODING_FLOAT32);
    meanTextLowField->convertType(Mdvx::ENCODING_FLOAT32,
                                  Mdvx::COMPRESSION_GZIP);
    _outMdvx.addField(meanTextLowField);
    
    Mdvx::field_header_t meanTextMidFhdr = fhdr2d;
    MdvxField::setFieldName("DbzTextureMid", meanTextMidFhdr);
    MdvxField::setFieldNameLong("Mean texture mid", meanTextMidFhdr);
    MdvxField::setUnits("dBZ", meanTextMidFhdr);
    MdvxField *meanTextMidField =
      new MdvxField(meanTextMidFhdr, vhdr2d, NULL, false, false, false);
    meanTextMidField->setVolData(_finder.getMeanTextureMid(), 
                                 planeSize32,
                                 Mdvx::ENCODING_FLOAT32);
    meanTextMidField->convertType(Mdvx::ENCODING_FLOAT32,
                                  Mdvx::COMPRESSION_GZIP);
    _outMdvx.addField(meanTextMidField);
    
    Mdvx::field_header_t meanTextHighFhdr = fhdr2d;
    MdvxField::setFieldName("DbzTextureHigh", meanTextHighFhdr);
    MdvxField::setFieldNameLong("Mean texture high", meanTextHighFhdr);
    MdvxField::setUnits("dBZ", meanTextHighFhdr);
    MdvxField *meanTextHighField =
      new MdvxField(meanTextHighFhdr, vhdr2d, NULL, false, false, false);
    meanTextHighField->setVolData(_finder.getMeanTextureHigh(), 
                                  planeSize32,
                                  Mdvx::ENCODING_FLOAT32);
    meanTextHighField->convertType(Mdvx::ENCODING_FLOAT32,
                                   Mdvx::COMPRESSION_GZIP);
    _outMdvx.addField(meanTextHighField);
    
    // add max texture field
    
    Mdvx::field_header_t maxTextFhdr = fhdr2d;
    MdvxField *maxTextField =
      new MdvxField(maxTextFhdr, vhdr2d, NULL, false, false, false);
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
    MdvxField *maxField =
      new MdvxField(maxFhdr, vhdr2d, NULL, false, false, false);
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
    MdvxField *backgrField =
      new MdvxField(backgrFhdr, vhdr2d, NULL, false, false, false);
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
    MdvxField *convRadField =
      new MdvxField(convRadFhdr, vhdr2d, NULL, false, false, false);
    convRadField->setVolData(_finder.getConvRadiusKm(),
                             planeSize32,
                             Mdvx::ENCODING_FLOAT32);
    convRadField->convertType(Mdvx::ENCODING_FLOAT32,
                              Mdvx::COMPRESSION_GZIP);
    convRadField->setFieldName("ConvRadius");
    convRadField->setFieldNameLong("ConvectiveRadius");
    convRadField->setUnits("km");
    _outMdvx.addField(convRadField);

    // convective base, top and depth
    
    Mdvx::field_header_t convBaseFhdr = fhdr2d;
    MdvxField *convBaseField =
      new MdvxField(convBaseFhdr, vhdr2d, NULL, false, false, false);
    convBaseField->setVolData(_finder.getConvBaseKm(),
                              planeSize32,
                              Mdvx::ENCODING_FLOAT32);
    convBaseField->convertType(Mdvx::ENCODING_FLOAT32,
                               Mdvx::COMPRESSION_GZIP);
    convBaseField->setFieldName("ConvBase");
    convBaseField->setFieldNameLong("ConvectiveBase");
    convBaseField->setUnits("km");
    _outMdvx.addField(convBaseField);

    Mdvx::field_header_t convTopFhdr = fhdr2d;
    MdvxField *convTopField =
      new MdvxField(convTopFhdr, vhdr2d, NULL, false, false, false);
    convTopField->setVolData(_finder.getConvTopKm(),
                              planeSize32,
                              Mdvx::ENCODING_FLOAT32);
    convTopField->convertType(Mdvx::ENCODING_FLOAT32,
                               Mdvx::COMPRESSION_GZIP);
    convTopField->setFieldName("ConvTop");
    convTopField->setFieldNameLong("ConvectiveTop");
    convTopField->setUnits("km");
    _outMdvx.addField(convTopField);
    
    Mdvx::field_header_t convDepthFhdr = fhdr2d;
    MdvxField *convDepthField =
      new MdvxField(convDepthFhdr, vhdr2d, NULL, false, false, false);
    convDepthField->setVolData(_finder.getConvDepthKm(),
                               planeSize32,
                               Mdvx::ENCODING_FLOAT32);
    convDepthField->convertType(Mdvx::ENCODING_FLOAT32,
                                Mdvx::COMPRESSION_GZIP);
    convDepthField->setFieldName("ConvDepth");
    convDepthField->setFieldNameLong("ConvectiveDepth");
    convDepthField->setUnits("km");
    _outMdvx.addField(convDepthField);
    
    // stratiform base, top and depth
    
    Mdvx::field_header_t stratBaseFhdr = fhdr2d;
    MdvxField *stratBaseField =
      new MdvxField(stratBaseFhdr, vhdr2d, NULL, false, false, false);
    stratBaseField->setVolData(_finder.getStratBaseKm(),
                               planeSize32,
                               Mdvx::ENCODING_FLOAT32);
    stratBaseField->convertType(Mdvx::ENCODING_FLOAT32,
                                Mdvx::COMPRESSION_GZIP);
    stratBaseField->setFieldName("StratBase");
    stratBaseField->setFieldNameLong("StratiformBase");
    stratBaseField->setUnits("km");
    _outMdvx.addField(stratBaseField);
    
    Mdvx::field_header_t stratTopFhdr = fhdr2d;
    MdvxField *stratTopField =
      new MdvxField(stratTopFhdr, vhdr2d, NULL, false, false, false);
    stratTopField->setVolData(_finder.getStratTopKm(),
                              planeSize32,
                              Mdvx::ENCODING_FLOAT32);
    stratTopField->convertType(Mdvx::ENCODING_FLOAT32,
                               Mdvx::COMPRESSION_GZIP);
    stratTopField->setFieldName("StratTop");
    stratTopField->setFieldNameLong("StratiformTop");
    stratTopField->setUnits("km");
    _outMdvx.addField(stratTopField);
    
    Mdvx::field_header_t stratDepthFhdr = fhdr2d;
    MdvxField *stratDepthField =
      new MdvxField(stratDepthFhdr, vhdr2d, NULL, false, false, false);
    stratDepthField->setVolData(_finder.getStratDepthKm(),
                                planeSize32,
                                Mdvx::ENCODING_FLOAT32);
    stratDepthField->convertType(Mdvx::ENCODING_FLOAT32,
                                 Mdvx::COMPRESSION_GZIP);
    stratDepthField->setFieldName("StratDepth");
    stratDepthField->setFieldNameLong("StratiformDepth");
    stratDepthField->setUnits("km");
    _outMdvx.addField(stratDepthField);
    
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
    MdvxField *partitionField =
      new MdvxField(partitionFhdr, vhdr2d, NULL, false, false, false);
    partitionField->setVolData(_finder.getPartition(),
                               volSize08,
                               Mdvx::ENCODING_INT8);
    partitionField->convertType(Mdvx::ENCODING_INT8,
                                Mdvx::COMPRESSION_GZIP);
    partitionField->setFieldName(_params.partition_field_name);
    partitionField->setFieldNameLong("1 = stratiform, 2 = convective");
    partitionField->setUnits("");
    _outMdvx.addField(partitionField);
    
    Mdvx::field_header_t partLowFhdr = fhdr2d;
    MdvxField *partLowField =
      new MdvxField(partLowFhdr, vhdr2d, NULL, false, false, false);
    partLowField->setVolData(_finder.getPartitionLow(),
                             volSize08,
                             Mdvx::ENCODING_INT8);
    partLowField->convertType(Mdvx::ENCODING_INT8,
                              Mdvx::COMPRESSION_GZIP);
    partLowField->setFieldName("PartitionLow");
    partLowField->setFieldNameLong("1 = stratiform, 2 = convective");
    partLowField->setUnits("");
    _outMdvx.addField(partLowField);
    
    Mdvx::field_header_t partMidFhdr = fhdr2d;
    MdvxField *partMidField =
      new MdvxField(partMidFhdr, vhdr2d, NULL, false, false, false);
    partMidField->setVolData(_finder.getPartitionMid(),
                             volSize08,
                             Mdvx::ENCODING_INT8);
    partMidField->convertType(Mdvx::ENCODING_INT8,
                              Mdvx::COMPRESSION_GZIP);
    partMidField->setFieldName("PartitionMid");
    partMidField->setFieldNameLong("1 = stratiform, 2 = convective");
    partMidField->setUnits("");
    _outMdvx.addField(partMidField);
    
    Mdvx::field_header_t partHighFhdr = fhdr2d;
    MdvxField *partHighField =
      new MdvxField(partHighFhdr, vhdr2d, NULL, false, false, false);
    partHighField->setVolData(_finder.getPartitionHigh(),
                             volSize08,
                             Mdvx::ENCODING_INT8);
    partHighField->convertType(Mdvx::ENCODING_INT8,
                              Mdvx::COMPRESSION_GZIP);
    partHighField->setFieldName("PartitionHigh");
    partHighField->setFieldNameLong("1 = stratiform, 2 = convective");
    partHighField->setUnits("");
    _outMdvx.addField(partHighField);
    
  }
  
  // the following 3d fields are floats
  
  Mdvx::field_header_t fhdr3d = dbzField->getFieldHeader();
  Mdvx::vlevel_header_t vhdr3d = dbzField->getVlevelHeader();
  fhdr3d.missing_data_value = _missing;
  fhdr3d.bad_data_value = _missing;

  if (_params.write_convective_dbz) {

    MdvxField *convDbzField =
      new MdvxField(fhdr3d, vhdr3d, NULL, false, false, false);
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

    MdvxField *stratDbzField =
      new MdvxField(fhdr3d, vhdr3d, NULL, false, false, false);
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
    
    MdvxField *volTextureField =
      new MdvxField(fhdr3d, vhdr3d, NULL, false, false, false);
    volTextureField->setVolData(_finder.getVolTexture(),
                                fhdr3d.volume_size,
                                Mdvx::ENCODING_FLOAT32);
    volTextureField->convertType(Mdvx::ENCODING_INT16,
                                 Mdvx::COMPRESSION_GZIP);
    volTextureField->setFieldName("DbzTexture3D");
    volTextureField->setFieldNameLong("ReflectivityTexture3D");
    volTextureField->setUnits("dBZ");
    _outMdvx.addField(volTextureField);

    // echo the input field
    
    MdvxField *volDbzField =
      new MdvxField(fhdr3d, vhdr3d, NULL, false, false, false);
    volDbzField->setVolData(_finder.getVolDbz(),
                            fhdr3d.volume_size,
                            Mdvx::ENCODING_FLOAT32);
    volDbzField->convertType(Mdvx::ENCODING_INT16,
                             Mdvx::COMPRESSION_GZIP);
    volDbzField->setFieldName("Dbz3D");
    volDbzField->setFieldNameLong("Reflectivity3D");
    volDbzField->setUnits("dBZ");
    _outMdvx.addField(volDbzField);

    // freezing level, divergence level ht, temp field

    _outMdvx.addField(new MdvxField(_fzHtField));
    _outMdvx.addField(new MdvxField(_divHtField));
    _outMdvx.addField(new MdvxField(*_tempField));

  }
  
}

    Mdvx::field_header_t fractionFhdr = fhdr2d;
    MdvxField::setFieldName("FractionActive", fractionFhdr);
    MdvxField::setFieldNameLong("Fraction of texture kernel active", fractionFhdr);
    MdvxField::setUnits("", fractionFhdr);
    MdvxField *fractionField =
      new MdvxField(fractionFhdr, vhdr2d, NULL, false, false, false);
    fractionField->setVolData(_finder.getFractionActive(),
                              planeSize32,
                              Mdvx::ENCODING_FLOAT32);
    fractionField->convertType(Mdvx::ENCODING_FLOAT32,
                               Mdvx::COMPRESSION_GZIP);
/////////////////////////////////////////////////////////
// read in temp profile
//
// Returns 0 on success, -1 on failure.

int ConvStrat::_readTempProfile(time_t dbzTime,
                                const MdvxField *dbzField)

{

  _tempMdvx.clearRead();
  _tempMdvx.setReadTime(Mdvx::READ_CLOSEST,
                       _params.temp_profile_url,
                       _params.temp_profile_search_margin,
                       dbzTime);
  _tempMdvx.addReadField(_params.temp_profile_field_name);

  if (_tempMdvx.readVolume()) {
    cerr << "ERROR - ConvStrat::_readTempProfile" << endl;
    cerr << "  Cannot read temperature profile" << endl;
    cerr << _tempMdvx.getErrStr() << endl;
    return -1;
  }

  _tempField = _tempMdvx.getField(_params.temp_profile_field_name);
  if (_tempField == NULL) {
    cerr << "ERROR - ConvStrat::_readTempProfile" << endl;
    cerr << "  Cannot find field in temp file: "
         << _params.temp_profile_field_name << endl;
    cerr << "  URL: " << _params.temp_profile_url << endl;
    cerr << "  Time: " << DateTime::strm(_tempMdvx.getValidTime()) << endl;
    return -1;
  }
  
  // fill the temperature level arrays
  
  _computeHts(_params.freezing_level_temp, _fzHtField,
              "FzHt", "height_of_freezing_level", "km");
  _computeHts(_params.divergence_level_temp, _divHtField,
              "DivHt", "height_of_divergence_level", "km");

  // remap from model to radar grid coords
  // use of the lookup table makes this more efficient
  
  MdvxProj proj(dbzField->getFieldHeader());
  MdvxRemapLut lut;
  if (_fzHtField.remap(lut, proj)) {
    cerr << "ERROR - ConvStrat::_readTempProfile" << endl;
    cerr << "  Cannot convert model temp grid to radar grid" << endl;
    cerr << "  URL: " << _params.temp_profile_url << endl;
    cerr << "  Time: " << DateTime::strm(_tempMdvx.getValidTime()) << endl;
    return -1;
  }
  if (_divHtField.remap(lut, proj)) {
    cerr << "ERROR - ConvStrat::_readTempProfile" << endl;
    cerr << "  Cannot convert model temp grid to radar grid" << endl;
    cerr << "  URL: " << _params.temp_profile_url << endl;
    cerr << "  Time: " << DateTime::strm(_tempMdvx.getValidTime()) << endl;
    return -1;
  }
                       
  return 0;

}

/////////////////////////////////////////////////////////
// fill temperature level ht array

void ConvStrat::_computeHts(double tempC,
                            MdvxField &htField,
                            const string &fieldName,
                            const string &longName,
                            const string &units)

{

  // set up the height field

  htField.clearVolData();

  Mdvx::field_header_t fhdr = _tempField->getFieldHeader();
  fhdr.nz = 1;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  size_t planeSize32 = fhdr.nx * fhdr.ny * sizeof(fl32);
  fhdr.volume_size = planeSize32;
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = 4;
  fhdr.missing_data_value = _missing;
  fhdr.bad_data_value = _missing;
  
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  vhdr.level[0] = 0;
  vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  
  htField.setHdrsAndVolData(fhdr, vhdr, NULL,
                            true, false, true);

  htField.setFieldName(fieldName);
  htField.setFieldNameLong(longName);
  htField.setUnits(units);
  
  // get hts array pointer
  
  fl32 *hts = (fl32 *) htField.getVol();

  // get temp array

  fl32 *temp = (fl32 *) _tempField->getVol();
  const Mdvx::field_header_t tempFhdr = _tempField->getFieldHeader();
  
  // get Z profile for temperatures
  
  const Mdvx::vlevel_header_t tempVhdr = _tempField->getVlevelHeader();
  vector<double> zProfile;
  for (si64 iz = 0; iz < tempFhdr.nz; iz++) {
    zProfile.push_back(tempVhdr.level[iz]);
  } // iz
  
  // loop through the (x, y) plane
  
  si64 planeIndex = 0;
  size_t nxy = fhdr.nx * fhdr.ny;
  for (si64 iy = 0; iy < tempFhdr.ny; iy++) {
    for (si64 ix = 0; ix < tempFhdr.nx; ix++, planeIndex++) {

      // interpolate to find the height for tempC

      double interpHt = tempVhdr.level[0]; // if temp level is below grid
      si64 index = planeIndex;
      double tempBelow = temp[index];
      index += nxy;
      for (si64 iz = 1; iz < tempFhdr.nz; iz++, index += nxy) {
        double tempAbove = temp[index];
        if ((tempC <= tempBelow && tempC >= tempAbove) ||
            (tempC >= tempBelow && tempC <= tempAbove)) {
          double deltaTemp = tempAbove - tempBelow;
          double deltaHt = zProfile[iz] - zProfile[iz-1];
          interpHt = zProfile[iz] + ((tempC - tempBelow) / deltaTemp) * deltaHt;
          hts[planeIndex] = interpHt;
          break;
        }
        tempBelow = tempAbove;
        // check if temp level is above grid
        if (iz == tempFhdr.nz - 1) {
          hts[planeIndex] = tempVhdr.level[tempFhdr.nz-1];
        }
      } // iz

    } // ix
  } // iy
  
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
  
