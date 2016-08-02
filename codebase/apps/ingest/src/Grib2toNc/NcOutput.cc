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
///////////////////////////////////////////////////
// NcOutput 
//
///////////////////////////////////////////////////

#include "NcOutput.hh"

#include <math.h>

#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/file_io.h>
#include <toolsa/utim.h> // For unix_to_date
#include <euclid/Pjg.hh>
#include <euclid/PjgLambertConfMath.hh>
#ifndef NOT_RAL
#include <toolsa/pmu.h>
#include <dsserver/DsLdataInfo.hh>
#endif

#include "Params.hh"

using namespace std;

// standard strings used by CF NetCDF

const char* NcOutput::units_missing = "-";
const char* NcOutput::_360_day = "360_day";
const char* NcOutput::_365_day = "365_day";
const char* NcOutput::_366_day = "366_day";
const char* NcOutput::FillValue = "_FillValue";
const char* NcOutput::add_offset = "add_offset";
const char* NcOutput::albers_conical_equal_area = "albers_conical_equal_area";
const char* NcOutput::all_leap = "all_leap";
const char* NcOutput::ancillary_variables = "ancillary_variables";
const char* NcOutput::area = "area";
const char* NcOutput::axis = "axis";
const char* NcOutput::azimuthal_equidistant = "azimuthal_equidistant";
const char* NcOutput::bounds = "bounds";
const char* NcOutput::calendar = "calendar";
const char* NcOutput::cell_measures = "cell_measures";
const char* NcOutput::cell_methods = "cell_methods";
const char* NcOutput::cf_version = "CF-1.6";
const char* NcOutput::comment = "comment";
const char* NcOutput::compress = "compress";
const char* NcOutput::conventions = "Conventions";
const char* NcOutput::coordinates = "coordinates";
const char* NcOutput::degrees = "degrees";
const char* NcOutput::degrees_east = "degrees_east";
const char* NcOutput::degrees_north = "degrees_north";
const char* NcOutput::detection_minimum = "detection_minimum";
const char* NcOutput::down = "down";
const char* NcOutput::earth_radius = "earth_radius";
const char* NcOutput::false_easting = "false_easting";
const char* NcOutput::false_northing = "false_northing";
const char* NcOutput::flag_meanings = "flag_meanings";
const char* NcOutput::flag_values = "flag_values";
const char* NcOutput::forecast_period = "forecast_period";
const char* NcOutput::forecast_period_long = "time interval between the forecast reference time and the valid time";
const char* NcOutput::forecast_reference_time = "forecast_reference_time";
const char* NcOutput::forecast_reference_time_long = "the time of the analysis from which the forecast was made";
const char* NcOutput::formula_terms = "formula_terms";
const char* NcOutput::gregorian = "gregorian";
const char* NcOutput::grid_latitude = "grid_latitude";
const char* NcOutput::grid_longitude = "grid_longitude";
const char* NcOutput::grid_mapping = "grid_mapping";
const char* NcOutput::grid_mapping_attribute = "grid_mapping_attribute";
const char* NcOutput::grid_mapping_name = "grid_mapping_name";
const char* NcOutput::grid_north_pole_latitude = "grid_north_pole_latitude";
const char* NcOutput::grid_north_pole_longitude = "grid_north_pole_longitude";
const char* NcOutput::history = "history";
const char* NcOutput::hybrid_level_standard_name = "atmosphere_hybrid_sigma_pressure_coordinate";
const char* NcOutput::institution = "institution";
const char* NcOutput::inverse_flattening = "inverse_flattening";
const char* NcOutput::julian = "julian";
const char* NcOutput::lambert_azimuthal_equal_area = "lambert_azimuthal_equal_area";
const char* NcOutput::lambert_conformal_conic = "lambert_conformal_conic";
const char* NcOutput::latitude = "latitude";
const char* NcOutput::latitude_longitude = "latitude_longitude";
const char* NcOutput::latitude_of_projection_origin = "latitude_of_projection_origin";
const char* NcOutput::layer = "layer";
const char* NcOutput::leap_month = "leap_month";
const char* NcOutput::leap_year = "leap_year";
const char* NcOutput::level = "level";
const char* NcOutput::long_name = "long_name";
const char* NcOutput::longitude = "longitude";
const char* NcOutput::longitude_of_central_meridian = "longitude_of_central_meridian";
const char* NcOutput::longitude_of_prime_meridian = "longitude_of_prime_meridian";
const char* NcOutput::longitude_of_projection_origin = "longitude_of_projection_origin";
const char* NcOutput::maximum = "maximum";
const char* NcOutput::mean = "mean";
const char* NcOutput::median = "median";
const char* NcOutput::mercator = "mercator";
const char* NcOutput::mid_range = "mid_range";
const char* NcOutput::minimum = "minimum";
const char* NcOutput::missing_value = "missing_value";
const char* NcOutput::mode = "mode";
const char* NcOutput::month_lengths = "month_lengths";
const char* NcOutput::noleap = "noleap";
const char* NcOutput::none = "none";
const char* NcOutput::number_of_observations = "number_of_observations";
const char* NcOutput::perspective_point_height = "perspective_point_height";
const char* NcOutput::point = "point";
const char* NcOutput::polar_radar = "polar_radar";
const char* NcOutput::polar_stereographic = "polar_stereographic";
const char* NcOutput::positive = "positive";
const char* NcOutput::projection_x_coordinate = "projection_x_coordinate";
const char* NcOutput::projection_y_coordinate = "projection_y_coordinate";
const char* NcOutput::proleptic_gregorian = "proleptic_gregorian";
const char* NcOutput::references = "references";
const char* NcOutput::reference_date = "reference_date";
const char* NcOutput::region = "region";
const char* NcOutput::rotated_latitude_longitude = "rotated_latitude_longitude";
const char* NcOutput::scale_factor = "scale_factor";
const char* NcOutput::scale_factor_at_central_meridian = "scale_factor_at_central_meridian";
const char* NcOutput::scale_factor_at_projection_origin = "scale_factor_at_projection_origin";
const char* NcOutput::seconds = "seconds";
const char* NcOutput::secs_since_jan1_1970 = "seconds since 1970-01-01T00:00:00Z";
const char* NcOutput::semi_major_axis = "semi_major_axis";
const char* NcOutput::semi_minor_axis = "semi_minor_axis";
const char* NcOutput::sigma_level = "sigma_level";
const char* NcOutput::source = "source";
const char* NcOutput::standard = "standard";
const char* NcOutput::standard_deviation = "standard_deviation";
const char* NcOutput::standard_error = "standard_error";
const char* NcOutput::standard_name = "standard_name";
const char* NcOutput::standard_parallel = "standard_parallel";
const char* NcOutput::start_time = "start_time";
const char* NcOutput::status_flag = "status_flag";
const char* NcOutput::stereographic = "stereographic";
const char* NcOutput::stop_time = "stop_time";
const char* NcOutput::straight_vertical_longitude_from_pole = "straight_vertical_longitude_from_pole";
const char* NcOutput::sum = "sum";
const char* NcOutput::time = "time";
const char* NcOutput::time_long = "data valid time";
const char* NcOutput::time_bounds = "time_bounds";
const char* NcOutput::title = "title";
const char* NcOutput::transverse_mercator = "transverse_mercator";
const char* NcOutput::units = "units";
const char* NcOutput::up = "up";
const char* NcOutput::valid_max = "valid_max";
const char* NcOutput::valid_min = "valid_min";
const char* NcOutput::valid_range = "valid_range";
const char* NcOutput::variance = "variance";
const char* NcOutput::vertical = "vertical";
const char* NcOutput::vertical_perspective = "vertical_perspective";
const char* NcOutput::volume = "volume";


NcOutput::NcOutput(Params *params, char* outputFile)
{
  _params = params;
  _outputFile = outputFile;
  _ncFile = NULL;
  _ncErr = NULL;
  _numUniqueGrid = 0;
  _numUniqueVertical = 0;
}

NcOutput::~NcOutput()
{
  clear();
}

void NcOutput::clear() 
{
  for (int i = 0; i < (int) _fieldData.size(); i++)
    delete [] _fieldData[i];
  _fieldData.clear();
  _fieldInfo.clear();
  _uniqueGrid.clear();
  _uniqueVertical.clear();
  _fieldVar.clear();
  _uniqueGridxDim.clear();
  _uniqueGridyDim.clear();
  _uniqueGridxVar.clear();
  _uniqueGridyVar.clear();
  _uniqueGridlatVar.clear();
  _uniqueGridlonVar.clear();
  _uniqueGridprojVar.clear();
  _uniqueVerticalzDim.clear();
  _uniqueVerticalzVar.clear();
  _numUniqueGrid = 0;
  _numUniqueVertical = 0;
}

