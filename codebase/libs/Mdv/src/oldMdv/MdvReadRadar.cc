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
// MdvReadRadar.cc
//
// Class for handling access to Mdv radar structs
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 1999
//
//////////////////////////////////////////////////////////
//
// See <Mdv/mdv/MdvReadRadar.hh> for details.
//
///////////////////////////////////////////////////////////

#include <Mdv/mdv/MdvReadRadar.hh>
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <Mdv/mdv/MdvRead.hh>
#include <Mdv/mdv/MdvReadChunk.hh>
#include <rapformats/dobson.h>
using namespace std;

/////////////////////////////
// Constructor
//

MdvReadRadar::MdvReadRadar()

{

  _radarParamsAvail = false;
  _radarElevsAvail = false;
  MEM_zero(_radarParams);
  DsRadarElev_init(&_radarElevs);

}

/////////////////////////////
// Destructor

MdvReadRadar::~MdvReadRadar()

{
  DsRadarElev_free(&_radarElevs);
}

/////////////////////////////
// clear

void MdvReadRadar::clear()

{
  _radarParamsAvail = false;
  _radarElevsAvail = false;
}

//////////////////////////////
// load radar data from chunk

void MdvReadRadar::_loadFromChunk(MdvReadChunk &chunk, int nfields)

{

  MDV_chunk_header_t &hdr = chunk.getHeader();
  void *data = chunk.getData();

  if (hdr.chunk_id == MDV_CHUNK_DOBSON_VOL_PARAMS) {

    // dobson format radar paramaters
    
    vol_params_t *vol_params = (vol_params_t *) data;
    radar_params_t *rparams = &vol_params->radar;

    _radarParams.radar_id = rparams->radar_id;
    _radarParams.nfields = nfields;
    _radarParams.ngates = rparams->ngates;
    _radarParams.samples_per_beam = rparams->samples_per_beam;
    
    _radarParams.altitude = (double) rparams->altitude / 1000.0;
    _radarParams.latitude = (double) rparams->latitude / 1000000.0;
    _radarParams.longitude = (double) rparams->longitude / 1000000.0;
    _radarParams.gate_spacing = (double) rparams->gate_spacing / 1000000.0;
    _radarParams.start_range = (double) rparams->start_range / 1000000.0;
    _radarParams.horiz_beam_width = (double) rparams->beam_width / 1000000.0;
    _radarParams.vert_beam_width = (double) rparams->beam_width / 1000000.0;
    _radarParams.pulse_width = (double) rparams->pulse_width / 1000.0;
    _radarParams.prf = (double) rparams->prf / 1000.0;
    _radarParams.wavelength = (double) rparams->wavelength / 10000.0;

    STRncopy(_radarParams.radar_name, rparams->name, DS_LABEL_LEN);

    _radarParamsAvail = true;

  } else if (hdr.chunk_id == MDV_CHUNK_DSRADAR_PARAMS) {

    _radarParams = *((DsRadarParams_t *) data);
    _radarParamsAvail = true;

  } else if (hdr.chunk_id == MDV_CHUNK_DOBSON_ELEVATIONS) {

    int nelev = hdr.size / sizeof(si32);
    si32 *int_elev = (si32 *) data;
    DsRadarElev_alloc(&_radarElevs, nelev);
    for (int i = 0; i < nelev; i++) {
      _radarElevs.elev_array[i] =
	(fl32) ((double) int_elev[i] / 1000000.0);
    }
    _radarElevsAvail = true;

  } else if (hdr.chunk_id == MDV_CHUNK_DSRADAR_ELEVATIONS) {

    DsRadarElev_unload_chunk(&_radarElevs, (ui08 *) data, hdr.size);
    _radarElevsAvail = true;

  }

}
