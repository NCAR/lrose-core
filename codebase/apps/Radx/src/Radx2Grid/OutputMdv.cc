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
// OutputMdv.cc
//
// Handles output to MDV files.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2012
//
///////////////////////////////////////////////////////////////

#include "OutputMdv.hh"
#include <dataport/bigend.h>

#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <toolsa/Path.hh>
#include <toolsa/TaArray.hh>
#include <dsserver/DsLdataInfo.hh>

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxChunk.hh>
#include <Mdv/Mdv2NcfTrans.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRay.hh>

using namespace std;

//////////////
// Constructor

OutputMdv::OutputMdv(const string &prog_name,
		     const Params &params) :
        _progName(prog_name),
        _params(params)
  
{

}

/////////////
// destructor

OutputMdv::~OutputMdv()

{
  
}

/////////////////////////
// set the master header

void OutputMdv::setMasterHeader(const RadxVol &vol)

{
  
  // set master header
  
  Mdvx::master_header_t mhdr;
  MEM_zero(mhdr);

  mhdr.index_number = vol.getVolumeNumber();

  mhdr.time_gen = time(NULL);
  mhdr.time_begin = vol.getStartTimeSecs();
  mhdr.time_end = vol.getEndTimeSecs();
  mhdr.time_centroid = vol.getEndTimeSecs();
  if (_params.name_file_from_start_time) {
    mhdr.time_centroid = vol.getStartTimeSecs();
  }
  mhdr.time_expire = mhdr.time_centroid;
  
  mhdr.num_data_times = 1;
  mhdr.data_dimension = 3;  
  
  mhdr.data_collection_type = Mdvx::DATA_MEASURED;
  mhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
  mhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  if (_params.interp_mode == Params::INTERP_MODE_PPI ||
      _params.interp_mode == Params::INTERP_MODE_POLAR) {
    mhdr.vlevel_type = Mdvx::VERT_TYPE_ELEV;
  }

  if (vol.getNSweeps() > 0) {
    const RadxSweep *sweep = vol.getSweeps()[0];
    if (sweep->getSweepMode() == Radx::SWEEP_MODE_RHI) {
      mhdr.native_vlevel_type = Mdvx::VERT_TYPE_AZ;
    }
  }
  mhdr.vlevel_included = TRUE;
  mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  mhdr.data_ordering = Mdvx::ORDER_XYZ;
  mhdr.field_grids_differ = FALSE;
  mhdr.sensor_lon = vol.getLongitudeDeg();
  mhdr.sensor_lat = vol.getLatitudeDeg();
  mhdr.sensor_alt = vol.getAltitudeKm();
  
  _mdvx.setMasterHeader(mhdr);
  
  _mdvx.setDataSetInfo(vol.getHistory().c_str());
  _mdvx.setDataSetName(vol.getInstrumentName().c_str());
  _mdvx.setDataSetSource(vol.getSource().c_str());
  
}

////////////////////
// addField()
//

void OutputMdv::addField(const RadxVol &vol,
                         MdvxProj &proj,
                         const vector<double> &vlevels,
                         const string &field_name,
                         const string &field_name_long,
			 const string &units,
                         Radx::DataType_t inputDataType,
                         double inputScale,
                         double inputOffset,
                         fl32 missingVal,
			 const fl32 *data)
  
{
  
  int nz = (int) vlevels.size();
  Mdvx::coord_t coord = proj.getCoord();

  if (_params.debug) {
    cerr << "  Adding field: " << field_name << endl;
  }

  if (coord.nx == 0 || coord.ny == 0 || nz == 0) {
    cerr << "WARNING - OutputMdv::addField" << endl;
    cerr << "  Zero length field, not adding, name: " << field_name << endl;
    cerr << "  nx, ny, nz: "
         << coord.nx << ", " << coord.ny << ", " << nz << endl;
    return;
  }

  // field header

  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);

  fhdr.nx = coord.nx;
  fhdr.ny = coord.ny;
  fhdr.nz = nz;

  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  if (_params.interp_mode == Params::INTERP_MODE_PPI ||
      _params.interp_mode == Params::INTERP_MODE_POLAR) {
    fhdr.vlevel_type = Mdvx::VERT_TYPE_ELEV;
  }
  
  if (vol.getNSweeps() > 0) {
    const RadxSweep *sweep = vol.getSweeps()[0];
    if (sweep->getSweepMode() == Radx::SWEEP_MODE_RHI) {
      fhdr.native_vlevel_type = Mdvx::VERT_TYPE_AZ;
    }
  }
  fhdr.proj_type = proj.getProjType();
  fhdr.dz_constant = false;

  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = 4;
  fhdr.volume_size = coord.nx * coord.ny * nz * sizeof(fl32);
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;
  
  fhdr.proj_origin_lat = vol.getLatitudeDeg();
  fhdr.proj_origin_lon = vol.getLongitudeDeg();
  fhdr.user_data_fl32[0] = vol.getAltitudeKm();

  fhdr.grid_dx = coord.dx;
  fhdr.grid_dy = coord.dy;
  fhdr.grid_dz = 1.0;
  if (nz > 1) {
    fhdr.grid_dz = vlevels[1] - vlevels[0];
  }

  fhdr.grid_minx = coord.minx;
  fhdr.grid_miny = coord.miny;
  fhdr.grid_minz = vlevels[0];

  fhdr.scale = 1.0;
  fhdr.bias = 0.0;
  fhdr.bad_data_value = missingVal;
  fhdr.missing_data_value = missingVal;
  
  fhdr.min_value = 0;
  fhdr.max_value = 0;
  fhdr.min_value_orig_vol = 0;
  fhdr.max_value_orig_vol = 0;

  proj.syncXyToFieldHdr(fhdr);
  
  // vlevel header
  
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  int nLevels = nz;
  if (nLevels > MDV_MAX_VLEVELS) {
    nLevels = MDV_MAX_VLEVELS;
  }
  for (int i = 0; i < nLevels; i++) {
    vhdr.level[i] = vlevels[i];
    vhdr.type[i] = fhdr.vlevel_type;
  }
  
  // create field

  MdvxField *fld = new MdvxField(fhdr, vhdr, data);
  if (_params.output_format == Params::MDV) {
    if (inputDataType == Radx::SI08 ||
        inputDataType == Radx::UI08 ||
        inputDataType == Radx::SI16 ||
        inputDataType == Radx::UI16) {
      fld->convertType(Mdvx::ENCODING_INT16,
                       Mdvx::COMPRESSION_GZIP,
                       Mdvx::SCALING_DYNAMIC);
    } else {
      fld->convertType(Mdvx::ENCODING_FLOAT32,
                       Mdvx::COMPRESSION_GZIP);
    }
  } else if (_params.output_format == Params::CF_NETCDF) {
    if (inputDataType == Radx::SI08 ||
        inputDataType == Radx::UI08 ||
        inputDataType == Radx::SI16 ||
        inputDataType == Radx::UI16) {
      fld->convertType(Mdvx::ENCODING_INT16);
    } else {
      fld->convertType(Mdvx::ENCODING_FLOAT32);
    }
  }

  if (_params.auto_remap_flat_to_latlon &&
      _params.grid_projection == Params::PROJ_FLAT) {
    fld->autoRemap2Latlon(_remapLut);
  }

  // set strings
  
  fld->setFieldName(field_name.c_str());
  fld->setFieldNameLong(field_name_long.c_str());
  fld->setUnits(units.c_str());
  fld->setTransform("");
  
  // add to object
  
  _mdvx.addField(fld);

}