int NcOutput::numFields()
{
  return _fieldData.size();
}

void NcOutput::addField(fl32 *field, Grib2Nc::FieldInfo fieldInfo)
{
   // Remap Data if requested
   if(_params->remap_output) {
     _remap(field, &fieldInfo);
   }
   
   //
   // Calculate unique VerticalInfo and GridInfo
   // These ints point to the fieldInfo that has the unique (the first instance of)
   // by setting it to .size()+1 if we can't find a previous match (ie we are the first) 
   // or by setting it to a matching previous' number.
   // Only this unique, first instance, of each will be added to the output and used.
   int uniqueVertical = _fieldInfo.size();
   int uniqueGrid = _fieldInfo.size();

   for (int i = 0; i < (int) _fieldInfo.size(); i++) {

     if(fieldInfo.gridInfo.ncfGridName.compare(_fieldInfo[i].gridInfo.ncfGridName) == 0 &&
	fieldInfo.gridInfo.nx == _fieldInfo[i].gridInfo.nx &&
	fieldInfo.gridInfo.ny == _fieldInfo[i].gridInfo.ny &&
	fieldInfo.gridInfo.dx == _fieldInfo[i].gridInfo.dx &&
	fieldInfo.gridInfo.dy == _fieldInfo[i].gridInfo.dy &&
	fieldInfo.gridInfo.minx == _fieldInfo[i].gridInfo.minx &&
	fieldInfo.gridInfo.miny == _fieldInfo[i].gridInfo.miny &&
	fieldInfo.gridInfo.lat1 == _fieldInfo[i].gridInfo.lat1 &&
	fieldInfo.gridInfo.lat2 == _fieldInfo[i].gridInfo.lat2 &&
	fieldInfo.gridInfo.pole_type == _fieldInfo[i].gridInfo.pole_type &&
	fieldInfo.gridInfo.tan_lon == _fieldInfo[i].gridInfo.tan_lon) 
     {
       uniqueGrid = _uniqueGrid[i];
     }

     if(fieldInfo.vlevelInfo.standardName.compare(_fieldInfo[i].vlevelInfo.standardName) == 0 &&
	fieldInfo.vlevelInfo.units.compare(_fieldInfo[i].vlevelInfo.units) == 0 &&
	fieldInfo.vlevelInfo.nz == _fieldInfo[i].vlevelInfo.nz)
     {
       bool matchedVerts = true;
       for(int z = 0; z < fieldInfo.vlevelInfo.nz; z++)
	 if(fieldInfo.vlevelInfo.zData[z] != _fieldInfo[i].vlevelInfo.zData[z])
	   matchedVerts = false;
       if(matchedVerts == true)
	 uniqueVertical = _uniqueVertical[i];
     }
   }

   if(uniqueGrid ==  _fieldInfo.size())
     _numUniqueGrid ++;
   if(uniqueVertical ==  _fieldInfo.size())
     _numUniqueVertical ++;

   _fieldInfo.push_back(fieldInfo);
   _fieldData.push_back(field);
   _uniqueGrid.push_back(uniqueGrid);
   _uniqueVertical.push_back(uniqueVertical);
}

int NcOutput::writeNc( time_t genTime, long int leadSecs )
{
#ifndef NOT_RAL
  PMU_auto_register("In NcOutput::writeNc");
#endif
  
  if(numFields() == 0) {
    cerr << "ERROR: No fields added" << endl << flush;
    return -1;
  }

  string outputDir = "";
  string relDir = "";
  char outputFile[1024];

  if(_outputFile == NULL)
  {
    int year, month, day, hour, minute, seconds;
    time_t fileTime = genTime;
    if(_params->output_filename == Params::CSS_WX_FILENAME)
      fileTime += leadSecs;
    
    UTIMstruct timeStruct;
    UTIMunix_to_date(fileTime, &timeStruct);
    year = timeStruct.year;
    month = timeStruct.month;
    day = timeStruct.day;
    hour = timeStruct.hour;
    minute = timeStruct.min;
    seconds = timeStruct.sec;

    //
    // compute output filename and directories    
    if (_params->write_to_day_dir || _params->output_filename == Params::RAP_FILENAME) {
      char dayStr[128];
      sprintf(dayStr, "%.4d%.2d%.2d", year, month, day);
      relDir = dayStr;
    }
       
    if(_params->output_filename == Params::ISO8601_FILENAME)
    {
      if (leadSecs > 0) { 
	int leadTimeHrs = leadSecs/3600;
	int leadTimeMins = (leadSecs % 3600 )/ 60;
	sprintf(outputFile, "%s%.4d-%.2d-%.2dT%.2d:%.2d:%.2d.PT%.2d:%.2d.nc",
		_params->output_basename, year, month, day, hour, minute, seconds,
		leadTimeHrs, leadTimeMins);
      } else {
	sprintf(outputFile, "%s%.4d-%.2d-%.2dT%.2d:%.2d:%.2d.nc",
		_params->output_basename, year, month, day, hour, minute, seconds);
      }
      
    } else if(_params->output_filename == Params::TDS_FILENAME)
    {
      if (leadSecs > 0) { 
	int leadTimeHrs = leadSecs/3600;
	int leadTimeMins = (leadSecs % 3600 )/ 60;
	sprintf(outputFile, "%s%.4d%.2d%.2d_%.2d%.2d%.2d_f%.2d%.2d.nc",
		_params->output_basename, year, month, day, hour, minute, seconds,
		leadTimeHrs, leadTimeMins);   
      } else {
	sprintf(outputFile, "%s%.4d%.2d%.2d_%.2d%.2d%.2d.nc",
		_params->output_basename, year, month, day, hour, minute, seconds);
      }
      
    } else if(_params->output_filename == Params::RAP_FILENAME)
    {
      if (leadSecs > 0) { 
	char g_hhmmss[25];
	sprintf(g_hhmmss, "g_%.2d%.2d%.2d", hour, minute, seconds);
	relDir += PATH_DELIM;
	relDir += g_hhmmss;
	sprintf(outputFile, "%sf_%08i.nc", _params->output_basename, leadSecs);

      } else {
	sprintf(outputFile, "%s%.2d%.2d%.2d.nc",
		_params->output_basename, hour, minute, seconds);
      }
    
    } else if(_params->output_filename == Params::CSS_WX_FILENAME)
    {
      sprintf(outputFile, "%s%.4d%.2d%.2d_v_%.2d%.2d%.2d_l_%07i.nc",
	      _params->output_basename,
	      year, month, day, hour, minute, seconds,
	      leadSecs);   

    }

    outputDir = _params->output_dir;
        
  } else
  {
    // Output file name specified
    // Figure out directory and file name
    string file = _outputFile;
    size_t delimPos = file.rfind( "/" );

    if(delimPos == file.length()-1) {
      cerr << "ERROR: Output directory given where outputfile name expected" << endl;
      return -1;
    } else 
      if( delimPos > file.length() ) {
	// no directory given, current directoy implied
	strncpy(outputFile, _outputFile, 1023);
	outputDir = ".";
      } else {
	strncpy(outputFile, file.substr( delimPos+1 ).c_str(), 1023);
	outputDir  = file.substr( 0, delimPos );
      }

    relDir = "";

  }

  string fullPath = outputDir;

  if(relDir != "") {
    fullPath += PATH_DELIM;
    fullPath += relDir;
  }

  //
  // ensure output dir exists  
  if (ta_makedir_recurse(fullPath.c_str())) {
    cerr << "ERROR - Cannot make output dir: " << fullPath << endl;
    return -1;
  }

  fullPath += PATH_DELIM;
  fullPath += outputFile;

  if (_openNcFile(fullPath))
    return -1;

  if (_writeNcFile(genTime, leadSecs))
    return -1;

  _closeNcFile();
  
  cout << "File written: " << fullPath << endl;

  // latest data info
#ifndef NOT_RAL
  if ( _params->writeLdataInfo ) {
    string relPath;
    if(relDir != "") {
      relPath = relDir;
      relPath += PATH_DELIM;
      relPath += outputFile;
    } else {
      relPath = outputFile;
    }

    DsLdataInfo ldata(outputDir, _params->debug);

    ldata.setWriter("Grib2toNc");
    ldata.setDataFileExt("nc");
    ldata.setDataType("netCDF");
    ldata.setRelDataPath(relPath);
    if (leadSecs > 0)
      ldata.setIsFcast(true);
    ldata.setLeadTime(leadSecs);
    ldata.write(genTime);
  }
#endif

  // Clean up
  clear();
  
  return 0;
}

