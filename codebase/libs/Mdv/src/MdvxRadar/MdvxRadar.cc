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
//////////////////////////////////////////////////////////
// MdvxRadar.cc
//
// Class for handling radar info in Mdvx
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1999
//
//////////////////////////////////////////////////////////
//
// See <Mdv/MdvxRadar.hh> for details.
//
///////////////////////////////////////////////////////////

#include <iomanip>
#include <Mdv/MdvxRadar.hh>
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxChunk.hh>
#include <dataport/bigend.h>
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <rapformats/dobson.h>
#include <rapformats/var_elev.h>
using namespace std;

/////////////////////////////
// Constructor
//

MdvxRadar::MdvxRadar()

{
  clear();
}

/////////////////////////////
// Destructor

MdvxRadar::~MdvxRadar()

{

}

/////////////////////////////
// clear

void MdvxRadar::clear()

{
  _paramsAvail = false;
  _calibAvail = false;
  _elevAvail = false;
  _azAvail = false;
  _varElevAvail = false;
}

////////////////////////////////////
// load radar object from MdvxObject
//
// returns 0 on success, -1 on failure

int MdvxRadar::loadFromMdvx(const Mdvx &mdvx)

{

  clear();

  const Mdvx::master_header_t &mhdr = mdvx.getMasterHeader();
  
  for (int i = 0; i < mhdr.n_chunks; i++) {
    
    const MdvxChunk *chunk = mdvx.getChunkByNum(i);
    const Mdvx::chunk_header_t &chdr = chunk->getHeader();
    
    switch (chdr.chunk_id) {

    case Mdvx::CHUNK_DOBSON_VOL_PARAMS: {
      
      // dobson format radar paramaters
      
      vol_params_t vol_params;
      memcpy(&vol_params, chunk->getData(), sizeof(vol_params_t));
      radar_params_t *rparams = &vol_params.radar;
      int nbytes_char = N_RADAR_PARAMS_LABELS * R_FILE_LABEL_LEN;
      BE_to_array_32(rparams, sizeof(radar_params_t) - nbytes_char);
      
      _params.radarId = rparams->radar_id;
      _params.numFields = mhdr.n_fields;
      _params.numGates = rparams->ngates;
      _params.samplesPerBeam = rparams->samples_per_beam;
      
      _params.altitude = (double) rparams->altitude / 1000.0;
      _params.latitude = (double) rparams->latitude / 1000000.0;
      _params.longitude = (double) rparams->longitude / 1000000.0;
      _params.gateSpacing = (double) rparams->gate_spacing / 1000000.0;
      _params.startRange = (double) rparams->start_range / 1000000.0;
      _params.horizBeamWidth = (double) rparams->beam_width / 1000000.0;
      _params.vertBeamWidth = (double) rparams->beam_width / 1000000.0;
      _params.pulseWidth = (double) rparams->pulse_width / 1000.0;
      _params.pulseRepFreq = (double) rparams->prf / 1000.0;
      _params.wavelength = (double) rparams->wavelength / 10000.0;
      _params.radarName = rparams->name;
      
      _paramsAvail = true;

    }
    break;
      
    case Mdvx::CHUNK_DSRADAR_PARAMS: {
      
      DsRadarParams_t params;
      memcpy(&params, chunk->getData(), sizeof(DsRadarParams_t));
      BE_to_DsRadarParams(&params);
      _params.copy(params);
      _paramsAvail = true;

    }
    break;
      
    case Mdvx::CHUNK_DSRADAR_CALIB: {
      
      ds_radar_calib_t calib;
      memcpy(&calib, chunk->getData(), sizeof(ds_radar_calib_t));
      BE_to_ds_radar_calib(&calib);
      _calib.set(calib);
      _calibAvail = true;

    }
    break;
      
    case Mdvx::CHUNK_DOBSON_ELEVATIONS: {

      MemBuf elevBuf;
      elevBuf.add(chunk->getData(), chdr.size);
      BE_to_array_32(elevBuf.getPtr(), elevBuf.getLen());
      
      int nelev = chdr.size / sizeof(si32);
      si32 *int_elev = (si32 *) elevBuf.getPtr();
      _elev.alloc(nelev);
      fl32 *elevs = _elev.getElevArray();
      for (int i = 0; i < nelev; i++) {
	elevs[i] = int_elev[i] / 1000000.0;
      }
      _elevAvail = true;

    }
    break;
      
    case Mdvx::CHUNK_DSRADAR_ELEVATIONS: {

      MemBuf elevBuf;
      elevBuf.add(chunk->getData(), chdr.size);
      BE_to_array_32(elevBuf.getPtr(), elevBuf.getLen());
      _elev.loadFromChunk((ui08 *) elevBuf.getPtr(), elevBuf.getLen());
      _elevAvail = true;

    }
    break;
      
    case Mdvx::CHUNK_DSRADAR_AZIMUTHS: {

      MemBuf azBuf;
      azBuf.add(chunk->getData(), chdr.size);
      BE_to_array_32(azBuf.getPtr(), azBuf.getLen());
      _az.loadFromChunk((ui08 *) azBuf.getPtr(), azBuf.getLen());
      _azAvail = true;

    }
    break;
      
    case Mdvx::CHUNK_VARIABLE_ELEV: {

      MemBuf workBuf;
      workBuf.add(chunk->getData(), chdr.size);
      VAR_ELEV_variable_elev_to_BE(workBuf.getPtr(), workBuf.getLen());
      
      ui32 be_nelev;
      memcpy(&be_nelev, workBuf.getPtr(), sizeof(ui32));
      _nVarElev = BE_to_ui32(be_nelev);

      _varElevBuf.free();
      _varElevBuf.add((ui08 *) chunk->getData() + sizeof(ui32),
		      chdr.size - sizeof(ui32));

      BE_to_array_32(_varElevBuf.getPtr(), _varElevBuf.getLen());
      _varElevAvail = true;

    }

    break;

    default:
      break;
      
    } // switch

  } // i

  if (!_paramsAvail &&
      !_calibAvail &&
      !_elevAvail &&
      !_azAvail &&
      !_varElevAvail) {
    return -1;
  }
  
  return 0;

}