void OutputMdv::addConvStratBool(const RadxVol &vol,
                                 MdvxProj &proj,
                                 const string &field_name,
                                 const string &field_name_long,
                                 ui08 missingVal,
                                 const ui08 *data)
  
{
  
  Mdvx::coord_t coord = proj.getCoord();

  if (_params.debug) {
    cerr << "  Adding field: " << field_name << endl;
  }

  if (coord.nx == 0 || coord.ny == 0) {
    cerr << "WARNING - OutputMdv::addField" << endl;
    cerr << "  Zero length field, not adding, name: " << field_name << endl;
    cerr << "  nx, ny: "
         << coord.nx << ", " << coord.ny << endl;
    return;
  }

  // field header
  
  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);

  fhdr.nx = coord.nx;
  fhdr.ny = coord.ny;
  fhdr.nz = 1;

  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  
  fhdr.proj_type = proj.getProjType();
  fhdr.dz_constant = true;
  
  int volSize08 = fhdr.nx * fhdr.ny * sizeof(ui08);
  fhdr.volume_size = volSize08;
  fhdr.encoding_type = Mdvx::ENCODING_INT8;
  fhdr.data_element_nbytes = 1;

  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_SPECIFIED;
  
  fhdr.proj_origin_lat = vol.getLatitudeDeg();
  fhdr.proj_origin_lon = vol.getLongitudeDeg();
  fhdr.user_data_fl32[0] = vol.getAltitudeKm();
  
  fhdr.grid_dx = coord.dx;
  fhdr.grid_dy = coord.dy;
  fhdr.grid_dz = 1.0;
  
  fhdr.grid_minx = coord.minx;
  fhdr.grid_miny = coord.miny;
  fhdr.grid_minz = vol.getAltitudeKm();

  fhdr.scale = 1.0;
  fhdr.bias = 0.0;
  fhdr.missing_data_value = missingVal;
  fhdr.bad_data_value = missingVal;
  
  fhdr.min_value = 0;
  fhdr.max_value = 0;
  fhdr.min_value_orig_vol = 0;
  fhdr.max_value_orig_vol = 0;

  proj.syncXyToFieldHdr(fhdr);
  
  // vlevel header
  
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  vhdr.level[0] = fhdr.grid_minz;
  vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  
  // create field

  MdvxField *fld = new MdvxField(fhdr, vhdr, data);
  if (_params.output_format == Params::MDV) {
    fld->requestCompression(Mdvx::COMPRESSION_GZIP);
  }

  if (_params.auto_remap_flat_to_latlon &&
      _params.grid_projection == Params::PROJ_FLAT) {
    fld->autoRemap2Latlon(_remapLut);
  }

  // set strings
  
  fld->setFieldName(field_name.c_str());
  fld->setFieldNameLong(field_name_long.c_str());
  fld->setUnits("");
  fld->setTransform("");
  
  // add to object
  
  _mdvx.addField(fld);

}

////////////////////////////////////////
// add convective stratiform fields

void OutputMdv::addConvStratFields(const ConvStratFinder &convStrat,
                                   const RadxVol &vol,
                                   MdvxProj &proj,
                                   const vector<double> &vlevels)
  
{

  Mdvx::coord_t projCoord = proj.getCoord();
  if ((int) convStrat.getGridNx() != projCoord.nx ||
      (int) convStrat.getGridNy() != projCoord.ny) {
    cerr << "ERROR - OutputMdv::addConvStratFields" << endl;
    cerr << "  ConvStrat grid size does not match Proj grid size" << endl;
    cerr << "  ConvStrat nx, ny: " 
         << convStrat.getGridNx() << ", "
         << convStrat.getGridNy() << endl;
    MdvxProj::printCoord(projCoord, cerr);
    return;
  }

  vector<double> vlevel2D;
  vlevel2D.push_back(vol.getAltitudeKm());
  
  if (_params.conv_strat_write_partition) {
    addConvStratBool(vol, proj,
                     "ConvStrat",
                     "1 = stratiform, 2 = convective",
                     ConvStratFinder::CATEGORY_MISSING,
                     convStrat.getEchoType2D());
  }

  if (_params.conv_strat_write_max_texture) {
    addField(vol, proj, vlevel2D,
             "DbzTextureMax",
             "max_of_dbz_texture_over_height",
             "dBZ",
             Radx::FL32, 1.0, 0.0,
             convStrat.getMissingFl32(),
             convStrat.getTexture2D());
  }
  
  if (_params.conv_strat_write_convective_dbz) {
    addField(vol, proj, vlevels,
             "DbzConv",
             "dbz_in_convection",
             "dBZ",
             Radx::FL32, 1.0, 0.0,
             convStrat.getMissingFl32(),
             convStrat.getConvectiveDbz());
  }

  if (_params.conv_strat_write_debug_fields) {

    addField(vol, proj, vlevel2D,
             "DbzColMax",
             "dbz_column_maximum",
             "dBZ",
             Radx::FL32, 1.0, 0.0,
             convStrat.getMissingFl32(),
             convStrat.getDbzColMax());
    
    addField(vol, proj, vlevel2D,
             "FractionActive",
             "fraction_of_texture_kernel_with_active_dbz",
             "",
             Radx::FL32, 1.0, 0.0,
             convStrat.getMissingFl32(),
             convStrat.getFractionActive());


  }

}
  
////////////////////////////////////////
// addChunks()
//
// Add the DsRadarParams and elevation list as mdv chunks.

void OutputMdv::addChunks(const RadxVol &vol,
                          int nFields)
  