//////////////////////////////////////////////
// open netcdf file
// create error object so we can handle errors
int NcOutput::_openNcFile(const string &path)
  
{

  _closeNcFile();

  NcFile::FileFormat ncFormat = (NcFile::FileFormat) _params->file_format;
  _ncFile = new NcFile(path.c_str(), NcFile::Replace, NULL, 0, ncFormat);
  
  if (!_ncFile || !_ncFile->is_valid()) {
    cerr << "ERROR - Cannot open netCDF file: " << path << endl;
    if (_ncFile) {
      delete _ncFile;
      _ncFile = NULL;
    }
    return -1;
  }
  _ncFile->set_fill(NcFile::NoFill);

  // Change the error behavior of the netCDF C++ API by creating an
  // NcError object. Until it is destroyed, this NcError object will
  // ensure that the netCDF C++ API silently returns error codes
  // on any failure, and leaves any other error handling to the
  // calling program.
  
  _ncErr = new NcError(NcError::silent_nonfatal);
 
  return 0;

}

//////////////////////////////////////
// close netcdf file if open
void NcOutput::_closeNcFile()
  
{
  
  // close file if open, delete ncFile
  
  if (_ncFile) {
    _ncFile->close();
    delete _ncFile;
    _ncFile = NULL;
  }

  if (_ncErr) {
    delete _ncErr;
    _ncErr = NULL;
  }

}

//////////////////////////////////////
// write all data to the open netcdf file
int NcOutput::_writeNcFile(time_t genTime, long int leadSecs)
{
  if(_ncFile == NULL)
    return -1;

  if(_addGlobalAttributes())
    return -1;
  if(_addDimensions())
    return -1;
  if(_addTimeVariables(genTime, leadSecs))
    return -1;
  if(_addCoordinateVariables())
    return -1;
  if(_addProjectionVariables())
    return -1;
  if(_addFieldDataVariables())
    return -1;

  if(_putTimeVariables(genTime, leadSecs))
    return -1;
  if(_putCoordinateVariables())
    return -1;
  if(_putFieldDataVariables())
    return -1;

  return 0;
}

int NcOutput::_addGlobalAttributes()
{
  int iret = 0;

  // Add CF global attributes
  iret |= !_ncFile->add_att(NcOutput::conventions , NcOutput::cf_version);  
  if (_fieldInfo[0].generatingCenter.size() > 1 && strlen(_params->data_set_source) < 1) {
    iret |= !_ncFile->add_att(NcOutput::source , _fieldInfo[0].generatingCenter.c_str());
  } else {
    iret |= !_ncFile->add_att(NcOutput::source , _params->data_set_source);
  }
  if (_fieldInfo[0].generatingProcess.size() > 1 && strlen(_params->data_set_title) < 1) {
    iret |= !_ncFile->add_att(NcOutput::title , _fieldInfo[0].generatingProcess.c_str());    
  } else {
    iret |= !_ncFile->add_att(NcOutput::title , _params->data_set_title);
  }
  iret |= !_ncFile->add_att(NcOutput::history , _params->data_set_info);

  if(iret != 0) {
    cerr << "ERROR - Cannot add Global Attributes" << endl;
    cerr << _ncErr->get_errmsg() << endl;
    return -1;
  }
  return 0;

}

///////////////////////////////////////////////////////////////////////////
// addDimensions()
//
//  Add NcDims to the NetCDF file. We loop through the
//  GridInfo objects and record the unique dimensions of the
//  x and y coordinates. Then we loop through the VlevelInfo
//  objects and record the unique dimensions of the vertical coordinates
int NcOutput::_addDimensions()
{
  if (!(_timeDim = _ncFile->add_dim(NcOutput::time, 1))) {
    cerr << "ERROR - Cannot add " <<NcOutput::time << " grid dimension" << endl;
    cerr << _ncErr->get_errmsg() << endl;
    return -1;
  }

  //if (!(_boundsDim = _ncFile->add_dim(NcOutput::bounds, 2))) {
  //return -1;
  //}

  //
  // Add dimensions of the unique gridInfos
  int gridNum = 0;
  char xDimName[4],  yDimName[4];  
  for (int i = 0; i < (int) _uniqueGrid.size(); i++) 
  {
    if(_uniqueGrid[i] == i)
    {
      NcDim* xDim;
      NcDim* yDim;
      if(_numUniqueGrid > 1) {
	sprintf(xDimName, "x%d", gridNum);
	sprintf(yDimName, "y%d", gridNum);
      } else {
	sprintf(xDimName, "x");
	sprintf(yDimName, "y");
      }

      if (!(xDim = _ncFile->add_dim(xDimName, _fieldInfo[i].gridInfo.nx))) {
	cerr << "ERROR - Cannot add " << xDimName << " grid dimension" << endl;
	cerr << _ncErr->get_errmsg() << endl;
	return -1;
      }
      if (!(yDim = _ncFile->add_dim(yDimName, _fieldInfo[i].gridInfo.ny))) {
	cerr << "ERROR - Cannot add " << yDimName << " grid dimension" << endl;
	cerr << _ncErr->get_errmsg() << endl;
	return -1;
      }
      _uniqueGridxDim.push_back(xDim);
      _uniqueGridyDim.push_back(yDim);
      gridNum++;
    } else {
      _uniqueGridxDim.push_back(NULL);
      _uniqueGridyDim.push_back(NULL);
    }
  }

  //
  // Add dimensions of unique vertical levels
  int vlevelNum = 0;
  char zDimName[4];
  for (int i = 0; i <  (int) _uniqueVertical.size(); i++) 
  {
    if(_uniqueVertical[i] == i)
    {
      NcDim* zDim;
      if(_numUniqueVertical > 1)
	sprintf(zDimName, "z%d", vlevelNum);
      else
	sprintf(zDimName, "z");

      if(!(zDim = _ncFile->add_dim(zDimName, _fieldInfo[i].vlevelInfo.nz))) {
	cerr << "ERROR - Cannot add " << zDimName << " grid dimension" << endl;
	cerr << _ncErr->get_errmsg() << endl;
	return -1;
      }
      _uniqueVerticalzDim.push_back(zDim);
      vlevelNum++;
    } else {
      _uniqueVerticalzDim.push_back(NULL);
    }
  }

  return 0;
}

