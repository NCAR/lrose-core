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
// OutputFile.cc
//
// Class for output file.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2005
//
/////////////////////////////////////////////////////////////

#include "OutputFile.hh"

#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <toolsa/toolsa_macros.h>
#include <Mdv/MdvxField.hh>

using namespace std;

// Constructor

OutputFile::OutputFile(const string &prog_name,
                       const Params &params,
                       const Params::output_file_t &output_file,
                       ChannelSet &channelSet,
                       const FieldSet &fieldSet) :
        _progName(prog_name),
        _params(params),
        _channelSet(channelSet)
  
{

  // initialize members

  _OK = true;
  _dataStartTime = 0;
  _dataEndTime = 0;
  _sensorLon = -999;
  _lat = NULL;
  _lon = NULL;
  _needSunAngle = false;

  _outputUrl = output_file.output_url;

  // initialize grid

  _projection = (Mdvx::projection_type_t) output_file.projection;
  _originLat = output_file.origin_lat;
  _originLon = output_file.origin_lon;
  _lambertLat1 = output_file.lambert_lat1;
  _lambertLat2 = output_file.lambert_lat2;
  _nx = output_file.nx;
  _ny = output_file.ny;
  _minx = output_file.minx;
  _miny = output_file.miny;
  _dx = output_file.dx;
  _dy = output_file.dy;

  _nPointsGrid = _nx * _ny;

  _xLookup = new double[_nPointsGrid];
  _yLookup = new double[_nPointsGrid];

  // initialize projection

  _setUpProj();

  // load vector of field names from comma-delimited list
  
  vector<string> fieldNames;
  Hrit::tokenize(output_file.fields, ",", fieldNames);
  if (fieldNames.size() == 0) {
    cerr << "ERROR - constructing OutputFile object" << endl;
    cerr << "  Must have at least one field name" << endl;
    cerr << "  outputUrl: " << _outputUrl << endl;
    _OK = false;
    return;
  }

  // check requested field names have all been defined
  
  for (int ii = 0; ii < (int) fieldNames.size(); ii++) {
    const Field *field = fieldSet.getField(fieldNames[ii]);
    if (field != NULL) {
      _fields.push_back(field);
    } else {
      cerr << "ERROR - constructing OutputFile object" << endl;
      cerr << "  outputUrl: " << _outputUrl << endl;
      cerr << "  Undefined field name: " << fieldNames[ii] << endl;
      _OK = false;
    }
  }
  if (!_OK) {
    return;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Creating data set for outputUrl: " << _outputUrl << endl;
    cerr << "  Field names:" << endl;
    for (int ii = 0; ii < (int) fieldNames.size(); ii++) {
      cerr << " " << fieldNames[ii];
    }
    cerr << endl;
  }
  
  // add channels to a map container - if duplicate channels are used,
  // they are not duplicated in this map
  // also add to vector for easy iteration
  
  for (int ii = 0; ii < (int) _fields.size(); ii++) {
    const Channel *chan = _fields[ii]->getChannel();
    int channelId = chan->getId();
    if (_channelFilesMap.find(channelId) == _channelFilesMap.end()) {
      ChannelFiles *files = new
        ChannelFiles(_progName, _params, chan,
                     _nPointsGrid, _xLookup, _yLookup);
      ChannelFilesEntry entry(channelId, files);
      _channelFilesMap.insert(entry);
      _channelFiles.push_back(files);
    }
  }

  // check for albedo computations.
  // if albedo needed, set up lat/lon arrays
  
  for (int ifield = 0; ifield < (int) _fields.size(); ifield++) {
    const Field *field = _fields[ifield];
    if (field->getOutputUnits() == Field::ALBEDO) {
      _needSunAngle = true;
    }
  }
  if (_needSunAngle) {
    // allocate lat/lon arrays
    _lat = new fl32[_nPointsGrid];
    _lon = new fl32[_nPointsGrid];
  }

}

// Destructor

OutputFile::~OutputFile()

{

  for (int ii = 0; ii < (int) _channelFiles.size(); ii++) {
    delete _channelFiles[ii];
  }

  if (_xLookup != NULL) {
    delete[] _xLookup;
  }

  if (_yLookup != NULL) {
    delete[] _yLookup;
  }

  if (_lat != NULL) {
    delete[] _lat;
  }

  if (_lon != NULL) {
    delete[] _lon;
  }

}

///////////////////////////////////////////////////////////
// Check to see whether all sets are complete
//
// Returns true if all file sets are complete
 
bool OutputFile::complete()