{

  const vector<RadxRay *> &rays = vol.getRays();
  if (rays.size() < 1) {
    return;
  }
  if (vol.getNSweeps() < 1) {
    return;
  }

  MdvxChunk *chunk;

  // add a radar params chunk
  
  DsRadarParams rparams;
  if (_getDsrParams(vol, rparams) == 0) {
    chunk = new MdvxChunk;
    chunk->setId(Mdvx::CHUNK_DSRADAR_PARAMS);
    chunk->setInfo("DsRadar params");
    DsRadarParams_t rparamsStruct;
    rparams.encode(&rparamsStruct);
    chunk->setData(&rparamsStruct, sizeof(DsRadarParams_t));
    _mdvx.addChunk(chunk);
  }

  // add a calibration chunk

  DsRadarCalib cal;
  if (_getDsrCal(vol, cal) == 0) {
    chunk = new MdvxChunk;
    chunk->setId(Mdvx::CHUNK_DSRADAR_CALIB);
    chunk->setInfo("DsRadar calib");
    ds_radar_calib_t calibStruct;
    cal.encode(&calibStruct);
    chunk->setData(&calibStruct, sizeof(ds_radar_calib_t));
    _mdvx.addChunk(chunk);
  }

  // add status XML chunk

  string statusXml = vol.getStatusXml();
  if (statusXml.size() > 0) {
    chunk = new MdvxChunk;
    chunk->setId(Mdvx::CHUNK_TEXT_DATA);
    chunk->setInfo("Radar status XML");
    chunk->setData(statusXml.c_str(), statusXml.size() + 1);
    _mdvx.addChunk(chunk);
  }

  // add elevations array if appropriate

  chunk = new MdvxChunk;
  int nz = (int) vol.getNSweeps();
  if (nz < 1) {
    return;
  }
  if (nz > MDV_MAX_VLEVELS) {
    nz = MDV_MAX_VLEVELS;
  }

  const vector<RadxSweep *> &sweeps = vol.getSweeps();
  const RadxSweep *sweep0 = sweeps[0];

  if (sweep0->getSweepMode() == Radx::SWEEP_MODE_RHI) {

    // create a new azimuths chunk
    
    chunk->setId(Mdvx::CHUNK_DSRADAR_AZIMUTHS);
    chunk->setInfo("RHI azimuth angles");
    
    MemBuf azBuf;
    si32 numAz = nz;
    azBuf.add(&numAz, sizeof(si32));
    
    for(int i=0; i < nz; i++ ) {
      const RadxSweep *sweep = sweeps[i];
      fl32 az = sweep->getFixedAngleDeg();
      azBuf.add((char *) &az, sizeof(fl32));
    }
    BE_from_array_32(azBuf.getPtr(), azBuf.getLen());
    chunk->setData(azBuf.getPtr(), azBuf.getLen());
    _mdvx.addChunk(chunk);

  } else {

    // create a new elevations chunk
    
    chunk->setId(Mdvx::CHUNK_DSRADAR_ELEVATIONS);
    chunk->setInfo("Radar Elevation angles");
    
    // load the chunk with elevation angles
    
    MemBuf elevBuf;
    si32 numElev = nz;
    elevBuf.add(&numElev, sizeof(si32));
    
    for(int i=0; i < nz; i++ ) {
      const RadxSweep *sweep = sweeps[i];
      fl32 elevation = sweep->getFixedAngleDeg();
      elevBuf.add((char *) &elevation, sizeof(fl32));
    }
    BE_from_array_32(elevBuf.getPtr(), elevBuf.getLen());
    chunk->setData(elevBuf.getPtr(), elevBuf.getLen());
    _mdvx.addChunk(chunk);

  }

}

//////////////////////////////////////////////////
// get radar params in Dsr form

int OutputMdv::_getDsrParams(const RadxVol &vol,
                             DsRadarParams &rparams)

{

  if (vol.getNSweeps() < 1) {
    return -1;
  }

  const RadxSweep *sweep0 = vol.getSweeps()[0];
  const RadxRay *ray0 = vol.getRays()[0];

  // radar parameters

  rparams.radarId = 0;
  rparams.radarType = _getDsRadarType(vol.getPlatformType());
  rparams.numFields = ray0->getFields().size();
  rparams.numGates = ray0->getNGates();
  rparams.samplesPerBeam = ray0->getNSamples();
  rparams.scanType = 0;
  rparams.scanMode = _getDsScanMode(sweep0->getSweepMode());
  rparams.followMode = _getDsFollowMode(sweep0->getFollowMode());
  rparams.polarization =
    _getDsPolarizationMode(sweep0->getPolarizationMode());
  rparams.scanTypeName =
    Radx::sweepModeToStr(sweep0->getSweepMode());
  rparams.prfMode = _getDsPrfMode(sweep0->getPrtMode(), ray0->getPrtRatio());
  
  if (vol.getRcalibs().size() > 0) {
    rparams.radarConstant = vol.getRcalibs()[0]->getRadarConstantH();
  }

  rparams.altitude = vol.getAltitudeKm();
  rparams.latitude = vol.getLatitudeDeg();
  rparams.longitude = vol.getLongitudeDeg();
  rparams.gateSpacing = vol.getGateSpacingKm();
  rparams.startRange = vol.getStartRangeKm();
  rparams.horizBeamWidth = vol.getRadarBeamWidthDegH();
  rparams.vertBeamWidth = vol.getRadarBeamWidthDegV();
  rparams.antennaGain = vol.getRadarAntennaGainDbH();
  rparams.wavelength = vol.getWavelengthM() * 100.0;

  rparams.pulseWidth = ray0->getPulseWidthUsec();
  rparams.pulseRepFreq = 1.0 / ray0->getPrtSec();
  rparams.prt = ray0->getPrtSec();
  rparams.prt2 = ray0->getPrtSec() / ray0->getPrtRatio();
  rparams.unambigRange = ray0->getUnambigRangeKm();
  rparams.unambigVelocity = ray0->getNyquistMps();

  if (vol.getRcalibs().size() > 0) {
    const RadxRcalib &cal = *vol.getRcalibs()[0];
    rparams.xmitPeakPower = pow(10.0, cal.getXmitPowerDbmH() / 10.0) / 1000.0;
    rparams.receiverGain = cal.getReceiverGainDbHc();
    rparams.receiverMds = cal.getNoiseDbmHc() - rparams.receiverGain;
    rparams.systemGain = rparams.antennaGain + rparams.receiverGain;
    rparams.measXmitPowerDbmH = cal.getXmitPowerDbmH();
    rparams.measXmitPowerDbmV = cal.getXmitPowerDbmV();
  }

  rparams.radarName = vol.getInstrumentName() + "/" + vol.getSiteName();
  rparams.scanTypeName = vol.getScanName();

  return 0;

}

//////////////////////////////////////////////////
// get calibration in DSR form

int OutputMdv::_getDsrCal(const RadxVol &vol,
                          DsRadarCalib &dsrCal)
  
