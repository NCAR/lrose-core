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
/**
 * @file OutputData.cc
 */
#include "OutputData.hh"
#include <Radx/RadxRay.hh>
#include <Radx/RadxPath.hh>
#include <Mdv/GenericRadxFile.hh>
#include <dsserver/DsLdataInfo.hh>
#include <toolsa/LogMsg.hh>
#include <toolsa/DateTime.hh>

#define KM_TO_METERS 1000.0

//----------------------------------------------------------------
OutputData::OutputData(const Parms &params) :  Data(params)
{
}

//----------------------------------------------------------------
OutputData::OutputData (const Parms &params, const Data &input) : 
  Data(params)
{
  Geom::operator=(input);

  _inputElev = input.getElev();
  _sweeps.push_back(Sweep(_inputElev[0], *this, _params));
}

//----------------------------------------------------------------
OutputData::~OutputData(void)
{
}

//----------------------------------------------------------------
void OutputData::writeVolume(const time_t &t, const Data &inp, int vol_index)
{
  _outVol = RadxVol();

  for (size_t ielev=0; ielev < _sweeps.size(); ++ielev)
  {
    double elev = _sweeps[ielev].elev();
    for (int iaz=0; iaz < Geom::nOutputAz();  ++iaz)
    {
      double az = Geom::ithOutputAz(iaz);
      RadxRay *ray = new RadxRay;
      ray->setSweepNumber(ielev);
      ray->setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);
      ray->setVolumeNumber(vol_index);

      ray->setTime(t, 0);
      ray->setAzimuthDeg(az);
      ray->setElevationDeg(elev);
      ray->setFixedAngleDeg(elev);
      ray->setIsIndexed(true);
      ray->setAngleResDeg(Geom::deltaOutputAz());
      ray->setNSamples(1);
      ray->setRangeGeom(Geom::r0()/KM_TO_METERS, Geom::dr()/KM_TO_METERS);

      _outVol.addRay(ray);

      _addFieldsToRay(_sweeps[ielev], iaz, ray);
    }
  }
  _writeVolume(inp);
}
      