{

  for (int ii = 0; ii < (int) _channelFiles.size(); ii++) {
    if (!_channelFiles[ii]->complete()) {
      return false;
    }
  }
  return true;

}

///////////////////////////////////////////////////////////
// Clear the data sets
 
void OutputFile::_clear()

{

  for (int ii = 0; ii < (int) _channelFiles.size(); ii++) {
    _channelFiles[ii]->clear();
  }

  _dataStartTime = 0;
  _dataEndTime = 0;

}

///////////////////////////////////////////////////////////
// process an input file
// returns 0 on success, -1 if already in the set
  
int OutputFile::processFile(const string &path,
                            const Hrit &hrit)

{
  
  // compute lookup if longitude has changed
  
  if (_sensorLon != hrit.getGeosSubLon()) {
    _sensorLon = hrit.getGeosSubLon();
    _computeLookups();
  }
  
  // Check if the start time has changed.
  // If so, write out and clear.
  
  int iret = 0;
  if (_dataStartTime != hrit.getAnnotTime()) {
    if (write()) {
      iret = -1;
    }
    _clear();
  }

  // offer the file for addition to input sets

  for (int ii = 0; ii < (int) _channelFiles.size(); ii++) {
    if (_channelFiles[ii]->addFile(path, hrit) == 0) {
      // successful add, update times accordingly
      if (_dataStartTime == 0) {
        _dataStartTime = hrit.getAnnotTime();
        _dataEndTime = _dataStartTime;
      } else {
        if (hrit.getDataTime() > _dataEndTime) {
          _dataEndTime = hrit.getDataTime();
        }
      }
    }
  }

  // write out and clear if we are complete

  if (complete()) {
    if (write()) {
      iret = -1;
    }
    _clear();
  }

  return iret;

}

///////////////////////////////////////////////////////////
// Set up grid projection
 
void OutputFile::_setUpProj()

{

  Mdvx::coord_t coord;
  MEM_zero(coord);

  if (_projection == Mdvx::PROJ_LATLON) {
    coord.proj_origin_lon = _minx + (_dx * _nx) / 2.0;
    coord.proj_origin_lat = _miny + (_dy * _ny) / 2.0;
  } else {
    coord.proj_origin_lon = _originLon;
    coord.proj_origin_lat = _originLat;
  }

  if (_projection == Mdvx::PROJ_LAMBERT_CONF) {
    coord.proj_params.lc2.lat1 = _lambertLat1;
    coord.proj_params.lc2.lat2 = _lambertLat2;
  }

  coord.minx = _minx;
  coord.miny = _miny;
  coord.minz = 0.0;

  coord.dx = _dx;
  coord.dy = _dy;
  coord.dz = 1.0;

  coord.proj_type = _projection;
  
  coord.nx = _nx;
  coord.ny = _ny;
  coord.nz = 1;
  coord.dz_constant = true;

  coord.sensor_lon = _sensorLon;
  coord.sensor_lat = 0.0;
  coord.sensor_z = 42164;

  _proj.init(coord);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  OutputUrl: " << _outputUrl << endl;
    cerr << "  Output grid: nx, ny: " << _nx << ", " << _ny << endl;
    cerr << "  Output grid: minx, maxx: "
         << _minx << ", " << _minx + _nx * _dx << endl;
    cerr << "  Output grid: miny, maxy: "
         << _miny << ", " << _miny + _ny * _dy << endl;
  }

}
  
///////////////////////////////////////////////////////////
// Compute lookup table
 
void OutputFile::_computeLookups()

{

  // Compute the lookup table - xx and yy.
  // This is the intermediate coord system.
  // See section 4.4.3.2 of HRIT specs document.

  int ii = 0;
  for (int iy = 0; iy < _ny; iy++) {
    for (int ix = 0; ix < _nx; ix++, ii++) {
      double lat, lon;
      _proj.xyIndex2latlon(ix, iy, lat, lon);
      double latRad = lat * DEG_TO_RAD;
      double lonRad = lon * DEG_TO_RAD;
      double sublonRad = _sensorLon * DEG_TO_RAD;
      double c_lat = atan(0.993243 * tan(latRad));
      double rL = 6356.5838 / sqrt(1 - 0.00675701 * cos(c_lat) * cos(c_lat));
      double r1 = 42164 - rL * cos(c_lat) * cos(lonRad - sublonRad);
      double r2 = -1.0 * rL * cos(c_lat) * sin(lonRad - sublonRad);
      double r3 = rL * sin(c_lat);
      double rN = sqrt(r1 * r1 + r2 * r2 + r3 * r3);
      if (r1 == 0.0 && r2 == 0.0) {
        _xLookup[ii] = 0.0;
      } else {
        _xLookup[ii] = atan2(-r2, r1) * RAD_TO_DEG;
      }
      _yLookup[ii] = asin(-r3 / rN) * RAD_TO_DEG;
      if (_lat != NULL) {
        _lat[ii] = (fl32) lat;
      }
      if (_lon != NULL) {
        _lon[ii] = (fl32) lon;
      }
    }
  }

}
  
