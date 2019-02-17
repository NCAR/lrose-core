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
 * @file VolHandler.cc
 */
#include "VolHandler.hh"
#include <Radx/RadxVol.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxPath.hh>
#include <Mdv/GenericRadxFile.hh>
#include <dsserver/DsLdataInfo.hh>

#include <string>
using std::string;

//----------------------------------------------------------------
VolHandler::VolHandler(const Parms &params) :
  _params(params)
{
  for (int i=0; i<_params.nelev(); ++i)
  {
    _scan.push_back(ScanHandler(_params.ithElev(i), _params));
  }		    
}

//----------------------------------------------------------------
VolHandler::~VolHandler(void)
{
}

//----------------------------------------------------------------
void VolHandler::finish(int nScans)
{
  // loop through scans
  for (int iscan = 0; iscan < nScans; iscan++)
  {
    ScanHandler &scan = _scan[iscan];
    scan.finish();
    double elev = scan.elevDegrees();
    int ielev = scan.elevIndex();

    // loop through beams
    for (auto &beam : scan)
    {
      _addBeam(beam, elev, ielev);
    }
  }
}
    
//----------------------------------------------------------------
int VolHandler::write(void)
{

  // load up vol metadata from rays
  _vol.loadVolumeInfoFromRays();
  _vol.loadSweepInfoFromRays();

  // set vol metadata
  _vol.setTitle(_params.title);
  _vol.setInstitution(_params.institution);
  _vol.setReferences(_params.references);

  _vol.setInstrumentName(_params.radar_name);
  _vol.setSiteName(_params.radar_name);
  _vol.setInstrumentType(Radx::INSTRUMENT_TYPE_RADAR);
  _vol.setPlatformType(Radx::PLATFORM_TYPE_FIXED);
  _vol.setPrimaryAxis(Radx::PRIMARY_AXIS_Z);
  _vol.setLatitudeDeg(_params.radar_location.latitudeDeg);
  _vol.setLongitudeDeg(_params.radar_location.longitudeDeg);
  _vol.setAltitudeKm(_params.radar_location.altitudeKm);
  _vol.setWavelengthCm(_params.radar_wavelength_cm);
  _vol.setRadarBeamWidthDegH(_params.horiz_beam_width_deg);
  _vol.setRadarBeamWidthDegV(_params.vert_beam_width_deg);

  // write the file

  return _writeVol();
}

//----------------------------------------------------------------
void VolHandler::_addBeam(const RayHandler &beam, double elev, int ielev)
{
  double az = beam.azDegrees();
      
  // create ray
      
  RadxRay *ray = new RadxRay;
      
  ray->setSweepNumber(ielev);
  ray->setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);
  ray->setVolumeNumber(0);

  ray->setTime(_params._utime, 0);
  ray->setAzimuthDeg(az);
  ray->setElevationDeg(elev);
  ray->setFixedAngleDeg(elev);
  ray->setIsIndexed(true);
  ray->setAngleResDeg(_params.azimuths.delta);
  ray->setNSamples(1);
  ray->setRangeGeom(_params.gates.start, _params.gates.delta);

  // add ray to vol - vol will free it later
      
  _vol.addRay(ray);

  // loop through fields

  for (int ifield = 0; ifield < _params.output_fields_n; ifield++)
  {
    _addFieldToRay(_params._output_fields[ifield],
		   beam, ray);
  }        
}

//----------------------------------------------------------------
void VolHandler::_addFieldToRay(const Params::output_field_t &ofield,
				const RayHandler &beam, RadxRay *ray)
{
  string name = ofield.name;
  // create field
  RadxField *field = new RadxField(ofield.name, ofield.units);
  field->setStandardName(ofield.standard_name);
  field->setLongName(ofield.long_name);
  field->setMissingFl32(-9999.0);

  int nGates = _params.gates.count;
  Radx::fl32 *data = beam.createData(ofield.type);
  field->addDataFl32(nGates, data);
  // convert to desired output type
  if (ofield.encoding == Params::OUTPUT_SHORT) {
    field->convertToSi16();
  } else if (ofield.encoding == Params::OUTPUT_BYTE) {
    field->convertToSi08();
  }

  // add field to ray

  ray->addField(field);
}

//----------------------------------------------------------------
int VolHandler::_writeVol(void)
{

  // output file

  GenericRadxFile outFile;
  _setupWrite(outFile);
  
  // write to dir

  string outputDir(_params.output_dir);
  if (_params.append_radar_name_to_output_dir) {
    outputDir += PATH_DELIM;
    outputDir += _vol.getInstrumentName();
  }
  if (outFile.writeToDir(_vol, outputDir, true, false)) {
    cerr << "ERROR - RadxConvert::_writeVol" << endl;
    cerr << "  Cannot write file to dir: " << outputDir << endl;
    cerr << outFile.getErrStr() << endl;
    return -1;
  }
  string outputPath = outFile.getPathInUse();
  if (_params.debug) {
    cerr << "Wrote file: " << outputPath << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// set up write

void VolHandler::_setupWrite(RadxFile &file)
{
  if (_params.debug) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setVerbose(true);
  }

  file.setWriteFileNameMode(RadxFile::FILENAME_WITH_START_TIME_ONLY);
  file.setWriteInstrNameInFileName(true);
  file.setWriteCompressed(true);
  file.setCompressionLevel(4);

  // set output format

  switch (_params.output_format) {
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

  file.setNcFormat(RadxFile::NETCDF4);

  // write sweeps individually?

  if (_params.write_individual_sweeps) {
    file.setWriteIndividualSweeps(true);
  }

}