//----------------------------------------------------------------
void OutputData::_addFieldsToRay(const Sweep &s, int iaz, RadxRay *ray)
{
  // create field

#ifdef CPP11
  for (auto & fld :  s)
  {
#else
  for (size_t i=0; i<s.size(); ++i)
  {
    const Field &fld = s[i];
#endif

    string name = fld.getName();

    const Params::output_field_t *pfield = _params.matchingOutput(name);
    if (pfield != NULL)
    {
      _addField(*pfield, name, fld, iaz, ray);
    }
    else
    {
      const Params::rainrate_field_t *rfield = _params.matchingRainrate(name);
      if (rfield != NULL)
      {
	_addRateField(*rfield, name, fld, iaz, ray);
      }
      else
      {
	LOGF(LogMsg::ERROR, "Field %s not found in output data", 
	     name.c_str());
      }
    }
  }
}

//----------------------------------------------------------------
void OutputData::_addField(const Params::output_field_t &pfield, string &name, 
			   const Field &fld, int iaz, RadxRay *ray)
{
    RadxField *field = new RadxField(name, pfield.units);
    field->setStandardName(pfield.standard_name);
    field->setLongName(pfield.long_name);
    field->setMissingFl32(fld.getMissing());
    field->setRangeGeom(Geom::r0()/KM_TO_METERS, Geom::dr()/KM_TO_METERS);

    int nGates = Geom::nGate();
    Radx::fl32 *data = fld.createData(iaz, nGates);
    field->addDataFl32(nGates, data);
    delete [] data;

    // convert to desired output type
    if (pfield.encoding == Params::OUTPUT_SHORT)
    {
      field->convertToSi16();
    }
    else if (pfield.encoding == Params::OUTPUT_BYTE)
    {
      field->convertToSi08();
    }
    // add field to ray
    ray->addField(field);
}

//----------------------------------------------------------------
void OutputData::_addRateField(const Params::rainrate_field_t &pfield,
			       string &name, const Field &fld,
			       int iaz, RadxRay *ray)
{
    RadxField *field = new RadxField(name, pfield.units);
    field->setStandardName(pfield.standard_name);
    field->setLongName(pfield.long_name);
    field->setMissingFl32(fld.getMissing());
    field->setRangeGeom(Geom::r0()/KM_TO_METERS, Geom::dr()/KM_TO_METERS);

    int nGates = Geom::nGate();
    Radx::fl32 *data = fld.createData(iaz, nGates);
    field->addDataFl32(nGates, data);
    delete [] data;

    // convert to desired output type
    if (pfield.encoding == Params::OUTPUT_SHORT)
    {
      field->convertToSi16();
    }
    else if (pfield.encoding == Params::OUTPUT_BYTE)
    {
      field->convertToSi08();
    }
    // add field to ray
    ray->addField(field);
}

//----------------------------------------------------------------
void OutputData::_writeVolume(const Data &inp)
{
  if (strlen(_params.output_dir) == 0)
  {
    // not writing this data out
    return;
  }

  // load up vol metadata from rays
  _outVol.loadVolumeInfoFromRays();
  _outVol.loadSweepInfoFromRays();

  _outVol.setTitle(_params.title);
  _outVol.setInstitution(_params.institution);
  _outVol.setReferences(_params.references);
  _outVol.setSource(_params.source);
  _outVol.setHistory(_params.history);
  _outVol.setComment(_params.comment);

  // get these from input volume
  _outVol.setInstrumentName(inp.getVol().getInstrumentName());
  _outVol.setSiteName(inp.getVol().getSiteName());
  _outVol.setInstrumentType(inp.getVol().getInstrumentType());
  _outVol.setPlatformType(inp.getVol().getPlatformType());
  _outVol.setPrimaryAxis(inp.getVol().getPrimaryAxis());
  _outVol.setLatitudeDeg(inp.getVol().getLatitudeDeg());
  _outVol.setLongitudeDeg(inp.getVol().getLongitudeDeg());
  _outVol.setAltitudeKm(inp.getVol().getAltitudeKm());
  _outVol.setWavelengthCm(inp.getVol().getWavelengthCm());
  _outVol.setRadarBeamWidthDegH(inp.getVol().getRadarBeamWidthDegH());
  _outVol.setRadarBeamWidthDegV(inp.getVol().getRadarBeamWidthDegV());


  // write the file

  GenericRadxFile outFile;

  _setupWrite(outFile);
  
  if (outFile.writeToDir(_outVol, _params.output_dir,
                         _params.append_day_dir_to_output_dir,
                         _params.append_year_dir_to_output_dir))
  {
    LOGF(LogMsg::ERROR, 
	 "  Cannot write file to dir: %s", _params.output_dir);
    LOG(LogMsg::ERROR, outFile.getErrStr().c_str());
    return;
  }
  string outputPath = outFile.getPathInUse();
  LOGF(LogMsg::DEBUG, "Done writing file, path: %s",
       outputPath.c_str());

  // in realtime mode, write latest data info file
  
  if (_params.write_latest_data_info)
  {
    DsLdataInfo ldata(_params.output_dir);
    if (_params.debug_verbose)
    {
      ldata.setDebug(true);
    }
    string relPath;
    RadxPath::stripDir(_params.output_dir, outputPath, relPath);
    ldata.setRelDataPath(relPath);
    ldata.setWriter(_params._progName);
    if (ldata.write(_outVol.getEndTimeSecs()))
    {
      LOGF(LogMsg::WARNING, "  Cannot write latest data info file to dir: %s",
           _params.output_dir);
    }
  }

  return;
}

//----------------------------------------------------------------
void OutputData::_setupWrite(RadxFile &file)
{
  if (_params.debug_verbose)
  {
    file.setDebug(true);
  }

  if (_params.output_filename_mode == Params::START_TIME_ONLY)
  {
    file.setWriteFileNameMode(RadxFile::FILENAME_WITH_START_TIME_ONLY);
  }
  else if (_params.output_filename_mode == Params::END_TIME_ONLY)
  {
    file.setWriteFileNameMode(RadxFile::FILENAME_WITH_END_TIME_ONLY);
  }
  else
  {
    file.setWriteFileNameMode(RadxFile::FILENAME_WITH_START_AND_END_TIMES);
  }

  if (_params.output_compressed)
  {
    file.setWriteCompressed(true);
    file.setCompressionLevel(_params.compression_level);
  }
  else
  {
    file.setWriteCompressed(false);
  }

  if (_params.output_native_byte_order)
  {
    file.setWriteNativeByteOrder(true);
  }
  else
  {
    file.setWriteNativeByteOrder(false);
  }

  // set output format

  switch (_params.output_format)
  {
  case Params::OUTPUT_FORMAT_UF:
    file.setFileFormat(RadxFile::FILE_FORMAT_UF);
    break;
  case Params::OUTPUT_FORMAT_DORADE:
    file.setFileFormat(RadxFile::FILE_FORMAT_DORADE);
    break;
  case Params::OUTPUT_FORMAT_FORAY:
    file.setFileFormat(RadxFile::FILE_FORMAT_FORAY_NC);
    break;
  case Params::OUTPUT_FORMAT_NEXRAD:
    file.setFileFormat(RadxFile::FILE_FORMAT_NEXRAD_AR2);
    break;
  case Params::OUTPUT_FORMAT_MDV_RADIAL:
    file.setFileFormat(RadxFile::FILE_FORMAT_MDV_RADIAL);
    break;
  default:
  case Params::OUTPUT_FORMAT_CFRADIAL:
    file.setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL);
  }

  // set netcdf format - used for CfRadial

  switch (_params.netcdf_style)
  {
  case Params::NETCDF4_CLASSIC:
    file.setNcFormat(RadxFile::NETCDF4_CLASSIC);
    break;
  case Params::NC64BIT:
    file.setNcFormat(RadxFile::NETCDF_OFFSET_64BIT);
    break;
  case Params::NETCDF4:
    file.setNcFormat(RadxFile::NETCDF4);
    break;
  default:
    file.setNcFormat(RadxFile::NETCDF_CLASSIC);
  }

  if (_params.write_individual_sweeps)
  {
    file.setWriteIndividualSweeps(true);
  }

  if (_params.output_force_ngates_vary)
  {
    file.setWriteForceNgatesVary(true);
  }
}