//////////////////////////////////////////////
// add variables for times
int NcOutput::_addTimeVariables(time_t genTime, long int leadSecs)
{

  int iret = 0;

  string timeVarName("time");

  if (!(_timeVar = _ncFile->add_var(timeVarName.c_str(), ncDouble, _timeDim))) {
    cerr << "ERROR - Cannot add " << timeVarName << " variable" << endl;
    cerr << _ncErr->get_errmsg() << endl;
    return -1;
  }
  iret |= !_timeVar->add_att(NcOutput::standard_name, NcOutput::time);
  iret |= !_timeVar->add_att(NcOutput::long_name, NcOutput::time_long);
  iret |= !_timeVar->add_att(NcOutput::units, NcOutput::secs_since_jan1_1970);
  iret |= !_timeVar->add_att(NcOutput::calendar, NcOutput::gregorian);
  iret |= !_timeVar->add_att(NcOutput::axis, "T");
  //iret |= !_timeVar->add_att(NcOutput::bounds, "time_bounds");

  UTIMstruct timeStruct;
  UTIMunix_to_date(genTime+leadSecs, &timeStruct);
  char date_str[32];
  sprintf(date_str, "%.4d-%.2d-%.2dT%.2d:%.2d:%.2dZ",
          timeStruct.year, timeStruct.month, timeStruct.day,
          timeStruct.hour, timeStruct.min, timeStruct.sec);
  iret |= !_timeVar->add_att(NcOutput::reference_date , date_str);

  if(iret != 0) {
    cerr << "ERROR - Failed to add a variable attribute for " << timeVarName << endl;
    cerr << _ncErr->get_errmsg() << endl;
    return -1;
  }

  if (leadSecs > 0) {

    // forecast period or lead time    
    if (!(_forecastPeriodVar = _ncFile->add_var(NcOutput::forecast_period, ncDouble, _timeDim))) 
    {
      cerr << "ERROR - Cannot add " << NcOutput::forecast_period << " variable" << endl;
      cerr << _ncErr->get_errmsg() << endl;
      return -1;
    }
    
    iret |= !_forecastPeriodVar->add_att(NcOutput::standard_name, NcOutput::forecast_period);
    iret |= !_forecastPeriodVar->add_att(NcOutput::long_name, NcOutput::forecast_period_long);
    iret |= !_forecastPeriodVar->add_att(NcOutput::units, NcOutput::seconds);
    if(iret != 0) {
      cerr << "ERROR - Failed to add a variable attribute for " << NcOutput::forecast_period << endl;
      cerr << _ncErr->get_errmsg() << endl;
      return -1;
    }

    // Forecast reference time
    if (!(_forecastReferenceVar = _ncFile->add_var(NcOutput::forecast_reference_time, ncDouble, _timeDim))) 
    {
      cerr << "ERROR - Cannot add " << NcOutput::forecast_reference_time << " variable" << endl;
      cerr << _ncErr->get_errmsg() << endl;
      return -1;
    }
    
    iret |= !_forecastReferenceVar->add_att(NcOutput::standard_name, NcOutput::forecast_reference_time);
    iret |= !_forecastReferenceVar->add_att(NcOutput::long_name, NcOutput::forecast_reference_time_long);
    iret |= !_forecastReferenceVar->add_att(NcOutput::units, NcOutput::secs_since_jan1_1970);
    iret |= !_forecastReferenceVar->add_att(NcOutput::calendar, NcOutput::gregorian);

    UTIMunix_to_date(genTime, &timeStruct);
    sprintf(date_str, "%.4d-%.2d-%.2dT%.2d:%.2d:%.2dZ",
	    timeStruct.year, timeStruct.month, timeStruct.day,
	    timeStruct.hour, timeStruct.min, timeStruct.sec);
    iret |= !_forecastReferenceVar->add_att(NcOutput::reference_date , date_str);

    if(iret != 0) {
      cerr << "ERROR - Failed to add a variable attribute for " << NcOutput::forecast_reference_time << endl;
      cerr << _ncErr->get_errmsg() << endl;
      return -1;
    }

  } else {
    _forecastPeriodVar = NULL;
    _forecastReferenceVar = NULL;
  }
  
  return 0;

}

////////////////////////////////////////////////
// add variables and attributes for unique grid coordinates
int NcOutput::_addCoordinateVariables()
{

  // Note that coordinate variables have the same name as their dimension
  // so we will use the same naming scheme for vars as we did for dimensions
  // in method _addDimensions()

  int gridNum = 0;
  char xVarName[4],  yVarName[4];
  NcVar* xVar;
  NcVar* yVar;
  NcVar* latVar;
  NcVar* lonVar;
  for (int i = 0; i < (int) _uniqueGrid.size(); i++) 
  {
    latVar = NULL;
    lonVar = NULL;
    if(_uniqueGrid[i] == i)
    {
      if(_numUniqueGrid > 1) {
	sprintf(xVarName, "x%d", gridNum);
	sprintf(yVarName, "y%d", gridNum);
      } else {
	sprintf(xVarName, "x");
	sprintf(yVarName, "y");
      }

      NcDim *xDim = _uniqueGridxDim[_uniqueGrid[i]];
      NcDim *yDim = _uniqueGridyDim[_uniqueGrid[i]];
      
      if(xDim == NULL) {
	cerr << "ERROR - Cannot find unique netcdf dimensions matching variable " << xVarName << endl;
	return -1;
      }  

      if(yDim == NULL) {
	cerr << "ERROR - Cannot find unique netcdf dimensions matching variable " << yVarName << endl;
	return -1;
      }  

      if(!(xVar = _ncFile->add_var(xVarName, ncFloat, xDim))) {
	cerr << "ERROR - Cannot add " << xVarName << " variable" << endl;
	cerr << _ncErr->get_errmsg() << endl;
	return -1;
      }

      if(!(yVar = _ncFile->add_var(yVarName, ncFloat, yDim))) {
	cerr << "ERROR - Cannot add " << yVarName << " variable" << endl;
	cerr << _ncErr->get_errmsg() << endl;
	return -1;
      }
      
      int iret = 0;
      // Add attributes of coordinate variables
      if (_fieldInfo[i].gridInfo.ncfGridName.compare(NcOutput::latitude_longitude) == 0) {
	
	// Basic lat/lon grid
	iret |= !xVar->add_att(NcOutput::standard_name, NcOutput::longitude);
	iret |= !xVar->add_att(NcOutput::long_name, NcOutput::longitude);
	iret |= !xVar->add_att(NcOutput::units, NcOutput::degrees_east);
	if(iret != 0) {
	  cerr << "ERROR - Failed to add a coordinate variable attribute for " << xVarName << endl;
	  cerr << _ncErr->get_errmsg() << endl;
	  return -1;
	}

	iret |= !yVar->add_att(NcOutput::standard_name, NcOutput::latitude);
	iret |= !yVar->add_att(NcOutput::long_name, NcOutput::latitude);
	iret |= !yVar->add_att(NcOutput::units, NcOutput::degrees_north);
	if(iret != 0) {
	  cerr << "ERROR - Failed to add a coordinate variable attribute for " << yVarName << endl;
	  cerr << _ncErr->get_errmsg() << endl;
	  return -1;
	}
	_uniqueGridlatVar.push_back(NULL);
	_uniqueGridlonVar.push_back(NULL);

      } else {
	
	// Any other grid type
	iret |= !xVar->add_att(NcOutput::standard_name, NcOutput::projection_x_coordinate);
	iret |= !xVar->add_att(NcOutput::units, "km");
	if(iret != 0) {
	  cerr << "ERROR - Failed to add a coordinate variable attribute for " << xVarName << endl;
	  cerr << _ncErr->get_errmsg() << endl;
	  return -1;
	}
	iret |= !yVar->add_att(NcOutput::standard_name, NcOutput::projection_y_coordinate);
	iret |= !yVar->add_att(NcOutput::units, "km");
	if(iret != 0) {
	  cerr << "ERROR - Failed to add a coordinate variable attribute for " << yVarName << endl;
	  cerr << _ncErr->get_errmsg() << endl;
	  return -1;
	}
	
	// Add auxiliary variables to NcFile if grids are not lat lon grids
  
	char latVarName[7],  lonVarName[7];

	if(_numUniqueGrid > 1) {
	  sprintf(latVarName, "lat%d", gridNum);
	  sprintf(lonVarName, "lon%d", gridNum);
	} else {
	  sprintf(latVarName, "lat");
	  sprintf(lonVarName, "lon");
	}

	if ((latVar = _ncFile->add_var(latVarName, ncFloat,
				       yDim, xDim)) == NULL) {
	  cerr << "ERROR - Cannot add " << latVarName << " variable" << endl;
	  cerr << _ncErr->get_errmsg() << endl;
	  return -1;
	}
	if ((lonVar = _ncFile->add_var(lonVarName, ncFloat,
				       yDim, xDim)) == NULL) {
	  cerr << "ERROR - Cannot add " << lonVarName << " variable" << endl;
	  cerr << _ncErr->get_errmsg() << endl;
	  return -1;
	}
	
	iret |= !latVar->add_att(NcOutput::standard_name, NcOutput::latitude);
	iret |= !latVar->add_att(NcOutput::units, NcOutput::degrees_north);
	if(iret != 0) {
	  cerr << "ERROR - Failed to add a coordinate variable attribute for " << latVarName << endl;
	  cerr << _ncErr->get_errmsg() << endl;
	  return -1;
	}
	iret |= !lonVar->add_att(NcOutput::standard_name, NcOutput::longitude);
	iret |= !lonVar->add_att(NcOutput::units, NcOutput::degrees_east);
	if(iret != 0) {
	  cerr << "ERROR - Failed to add a coordinate variable attribute for " << lonVarName << endl;
	  cerr << _ncErr->get_errmsg() << endl;
	  return -1;
	}
	_uniqueGridlatVar.push_back(latVar);
	_uniqueGridlonVar.push_back(lonVar);

      } // end else (not a PROJ_LATLON)
      
      iret |= !xVar->add_att(NcOutput::axis, "X");
      if(iret != 0) {
	cerr << "ERROR - Failed to add a coordinate variable attribute for " << xVarName << endl;
	cerr << _ncErr->get_errmsg() << endl;
	return -1;
      }
      iret |= !yVar->add_att(NcOutput::axis, "Y");
      if(iret != 0) {
	cerr << "ERROR - Failed to add a coordinate variable attribute for " << yVarName << endl;
	cerr << _ncErr->get_errmsg() << endl;
	return -1;
      }
      _uniqueGridxVar.push_back(xVar);
      _uniqueGridyVar.push_back(yVar);
      gridNum++;
    } else {
      _uniqueGridxVar.push_back(NULL);
      _uniqueGridyVar.push_back(NULL);
      _uniqueGridlatVar.push_back(NULL);
      _uniqueGridlonVar.push_back(NULL);

    }
  } // end unique grids loop

  // Add vertical coordinate variables
  int vlevelNum = 0;
  char zDimName[4];
  char zVarName[4];
  NcVar* zVar;
  for (int i = 0; i <  (int) _uniqueVertical.size(); i++) 
  {
    if(_uniqueVertical[i] == i)
    {
      if(_numUniqueVertical > 1)
	sprintf(zVarName, "z%d", vlevelNum);
      else
	sprintf(zVarName, "z");

      NcDim *zDim = _uniqueVerticalzDim[_uniqueVertical[i]];

      if(zDim == NULL) {
	cerr << "ERROR - Cannot find unique netcdf dimensions matching variable " << zVarName << endl;
	return -1;
      }  

      if ((zVar = _ncFile->add_var(zVarName, ncFloat, zDim)) == NULL) {
	cerr << "ERROR - Cannot add " << zVarName << " variable" << endl;
	cerr << _ncErr->get_errmsg() << endl;
	return -1;
      }
      
      // Add Attributes
      int iret = 0;      
      if (_fieldInfo[i].vlevelInfo.standardName.size() > 0) {
	iret |= !zVar->add_att(NcOutput::standard_name, _fieldInfo[i].vlevelInfo.standardName.c_str());
      }
      
      iret |= !zVar->add_att(NcOutput::long_name, _fieldInfo[i].vlevelInfo.longName.c_str());
      
      if(_fieldInfo[i].vlevelInfo.units.compare(units_missing) != 0) {
	iret |= !zVar->add_att(NcOutput::units, _fieldInfo[i].vlevelInfo.units.c_str());
      }

      if (_fieldInfo[i].vlevelInfo.positive.size() > 0) {
	iret |= !zVar->add_att(NcOutput::positive, _fieldInfo[i].vlevelInfo.positive.c_str());
      }
      
      iret |= !zVar->add_att(NcOutput::axis, "Z");

      if(iret != 0) {
	cerr << "ERROR - Failed to add a coordinate variable attribute for " << zVarName << endl;
	cerr << _ncErr->get_errmsg() << endl;
	return -1;
      }
      _uniqueVerticalzVar.push_back(zVar);
      vlevelNum++;
    } else {
      _uniqueVerticalzVar.push_back(NULL);
    }
  } // end unique vertical info loop


  return 0;

}