///////////////////////////////////////
// create chunk object for radar params
//
// Performs a 'new' to create the chunk.
// Passes ownership of chunk object to calling routine.
//
// Returns pointer to chunk on success, NULL on failure

MdvxChunk *MdvxRadar::createParamsChunk()
  
{

  if (!_paramsAvail) {
    return (NULL);
  }

  MdvxChunk *chunk = new MdvxChunk;
  chunk->setId(Mdvx::CHUNK_DSRADAR_PARAMS);
  chunk->setInfo("DsRadar params");
  DsRadarParams_t rparams;
  _params.loadStruct(&rparams);
  BE_from_DsRadarParams(&rparams);
  chunk->setData(&rparams, sizeof(DsRadarParams_t));
  
  return chunk;
  
}

///////////////////////////////////////
// create chunk object for radar calib
//
// Performs a 'new' to create the chunk.
// Passes ownership of chunk object to calling routine.
//
// Returns pointer to chunk on success, NULL on failure

MdvxChunk *MdvxRadar::createCalibChunk()
  
{

  if (!_calibAvail) {
    return (NULL);
  }

  MdvxChunk *chunk = new MdvxChunk;
  chunk->setId(Mdvx::CHUNK_DSRADAR_CALIB);
  chunk->setInfo("DsRadar calib");
  ds_radar_calib_t calib = _calib.getStructAsBE();
  chunk->setData(&calib, sizeof(ds_radar_calib_t));
  
  return chunk;
  
}

///////////////////////////////////////////
// create chunk object for radar elevations
//
// Performs a 'new' to create the chunk.
// Passes ownership of chunk object to calling routine.
//
// Returns pointer to chunk on success, NULL on failure

MdvxChunk *MdvxRadar::createElevChunk()
  
{

  if (!_elevAvail) {
    return (NULL);
  }

  MdvxChunk *chunk = new MdvxChunk;
  chunk->setId(Mdvx::CHUNK_DSRADAR_ELEVATIONS);
  chunk->setInfo("DsRadar elevations");

  _elev.saveToChunk();
  BE_from_array_32(_elev.getChunkData(), _elev.getChunkLen());
  chunk->setData(_elev.getChunkData(), _elev.getChunkLen());
  
  return chunk;
  
}

///////////////////////////////////////////
// create chunk object for radar azimuths
//
// Performs a 'new' to create the chunk.
// Passes ownership of chunk object to calling routine.
//
// Returns pointer to chunk on success, NULL on failure

MdvxChunk *MdvxRadar::createAzChunk()
  
{

  if (!_azAvail) {
    return (NULL);
  }

  MdvxChunk *chunk = new MdvxChunk;
  chunk->setId(Mdvx::CHUNK_DSRADAR_AZIMUTHS);
  chunk->setInfo("DsRadar azimuths");

  _az.saveToChunk();
  BE_from_array_32(_az.getChunkData(), _az.getChunkLen());
  chunk->setData(_az.getChunkData(), _az.getChunkLen());
  
  return chunk;
  
}

////////////////////////////////////////////////////
// create chunk object for variable radar elevations
//
// Performs a 'new' to create the chunk.
// Passes ownership of chunk object to calling routine.
//
// Returns pointer to chunk on success, NULL on failure

MdvxChunk *MdvxRadar::createVarElevChunk()
  
{
  
  if (!_varElevAvail) {
    return (NULL);
  }

  MdvxChunk *chunk = new MdvxChunk;
  chunk->setId(Mdvx::CHUNK_VARIABLE_ELEV);
  chunk->setInfo("Variable elevations");

  MemBuf workBuf;
  ui32 n_elev = _nVarElev;
  workBuf.add(&n_elev, sizeof(ui32));
  workBuf.add(_varElevBuf.getPtr(), _varElevBuf.getLen());
  BE_from_array_32(workBuf.getPtr(), workBuf.getLen());
  chunk->setData(workBuf.getPtr(), workBuf.getLen());
  
  return chunk;
  
}

////////
// print

void MdvxRadar::print(ostream &out) const

{
  
  if (_paramsAvail) {
    _params.print(out);
  }
  
  if (_calibAvail) {
    _calib.print(out);
  }
  
  if (_elevAvail) {
    _elev.print(out);
  }

  if (_azAvail) {
    _az.print(out);
  }

  if (_varElevAvail) {
    printVarElev(out);
  }

}

void MdvxRadar::printVarElev(ostream &out) const

{

  out << "RADAR VARIABLE ELEVATIONS ARRAY" << endl;
  out << "-------------------------------" << endl;

  out << "  Naz: " << _nVarElev;
  out << "  Elevation array: " << endl;
  const fl32 *elev = getVarElevs();
  for (int i = 0; i < _nVarElev; i++) {
    out << "  elev[" << setw(3) << i << "]: " << elev[i] << endl;
  }
  out << endl;

}