///////////////////////////////////////////////////////////
// Compute sun angle
 
void OutputFile::_computeSunAngle(time_t data_time)

{
  
  DateTime MJD2000(2000, 1, 1, 12, 0, 0);

}


///////////////////////////////////////////////////////////
// Write
//
// Returns 0 on success, -1 on failure
 
int OutputFile::write()

{
  
  PMU_auto_register("OutputFile::write");

  if (_dataStartTime == 0) {
    // no data yet
    return 0;
  }

  // get new cal if needed

  if (_params.calibration_source == Params::CALIBRATION_FILE) {
    _channelSet.readNewCalibFile(_params.calibration_file_path);
  }

  // set up sun angle computations if needed

  if (_needSunAngle) {
    int deltaTime = _dataEndTime - _dataStartTime;
    _sunAngle.initForTime(_dataEndTime - deltaTime / 2);
  }

  // initialize headers
  
  DsMdvx mdvx;
  Mdvx::master_header_t mhdr;
  MEM_zero(mhdr);
  mdvx.clearFields();
  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);

  // sync master and field header from proj
  
  _proj.syncToHdrs(mhdr, fhdr);

  // fill in the master header except for grid stuff
  
  mhdr.time_begin = _dataStartTime;
  mhdr.time_end = _dataEndTime;
  mhdr.time_centroid = _dataEndTime;
  mhdr.time_expire = _dataEndTime + 3600;
  
  mhdr.num_data_times = 1;
  mhdr.data_dimension = 2;
  
  mhdr.data_collection_type = Mdvx::DATA_MEASURED;
  mhdr.native_vlevel_type = Mdvx::VERT_SATELLITE_IMAGE;
  mhdr.vlevel_type = Mdvx::VERT_SATELLITE_IMAGE;
  mhdr.vlevel_included = TRUE;
  mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  mhdr.data_ordering = Mdvx::ORDER_XYZ;
  mhdr.n_chunks = 0;
  mhdr.field_grids_differ = FALSE;
  mhdr.sensor_lon = _sensorLon;
  mhdr.sensor_lat = 0.0;
  mhdr.sensor_alt = 42164;

  // set the master header
  
  mdvx.setMasterHeader(mhdr);
  mdvx.setDataSetInfo(_params.data_set_info);
  mdvx.setDataSetName(_params.data_set_name);
  mdvx.setDataSetSource(_params.data_set_source);

  // fill in field headers and vlevel headers as far as we can
  
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_SPECIFIED;

  fhdr.native_vlevel_type = Mdvx::VERT_SATELLITE_IMAGE;
  fhdr.vlevel_type = Mdvx::VERT_SATELLITE_IMAGE;
  fhdr.dz_constant = true;
  
  // vlevel header
    
  vhdr.type[0] = Mdvx::VERT_SATELLITE_IMAGE;
  vhdr.level[0] = 0.0;

  // create fields
  
  for (int ifield = 0; ifield < (int) _fields.size(); ifield++) {

    const Field *field = _fields[ifield];

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Add output field: " << field->getName() << endl;
    }

    // get channel and files associated with this field

    int channelId = field->getChannelId();
    ChannelFilesMap::const_iterator ichan = _channelFilesMap.find(channelId);
    if (ichan == _channelFilesMap.end()) {
      cerr << "ERROR - OutputFile::write" << endl;
      cerr << "  Bad channel id: " << channelId << endl;
      cerr << "  Field: " << field->getName() << endl;
      continue;
    }
    const ChannelFiles &channelFiles = *(ichan->second);
    const Channel *channel = channelFiles.getChannel();

    // set up MDVX field object

    MdvxField *outField = NULL;

    switch (field->getOutputUnits()) {

      case Field::RADIANCE: {

        fhdr.encoding_type = Mdvx::ENCODING_INT16;
        fhdr.data_element_nbytes = sizeof(ui16);
        fhdr.volume_size =
          fhdr.nx * fhdr.ny * fhdr.nz * fhdr.data_element_nbytes;
        
        fhdr.bad_data_value = 0;
        fhdr.missing_data_value = 0;
        
        fhdr.scale = channel->getCalSlope();
        fhdr.bias = channel->getCalOffset();

        outField = new MdvxField(fhdr, vhdr, channelFiles.getData());
        outField->setUnits("radiance");

        break;

      } // RADIANCE
        
      case Field::DEG_K:
      case Field::DEG_C: {
        
        fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
        fhdr.data_element_nbytes = sizeof(fl32);
        fhdr.volume_size =
          fhdr.nx * fhdr.ny * fhdr.nz * fhdr.data_element_nbytes;
        
        fhdr.bad_data_value = -9999.0;
        fhdr.missing_data_value = -9999.0;
        
        fhdr.scale = 1.0;
        fhdr.bias = 0.0;

        fl32 *btemp = new fl32[_nPointsGrid];
        double calSlope = channel->getCalSlope();
        double calOffset = channel->getCalOffset();
        const ui16 *counts = channelFiles.getData();
        for (int jj = 0; jj < _nPointsGrid; jj++) {
          if (counts[jj] == 0) {
            btemp[jj] = -9999.0;
          } else {
            double radiance = (double) counts[jj] * calSlope + calOffset;
            double deg = channel->temp(radiance);
            if (field->getOutputUnits() == Field::DEG_C) {
              deg += Channel::kelvinToCelsius;
            }  
            btemp[jj] = (fl32) deg;
          }
        } // jj
          
        outField = new MdvxField(fhdr, vhdr, btemp);
        delete[] btemp;
        
        if (field->getOutputUnits() == Field::DEG_K) {
          outField->setUnits("K");
        } else {
          outField->setUnits("C");
        }

        break;

      } // BRIGHTNESS TEMP

      case Field::ALBEDO: {
        
        fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
        fhdr.data_element_nbytes = sizeof(fl32);
        fhdr.volume_size =
          fhdr.nx * fhdr.ny * fhdr.nz * fhdr.data_element_nbytes;
        
        fhdr.bad_data_value = -9999.0;
        fhdr.missing_data_value = -9999.0;
        
        fhdr.scale = 1.0;
        fhdr.bias = 0.0;

        fl32 *albedo = new fl32[_nPointsGrid];
        double calSlope = channel->getCalSlope();
        double calOffset = channel->getCalOffset();
        const ui16 *counts = channelFiles.getData();
        double sunDist = _sunAngle.getDist();
        double toArad = channel->getSolarFactor() / (sunDist * sunDist);

        for (int jj = 0; jj < _nPointsGrid; jj++) {
          if (counts[jj] == 0) {
            albedo[jj] = -9999.0;
          } else {
            double radiance = (double) counts[jj] * calSlope + calOffset;
            double sinAlt = _sunAngle.computeSinAlt(_lat[jj], _lon[jj]);
            if (sinAlt < 0.1) {
              sinAlt = 0.1;
            }
            double reflectance = ((100.0 * radiance) / toArad) / sinAlt;
            if (reflectance > 100.0) {
              albedo[jj] = (fl32) 100.0;
            } else {
              albedo[jj] = (fl32) reflectance;
            }
          }
        } // jj
          
        outField = new MdvxField(fhdr, vhdr, albedo);
        delete[] albedo;
        outField->setUnits("%");
        
        break;

      } // BRIGHTNESS TEMP

      default: { // counts

        fhdr.encoding_type = Mdvx::ENCODING_INT16;
        fhdr.data_element_nbytes = sizeof(ui16);
        fhdr.volume_size =
          fhdr.nx * fhdr.ny * fhdr.nz * fhdr.data_element_nbytes;
        
        fhdr.bad_data_value = 0;
        fhdr.missing_data_value = 0;
        
        fhdr.scale = 1.0;
        fhdr.bias = 0.0;

        outField = new MdvxField(fhdr, vhdr, channelFiles.getData());
        outField->setUnits("count");

        break;

      } // COUNTS

    } // switch on output units

    // set names

    outField->setFieldName(field->getName().c_str());
    outField->setFieldNameLong(field->getName().c_str());
    outField->setTransform("none");

    // convert and compress

    outField->convertRounded(field->getOutputEncoding(),
                              Mdvx::COMPRESSION_GZIP);

    // add field to mdvx object
    
    mdvx.addField(outField);
    
  } // ifield
    
  // write output

  mdvx.setAppName(_progName);
  mdvx.setWriteLdataInfo();
  if (mdvx.writeToDir(_outputUrl)) {
    cerr << "ERROR - OutputFile::write" << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Wrote MDV file: " << mdvx.getPathInUse() << endl;
  }

  return 0;

}