////////////////////////////////////////////////////////////
// add the unique nc projection variables
int NcOutput::_addProjectionVariables()
{

  int gridNum = 0;
  char projVarName[16];
  NcVar* projVar;
  for (int i = 0; i < (int) _uniqueGrid.size(); i++) 
  {
    if(_uniqueGrid[i] == i)
    {

      if(_numUniqueGrid > 1)
	sprintf(projVarName, "%s_%d", NcOutput::grid_mapping, gridNum);
      else
	sprintf(projVarName, "%s", NcOutput::grid_mapping);
      
      if((projVar = _ncFile->add_var(projVarName, ncInt)) == NULL) 
      {
	cerr << "ERROR - Cannot add " << projVarName << " variable" << endl;
	cerr << _ncErr->get_errmsg() << endl;
	return -1;
      }

      int iret = 0;

      if(_fieldInfo[i].gridInfo.ncfGridName.compare(NcOutput::latitude_longitude) == 0) 
      {
	iret |= !projVar->add_att(NcOutput::grid_mapping_name, NcOutput::latitude_longitude);

      } else if(_fieldInfo[i].gridInfo.ncfGridName.compare(NcOutput::lambert_conformal_conic) == 0)
	{

	  iret |= !projVar->add_att(NcOutput::grid_mapping_name, NcOutput::lambert_conformal_conic);
	  iret |= !projVar->add_att(NcOutput::longitude_of_central_meridian, _fieldInfo[i].gridInfo.proj_origin_lon);
	  iret |= !projVar->add_att(NcOutput::latitude_of_projection_origin, _fieldInfo[i].gridInfo.proj_origin_lat);

	  float parallels[2];
	  parallels[0] = _fieldInfo[i].gridInfo.lat1;
	  parallels[1] = _fieldInfo[i].gridInfo.lat2;
	  if (_fieldInfo[i].gridInfo.lat1 == _fieldInfo[i].gridInfo.lat2) {
	    iret |= !projVar->add_att(NcOutput::standard_parallel, 1, parallels);
	  } else {
	    iret |= !projVar->add_att(NcOutput::standard_parallel, 2, parallels);
	  }
	  //iret |= !projVar->add_att(NcOutput::false_easting, _fieldInfo[i].gridInfo.false_easting);
	  //iret |= !projVar->add_att(NcOutput::false_northing, _fieldInfo[i].gridInfo.false_northing);

	} else if(_fieldInfo[i].gridInfo.ncfGridName.compare(NcOutput::polar_stereographic) == 0)
	{

	  iret |= !projVar->add_att(NcOutput::grid_mapping_name, NcOutput::polar_stereographic);
	  iret |= !projVar->add_att(NcOutput::straight_vertical_longitude_from_pole, _fieldInfo[i].gridInfo.tan_lon);
	  if(_fieldInfo[i].gridInfo.pole_type == 0) {
	    iret |= !projVar->add_att(NcOutput::latitude_of_projection_origin, 90.0);
	  } else {
	    iret |= !projVar->add_att(NcOutput::latitude_of_projection_origin, -90.0);
	  }
	  iret |= !projVar->add_att(NcOutput::scale_factor_at_projection_origin, _fieldInfo[i].gridInfo.central_scale);
	  //iret |= !projVar->add_att(NcOutput::false_easting, _fieldInfo[i].gridInfo.false_easting);
	  //iret |= !projVar->add_att(NcOutput::false_northing, _fieldInfo[i].gridInfo.false_northing);
	  
	} else if(_fieldInfo[i].gridInfo.ncfGridName.compare(NcOutput::mercator) == 0)
	{

	  iret |= !projVar->add_att(NcOutput::grid_mapping_name, NcOutput::mercator);
	  iret |= !projVar->add_att(NcOutput::longitude_of_projection_origin, _fieldInfo[i].gridInfo.proj_origin_lon);
	  iret |= !projVar->add_att(NcOutput::standard_parallel, _fieldInfo[i].gridInfo.proj_origin_lat);
	  //iret |= !projVar->add_att(NcOutput::false_easting, _fieldInfo[i].gridInfo.false_easting);
	  //iret |= !projVar->add_att(NcOutput::false_northing, _fieldInfo[i].gridInfo.false_northing);
	  
	}

      if(iret != 0) {
	cerr << "ERROR - Failed to add a projection variable attribute for " << projVarName << endl;
	cerr << _ncErr->get_errmsg() << endl;
	return -1;
      }
      _uniqueGridprojVar.push_back(projVar);
    } else {
      _uniqueGridprojVar.push_back(NULL);
    }
  }

  return 0;

}

