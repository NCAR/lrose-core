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
// ConvectionFinder.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// MAY 2014
//
////////////////////////////////////////////////////////////////////
//
// ConvectionFinder finds stratiform/convective regions in a Cartesian
// radar volume
//
/////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <dataport/bigend.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <toolsa/toolsa_macros.h>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxChunk.hh>
#include "ConvectionFinder.hh"
using namespace std;

const fl32 ConvectionFinder::_missing = -9999.0;

// Constructor

ConvectionFinder::ConvectionFinder(const string &progName,
                                   const Params &params) :
        _progName(progName),
        _params(params)
        
{

  // set up ConvStrat object

  if (_params.debug >= Params::DEBUG_EXTRA) {
    _convStrat.setVerbose(true);
  } else if (_params.debug) {
    _convStrat.setDebug(true);
  }
  _convStrat.setMinValidHtKm(_params.convection_finder_min_valid_height);
  _convStrat.setMaxValidHtKm(_params.convection_finder_max_valid_height);
  _convStrat.setMinValidDbz(_params.convection_finder_min_valid_dbz);
  _convStrat.setDbzForDefiniteConvection
    (_params.dbz_threshold_for_definite_convection);
  _convStrat.setConvectiveRadiusKm(_params.convective_radius_km);
  _convStrat.setTextureRadiusKm(_params.convection_finder_texture_radius_km);
  _convStrat.setMinValidFractionForTexture
    (_params.convection_finder_min_valid_fraction_for_texture);
  _convStrat.setMinTextureForConvection
    (_params.convection_finder_min_texture_value);

}

// destructor

ConvectionFinder::~ConvectionFinder()

{

}

//////////////////////////////////////////////////
// Run

int ConvectionFinder::run(const DsMdvx &inMdvx,
                          const MdvxField &dbzField)
  
{
  
  // register with procmap
  
  PMU_auto_register("ConvectionFinder::run");
  
  // set grid in ConvStrat object
  
  const Mdvx::field_header_t &fhdr = dbzField.getFieldHeader();
  const Mdvx::vlevel_header_t &vhdr = dbzField.getVlevelHeader();
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

  // compute the convective stratifom partition
  
  const fl32 *dbz = (const fl32*) dbzField.getVol();
  fl32 missingDbz = fhdr.missing_data_value;
  if (_convStrat.computePartition(dbz, missingDbz)) {
    cerr << "ERROR - ConvectionFinder::run()" << endl;
    cerr << "  Cannot compute convective stratiform partition" << endl;
    return -1;
  }
    
  // write out if requested

  if (_params.convection_finder_write_debug_files) {
    if (_doWrite(inMdvx, dbzField)) {
      cerr << "WARNING - ConvectionFinder::run()" << endl;
      cerr << "  Cannot write MDV files for debugging" << endl;
    }
  }

  return 0;

}

/////////////////////////////////////////////////////////
// add fields to the output object

void ConvectionFinder::_addFields(const MdvxField &dbzField,
                                  DsMdvx &outMdvx)
  
{

  outMdvx.clearFields();

  // initial fields are float32

  Mdvx::field_header_t fhdr = dbzField.getFieldHeader();
  fhdr.nz = 1;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  int volSize32 = fhdr.nx * fhdr.ny * sizeof(fl32);
  fhdr.volume_size = volSize32;
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = 4;
  fhdr.missing_data_value = _missing;
  fhdr.bad_data_value = _missing;
  
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  vhdr.level[0] = 0;
  vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  
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
  outMdvx.addField(fractionField);
  
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
  outMdvx.addField(textureField);
  
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
  outMdvx.addField(maxField);
  
  // the following fields are unsigned bytes

  int volSize08 = fhdr.nx * fhdr.ny * sizeof(ui08);
  fhdr.volume_size = volSize08;
  fhdr.encoding_type = Mdvx::ENCODING_INT8;
  fhdr.data_element_nbytes = 1;
  fhdr.missing_data_value = ConvStrat::CATEGORY_MISSING;
  fhdr.bad_data_value = ConvStrat::CATEGORY_MISSING;
  
  // convective flag from max dbz
  
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
  outMdvx.addField(convFromMaxField);

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
  outMdvx.addField(convFromTextureField);
  
  // partition
  
  Mdvx::field_header_t partitionFhdr = fhdr;
  MdvxField *partitionField = new MdvxField(partitionFhdr, vhdr);
  partitionField->setVolData(_convStrat.getPartition(),
                             volSize08,
                             Mdvx::ENCODING_INT8);
  partitionField->convertType(Mdvx::ENCODING_INT8,
                              Mdvx::COMPRESSION_GZIP);
  partitionField->setFieldName("ConvStratPart");
  partitionField->setFieldNameLong("1 = stratiform, 2 = convective");
  partitionField->setUnits("");
  outMdvx.addField(partitionField);

}

/////////////////////////////////////////////////////////
// perform the write
//
// Returns 0 on success, -1 on failure.

int ConvectionFinder::_doWrite(const DsMdvx &inMdvx,
                               const MdvxField &dbzField)
  
{
  
  // create output DsMdvx object
  // copying master header from input object
  
  DsMdvx outMdvx;
  if (_params.debug) {
    outMdvx.setDebug(true);
  }
  if (_params.mode == Params::REALTIME) {
    outMdvx.setWriteLdataInfo();
  }
  outMdvx.setMasterHeader(inMdvx.getMasterHeader());
  string info = inMdvx.getMasterHeader().data_set_info;
  info += " : Titan::ConvectionFinder used to identify stratiform regions";
  outMdvx.setDataSetInfo(info.c_str());
  
  // copy any chunks
  
  for (int i = 0; i < inMdvx.getNChunks(); i++) {
    MdvxChunk *chunk = new MdvxChunk(*inMdvx.getChunkByNum(i));
    outMdvx.addChunk(chunk);
  }
  
  // add fields
  
  _addFields(dbzField, outMdvx);
  
  // write out
  
  PMU_auto_register("Before write");
  if(outMdvx.writeToDir(_params.convection_finder_output_url)) {
    cerr << "ERROR - ConvectionFinder::run" << endl;
    cerr << "  Cannot write debug files." << endl;
    cerr << outMdvx.getErrStr() << endl;
    return -1;
  }

  return 0;

}
  