{

  if (vol.getRcalibs().size() < 1) {
    // no cal data
    return 0;
  }
  
  // use first calibration

  const RadxRcalib &radxCal = *vol.getRcalibs()[0];

  dsrCal.setCalibTime(radxCal.getCalibTime());

  dsrCal.setWavelengthCm(vol.getWavelengthCm());
  dsrCal.setBeamWidthDegH(vol.getRadarBeamWidthDegH());
  dsrCal.setBeamWidthDegV(vol.getRadarBeamWidthDegV());
  dsrCal.setAntGainDbH(vol.getRadarAntennaGainDbH());
  dsrCal.setAntGainDbV(vol.getRadarAntennaGainDbV());

  dsrCal.setPulseWidthUs(radxCal.getPulseWidthUsec());
  dsrCal.setXmitPowerDbmH(radxCal.getXmitPowerDbmH());
  dsrCal.setXmitPowerDbmV(radxCal.getXmitPowerDbmV());
  
  dsrCal.setTwoWayWaveguideLossDbH(radxCal.getTwoWayWaveguideLossDbH());
  dsrCal.setTwoWayWaveguideLossDbV(radxCal.getTwoWayWaveguideLossDbV());
  dsrCal.setTwoWayRadomeLossDbH(radxCal.getTwoWayRadomeLossDbH());
  dsrCal.setTwoWayRadomeLossDbV(radxCal.getTwoWayRadomeLossDbV());
  dsrCal.setReceiverMismatchLossDb(radxCal.getReceiverMismatchLossDb());
  
  dsrCal.setRadarConstH(radxCal.getRadarConstantH());
  dsrCal.setRadarConstV(radxCal.getRadarConstantV());
  
  dsrCal.setNoiseDbmHc(radxCal.getNoiseDbmHc());
  dsrCal.setNoiseDbmHx(radxCal.getNoiseDbmHx());
  dsrCal.setNoiseDbmVc(radxCal.getNoiseDbmVc());
  dsrCal.setNoiseDbmVx(radxCal.getNoiseDbmVx());
  
  dsrCal.setReceiverGainDbHc(radxCal.getReceiverGainDbHc());
  dsrCal.setReceiverGainDbHx(radxCal.getReceiverGainDbHx());
  dsrCal.setReceiverGainDbVc(radxCal.getReceiverGainDbVc());
  dsrCal.setReceiverGainDbVx(radxCal.getReceiverGainDbVx());
  
  dsrCal.setReceiverSlopeDbHc(radxCal.getReceiverSlopeDbHc());
  dsrCal.setReceiverSlopeDbHx(radxCal.getReceiverSlopeDbHx());
  dsrCal.setReceiverSlopeDbVc(radxCal.getReceiverSlopeDbVc());
  dsrCal.setReceiverSlopeDbVx(radxCal.getReceiverSlopeDbVx());
  
  dsrCal.setBaseDbz1kmHc(radxCal.getBaseDbz1kmHc());
  dsrCal.setBaseDbz1kmHx(radxCal.getBaseDbz1kmHx());
  dsrCal.setBaseDbz1kmVc(radxCal.getBaseDbz1kmVc());
  dsrCal.setBaseDbz1kmVx(radxCal.getBaseDbz1kmVx());
  
  dsrCal.setSunPowerDbmHc(radxCal.getSunPowerDbmHc());
  dsrCal.setSunPowerDbmHx(radxCal.getSunPowerDbmHx());
  dsrCal.setSunPowerDbmVc(radxCal.getSunPowerDbmVc());
  dsrCal.setSunPowerDbmVx(radxCal.getSunPowerDbmVx());
  
  dsrCal.setNoiseSourcePowerDbmH(radxCal.getNoiseSourcePowerDbmH());
  dsrCal.setNoiseSourcePowerDbmV(radxCal.getNoiseSourcePowerDbmV());
  
  dsrCal.setPowerMeasLossDbH(radxCal.getPowerMeasLossDbH());
  dsrCal.setPowerMeasLossDbV(radxCal.getPowerMeasLossDbV());
  
  dsrCal.setCouplerForwardLossDbH(radxCal.getCouplerForwardLossDbH());
  dsrCal.setCouplerForwardLossDbV(radxCal.getCouplerForwardLossDbV());
  
  dsrCal.setZdrCorrectionDb(radxCal.getZdrCorrectionDb());
  dsrCal.setLdrCorrectionDbH(radxCal.getLdrCorrectionDbH());
  dsrCal.setLdrCorrectionDbV(radxCal.getLdrCorrectionDbV());
  dsrCal.setSystemPhidpDeg(radxCal.getSystemPhidpDeg());
  
  dsrCal.setTestPowerDbmH(radxCal.getTestPowerDbmH());
  dsrCal.setTestPowerDbmV(radxCal.getTestPowerDbmV());
  
  return 0;

}

//////////////////////////////////////////////////
// get Dsr enums from Radx enums

int OutputMdv::_getDsRadarType(Radx::PlatformType_t ptype)

{
  switch (ptype) {
    case Radx::PLATFORM_TYPE_VEHICLE:
      return DS_RADAR_VEHICLE_TYPE;
    case Radx::PLATFORM_TYPE_SHIP:
      return DS_RADAR_SHIPBORNE_TYPE;
    case Radx::PLATFORM_TYPE_AIRCRAFT_FORE:
      return DS_RADAR_AIRBORNE_FORE_TYPE;
    case Radx::PLATFORM_TYPE_AIRCRAFT_AFT:
      return DS_RADAR_AIRBORNE_AFT_TYPE;
    case Radx::PLATFORM_TYPE_AIRCRAFT_TAIL:
      return DS_RADAR_AIRBORNE_TAIL_TYPE;
    case Radx::PLATFORM_TYPE_AIRCRAFT_BELLY:
      return DS_RADAR_AIRBORNE_LOWER_TYPE;
    case Radx::PLATFORM_TYPE_AIRCRAFT_ROOF:
      return DS_RADAR_AIRBORNE_UPPER_TYPE;
    default:
      return DS_RADAR_GROUND_TYPE;
  }
}

int OutputMdv::_getDsScanMode(Radx::SweepMode_t mode)

{
  switch (mode) {
    case Radx::SWEEP_MODE_SECTOR:
      return DS_RADAR_SECTOR_MODE;
    case Radx::SWEEP_MODE_COPLANE:
      return DS_RADAR_COPLANE_MODE;
    case Radx::SWEEP_MODE_RHI:
      return DS_RADAR_RHI_MODE;
    case Radx::SWEEP_MODE_VERTICAL_POINTING:
      return DS_RADAR_VERTICAL_POINTING_MODE;
    case Radx::SWEEP_MODE_IDLE:
      return DS_RADAR_IDLE_MODE;
    case Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE:
      return DS_RADAR_SURVEILLANCE_MODE;
    case Radx::SWEEP_MODE_SUNSCAN:
      return DS_RADAR_SUNSCAN_MODE;
    case Radx::SWEEP_MODE_POINTING:
      return DS_RADAR_POINTING_MODE;
    case Radx::SWEEP_MODE_MANUAL_PPI:
      return DS_RADAR_MANUAL_MODE;
    case Radx::SWEEP_MODE_MANUAL_RHI:
      return DS_RADAR_MANUAL_MODE;
    case Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE:
    default:
      return DS_RADAR_SURVEILLANCE_MODE;
  }
}

int OutputMdv::_getDsFollowMode(Radx::FollowMode_t mode)

{
  switch (mode) {
    case Radx::FOLLOW_MODE_SUN:
      return DS_RADAR_FOLLOW_MODE_SUN;
    case Radx::FOLLOW_MODE_VEHICLE:
      return DS_RADAR_FOLLOW_MODE_VEHICLE;
    case Radx::FOLLOW_MODE_AIRCRAFT:
      return DS_RADAR_FOLLOW_MODE_AIRCRAFT;
    case Radx::FOLLOW_MODE_TARGET:
      return DS_RADAR_FOLLOW_MODE_TARGET;
    case Radx::FOLLOW_MODE_MANUAL:
      return DS_RADAR_FOLLOW_MODE_MANUAL;
    default:
      return DS_RADAR_FOLLOW_MODE_NONE;
  }
}