int NcOutput::_addFieldDataVariables()
{

  int iret = 0;

  for (int i = 0; i < (int) _fieldInfo.size(); i++) 
  {

    if (_fieldInfo[i].name.size() < 1) {
      cerr << "ERROR - Cannot add variable to Nc file object, Field name is zero-length" << endl;
      return -1;
    }

    // Get the matching unique gridInfo and vlevelInfo dimensions
    NcDim *zDim = _uniqueVerticalzDim[_uniqueVertical[i]];
    NcDim *xDim = _uniqueGridxDim[_uniqueGrid[i]];
    NcDim *yDim = _uniqueGridyDim[_uniqueGrid[i]];

    if(zDim == NULL || xDim == NULL || yDim == NULL) {
      cerr << "ERROR - Cannot find unique netcdf dimensions matching variable " << _fieldInfo[i].name << endl;
      return -1;
    }

    // check that the field name is CF-netCDF compliant - 
    // i.e must start with a letter
    //   if not, add "nc_" to start of name
    // and must only contain letters, digits and underscores
    
    string fieldName;
    if (isalpha(_fieldInfo[i].name[0])) {
      fieldName = _fieldInfo[i].name;
    } else {
      fieldName = "nc_";
      fieldName += _fieldInfo[i].name;
    }
    for (int ii = 0; ii < (int) fieldName.size(); ii++) {
      if (!isalnum(fieldName[ii]) && fieldName[ii] != '_') {
	fieldName[ii] = '_';
      }
    }
    
    // Add NcVar to NcFile object and the attributes relevant to no data packing
    if (_params->debug) {
      cerr << "adding field: " << fieldName << endl;
    }
    
    NcType ncType = ncFloat;
    if(_fieldInfo[i].ncType == Params::DATA_PACK_BYTE)
      ncType = ncByte;
    if(_fieldInfo[i].ncType == Params::DATA_PACK_SHORT)
      ncType = ncShort;

    NcVar *fieldVar = _ncFile->add_var(fieldName.c_str(), ncType, _timeDim, zDim, yDim, xDim);

    if (fieldVar == NULL) {
      cerr << "ERROR - Cannot add variable '" << fieldName << "' to Nc file object" << endl;
      cerr << _ncErr->get_errmsg() << endl;
      if(_timeDim == NULL)
	cerr << "\ttimeDim: NULL" << endl;
      else
	cerr << "\ttimeDim: '" << _timeDim->name() << "' size: " << _timeDim->size() << endl;
      if(zDim == NULL)
	cerr << "\tzDim: NULL" << endl;
      else
	cerr << "\tzDim: '" << zDim->name() << "' size: " << zDim->size() << endl;
      if(yDim == NULL)
	cerr << "\tyDim: NULL" << endl;
      else
	cerr << "\tyDim: '" << yDim->name() << "' size: " << yDim->size() << endl;
      if(xDim == NULL)
	cerr << "\txDim: NULL" << endl;
      else
	cerr << "\txDim: '" << xDim->name() << "' size: " << xDim->size() << endl;
      return -1;
    }
  
    // data packing scheme for non-floats
    // We map the valid range (_minOut, _maxOut) to ( -2^(n-1)+ 1, 2^(n-1) -1)
    // and leave -2^(n-1) for the fill value.
    //
    // add_offset = (_maxOut + _minOut)/2
    // scale_factor = (_maxOut - _minOut)/(2^n - 2)
    // packedVal = (unpacked - offset)/scaleFactor
    // where n is the number of bits of the packed (integer) data type
    
    if (_fieldInfo[i].ncType == Params::DATA_PACK_SHORT) {
      
      iret |= !fieldVar->add_att(NcOutput::scale_factor, _fieldInfo[i].scaleFactor);
      iret |= !fieldVar->add_att(NcOutput::add_offset, _fieldInfo[i].addOffset);
      iret |= !fieldVar->add_att(NcOutput::valid_min, (si16)_fieldInfo[i].min_value);
      iret |= !fieldVar->add_att(NcOutput::valid_max, (si16)_fieldInfo[i].max_value);
      iret |= !fieldVar->add_att(NcOutput::FillValue, (si16)_fieldInfo[i].missing);
      
    } else if (_fieldInfo[i].ncType == Params::DATA_PACK_BYTE) {
      
      iret |= !fieldVar->add_att(NcOutput::scale_factor, _fieldInfo[i].scaleFactor);
      iret |= !fieldVar->add_att(NcOutput::add_offset, _fieldInfo[i].addOffset);
      iret |= !fieldVar->add_att(NcOutput::valid_min, (ncbyte)_fieldInfo[i].min_value);
      iret |= !fieldVar->add_att(NcOutput::valid_max, (ncbyte)_fieldInfo[i].max_value);
      iret |= !fieldVar->add_att(NcOutput::FillValue, (ncbyte)_fieldInfo[i].missing);
      
    } else {
      
      //iret |= !fieldVar->add_att(NcOutput::scale_factor, _fieldInfo[i].scaleFactor);
      //iret |= !fieldVar->add_att(NcOutput::add_offset, _fieldInfo[i].addOffset);
      iret |= !fieldVar->add_att(NcOutput::valid_min, _fieldInfo[i].min_value);
      iret |= !fieldVar->add_att(NcOutput::valid_max, _fieldInfo[i].max_value);
      iret |= !fieldVar->add_att(NcOutput::FillValue, _fieldInfo[i].missing);
      
    }
    
    if(_fieldInfo[i].standardName.size() > 0) {
      iret |= !fieldVar->add_att(NcOutput::standard_name, _fieldInfo[i].standardName.c_str());
    }
    
    iret |= !fieldVar->add_att(NcOutput::long_name, _fieldInfo[i].nameLong.c_str());
    if(_fieldInfo[i].units.compare(units_missing) != 0) {
      iret |= !fieldVar->add_att(NcOutput::units, _fieldInfo[i].units.c_str());
    }    

    // Add auxiliary variables if necessay
    if(_fieldInfo[i].gridInfo.ncfGridName.compare(NcOutput::latitude_longitude) != 0) 
    {
	
      char auxVarNames[1024];
      NcVar *latVar = _uniqueGridlatVar[_uniqueGrid[i]];
      NcVar *lonVar = _uniqueGridlonVar[_uniqueGrid[i]];  
      NcVar *projVar = _uniqueGridprojVar[_uniqueGrid[i]];  
      if(latVar == NULL || lonVar == NULL || projVar == NULL) {
	cerr << "ERROR - Cannot find unique lat/lon proj variables matching variable " << _fieldInfo[i].name << endl;
	return -1;
      }

      sprintf(auxVarNames, "%s %s", lonVar->name(), latVar->name());
      iret |= !fieldVar->add_att(NcOutput::coordinates, auxVarNames);
      
      sprintf(auxVarNames, "%s", projVar->name());
      iret |= !fieldVar->add_att(NcOutput::grid_mapping, auxVarNames);


    }

    if(iret != 0) {
      cerr << "ERROR - Failed to add variable attributes for " << _fieldInfo[i].name << endl;
      cerr << _ncErr->get_errmsg() << endl;
      return -1;
    }

    // Set compression if we are working with netCDF4 hdf files

    if (_params->compress_data &&
	(_params->file_format == Params::NETCDF4 || _params->file_format == Params::NETCDF4_CLASSIC)) {

      int varId, fileId, shuffle, deflateControl, deflateLevel, newLevel;
      fileId = _ncFile->id();
      varId = fieldVar->id();
      deflateLevel = _params->compression_level;
      
      nc_inq_var_deflate(fileId, varId, &shuffle, &deflateControl, &newLevel);
      
      deflateControl = 1;
      newLevel = deflateLevel; 
      
      int iret = nc_def_var_deflate(fileId, varId, shuffle,
				    deflateControl, newLevel);
      if (iret != NC_NOERR) {
	cerr << "ERROR - Failed setting compression for variable: " << _fieldInfo[i].name << endl;
	cerr << _ncErr->get_errmsg() << endl;
	return -1;
      }
    }

    _fieldVar.push_back(fieldVar);

  }

  return 0;
}

/////////////////////////////////////////////////////
// Write the time variables data to the NcFile
int NcOutput::_putTimeVariables(time_t genTime, long int leadSecs)
{

  int iret = 0;

  if(_timeVar == NULL) {
    cerr << "ERROR - Cannot put data for time var as it is NULL" << endl;
    return -1;
  }

  // Put the time data
  double time = (double) genTime + leadSecs;
  if(!(_timeVar->put(&time,1))) {
    cerr << "ERROR - Cannot put data for variable " << _timeVar->name() << endl;
    cerr << _ncErr->get_errmsg() << endl;
    return -1;
  }

  if (leadSecs> 0) {
    
    // forecast time
    
    double leadTime = (double) leadSecs;

    if(!(_forecastPeriodVar->put(&leadTime,1))) {
      cerr << "ERROR - Cannot put data for variable " << _forecastPeriodVar->name() << endl;
      cerr << _ncErr->get_errmsg() << endl;
      return -1;
    }

    double referenceTime = (double) genTime;

    if(!(_forecastReferenceVar->put(&genTime,1))) {
      cerr << "ERROR - Cannot put data for variable " << _forecastReferenceVar->name() << endl;
      cerr << _ncErr->get_errmsg() << endl;
      return -1;
    }
  }

  return iret;
}

