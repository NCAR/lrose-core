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
  _finder.setDbzForEchoTops(_params.dbz_for_echo_tops);
  _finder.setComputeConvRadius(_params.conv_radius_function.min_dbz,
                               _params.conv_radius_function.max_dbz,
                               _params.conv_radius_function.min_radius_km,
                               _params.conv_radius_function.max_radius_km,
                               _params.background_dbz_radius_km);
  _finder.setTextureRadiusKm(_params.texture_radius_km);
  _finder.setMinValidFractionForTexture
    (_params.min_valid_fraction_for_texture);
  _finder.setMinTextureForConvection(_params.min_texture_for_convection);
  _finder.setMaxTextureForStratiform(_params.max_texture_for_stratiform);
  
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
    
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getFractionActive(),
                                 Mdvx::ENCODING_FLOAT32,
                                 "FractionActive",
                                 "Fraction of texture kernel active",
                                 ""));
                      
    // add mean texture fields
    
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getMeanTexture(),
                                 Mdvx::ENCODING_FLOAT32,
                                 "DbzTextureMean",
                                 "Mean texture of dbz",
                                 "dBZ"));
    
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getMeanTextureLow(),
                                 Mdvx::ENCODING_FLOAT32,
                                 "DbzTextureLow",
                                 "Mean texture of dbz - low",
                                 "dBZ"));
    
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getMeanTextureMid(),
                                 Mdvx::ENCODING_FLOAT32,
                                 "DbzTextureMid",
                                 "Mean texture of dbz - mid",
                                 "dBZ"));
    
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getMeanTextureHigh(),
                                 Mdvx::ENCODING_FLOAT32,
                                 "DbzTextureHigh",
                                 "Mean texture of dbz - high",
                                 "dBZ"));
    
    // add max texture field
    
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getMaxTexture(),
                                 Mdvx::ENCODING_FLOAT32,
                                 "DbzTextureMax",
                                 "Max texture of dbz",
                                 "dBZ"));
    
    // max dbz
    
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getColMaxDbz(),
                                 Mdvx::ENCODING_FLOAT32,
                                 "DbzColMax",
                                 "Max dbz in column",
                                 "dBZ"));
    
    // background dbz
    
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getBackgroundDbz(),
                                 Mdvx::ENCODING_FLOAT32,
                                 "DbzBackground",
                                 "Background dbz",
                                 "dBZ"));
    
    // convective radius in km
    
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getConvRadiusKm(),
                                 Mdvx::ENCODING_FLOAT32,
                                 "ConvRadius",
                                 "ConvectiveRadius",
                                 "km"));
    
    // convective base, top
    
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getConvBaseKm(),
                                 Mdvx::ENCODING_FLOAT32,
                                 "ConvBase",
                                 "ConvectiveBase",
                                 "km"));
    
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getConvTopKm(),
                                 Mdvx::ENCODING_FLOAT32,
                                 "ConvTop",
                                 "ConvectiveTop",
                                 "km"));
    
    // stratiform base, top
    
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getStratBaseKm(),
                                 Mdvx::ENCODING_FLOAT32,
                                 "StratBase",
                                 "StratiformBase",
                                 "km"));
    
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getStratTopKm(),
                                 Mdvx::ENCODING_FLOAT32,
                                 "StratTop",
                                 "StratiformTop",
                                 "km"));
    
    // echo top
    
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getEchoTopKm(),
                                 Mdvx::ENCODING_FLOAT32,
                                 "EchoTop",
                                 "EchoTop",
                                 "km"));
    
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
    
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getPartition(),
                                 Mdvx::ENCODING_INT8,
                                 _params.partition_field_name,
                                 "1 = stratiform, 2 = convective",
                                 ""));
    
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getPartitionLow(),
                                 Mdvx::ENCODING_INT8,
                                 "PartitionLow",
                                 "1 = stratiform, 2 = convective",
                                 ""));
    
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getPartitionMid(),
                                 Mdvx::ENCODING_INT8,
                                 "PartitionMid",
                                 "1 = stratiform, 2 = convective",
                                 ""));
    
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getPartitionHigh(),
                                 Mdvx::ENCODING_INT8,
                                 "PartitionHigh",
                                 "1 = stratiform, 2 = convective",
                                 ""));
    
  }
  
  // the following 3d fields are floats
  
  Mdvx::field_header_t fhdr3d = dbzField->getFieldHeader();
  Mdvx::vlevel_header_t vhdr3d = dbzField->getVlevelHeader();
  fhdr3d.missing_data_value = _missing;
  fhdr3d.bad_data_value = _missing;

  if (_params.write_convective_dbz) {
    MdvxField *convDbz = _makeField(fhdr3d, vhdr3d,
                                    _finder.getConvectiveDbz(),
                                    Mdvx::ENCODING_INT16,
                                    _params.convective_dbz_field_name,
                                    "Convective reflectivity",
                                    "dBZ");
    if (_params.convert_convective_dbz_to_column_max) {
      convDbz->convert2Composite();
    }
    _outMdvx.addField(convDbz);
  }
  
  if (_params.write_stratiform_dbz) {
    MdvxField *stratDbz = _makeField(fhdr3d, vhdr3d,
                                     _finder.getStratiformDbz(),
                                     Mdvx::ENCODING_INT16,
                                     _params.stratiform_dbz_field_name,
                                     "Stratiform reflectivity",
                                     "dBZ");
    if (_params.convert_stratiform_dbz_to_column_max) {
      stratDbz->convert2Composite();
    }
    _outMdvx.addField(stratDbz);
  }
  
  if (_params.write_debug_fields) {
    
    // texture for full volume
    
    _outMdvx.addField(_makeField(fhdr3d, vhdr3d,
                                 _finder.getVolTexture(),
                                 Mdvx::ENCODING_INT16,
                                 "DbzTexture3D",
                                 "ReflectivityTexture3D",
                                 "dBZ"));
    // echo the input field
    
    _outMdvx.addField(_makeField(fhdr3d, vhdr3d,
                                 _finder.getVolDbz(),
                                 Mdvx::ENCODING_INT16,
                                 "Dbz3D",
                                 "Reflectivity3D",
                                 "dBZ"));

    // freezing level, divergence level ht, temp field

    _outMdvx.addField(new MdvxField(_fzHtField));
    _outMdvx.addField(new MdvxField(_divHtField));
    _outMdvx.addField(new MdvxField(*_tempField));

  }
  
}

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

/////////////////////////////////////////////////////////
// create a float field

MdvxField *ConvStrat::_makeField(Mdvx::field_header_t &fhdrTemplate,
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

MdvxField *ConvStrat::_makeField(Mdvx::field_header_t &fhdrTemplate,
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

