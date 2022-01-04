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
  _tempField = NULL;

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
  _finder.setMinConvectivityForConvective(_params.min_convectivity_for_convective);
  if (_params.clumping_use_dual_thresholds) {
    _finder.setUseDualThresholds(_params.clumping_secondary_convectivity,
                                 _params.all_subclumps_min_area_fraction,
                                 _params.each_subclump_min_area_fraction,
                                 _params.each_subclump_min_area_km2);
  }
  _finder.setMaxConvectivityForStratiform(_params.max_convectivity_for_stratiform);
  _finder.setMinGridOverlapForClumping(1);
  _finder.setTextureRadiusKm(_params.texture_radius_km);
  _finder.setMinValidFractionForTexture
    (_params.min_valid_fraction_for_texture);
  _finder.setMinValidFractionForFit
    (_params.min_valid_fraction_for_fit);
  _finder.setMinVolForConvectiveKm3(_params.min_valid_volume_for_convective);
  _finder.setTextureLimitLow(_params.texture_limit_low);
  _finder.setTextureLimitHigh(_params.texture_limit_high);
  _finder.setMinVertExtentForConvectiveKm(_params.min_vert_extent_for_convective);
  _finder.setDbzForEchoTops(_params.dbz_for_echo_tops);

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

    _finder.setConstantHtThresholds(_params.shallow_threshold_ht,
                                    _params.deep_threshold_ht);
    
    // optionally read in temperature profile from a model
    // and set the freezing and divergence level ht grids

    if (_params.vert_levels_type == Params::VERT_LEVELS_BY_TEMP) {
      if (_readTempProfile(_inMdvx.getValidTime(), dbzField) == 0) {
        _finder.setGridHtThresholds((fl32 *) _shallowHtField.getVol(),
                                    (fl32 *) _deepHtField.getVol(),
                                    fhdr.nx * fhdr.ny);
      } else {
        cerr << "WARNING - ConvStrat::Run()" << endl;
        cerr << "  Cannot read in temperature profile, url: "
             << _params.temp_profile_url << endl;
        cerr << "  Using specifed heights instead" << endl;
        cerr << "    ==>> shallowHtKm: " << _params.shallow_threshold_ht << endl;
        cerr << "    ==>> deepHtKm: " << _params.deep_threshold_ht << endl;
      }
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

  if (_params.write_texture) {
    // add 2D max texture field
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getTexture2D(),
                                 Mdvx::ENCODING_INT16,
                                 "DbzTexture2D",
                                 "reflectivity_texture_2D",
                                 "dBZ"));
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getTextureColMax(),
                                 Mdvx::ENCODING_INT16,
                                 "DbzTextureColMax",
                                 "reflectivity_texture_column_max",
                                 "dBZ"));
  }
  
  if (_params.write_convectivity) {
    // convectivity max in 2D
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getConvectivity2D(),
                                 Mdvx::ENCODING_INT16,
                                 "Convectivity2D",
                                 "likelihood_of_convection_2D",
                                 ""));
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getConvectivityColMax(),
                                 Mdvx::ENCODING_INT16,
                                 "ConvectivityColMax",
                                 "likelihood_of_convection_column_max",
                                 ""));
  }

  if (_params.write_col_max_dbz) {
    // col max dbz
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getColMaxDbz(),
                                 Mdvx::ENCODING_INT16,
                                 "DbzColMax",
                                 "max_dbz_in_column",
                                 "dBZ"));
  }
    
  if (_params.write_fraction_active) {
    // load up fraction of texture kernel covered
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getFractionActive(),
                                 Mdvx::ENCODING_INT16,
                                 "FractionActive",
                                 "fraction_of_texture_kernel_active",
                                 ""));
  }
    
  if (_params.write_tops) {
    // tops
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getConvTopKm(),
                                 Mdvx::ENCODING_INT16,
                                 "ConvTops",
                                 "convective_tops",
                                 "km"));
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getStratTopKm(),
                                 Mdvx::ENCODING_INT16,
                                 "StratTops",
                                 "stratiform_tops",
                                 "km"));
    char longName[256];
    snprintf(longName, 256, "%g_dbz_echo_tops", _params.dbz_for_echo_tops);
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getEchoTopKm(),
                                 Mdvx::ENCODING_INT16,
                                 "EchoTops", longName, "km"));
  }
  
  if (_params.write_height_grids &&
      _params.vert_levels_type == Params::VERT_LEVELS_BY_TEMP) {
    _shallowHtField.convertType(Mdvx::ENCODING_INT16, Mdvx::COMPRESSION_GZIP);
    _outMdvx.addField(new MdvxField(_shallowHtField));
    _deepHtField.convertType(Mdvx::ENCODING_INT16, Mdvx::COMPRESSION_GZIP);
    _outMdvx.addField(new MdvxField(_deepHtField));
  }
  
  if (_params.write_temperature && _tempField != NULL) {
    MdvxField * tempField = new MdvxField(*_tempField);
    tempField->convertType(Mdvx::ENCODING_INT16, Mdvx::COMPRESSION_GZIP);
    _outMdvx.addField(tempField);
  }

  // the following 2d fields are unsigned bytes
  
  size_t planeSize08 = fhdr2d.nx * fhdr2d.ny * sizeof(ui08);
  fhdr2d.volume_size = planeSize08;
  fhdr2d.encoding_type = Mdvx::ENCODING_INT8;
  fhdr2d.data_element_nbytes = 1;
  fhdr2d.missing_data_value = ConvStratFinder::CATEGORY_MISSING;
  fhdr2d.bad_data_value = ConvStratFinder::CATEGORY_MISSING;
  
  // partition field
  
  if (_params.write_partition) {
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getPartition2D(),
                                 Mdvx::ENCODING_INT8,
                                 "Partition2D",
                                 "convective_stratiform_partition_2D",
                                 ""));
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getPartitionColMax(),
                                 Mdvx::ENCODING_INT8,
                                 "PartitionColMax",
                                 "convective_stratiform_partition_col_max",
                                 ""));
  }
  
  // the following 3d fields are floats
  
  Mdvx::field_header_t fhdr3d = dbzField->getFieldHeader();
  Mdvx::vlevel_header_t vhdr3d = dbzField->getVlevelHeader();
  fhdr3d.missing_data_value = _missing;
  fhdr3d.bad_data_value = _missing;

  if (_params.write_convective_dbz) {
    // reflectivity only where convection has been identified
    MdvxField *convDbz = _makeField(fhdr3d, vhdr3d,
                                    _finder.getConvectiveDbz(),
                                    Mdvx::ENCODING_INT16,
                                    "DbzConv",
                                    "convective_reflectivity",
                                    "dBZ");
    _outMdvx.addField(convDbz);
  }
  
  if (_params.write_texture) {
    // texture in 3D
    _outMdvx.addField(_makeField(fhdr3d, vhdr3d,
                                 _finder.getTexture3D(),
                                 Mdvx::ENCODING_INT16,
                                 "DbzTexture3D",
                                 "reflectivity_texture_3D",
                                 "dBZ"));
  }

  if (_params.write_convectivity) {
    // convectivity in 3D
    _outMdvx.addField(_makeField(fhdr3d, vhdr3d,
                                 _finder.getConvectivity3D(),
                                 Mdvx::ENCODING_INT16,
                                 "Convectivity3D",
                                 "likelihood_of_convection_2D",
                                 ""));
  }

  if (_params.write_3D_dbz) {
    // echo the input DBZ field
    _outMdvx.addField(_makeField(fhdr3d, vhdr3d,
                                 _finder.getDbz3D(),
                                 Mdvx::ENCODING_INT16,
                                 "Dbz3D",
                                 "reflectivity_3D",
                                 "dBZ"));
  }
  
  if (_params.write_partition) {
    // partition for full volume
    size_t volSize08 = fhdr3d.nx * fhdr3d.ny * fhdr3d.nz * sizeof(ui08);
    fhdr3d.volume_size = volSize08;
    fhdr3d.encoding_type = Mdvx::ENCODING_INT8;
    fhdr3d.data_element_nbytes = 1;
    fhdr3d.missing_data_value = ConvStratFinder::CATEGORY_MISSING;
    fhdr3d.bad_data_value = ConvStratFinder::CATEGORY_MISSING;
    _outMdvx.addField(_makeField(fhdr3d, vhdr3d,
                                 _finder.getPartition3D(),
                                 Mdvx::ENCODING_INT8,
                                 "Partition3D",
                                 "convective_stratiform_partition_3D",
                                 ""));
  }

  if (_params.clumping_write_debug_fields) {
    _addClumpingDebugFields();
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
  
  _computeHts(_params.shallow_threshold_temp, _shallowHtField,
              "ShallowHt", "shallow_threshold_ht", "km");
  _computeHts(_params.deep_threshold_temp, _deepHtField,
              "DeepHt", "deep_threshold_ht", "km");

  // remap from model to radar grid coords
  // use of the lookup table makes this more efficient
  
  MdvxProj proj(dbzField->getFieldHeader());
  MdvxRemapLut lut;
  if (_shallowHtField.remap(lut, proj)) {
    cerr << "ERROR - ConvStrat::_readTempProfile" << endl;
    cerr << "  Cannot convert model temp grid to radar grid" << endl;
    cerr << "  URL: " << _params.temp_profile_url << endl;
    cerr << "  Time: " << DateTime::strm(_tempMdvx.getValidTime()) << endl;
    return -1;
  }
  if (_deepHtField.remap(lut, proj)) {
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

  _tempField->convertType(Mdvx::ENCODING_FLOAT32);
  const Mdvx::field_header_t tempFhdr = _tempField->getFieldHeader();
  fl32 *temp = (fl32 *) _tempField->getVol();
  fl32 tempMiss = tempFhdr.missing_data_value;
  
  // get Z profile for temperatures
  
  const Mdvx::vlevel_header_t tempVhdr = _tempField->getVlevelHeader();
  vector<double> zProfile;
  for (si64 iz = 0; iz < tempFhdr.nz; iz++) {
    zProfile.push_back(tempVhdr.level[iz]);
  } // iz
  
  // loop through the (x, y) plane
  
  si64 xyIndex = 0;
  size_t nxy = fhdr.nx * fhdr.ny;
  for (si64 iy = 0; iy < tempFhdr.ny; iy++) {
    for (si64 ix = 0; ix < tempFhdr.nx; ix++, xyIndex++) {

      // initialize

      fl32 bottomTemp = tempMiss;
      double bottomHt = tempVhdr.level[0]; // if temp is below grid
      
      fl32 topTemp = tempMiss;
      double topHt = tempVhdr.level[tempFhdr.nz - 1]; // if temp is above grid
      
      hts[xyIndex] = bottomHt;
      
      // loop through heights, looking for temps that straddle
      // the required temp

      bool htFound = false;
      for (si64 iz = 1; iz < tempFhdr.nz; iz++) {
        si64 zIndexBelow = xyIndex + (iz - 1) * nxy; 
        si64 zIndexAbove = zIndexBelow + nxy; 
        double tempBelow = temp[zIndexBelow];
        double tempAbove = temp[zIndexAbove];
        // set bottom temp
        if (tempBelow != tempMiss && bottomTemp == tempMiss) {
          bottomTemp = tempBelow;
        }
        // set top temp
        if (tempAbove != tempMiss) {
          topTemp = tempAbove;
        }
        if (!htFound && (tempBelow != tempMiss) && (tempAbove != tempMiss)) {
          // check for normal profile and inversion
          if ((tempBelow >= tempC && tempAbove <= tempC) ||
              (tempBelow <= tempC && tempAbove >= tempC)) {
            double deltaTemp = tempAbove - tempBelow;
            double deltaHt = zProfile[iz] - zProfile[iz-1];
            double interpHt =
              zProfile[iz] + ((tempC - tempBelow) / deltaTemp) * deltaHt;
            hts[xyIndex] = interpHt;
            htFound = true;
          }
        }
      } // iz
      
      if (!htFound) {
        if (tempC >= bottomTemp) {
          // required temp is below grid
          hts[xyIndex] = bottomHt;
        } else if (tempC <= topTemp) {
          // required temp is above grid
          hts[xyIndex] = topHt;
        }
      }

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

/////////////////////////////////////////////////////////
// addClumpingDebugFields()
//
// add debug fields for dual threshold clumps

void ConvStrat::_addClumpingDebugFields()
  
{

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

  // add clump composite reflectivity

  _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                               _finder.getClumpingMgr().getDualThreshCompFileGrid(),
                               Mdvx::ENCODING_INT16,
                               "ClumpsCompDbz",
                               "ClumpsCompDbz",
                               "dBZ"));

  // add sub clump grids

  _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                               _finder.getClumpingMgr().getDualThreshAllFileGrid(),
                               Mdvx::ENCODING_INT8,
                               "AllClumps",
                               "AllClumps",
                               "count"));
  
  _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                               _finder.getClumpingMgr().getDualThreshValidFileGrid(),
                               Mdvx::ENCODING_INT8,
                               "ValidClumps",
                               "ValidClumps",
                               "count"));
  
  _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                               _finder.getClumpingMgr().getDualThreshGrownFileGrid(),
                               Mdvx::ENCODING_INT8,
                               "GrownClumps",
                               "GrownClumps",
                               "count"));
  
}