///////////////////////////////////////
// Write the unique coordinate vars to the NcFile
int NcOutput::_putCoordinateVariables()
{

  // Derive and then write the unique coordinate variable data
  for (int i = 0; i < (int) _uniqueGrid.size(); i++) 
  {
    if(_uniqueGrid[i] == i)
    {
      NcVar *xVar = _uniqueGridxVar[i];
      NcVar *yVar = _uniqueGridyVar[i];

      if(xVar == NULL || yVar == NULL) {
	cerr << "ERROR - Cannot put data for coordinate x/y variable as it is NULL" << endl;
	return -1;
      }

      float *xData = new float[_fieldInfo[i].gridInfo.nx];
      float *yData = new float[_fieldInfo[i].gridInfo.ny];

      float minx = _fieldInfo[i].gridInfo.minx;
      float dx = _fieldInfo[i].gridInfo.dx;
      float miny = _fieldInfo[i].gridInfo.miny;
      float dy = _fieldInfo[i].gridInfo.dy;
      
      for (int x = 0; x < _fieldInfo[i].gridInfo.nx; x++) {
	xData[x] = minx + x * dx;
      }
      
      for (int y = 0; y < _fieldInfo[i].gridInfo.ny; y++) {
	yData[y] = miny + y * dy;
      }

      if (!xVar->put( xData, _fieldInfo[i].gridInfo.nx)) {
	cerr << "ERROR - Cannot put data for variable " << xVar->name() << endl;
	cerr << _ncErr->get_errmsg() << endl;
	delete [] xData;
	delete [] yData;
	return -1;
      }

      if (!yVar->put( yData, _fieldInfo[i].gridInfo.ny)) {
	cerr << "ERROR - Cannot put data for variable " << yVar->name() << endl;
	cerr << _ncErr->get_errmsg() << endl;
	delete [] xData;
	delete [] yData;
	return -1;
      }

      if (_fieldInfo[i].gridInfo.ncfGridName.compare(NcOutput::latitude_longitude) != 0) {

	NcVar *latVar = _uniqueGridlatVar[i];
	NcVar *lonVar = _uniqueGridlonVar[i];

	if(latVar == NULL || lonVar == NULL) {
	  cerr << "ERROR - Cannot put data for coordinate lat/lon variable as it is NULL" << endl;
	  return -1;
	}

	Pjg pjg;

	if(_fieldInfo[i].gridInfo.ncfGridName.compare(NcOutput::lambert_conformal_conic) == 0) {

	  pjg.initLc2(_fieldInfo[i].gridInfo.proj_origin_lat,
		      _fieldInfo[i].gridInfo.proj_origin_lon,
		      _fieldInfo[i].gridInfo.lat1,
		      _fieldInfo[i].gridInfo.lat2,
		      _fieldInfo[i].gridInfo.nx,
		      _fieldInfo[i].gridInfo.ny,
		      1, // nz
		      _fieldInfo[i].gridInfo.dx,
		      _fieldInfo[i].gridInfo.dy,
		      1.0,  // dz
		      _fieldInfo[i].gridInfo.minx,
		      _fieldInfo[i].gridInfo.miny,
		      0.0  // minz 
		      );

	} else if(_fieldInfo[i].gridInfo.ncfGridName.compare(NcOutput::polar_stereographic) == 0) {

	  PjgTypes::pole_type_t pole_type = PjgTypes::POLE_NORTH;
	  if(_fieldInfo[i].gridInfo.pole_type == 1)
	    pole_type = PjgTypes::POLE_SOUTH;

	  pjg.initPolarStereo(_fieldInfo[i].gridInfo.tan_lon,
			      pole_type,
			      _fieldInfo[i].gridInfo.central_scale,
			      _fieldInfo[i].gridInfo.nx,
			      _fieldInfo[i].gridInfo.ny,
			      1, // nz
			      _fieldInfo[i].gridInfo.dx,
			      _fieldInfo[i].gridInfo.dy,
			      1.0,  // dz
			      _fieldInfo[i].gridInfo.minx,
			      _fieldInfo[i].gridInfo.miny,
			      0.0  // minz 
			      );

	} else if(_fieldInfo[i].gridInfo.ncfGridName.compare(NcOutput::mercator) == 0) {

	  pjg.initMercator(_fieldInfo[i].gridInfo.proj_origin_lat,
			   _fieldInfo[i].gridInfo.proj_origin_lon,
			   _fieldInfo[i].gridInfo.nx,
			   _fieldInfo[i].gridInfo.ny,
			   1, // nz
			   _fieldInfo[i].gridInfo.dx,
			   _fieldInfo[i].gridInfo.dy,
			   1.0,  // dz
			   _fieldInfo[i].gridInfo.minx,
			   _fieldInfo[i].gridInfo.miny,
			   0.0  // minz 
			   );

	}
	  

	float *lonData = new float[_fieldInfo[i].gridInfo.nx * _fieldInfo[i].gridInfo.ny];
	float *latData = new float[_fieldInfo[i].gridInfo.nx * _fieldInfo[i].gridInfo.ny];

	for (int y = 0; y < _fieldInfo[i].gridInfo.ny; y++)  {
	  for (int x = 0; x < _fieldInfo[i].gridInfo.nx; x++) {
	    double lat, lon;
	    pjg.xy2latlon( xData[x],  yData[y], lat, lon);
	    latData[ y * _fieldInfo[i].gridInfo.nx + x] = lat;
	    lonData[ y * _fieldInfo[i].gridInfo.nx + x] = lon;
	  }
	}

	if (!latVar->put(latData, _fieldInfo[i].gridInfo.ny, _fieldInfo[i].gridInfo.nx)) {
	  cerr << "ERROR - Cannot put data for variable " << latVar->name() << endl;
	  cerr << _ncErr->get_errmsg() << endl;
	  delete [] latData;
	  delete [] lonData;
	  return -1;
	}
	if (!lonVar->put(lonData, _fieldInfo[i].gridInfo.ny, _fieldInfo[i].gridInfo.nx)) {
	  cerr << "ERROR - Cannot put data for variable " << lonVar->name() << endl;
	  cerr << _ncErr->get_errmsg() << endl;
	  delete [] latData;
	  delete [] lonData;
	  return -1;
	}

	delete [] latData;
	delete [] lonData;

      } // end if not lat/lon grid

      delete [] xData;
      delete [] yData;

    }
  }  // end unique grid info loop

  // Put the unique zlevel data
  for (int i = 0; i <  (int) _uniqueVertical.size(); i++) 
  {
    if(_uniqueVertical[i] == i)
    {
      NcVar *zVar = _uniqueVerticalzVar[i];

      if(zVar == NULL) {
	cerr << "ERROR - Cannot put data for " << _fieldInfo[i].vlevelInfo.longName << " var as it is NULL" << endl;
	return -1;
      }

      if (!zVar->put(_fieldInfo[i].vlevelInfo.zData, _fieldInfo[i].vlevelInfo.nz)) {
	cerr << "ERROR - Cannot put data for variable " << zVar->name() << endl;
	cerr << _ncErr->get_errmsg() << endl;
	return -1;
      }

    }
  }  // end unique vertical info loop

  return 0;
}

///////////////////////////////////////
// Write the field variables to the NcFile
int NcOutput::_putFieldDataVariables()
{

  for (int i = 0; i <  (int) _fieldVar.size(); i++) 
  {
    NcVar *fieldVar = _fieldVar[i];

    if(fieldVar == NULL) {
      cerr << "ERROR - Cannot put data for variable '" << _fieldInfo[i].name << "' as it is NULL" << endl;
      return -1;
    }

    fieldVar->set_cur((long int)0);
    long int field_size = _fieldInfo[i].gridInfo.nx * _fieldInfo[i].gridInfo.ny * _fieldInfo[i].vlevelInfo.nz;

    int iret;
    if(_fieldInfo[i].ncType == Params::DATA_PACK_BYTE)
      iret = fieldVar->put((ncbyte*)_fieldData[i], 1, _fieldInfo[i].vlevelInfo.nz, _fieldInfo[i].gridInfo.ny, _fieldInfo[i].gridInfo.nx);
    else
      if(_fieldInfo[i].ncType == Params::DATA_PACK_SHORT)
	iret = fieldVar->put((si16*)_fieldData[i], 1, _fieldInfo[i].vlevelInfo.nz, _fieldInfo[i].gridInfo.ny, _fieldInfo[i].gridInfo.nx);
      else
	iret = fieldVar->put(_fieldData[i], 1, _fieldInfo[i].vlevelInfo.nz, _fieldInfo[i].gridInfo.ny, _fieldInfo[i].gridInfo.nx);

    if (!iret) {
      cerr << "ERROR - Cannot put data for variable " << fieldVar->name() << endl;
      cerr << _ncErr->get_errmsg() << endl;
      return -1;
    }

  }

  return 0;
}