int OutputMdv::_getDsPolarizationMode(Radx::PolarizationMode_t mode)
  
{
  switch (mode) {
    case Radx::POL_MODE_HORIZONTAL:
      return DS_POLARIZATION_HORIZ_TYPE;
    case Radx::POL_MODE_VERTICAL:
      return DS_POLARIZATION_VERT_TYPE;
    case Radx::POL_MODE_HV_ALT:
      return DS_POLARIZATION_DUAL_HV_ALT;
    case Radx::POL_MODE_HV_SIM:
      return DS_POLARIZATION_DUAL_HV_SIM;
    case Radx::POL_MODE_CIRCULAR:
      return DS_POLARIZATION_RIGHT_CIRC_TYPE;
    default:
      return DS_POLARIZATION_HORIZ_TYPE;
  }
}

int OutputMdv::_getDsPrfMode(Radx::PrtMode_t mode,
                             double prtRatio)
  
{
  switch (mode) {
    case Radx::PRT_MODE_FIXED:
      return DS_RADAR_PRF_MODE_FIXED;
    case Radx::PRT_MODE_STAGGERED:
    case Radx::PRT_MODE_DUAL:
      if (fabs(prtRatio - 0.6667 < 0.01)) {
        return DS_RADAR_PRF_MODE_STAGGERED_2_3;
      } else if (fabs(prtRatio - 0.75 < 0.01)) {
        return DS_RADAR_PRF_MODE_STAGGERED_3_4;
      } else if (fabs(prtRatio - 0.8 < 0.01)) {
        return DS_RADAR_PRF_MODE_STAGGERED_4_5;
      } else {
        return DS_RADAR_PRF_MODE_FIXED;
      }
    default:
      return DS_RADAR_PRF_MODE_FIXED;
  }
}

////////////////////////////////////////
// writeVol()
//
// Write out merged volume in MDV format.
//

int OutputMdv::writeVol()

{

  _outputDir = _params.output_dir;

  if (_params.output_format == Params::CF_NETCDF) {
    return _writeAsCfNetCDF();
  } else if (_params.output_format == Params::ZEBRA_NETCDF) {
    if (_params.grid_projection == Params::PROJ_LATLON) {
      return _writeZebraLatLonNetCDF();
    } else {
      return _writeZebraXYNetCDF();
    }
  } else {
    return _writeAsMdv();
  }

}

////////////////////////////////////////
// write as CF NetCDF

int OutputMdv::_writeAsCfNetCDF()

{

  _mdvx.setMdv2NcfAttr(_params.ncf_institution,
                       _params.ncf_references,
                       _params.ncf_comment);

  if (_params.netcdf_style == Params::CLASSIC) {
    _mdvx.setMdv2NcfFormat(DsMdvx::NCF_FORMAT_CLASSIC);
  } else if (_params.netcdf_style == Params::NC64BIT) {
    _mdvx.setMdv2NcfFormat(DsMdvx::NCF_FORMAT_OFFSET64BITS);
  } else if  (_params.netcdf_style == Params::NETCDF4_CLASSIC) {
    _mdvx.setMdv2NcfFormat(DsMdvx::NCF_FORMAT_NETCFD4_CLASSIC);
  } else {
    _mdvx.setMdv2NcfFormat(DsMdvx::NCF_FORMAT_NETCDF4);
  }
  
  _mdvx.setMdv2NcfCompression(_params.netcdf_compressed,
                              _params.netcdf_compression_level);
  
  _mdvx.setMdv2NcfOutput(_params.netcdf_include_latlon_arrays,
                         _params.netcdf_output_mdv_attributes,
                         _params.netcdf_output_mdv_chunks);

  // write out as CF
  
  string outputPath;
  if (_params.specify_output_filename) {
    outputPath = _params.output_dir;
    outputPath += PATH_DELIM;
    outputPath += _params.output_filename;
  } else {
    outputPath = _computeCfNetcdfPath();
  }

  Mdv2NcfTrans trans;
  trans.setDebug(_params.debug);
  if (trans.writeCf(_mdvx, outputPath)) {
    cerr << "ERROR - Mdv2NetCDF::_processData()" << endl;
    cerr << trans.getErrStr() << endl;
    return -1;
  }
  
  // write latest data info

  _writeLdataInfo(outputPath);
    
  return 0;

}

////////////////////////////////////////
// write as legacy MDV

int OutputMdv::_writeAsMdv()

{

  time_t ftime = _mdvx.getMasterHeader().time_centroid;

  if (_mdvx.getNFields() == 0) {
    cerr << "WARNING - no fields in file" << endl;
    cerr << "  Not writing MDV file, time : " << utimstr(ftime) << endl;
    cerr << "  Dir path: " << _outputDir << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Writing MDV file, time : " << utimstr(ftime) << endl;
    cerr << "  Dir path: " << _outputDir << endl;
  }

  // write out file

  if (_params.specify_output_filename) {
    string outputPath;
    outputPath = _params.output_dir;
    outputPath += PATH_DELIM;
    outputPath += _params.output_filename;
    if (_mdvx.writeToPath(outputPath)) {
      cerr << "ERROR - OutputMdv::writeAsMdv" << endl;
      cerr << _mdvx.getErrStr();
      return -1;
    }
  } else {
    if (_mdvx.writeToDir(_outputDir)) {
      cerr << "ERROR - OutputMdv::writeAsMdv" << endl;
      cerr << _mdvx.getErrStr();
      return -1;
    }
  }
  
  if (_params.debug) {
    cerr << "Done writing MDV file" << endl;
    cerr << "  File path: " << _mdvx.getPathInUse() << endl;
  }

  return 0;

}

//////////////////////////////////////
// Compute output path for netCDF file

