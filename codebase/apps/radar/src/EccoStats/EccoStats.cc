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
// EccoStats.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2024
//
////////////////////////////////////////////////////////////////////
//
// EccoStats computes statistics from the Ecco output files.
// See the Ecco app for details.
//
/////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <dataport/bigend.h>
#include <toolsa/toolsa_macros.h>
#include "EccoStats.hh"
using namespace std;

const fl32 EccoStats::_missingFl32 = -9999.0;

// Constructor

EccoStats::EccoStats(int argc, char **argv)

{

  isOK = true;
  
  // set programe name

  _progName = "EccoStats";
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

}

// destructor

EccoStats::~EccoStats()

{


}

//////////////////////////////////////////////////
// Run

int EccoStats::Run()
{
  
  int iret = 0;

  // loop until end of data
  
  _input.reset();
  while (!_input.endOfData()) {
    
    // do the read

    if (_doRead()) {
      cerr << "ERROR - EccoStats::Run()" << endl;
      umsleep(1000);
      iret = -1;
      continue;
    }

    // set grid in EccoStatsFinder object

    // MdvxField *dbzField = _inMdvx.getField(_params.dbz_field_name);
    // if (dbzField == NULL) {
    //   cerr << "ERROR - EccoStats::Run()" << endl;
    //   cerr << "  no dbz field found: " << _params.dbz_field_name << endl;
    //   return -1;
    // }
    // const Mdvx::field_header_t &fhdr = dbzField->getFieldHeader();
    // const Mdvx::vlevel_header_t &vhdr = dbzField->getVlevelHeader();
    // // bool isLatLon = (fhdr.proj_type == Mdvx::PROJ_LATLON);
    // vector<double> zLevels;
    // for (int iz = 0; iz < fhdr.nz; iz++) {
    //   zLevels.push_back(vhdr.level[iz]);
    // }
    
    // write out
    
    if (_doWrite()) {
      cerr << "ERROR - EccoStats::Run()" << endl;
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

int EccoStats::_doRead()
  
{
  
  _inMdvx.clear();
  if (_params.debug >= Params::DEBUG_EXTRA) {
    _inMdvx.setDebug(true);
  }
  // _inMdvx.addReadField(_params.dbz_field_name);
  // _inMdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  // _inMdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _inMdvx.printReadRequest(cerr);
  }
  
  // read in
  
  if (_input.readVolumeNext(_inMdvx)) {
    cerr << "ERROR - EccoStats::_doRead" << endl;
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

void EccoStats::_addFieldsToOutput()
  
{

  _outMdvx.clearFields();

  // initial fields are float32

  MdvxField *dbzField = _inMdvx.getField("Ecco");
  Mdvx::field_header_t fhdr2d = dbzField->getFieldHeader();
  fhdr2d.nz = 1;
  fhdr2d.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  size_t planeSize32 = fhdr2d.nx * fhdr2d.ny * sizeof(fl32);
  fhdr2d.volume_size = planeSize32;
  fhdr2d.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr2d.data_element_nbytes = 4;
  fhdr2d.missing_data_value = _missingFl32;
  fhdr2d.bad_data_value = _missingFl32;
  fhdr2d.scale = 1.0;
  fhdr2d.bias = 0.0;
  
  Mdvx::vlevel_header_t vhdr2d;
  MEM_zero(vhdr2d);
  vhdr2d.level[0] = 0;
  vhdr2d.type[0] = Mdvx::VERT_TYPE_SURFACE;

#ifdef JUNK
  
  if (_params.write_texture) {
    // add 2D max texture field
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getTexture2D(),
                                 Mdvx::ENCODING_INT16,
                                 "DbzTextureComp",
                                 "reflectivity_texture_composite",
                                 "dBZ"));
  }
  
  if (_params.write_convectivity) {
    // convectivity max in 2D
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getConvectivity2D(),
                                 Mdvx::ENCODING_INT16,
                                 "ConvectivityComp",
                                 "likelihood_of_convection_composite",
                                 ""));
  }

  if (_params.write_col_max_dbz) {
    // col max dbz
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getDbzColMax(),
                                 Mdvx::ENCODING_INT16,
                                 "DbzComp",
                                 "dbz_composite",
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
  
  // echoType field
  
  if (_params.write_partition) {
    _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                                 _finder.getEchoType2D(),
                                 Mdvx::ENCODING_INT8,
                                 "EchoTypeComp",
                                 "convective_stratiform_echo_type_composite",
                                 ""));
  }
  
  // the following 3d fields are floats
  
  Mdvx::field_header_t fhdr3d = dbzField->getFieldHeader();
  Mdvx::vlevel_header_t vhdr3d = dbzField->getVlevelHeader();
  fhdr3d.missing_data_value = _missingFl32;
  fhdr3d.bad_data_value = _missingFl32;

  if (_params.write_convective_dbz) {
    // reflectivity only where convection has been identified
    MdvxField *convDbz = _makeField(fhdr3d, vhdr3d,
                                    _finder.getConvectiveDbz(),
                                    Mdvx::ENCODING_INT16,
                                    "DbzConv",
                                    "convective_reflectivity_3D",
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
                                 "likelihood_of_convection_3D",
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

  if (_params.write_clumping_debug_fields) {
    _addClumpingDebugFields();
  }

  // add ht data if available
  
  if (_params.use_terrain_ht_data) {
    MdvxField *htFieldIn = _inMdvx.getField("TerrainHt");
    if (htFieldIn != NULL) {
      MdvxField *htFieldOut = new MdvxField(*htFieldIn);
      htFieldOut->convertType(Mdvx::ENCODING_ASIS, Mdvx::COMPRESSION_GZIP);
      _outMdvx.addField(htFieldOut);
    }
    // add water field
    if (_params.add_water_layer) {
      MdvxField *waterFieldIn = _inMdvx.getField("WaterFlag");
      if (waterFieldIn != NULL) {
        MdvxField *waterFieldOut = new MdvxField(*waterFieldIn);
        waterFieldOut->convertType(Mdvx::ENCODING_ASIS, Mdvx::COMPRESSION_GZIP);
        _outMdvx.addField(waterFieldOut);
      }
    }
  }

#endif
  
}

/////////////////////////////////////////////////////////
// perform the write
//
// Returns 0 on success, -1 on failure.

int EccoStats::_doWrite()
  
{
  
  // create output DsMdvx object
  // copying master header from input object
  
  _outMdvx.clear();
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _outMdvx.setDebug(true);
  }
  // _outMdvx.setWriteLdataInfo();
  Mdvx::master_header_t mhdr = _inMdvx.getMasterHeader();
  _outMdvx.setMasterHeader(mhdr);
  string info = _inMdvx.getMasterHeader().data_set_info;
  info += " : Stratfinder used to identify stratiform regions";
  _outMdvx.setDataSetInfo(info.c_str());
  _outMdvx.setMdv2NcfOutput(true, true, true, true);
  
  // copy any chunks
  
  // for (size_t i = 0; i < _inMdvx.getNChunks(); i++) {
  //   MdvxChunk *chunk = new MdvxChunk(*_inMdvx.getChunkByNum(i));
  //   _outMdvx.addChunk(chunk);
  // }
  
  // add fields
  
  _addFieldsToOutput();
  
  // write out
  
  if(_outMdvx.writeToDir(_params.output_dir)) {
    cerr << "ERROR - EccoStats::Run" << endl;
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

MdvxField *EccoStats::_makeField(Mdvx::field_header_t &fhdrTemplate,
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

MdvxField *EccoStats::_makeField(Mdvx::field_header_t &fhdrTemplate,
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

#ifdef JUNK

/////////////////////////////////////////////////////////
// addClumpingDebugFields()
//
// add debug fields for dual threshold clumps

void EccoStats::_addClumpingDebugFields()
  
{

  MdvxField *dbzField = _inMdvx.getField(_params.dbz_field_name);
  Mdvx::field_header_t fhdr2d = dbzField->getFieldHeader();
  fhdr2d.nz = 1;
  fhdr2d.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  size_t planeSize32 = fhdr2d.nx * fhdr2d.ny * sizeof(fl32);
  fhdr2d.volume_size = planeSize32;
  fhdr2d.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr2d.data_element_nbytes = 4;
  fhdr2d.missing_data_value = _missingFl32;
  fhdr2d.bad_data_value = _missingFl32;
  fhdr2d.scale = 1.0;
  fhdr2d.bias = 0.0;
  
  Mdvx::vlevel_header_t vhdr2d;
  MEM_zero(vhdr2d);
  vhdr2d.level[0] = 0;
  vhdr2d.type[0] = Mdvx::VERT_TYPE_SURFACE;

  // add clump composite reflectivity

  _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                               _finder.getClumpingMgr().getDualThreshDbzCompOutputGrid(),
                               Mdvx::ENCODING_INT16,
                               "ClumpsCompDbz",
                               "ClumpsCompDbz",
                               "dBZ"));
  
  _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                               _finder.getClumpingMgr().getDualThreshLargeClumpsOutputGrid(),
                               Mdvx::ENCODING_INT8,
                               "LargeClumps",
                               "LargeClumps",
                               "count"));
  
  // add sub clump grids

  _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                               _finder.getClumpingMgr().getDualThreshAllSubclumpsOutputGrid(),
                               Mdvx::ENCODING_INT8,
                               "AllSubclumps",
                               "AllSubclumps",
                               "count"));
  
  _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                               _finder.getClumpingMgr().getDualThreshValidSubclumpsOutputGrid(),
                               Mdvx::ENCODING_INT8,
                               "SmallSubclumps",
                               "SmallSubclumps",
                               "count"));
  
  _outMdvx.addField(_makeField(fhdr2d, vhdr2d,
                               _finder.getClumpingMgr().getDualThreshGrownSubclumpsOutputGrid(),
                               Mdvx::ENCODING_INT8,
                               "GrownSubclumps",
                               "GrownSubclumps",
                               "count"));
  
}

#endif