void NcOutput::_remap(fl32 *data, Grib2Nc::FieldInfo* fieldInfo)
{
  /*
  MdvxRemapLut lut;

  switch( _params->out_projection_info.type) {
  case Params::PROJ_LATLON:
    inputField->remap2Latlon(lut, _params->out_grid_info.nx, _params->out_grid_info.ny, 
			_params->out_grid_info.minx, _params->out_grid_info.miny, 
			_params->out_grid_info.dx, _params->out_grid_info.dy );
    break;

  case Params::PROJ_LAMBERT_CONF:
    if(inputField->getFieldHeader().proj_type == Mdvx::PROJ_LAMBERT_CONF)
      _remapLambertLambert(inputField);
    else
      inputField->remap2Lc2(lut, _params->out_grid_info.nx, _params->out_grid_info.ny, 
			    _params->out_grid_info.minx, _params->out_grid_info.miny, 
			    _params->out_grid_info.dx, _params->out_grid_info.dy,
			    _params->out_projection_info.origin_lat, 
			    _params->out_projection_info.origin_lon,
			    _params->out_projection_info.ref_lat_1, 
			    _params->out_projection_info.ref_lat_2 );
    break;
  default:
    cerr <<"-- unknown projection; remapping failed." << endl;
  }
  */
}
/*
void NcOutput::_remapLambertLambert(fl32 *data, Grib2Nc::FieldInfo fieldInfo)
{

  Mdvx::field_header_t fhdr = inputField->getFieldHeader();
  
  int nx = _params->out_grid_info.nx;
  int ny = _params->out_grid_info.ny;
  int nz = fhdr.nz;

  MdvxProj inproj, outproj;
  outproj.initLambertConf(_params->out_projection_info.origin_lat, 
			  _params->out_projection_info.origin_lon,
			  _params->out_projection_info.ref_lat_1,
			  _params->out_projection_info.ref_lat_2);

  outproj.setGrid(nx, ny, _params->out_grid_info.dx, _params->out_grid_info.dy,
		  _params->out_grid_info.minx, _params->out_grid_info.miny);
  
  inproj.init(fhdr);
  
  float *odata = new float[nx*ny*nz];
  float *idata = (float *)inputField->getVol();

  double lat, lon;
  double ix, iy;
  for(int y = 0; y < ny; y++) 
  {
    for(int x = 0; x < nx; x++)
    {

      outproj.xyIndex2latlon(x, y, lat, lon);

      int ingrid = inproj.latlon2xyIndex(lat, lon, ix, iy);

      // If we are within 1/100th of the dx past the end of the grid
      // allow it to be set to the end of the grid.  
      // (rounding issue from projection library)
      if(ix > fhdr.nx-1 && ix < fhdr.nx -.99)
	ix = fhdr.nx-1;
      if(iy > fhdr.ny-1 && iy < fhdr.ny -.99)
	iy = fhdr.ny-1;

      if(ingrid != -1 && ix >= 0 && iy >= 0)
	for(int z = 0; z < nz; z++)
	  odata[(z*ny*nx)+(y*nx)+x] = _interp2(&fhdr, ix, iy, z, idata);
      else
	for(int z = 0; z < nz; z++)
	  odata[(z*ny*nx)+(y*nx)+x] = fhdr.missing_data_value;
    }
  }

  fhdr.nx = _params->out_grid_info.nx;
  fhdr.ny = _params->out_grid_info.ny;
  fhdr.grid_minx = _params->out_grid_info.minx;
  fhdr.grid_miny = _params->out_grid_info.miny;
  fhdr.grid_dx = _params->out_grid_info.dx;
  fhdr.grid_dy = _params->out_grid_info.dy;
  fhdr.proj_type = _params->out_projection_info.type;
  fhdr.proj_origin_lat = _params->out_projection_info.origin_lat;
  fhdr.proj_origin_lon = _params->out_projection_info.origin_lon;
  fhdr.proj_param[0] = _params->out_projection_info.ref_lat_1;
  fhdr.proj_param[1] = _params->out_projection_info.ref_lat_2;
  fhdr.volume_size =
    fhdr.nx * fhdr.ny * fhdr.nz * fhdr.data_element_nbytes; 

  inputField->setFieldHeader(fhdr);
  inputField->setVolData(odata, fhdr.volume_size, Mdvx::ENCODING_FLOAT32);

  delete []odata;

}

float NcOutput::_interp2(Grib2Nc::FieldInfo, , double x, double y, int z, float *field)
{
  int ix = floor(x);
  int iy = floor(y);
  int ix1 = ix+1;
  int iy1 = iy+1;

  // Allow wraping in longitude if x is between -.5 and 0, and a global lat/lon model
  if(x > -.5 && ix == -1 && fieldHdr->proj_type == 0 && fieldHdr->proj_origin_lon == 0 &&
     (fieldHdr->nx * fieldHdr->grid_dx) + fieldHdr->grid_minx > 360.0)
    ix = floor((360.0 - fieldHdr->grid_minx) / fieldHdr->grid_dx);
  if(field == NULL || ix < 0 || y < 0 || ix1 > fieldHdr->nx || iy1 > fieldHdr->ny)
    return fieldHdr->missing_data_value;
  if(z < 0)
    z = 0;
  if(z >= fieldHdr->nz)
    z = fieldHdr->nz -1;
  if(field[(z*fieldHdr->ny*fieldHdr->nx)+(iy*fieldHdr->nx)+ix] == fieldHdr->missing_data_value ||
     field[(z*fieldHdr->ny*fieldHdr->nx)+(iy*fieldHdr->nx)+ix1] == fieldHdr->missing_data_value ||
     field[(z*fieldHdr->ny*fieldHdr->nx)+((iy1)*fieldHdr->nx)+ix] == fieldHdr->missing_data_value ||
     field[(z*fieldHdr->ny*fieldHdr->nx)+((iy1)*fieldHdr->nx)+ix1] == fieldHdr->missing_data_value)
    return fieldHdr->missing_data_value;

  float val;
  // Allow being exactly on the last point in the grid (x or y or both)
  if(ix1 == fieldHdr->nx && iy1 == fieldHdr->ny)
    val = field[(z*fieldHdr->ny*fieldHdr->nx)+(iy*fieldHdr->nx)+ix] * (1-(x-ix));
  else if(ix1 == fieldHdr->nx)
    val = field[(z*fieldHdr->ny*fieldHdr->nx)+(iy*fieldHdr->nx)+ix] * (1-(y-iy)) + 
      field[(z*fieldHdr->ny*fieldHdr->nx)+((iy1)*fieldHdr->nx)+ix] * (y-iy);
  else if(iy1 == fieldHdr->ny)
    val = field[(z*fieldHdr->ny*fieldHdr->nx)+(iy*fieldHdr->nx)+ix] * (1-(x-ix)) + 
	   field[(z*fieldHdr->ny*fieldHdr->nx)+(iy*fieldHdr->nx)+ix1] * (x-ix);
  else  // Normal 2D interpolation
    val = (field[(z*fieldHdr->ny*fieldHdr->nx)+(iy*fieldHdr->nx)+ix] * (1-(x-ix)) + 
	   field[(z*fieldHdr->ny*fieldHdr->nx)+(iy*fieldHdr->nx)+ix1] * (x-ix)) * (1-(y-iy)) + 
      (field[(z*fieldHdr->ny*fieldHdr->nx)+((iy1)*fieldHdr->nx)+ix] * (1-(x-ix)) + 
       field[(z*fieldHdr->ny*fieldHdr->nx)+((iy1)*fieldHdr->nx)+ix1] * (x-ix)) * (y-iy);
  
  if(val != val)
    return fieldHdr->missing_data_value;
  else
    return val;

}
*/