string OutputMdv::_computeCfNetcdfPath()
{

  // Get the proper time to assign to filename
  
  const Mdvx::master_header_t &mhdr = _mdvx.getMasterHeader();
  DateTime validTime(mhdr.time_centroid);
  DateTime genTime(mhdr.time_gen);
  
  bool isForecast = false;
  int year, month, day, hour, minute, seconds;
  
  if (mhdr.data_collection_type == Mdvx::DATA_EXTRAPOLATED ||
      mhdr.data_collection_type == Mdvx::DATA_FORECAST ||
      mhdr.forecast_time > 0) {
    year = genTime.getYear();
    month = genTime.getMonth();
    day =  genTime.getDay();
    hour = genTime.getHour();
    minute = genTime.getMin();
    seconds = genTime.getSec();
    isForecast = true;
  } else {
    year = validTime.getYear();
    month = validTime.getMonth();
    day =  validTime.getDay();
    hour = validTime.getHour();
    minute = validTime.getMin();
    seconds = validTime.getSec();
  }

  // compute output dir

  _outputDir = _params.output_dir;
  char dayStr[128];
  sprintf(dayStr, "%.4d%.2d%.2d", year, month, day);
  _outputDir += PATH_DELIM;
  _outputDir += dayStr;

  // ensure output dir exists
  
  if (ta_makedir_recurse(_outputDir.c_str())) {
    cerr << "ERROR - Mdv2NetCDF::_initNcFile()" << endl;
    cerr << "  Cannot make output dir: " << _outputDir;
  }

  // Create output filepath

  char outputPath[1024];
  
  if (_params.use_iso8601_filename_convention) {
    
    if (isForecast) { 
      int leadTime = mhdr.forecast_delta;
      int leadTimeHrs = leadTime/3600;
      int leadTimeMins = (leadTime % 3600 )/ 60;
      sprintf(outputPath, "%s/%s%.4d-%.2d-%.2dT%.2d:%.2d:%.2d.PT%.2d:%.2d.nc",
              _outputDir.c_str(), _params.netcdf_file_prefix,
              year, month, day, hour, minute, seconds,
              leadTimeHrs, leadTimeMins);   
    } else {
      sprintf(outputPath, "%s/%s%.4d-%.2d-%.2dT%.2d:%.2d:%.2d.nc",
              _outputDir.c_str(), _params.netcdf_file_prefix,
              year, month, day, hour, minute, seconds);
    }
    
  } else {
    
    string v_yyyymmdd = validTime.getDateStrPlain();
    string v_hhmmss = validTime.getTimeStrPlain();
    char filename[256];
    
    snprintf(filename, 256, "%s%s_%s%s.nc",
             _params.netcdf_file_prefix,
             v_yyyymmdd.c_str(), v_hhmmss.c_str(),
             _params.netcdf_file_suffix);
    
    if (isForecast) { 
      string g_hhmmss = genTime.getTimeStrPlain();
      snprintf(outputPath, 1024, "%s/g_%s/%s",
               _outputDir.c_str(), g_hhmmss.c_str(), filename);
    } else {
      snprintf(outputPath, 1024, "%s/%s", _outputDir.c_str(), filename);
    }
    
  }

  return outputPath;
  
}

//////////////////////////////////////

void OutputMdv::_writeLdataInfo(const string &outputPath)
{
  
  const Mdvx::master_header_t &mhdr = _mdvx.getMasterHeader();

  // Write LdataInfo file

  DsLdataInfo ldata(_params.output_dir, _params.debug);

  ldata.setWriter("Radx2Grid");
  ldata.setDataFileExt("nc");
  ldata.setDataType("netCDF");

  string fileName;
  Path::stripDir(_outputDir, outputPath, fileName);
  ldata.setRelDataPath(fileName);
  
  if (mhdr.data_collection_type == Mdvx::DATA_EXTRAPOLATED ||
      mhdr.data_collection_type == Mdvx::DATA_FORECAST ||
      mhdr.forecast_time > 0)
  {
    ldata.setIsFcast(true);
    int leadtime = mhdr.forecast_delta;
    ldata.setLeadTime(leadtime);
    ldata.write(mhdr.time_gen);
  }
  else
  {
    ldata.setIsFcast(false);
    ldata.write(mhdr.time_centroid);
  }
  
  if (_params.debug) {
    cerr << "OutputMdv::_writeLdataInfo(): Data written to "
         << outputPath << endl;
  }

}

/////////////////////////////////
// write Zebra-style netCDF file

int OutputMdv::_writeZebraLatLonNetCDF()
  
{

  time_t validTime = _mdvx.getValidTime();
  DateTime outputTime(validTime);

  // compute dir path

  string zebDir = _params.output_dir;
  char fileName[1024];

  if (_params.specify_output_filename) {

    sprintf(fileName, "%s", _params.output_filename);
    
  } else {

    char dayDir[32];
    sprintf(dayDir, "%.4d%.2d%.2d",
            outputTime.getYear(), outputTime.getMonth(), outputTime.getDay());
    zebDir += PATH_DELIM;
    zebDir += dayDir;

    // compute file name
    
    sprintf(fileName, "%s.%.4d%.2d%.2d.%.2d%.2d%.2d.nc",
            _params.netcdf_file_prefix,
            outputTime.getYear(), outputTime.getMonth(), outputTime.getDay(),
            outputTime.getHour(), outputTime.getMin(), outputTime.getSec());

  }

  // make dir

  if (ta_makedir_recurse(zebDir.c_str())) {
    int errNum = errno;
    cerr << "ERROR - OutputMdv::_writeZebraLatLonNetCDF" << endl;
    cerr << "  Cannot make zebra output dir: " << zebDir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // compute file path
  
  string zebPath(zebDir);
  zebPath += PATH_DELIM;
  zebPath += fileName;

  // open file

  Nc3File zebFile(zebPath.c_str(), Nc3File::Replace, NULL, 0, Nc3File::Classic);
  if (!zebFile.is_valid()) {
    cerr << "ERROR - OutputMdv::_writeZebraLatLonNetCDF" << endl;
    cerr << "  Cannot open netCDF file: " << zebPath << endl;
    return -1;
  }

  // create error object

  Nc3Error zebErr(Nc3Error::verbose_nonfatal);

  // add global attributes

  int iret = 0;
  iret |= !zebFile.add_att("projection" , (int) 2);
  iret |= !zebFile.add_att("projection_name" , "rectangular");
  iret |= !zebFile.add_att("data_set_name", _params.ncf_title);
  iret |= !zebFile.add_att("data_set_source", _params.ncf_source);

  if (iret) {
    cerr << "ERROR - OutputMdv::_writeZebraLatLonNetCDF" << endl;
    cerr << "  Cannot add attributes to  file: " << zebPath << endl;
    return -1;
  }

  // add time dimension

  Nc3Dim *timeDim = zebFile.add_dim("time", 1);
  if (timeDim == NULL) {
    cerr << "ERROR - OutputMdv::_writeZebraLatLonNetCDF" << endl;
    cerr << "  Cannot add time dim to  file: " << zebPath << endl;
    return -1;
  }

  // add altitude, latitude and longitude dimensions

  MdvxField *field0 = _mdvx.getField(0);
  const Mdvx::field_header_t &fhdr0 = field0->getFieldHeader();
  const Mdvx::vlevel_header_t &vhdr0 = field0->getVlevelHeader();

  Nc3Dim *altDim = zebFile.add_dim("altitude", fhdr0.nz);
  if (altDim == NULL) {
    cerr << "ERROR - OutputMdv::_writeZebraLatLonNetCDF" << endl;
    cerr << "  Cannot add altitude dim to file: " << zebPath << endl;
    return -1;
  }

  Nc3Dim *latDim = zebFile.add_dim("latitude", fhdr0.ny);
  if (latDim == NULL) {
    cerr << "ERROR - OutputMdv::_writeZebraLatLonNetCDF" << endl;
    cerr << "  Cannot add latitude dim to file: " << zebPath << endl;
    return -1;
  }

  Nc3Dim *lonDim = zebFile.add_dim("longitude", fhdr0.nx);
  if (lonDim == NULL) {
    cerr << "ERROR - OutputMdv::_writeZebraLatLonNetCDF" << endl;
    cerr << "  Cannot add longitude dim to file: " << zebPath << endl;
    return -1;
  }

  // add variables

  Nc3Var *baseTimeVar = zebFile.add_var("base_time", nc3Int);
  baseTimeVar->add_att("units", "seconds since 1970-01-01 00:00:00 +0000");

  Nc3Var *timeOffsetVar = zebFile.add_var("time_offset", nc3Float, timeDim);
  timeOffsetVar->add_att("units", "seconds");
  
  //   Nc3Var *timeVar = zebFile.add_var("time", ncInt, timeDim);
  //   timeVar->add_att("units", "seconds since 1970-01-01 00:00:00 +0000");
  //   timeVar->add_att("missing_value", -2147483647);
  //   timeVar->add_att("valid_min", -2147483646);
  //   timeVar->add_att("valid_max", 2147483647);

  Nc3Var *altVar = zebFile.add_var("altitude", nc3Float, altDim);
  altVar->add_att("units", "km");
  altVar->add_att("missing_value", -9999.0);
  altVar->add_att("valid_min", -2.0);
  altVar->add_att("valid_max", 100.0);

  Nc3Var *latVar = zebFile.add_var("latitude", nc3Float, latDim);
  latVar->add_att("units", "degrees_north");
  latVar->add_att("missing_value", -9999.0);
  latVar->add_att("valid_min", -90.0);
  latVar->add_att("valid_max", 90.0);

  Nc3Var *lonVar = zebFile.add_var("longitude", nc3Float, lonDim);
  lonVar->add_att("units", "degrees_east");
  lonVar->add_att("missing_value", -9999.0);
  lonVar->add_att("valid_min", -360.0);
  lonVar->add_att("valid_max", 360.0);

  vector<Nc3Var *> dataVars;
  for (size_t ifield = 0; ifield < _mdvx.getNFields(); ifield++) {
    MdvxField *field = _mdvx.getField(ifield);
    const Mdvx::field_header_t &fhdr = field->getFieldHeader();
    Nc3Var *dataVar =
      zebFile.add_var(field->getFieldName(),
                      nc3Float, timeDim, altDim, latDim, lonDim);
    if (dataVar == NULL) {
      cerr << "ERROR: " << zebErr.get_err() << endl;
      cerr << "  Bad field: " << field->getFieldName() << endl;
      cerr << "  Ignoring" << endl;
      dataVars.push_back(NULL);
      continue;
    }
    dataVar->add_att("units", field->getUnits());
    dataVar->add_att("_FillValue", fhdr.missing_data_value);
    dataVar->add_att("missing_value", fhdr.missing_data_value);
    dataVar->add_att("valid_min", -1.0e30);
    dataVar->add_att("valid_max", 1.0e30);
    dataVars.push_back(dataVar);
  }

  // load up alt, lat and lon arrays

  TaArray<float> alts_;
  float *alts = alts_.alloc(fhdr0.nz);
  for (int ii = 0; ii < fhdr0.nz; ii++) {
    alts[ii] = vhdr0.level[ii];
  }

  TaArray<float> lats_;
  float *lats = lats_.alloc(fhdr0.ny);
  float lat = fhdr0.grid_miny;
  for (int ii = 0; ii < fhdr0.ny; ii++, lat += fhdr0.grid_dy) {
    lats[ii] = lat;
  }

  TaArray<float> lons_;
  float *lons = lons_.alloc(fhdr0.nx);
  float lon = fhdr0.grid_minx;
  for (int ii = 0; ii < fhdr0.nx; ii++, lon += fhdr0.grid_dx) {
    lons[ii] = lon;
  }

  // load up time
  
  int baseTime[1];
  baseTime[0] = (int) validTime;
  iret |= !baseTimeVar->put(baseTime, 1);
  float timeOffsets[1];
  timeOffsets[0] = 0.0;
  iret |= !timeOffsetVar->put(timeOffsets, 1);

  //   int times[1];
  //   times[0] = (int) validTime;
  //   iret |= !timeVar->put(times, 1);

  // write meta data variables

  iret |= !altVar->put(alts, fhdr0.nz);
  iret |= !latVar->put(lats, fhdr0.ny);
  iret |= !lonVar->put(lons, fhdr0.nx);

  // write the field data

  for (size_t ifield = 0; ifield < _mdvx.getNFields(); ifield++) {
    if (dataVars[ifield] == NULL) {
      continue;
    }
    MdvxField *field = _mdvx.getField(ifield);
    const Mdvx::field_header_t &fhdr = field->getFieldHeader();
    if (_params.debug) {
      cerr << "adding field: " << field->getFieldName() << endl;
    }
    iret |= !dataVars[ifield]->put((float *) field->getVol(),
                                   1, fhdr.nz, fhdr.ny, fhdr.nx);
  }

  // close file

  zebFile.close();

  if (_params.debug) {
    cerr << "Wrote zebra nc file: " << zebPath << endl;
  }

  // check
  
  if (iret) {
    cerr << "ERROR - OutputMdv::_writeZebraLatLonNetCDF" << endl;
    cerr << "  Cannot write netCDF file: " << zebPath << endl;
    cerr << zebErr.get_errmsg() << endl;
    return -1;
  } 

  // write latest data info file
  
  DsLdataInfo ldata(_params.output_dir, _params.debug);
  ldata.setWriter("Mdv2ZebraNetCDF");
  ldata.setDataFileExt("nc");
  ldata.setDataType("nc");
  
  string relPath;
  Path::stripDir(_params.output_dir, zebPath, relPath);
  ldata.setRelDataPath(relPath);
  ldata.write(validTime);

  return 0;

}

//////////////////////////////////////////////
// write Zebra-style netCDF file - km coords

int OutputMdv::_writeZebraXYNetCDF()
  
{

  int iret = 0;
  time_t validTime = _mdvx.getValidTime();
  DateTime outputTime(validTime);

  // compute output path

  string zebDir = _params.output_dir;
  char fileName[1024];

  if (_params.specify_output_filename) {

    sprintf(fileName, "%s", _params.output_filename);
    
  } else {

    char dayDir[32];
    sprintf(dayDir, "%.4d%.2d%.2d",
            outputTime.getYear(), outputTime.getMonth(), outputTime.getDay());
    zebDir += PATH_DELIM;
    zebDir += dayDir;

    // compute file name
    
    sprintf(fileName, "%s.%.4d%.2d%.2d.%.2d%.2d%.2d.nc",
            _params.netcdf_file_prefix,
            outputTime.getYear(), outputTime.getMonth(), outputTime.getDay(),
            outputTime.getHour(), outputTime.getMin(), outputTime.getSec());

  }

  // make dir

  if (ta_makedir_recurse(zebDir.c_str())) {
    int errNum = errno;
    cerr << "ERROR - OutputMdv::_writeZebraLatLonNetCDF" << endl;
    cerr << "  Cannot make zebra output dir: " << zebDir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // compute file path
  
  string zebPath(zebDir);
  zebPath += PATH_DELIM;
  zebPath += fileName;

  if (_params.debug) {
    cerr << "Writing zebra file: " << zebPath << endl;
  }

  // open file

  Nc3File zebFile(zebPath.c_str(), Nc3File::Replace, NULL, 0, Nc3File::Classic);
  if (!zebFile.is_valid()) {
    cerr << "ERROR - OutputMdv::_writeZebraLatLonNetCDF" << endl;
    cerr << "  Cannot open netCDF file: " << zebPath << endl;
    return -1;
  }

  // create error object

  Nc3Error zebErr(Nc3Error::verbose_nonfatal);

  // add time dimension

  Nc3Dim *timeDim = zebFile.add_dim("time", 1);
  if (timeDim == NULL) {
    cerr << "ERROR - OutputMdv::_writeZebraLatLonNetCDF" << endl;
    cerr << "  Cannot add time dim to  file: " << zebPath << endl;
    return -1;
  }

  // add z, y, z dimensions

  const Mdvx::master_header_t &mhdr = _mdvx.getMasterHeader();
  MdvxField *field0 = _mdvx.getField(0);
  const Mdvx::field_header_t &fhdr0 = field0->getFieldHeader();
  // const Mdvx::vlevel_header_t &vhdr0 = field0->getVlevelHeader();
  
  Nc3Dim *zDim = zebFile.add_dim("z", fhdr0.nz);
  if (zDim == NULL) {
    cerr << "ERROR - OutputMdv::_writeZebraLatLonNetCDF" << endl;
    cerr << "  Cannot add z dim to file: " << zebPath << endl;
    return -1;
  }
  
  Nc3Dim *yDim = zebFile.add_dim("y", fhdr0.ny);
  if (yDim == NULL) {
    cerr << "ERROR - OutputMdv::_writeZebraLatLonNetCDF" << endl;
    cerr << "  Cannot add y dim to file: " << zebPath << endl;
    return -1;
  }

  Nc3Dim *xDim = zebFile.add_dim("x", fhdr0.nx);
  if (xDim == NULL) {
    cerr << "ERROR - OutputMdv::_writeZebraLatLonNetCDF" << endl;
    cerr << "  Cannot add x dim to file: " << zebPath << endl;
    return -1;
  }

  // add variables
  
  Nc3Var *baseTimeVar = zebFile.add_var("base_time", nc3Int);
  baseTimeVar->add_att("units", "seconds since 1970-01-01 00:00:00 +0000");

  Nc3Var *timeOffsetVar = zebFile.add_var("time_offset", nc3Float, timeDim);
  timeOffsetVar->add_att("units", "seconds since base_time");
  
  Nc3Var *latVar = zebFile.add_var("lat", nc3Float);
  latVar->add_att("units", "degrees_north");
  
  Nc3Var *lonVar = zebFile.add_var("lon", nc3Float);
  lonVar->add_att("units", "degrees_east");
  
  Nc3Var *altVar = zebFile.add_var("alt", nc3Float);
  altVar->add_att("units", "km");
  
  Nc3Var *xSpacingVar = zebFile.add_var("x_spacing", nc3Float);
  xSpacingVar->add_att("units", "km");
  
  Nc3Var *ySpacingVar = zebFile.add_var("y_spacing", nc3Float);
  ySpacingVar->add_att("units", "km");
  
  Nc3Var *zSpacingVar = zebFile.add_var("z_spacing", nc3Float);
  zSpacingVar->add_att("units", "km");
  
  vector<Nc3Var *> dataVars;
  for (size_t ifield = 0; ifield < _mdvx.getNFields(); ifield++) {
    MdvxField *field = _mdvx.getField(ifield);
    const Mdvx::field_header_t &fhdr = field->getFieldHeader();
    Nc3Var *dataVar =
      zebFile.add_var(field->getFieldName(),
                      nc3Float, timeDim, zDim, yDim, xDim);
    if (dataVar == NULL) {
      cerr << "ERROR: " << zebErr.get_err() << endl;
      cerr << "  Bad field: " << field->getFieldName() << endl;
      cerr << "  Ignoring" << endl;
      dataVars.push_back(NULL);
      continue;
    }
    dataVar->add_att("units", field->getUnits());
    dataVar->add_att("_FillValue", fhdr.missing_data_value);
    dataVar->add_att("missing_value", fhdr.missing_data_value);
    dataVars.push_back(dataVar);
  }

  // load up time
  
  int baseTime[1];
  baseTime[0] = (int) validTime;
  iret |= !baseTimeVar->put(baseTime, 1);
  float timeOffsets[1];
  timeOffsets[0] = 0.0;
  iret |= !timeOffsetVar->put(timeOffsets, 1);

  // compute location of lower left grid point

  MdvxProj proj(mhdr, fhdr0);
  double xLowerLeft = fhdr0.grid_minx - fhdr0.grid_dx / 2.0;
  double yLowerLeft = fhdr0.grid_miny - fhdr0.grid_dy / 2.0;
  double latLowerLeft, lonLowerLeft;
  proj.xy2latlon(xLowerLeft, yLowerLeft,
		 latLowerLeft, lonLowerLeft);
  
  // load up geometry variables
  
  float refLat[1]; refLat[0] = latLowerLeft;
  float refLon[1]; refLon[0] = lonLowerLeft;
  float refAlt[1]; refAlt[0] = fhdr0.grid_minz;
  float xSpacing[1]; xSpacing[0] = fhdr0.grid_dx;
  float ySpacing[1]; ySpacing[0] = fhdr0.grid_dy;
  float zSpacing[1]; zSpacing[0] = fhdr0.grid_dz;
  
  iret |= !altVar->put(refAlt, 1);
  iret |= !latVar->put(refLat, 1);
  iret |= !lonVar->put(refLon, 1);
  iret |= !xSpacingVar->put(xSpacing, 1);
  iret |= !ySpacingVar->put(ySpacing, 1);
  iret |= !zSpacingVar->put(zSpacing, 1);

  // write the field data
  
  for (size_t ifield = 0; ifield < _mdvx.getNFields(); ifield++) {
    if (dataVars[ifield] == NULL) {
      continue;
    }
    MdvxField *field = _mdvx.getField(ifield);
    const Mdvx::field_header_t &fhdr = field->getFieldHeader();
    if (_params.debug) {
      cerr << "adding field: " << field->getFieldName() << endl;
    }
    iret |= !dataVars[ifield]->put((float *) field->getVol(),
                                   1, fhdr.nz, fhdr.ny, fhdr.nx);
  }

  // close file

  zebFile.close();

  // check
  
  if (iret) {
    cerr << "ERROR - OutputMdv::_writeZebraLatLonNetCDF" << endl;
    cerr << "  Cannot write netCDF file: " << zebPath << endl;
    cerr << zebErr.get_errmsg() << endl;
    return -1;
  } 

  // write latest data info file
  
  DsLdataInfo ldata(_params.output_dir, _params.debug);
  ldata.setWriter("Mdv2ZebraNetCDF");
  ldata.setDataFileExt("nc");
  ldata.setDataType("nc");
  
  string relPath;
  Path::stripDir(_params.output_dir, zebPath, relPath);
  ldata.setRelDataPath(relPath);
  ldata.write(validTime);

  return 0;

}

