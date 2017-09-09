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
/*********************************************************************
 * MetarCsv2Spdb.cc: Program to convert pirep data into
 *                 SPDB format.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2013
 *
 * George McCabe
 *
 *********************************************************************/

// C++ include files
#include <cstdio>
#include <cerrno>
#include <toolsa/os_config.h>
#include <fstream>
#include <streambuf>

#include<sstream>
#include<iterator>
#include<vector>

// System/RAP include files
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
#include <toolsa/Path.hh>
#include <didss/DsInputPath.hh>
#include <toolsa/TaXml.hh>

// Local include files
#include "MetarCsv2Spdb.hh"

using namespace std;

/////////////////////////////////////////////////////
// Constructor

MetarCsv2Spdb::MetarCsv2Spdb(int argc, char **argv)

{
  isOK = true;
  _inputPath = NULL;
  _useStdin = false;


  // set programe name
  
  _progName = "MetarCsv2Spdb";
  ucopyright((char *) _progName.c_str());
  
  // get command line args

  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }
  
  // get TDRP params
  
  
  if (_params.loadFromArgs(argc, argv, _args.override.list,
                           NULL)) {
	  cerr << "ERROR: " << _progName << endl;
	  cerr << "Problem with TDRP parameters" << endl;
	  isOK = false;
	  return;
  }
  
  // input path object
  if (_params.mode == Params::FILELIST) {
    
	  // FILELIST mode
    
	  _inputPath = 
		  new DsInputPath(_progName,
		                  _params.debug >= Params::DEBUG_VERBOSE,
		                  _args.inputFileList);
    
  } else if (_params.mode == Params::ARCHIVE) {
    
	  if (_args.startTime != 0 && _args.endTime != 0) {
      
		  _inputPath = 
			  new DsInputPath((char *) _progName.c_str(),
			                  _params.debug >= Params::DEBUG_VERBOSE,
			                  _params.input_dir,
			                  _args.startTime,
			                  _args.endTime);
	  } else {
		  cerr << "ERROR: " << _progName << endl;
		  cerr << "In ARCHIVE mode, you must set start and end times." << endl;
		  _args.usage(_progName, cerr);
		  isOK = false;
	  }

  } else if (_params.mode == Params::STDIN) {
    _useStdin = true;
  } else {

	  _inputPath = 
		  new DsInputPath((char *) _progName.c_str(),
		                  _params.debug >= Params::DEBUG_VERBOSE,
		                  _params.input_dir,
		                  _params.max_realtime_valid_age,
		                  PMU_auto_register);
  }

  // initialize process registration
  PMU_auto_init(_progName.c_str(), _params.instance,
		PROCMAP_REGISTER_INTERVAL);  
}

///////////////////////////////
// Destructor

MetarCsv2Spdb::~MetarCsv2Spdb()
{

  // unregister process

  PMU_auto_unregister();

  // Free contained objects

  if (_inputPath) {
    delete _inputPath;
  }


}

/////////////////////////////////////////////////////////////////////////
// clear the indices into csv file
void MetarCsv2Spdb::_clearIndicies()
{

	_iIcao = MISSING_INDEX;
	_iLat = MISSING_INDEX;
	_iLon = MISSING_INDEX;
	_iAlt = MISSING_INDEX;
	_iObsTime = MISSING_INDEX;
	_iReportTime = MISSING_INDEX;
	_iSurfaceTemp = MISSING_INDEX;
	_iDewPtTemp = MISSING_INDEX;
	_iWindDir = MISSING_INDEX;
	_iWindSpeed = MISSING_INDEX;
	_iWindGust = MISSING_INDEX;
	_iHorizVisibility = MISSING_INDEX;
	_iAltimeter = MISSING_INDEX;
	_iSeaLvlPressure = MISSING_INDEX;
	_iQcField = MISSING_INDEX;
	_iCorrected = MISSING_INDEX;
	_iAuto = MISSING_INDEX;
	_iAutoStation = MISSING_INDEX;
	_iMaintenanceOn = MISSING_INDEX;
	_iNoSignal = MISSING_INDEX;
	_iLightningOff = MISSING_INDEX;
	_iFreezingRainOff = MISSING_INDEX;
	_iPresentWeatherOff = MISSING_INDEX;
	_iPresentWeather = MISSING_INDEX;
	_iCloudCoverage1 = MISSING_INDEX;
	_iCloudCoverage2 = MISSING_INDEX;
	_iCloudCoverage3 = MISSING_INDEX;
	_iCloudCoverage4 = MISSING_INDEX;
	_iCloudCoverage5 = MISSING_INDEX;
	_iCloudCoverage6 = MISSING_INDEX;
	_iCloudBase1 = MISSING_INDEX;
	_iCloudBase2 = MISSING_INDEX;
	_iCloudBase3 = MISSING_INDEX;
	_iCloudBase4 = MISSING_INDEX;
	_iCloudBase5 = MISSING_INDEX;
	_iCloudBase6 = MISSING_INDEX;
	_iPressureTendency = MISSING_INDEX;
	_iMaxT = MISSING_INDEX;
	_iMinT = MISSING_INDEX;
	_iMaxT24hr = MISSING_INDEX;
	_iMinT24hr = MISSING_INDEX;
	_iPrecip = MISSING_INDEX;
	_iPrecip3hr = MISSING_INDEX;
	_iPrecip6hr = MISSING_INDEX;
	_iPrecip24hr = MISSING_INDEX;
	_iSnow = MISSING_INDEX;
	_iVerticalVisibility = MISSING_INDEX;
	_iCeilingLow = MISSING_INDEX;
	_iMetarType = MISSING_INDEX;
	_iRawText = MISSING_INDEX;
	_iRemarks = MISSING_INDEX;
}

/////////////////////////////////////////////////////////////////////////
// parse the header line of the csv file to find indices
bool MetarCsv2Spdb::_parseHeader(string line)
{
  _clearIndicies();
  istringstream ss(line);

  //putting all the tokens in the vector
  vector<string> arrayTokens; 

  while(ss) {
    string s;
    if(!getline(ss, s, ',')) break;
    if(s[0]==' ') {
	    s = s.substr(1);
    }
    arrayTokens.push_back(s);
  }

  for(unsigned int i=0; i < arrayTokens.size(); i++)
	{
		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.icao_n)) {
			_iIcao = i;
    }
		else if(!strcmp(_params.field_names.icao_n,"") && _params.field_names.icao_i != -1) {
			_iIcao = _params.field_names.icao_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.lat_n)) {
			_iLat = i;
    }
		else if(!strcmp(_params.field_names.lat_n,"") && _params.field_names.lat_i != -1) {
			_iLat = _params.field_names.lat_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.lon_n))	{
			_iLon = i;
    }
		else if(!strcmp(_params.field_names.lon_n,"") && _params.field_names.lon_i != -1) {
			_iLon = _params.field_names.lon_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.altitude_n)) {
			_iAlt = i;
    }
		else if(!strcmp(_params.field_names.altitude_n,"") && _params.field_names.altitude_i != -1) {
			_iAlt = _params.field_names.altitude_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.obs_time_n)) {
			_iObsTime = i;
    }
		else if(!strcmp(_params.field_names.obs_time_n,"") && _params.field_names.obs_time_i != -1) {
			_iObsTime = _params.field_names.obs_time_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.report_time_n))	{
			_iReportTime = i;
    }
		else if(!strcmp(_params.field_names.report_time_n,"") && _params.field_names.report_time_i != -1) {
			_iReportTime = _params.field_names.report_time_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.surface_temp_n))	{
			_iSurfaceTemp = i;
    }
		else if(!strcmp(_params.field_names.surface_temp_n,"") && _params.field_names.surface_temp_i != -1) {
			_iSurfaceTemp = _params.field_names.surface_temp_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.dew_pt_temp_n))	{
			_iDewPtTemp = i;
    }
		else if(!strcmp(_params.field_names.dew_pt_temp_n,"") && _params.field_names.dew_pt_temp_i != -1) {
			_iDewPtTemp = _params.field_names.dew_pt_temp_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.wind_dir_n))	{
			_iWindDir = i;
    }
		else if(!strcmp(_params.field_names.wind_dir_n,"") && _params.field_names.wind_dir_i != -1) {
			_iWindDir = _params.field_names.wind_dir_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.wind_speed_n))	{
			_iWindSpeed = i;
    }
		else if(!strcmp(_params.field_names.wind_speed_n,"") && _params.field_names.wind_speed_i != -1) {
			_iWindSpeed = _params.field_names.wind_speed_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.wind_gust_n))	{
			_iWindGust = i;
    }
		else if(!strcmp(_params.field_names.wind_gust_n,"") && _params.field_names.wind_gust_i != -1) {
			_iWindGust = _params.field_names.wind_gust_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.horiz_visibility_n))	{
			_iHorizVisibility = i;
    }
		else if(!strcmp(_params.field_names.horiz_visibility_n,"") && _params.field_names.horiz_visibility_i != -1) {
			_iHorizVisibility = _params.field_names.horiz_visibility_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.sea_lvl_pressure_n))	{
			_iSeaLvlPressure = i;
    }
		else if(!strcmp(_params.field_names.sea_lvl_pressure_n,"") && _params.field_names.sea_lvl_pressure_i != -1) {
			_iSeaLvlPressure = _params.field_names.sea_lvl_pressure_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.qc_field_n))	{
			_iQcField = i;
    }
		else if(!strcmp(_params.field_names.qc_field_n,"") && _params.field_names.qc_field_i != -1) {
			_iQcField = _params.field_names.qc_field_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.corrected_n))	{
			_iCorrected = i;
    }
		else if(!strcmp(_params.field_names.corrected_n,"") && _params.field_names.corrected_i != -1) {
			_iCorrected = _params.field_names.corrected_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.auto_n))	{
			_iAuto = i;
    }
		else if(!strcmp(_params.field_names.auto_n,"") && _params.field_names.auto_i != -1) {
			_iAuto = _params.field_names.auto_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.auto_station_n))	{
			_iAutoStation = i;
    }
		else if(!strcmp(_params.field_names.auto_station_n,"") && _params.field_names.auto_station_i != -1) {
			_iAutoStation = _params.field_names.auto_station_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.maintenance_n))	{
			_iMaintenanceOn = i;
    }
		else if(!strcmp(_params.field_names.maintenance_n,"") && _params.field_names.maintenance_i != -1) {
			_iMaintenanceOn = _params.field_names.maintenance_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.no_signal_n))	{
			_iNoSignal = i;
    }
		else if(!strcmp(_params.field_names.no_signal_n,"") && _params.field_names.no_signal_i != -1) {
			_iNoSignal = _params.field_names.no_signal_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.lightning_off_n))	{
			_iLightningOff = i;
    }
		else if(!strcmp(_params.field_names.lightning_off_n,"") && _params.field_names.lightning_off_i != -1) {
			_iLightningOff = _params.field_names.lightning_off_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.freezing_rain_off_n))	{
			_iFreezingRainOff = i;
    }
		else if(!strcmp(_params.field_names.freezing_rain_off_n,"") && _params.field_names.freezing_rain_off_i != -1) {
			_iFreezingRainOff = _params.field_names.freezing_rain_off_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.present_weather_off_n))	{
			_iPresentWeatherOff = i;
    }
		else if(!strcmp(_params.field_names.present_weather_off_n,"") && _params.field_names.present_weather_off_i != -1) {
			_iPresentWeatherOff = _params.field_names.present_weather_off_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.present_weather_n))	{
			_iPresentWeather = i;
    }
		else if(!strcmp(_params.field_names.present_weather_n,"") && _params.field_names.present_weather_i != -1) {
			_iPresentWeather = _params.field_names.present_weather_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.cloud_coverage1_n))	{
			_iCloudCoverage1 = i;
    }
		else if(!strcmp(_params.field_names.cloud_coverage1_n,"") && _params.field_names.cloud_coverage1_i != -1) {
			_iCloudCoverage1 = _params.field_names.cloud_coverage1_i;
		}
		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.cloud_coverage2_n))	{
			_iCloudCoverage2 = i;
    }
		else if(!strcmp(_params.field_names.cloud_coverage2_n,"") && _params.field_names.cloud_coverage2_i != -1) {
			_iCloudCoverage2 = _params.field_names.cloud_coverage2_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.cloud_coverage3_n))	{
			_iCloudCoverage3 = i;
    }
		else if(!strcmp(_params.field_names.cloud_coverage3_n,"") && _params.field_names.cloud_coverage3_i != -1) {
			_iCloudCoverage3 = _params.field_names.cloud_coverage3_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.cloud_coverage4_n))	{
			_iCloudCoverage4 = i;
    }
		else if(!strcmp(_params.field_names.cloud_coverage4_n,"") && _params.field_names.cloud_coverage4_i != -1) {
			_iCloudCoverage4 = _params.field_names.cloud_coverage4_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.cloud_coverage5_n))	{
			_iCloudCoverage5 = i;
    }
		else if(!strcmp(_params.field_names.cloud_coverage5_n,"") && _params.field_names.cloud_coverage5_i != -1) {
			_iCloudCoverage5 = _params.field_names.cloud_coverage5_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.cloud_coverage6_n))	{
			_iCloudCoverage6 = i;
    }
		else if(!strcmp(_params.field_names.cloud_coverage6_n,"") && _params.field_names.cloud_coverage6_i != -1) {
			_iCloudCoverage6 = _params.field_names.cloud_coverage6_i;
		}


		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.cloud_base1_n))	{
			_iCloudBase1 = i;
    }
		else if(!strcmp(_params.field_names.cloud_base1_n,"") && _params.field_names.cloud_base1_i != -1) {
			_iCloudBase1 = _params.field_names.cloud_base1_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.cloud_base2_n))	{
			_iCloudBase2 = i;
    }
		else if(!strcmp(_params.field_names.cloud_base2_n,"") && _params.field_names.cloud_base2_i != -1) {
			_iCloudBase2 = _params.field_names.cloud_base2_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.cloud_base3_n))	{
			_iCloudBase3 = i;
    }
		else if(!strcmp(_params.field_names.cloud_base3_n,"") && _params.field_names.cloud_base3_i != -1) {
			_iCloudBase3 = _params.field_names.cloud_base3_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.cloud_base4_n))	{
			_iCloudBase4 = i;
    }
		else if(!strcmp(_params.field_names.cloud_base4_n,"") && _params.field_names.cloud_base4_i != -1) {
			_iCloudBase4 = _params.field_names.cloud_base4_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.cloud_base5_n))	{
			_iCloudBase5 = i;
    }
		else if(!strcmp(_params.field_names.cloud_base5_n,"") && _params.field_names.cloud_base5_i != -1) {
			_iCloudBase5 = _params.field_names.cloud_base5_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.cloud_base6_n))	{
			_iCloudBase6 = i;
    }
		else if(!strcmp(_params.field_names.cloud_base6_n,"") && _params.field_names.cloud_base6_i != -1) {
			_iCloudBase6 = _params.field_names.cloud_base6_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.pressure_tendency_n))	{
			_iPressureTendency = i;
    }
		else if(!strcmp(_params.field_names.pressure_tendency_n,"") && _params.field_names.pressure_tendency_i != -1) {
			_iPressureTendency = _params.field_names.pressure_tendency_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.maxT_n))	{
			_iMaxT = i;
    }
		else if(!strcmp(_params.field_names.maxT_n,"") && _params.field_names.maxT_i != -1) {
			_iMaxT = _params.field_names.maxT_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.minT_n))	{
			_iMinT = i;
    }
		else if(!strcmp(_params.field_names.minT_n,"") && _params.field_names.minT_i != -1) {
			_iMinT = _params.field_names.minT_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.maxT24hr_n))	{
			_iMaxT24hr = i;
    }
		else if(!strcmp(_params.field_names.maxT24hr_n,"") && _params.field_names.maxT24hr_i != -1) {
			_iMaxT24hr = _params.field_names.maxT24hr_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.minT24hr_n))	{
			_iMinT24hr = i;
    }
		else if(!strcmp(_params.field_names.minT24hr_n,"") && _params.field_names.minT24hr_i != -1) {
			_iMinT24hr = _params.field_names.minT24hr_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.precip_n))	{
			_iPrecip = i;
    }
		else if(!strcmp(_params.field_names.precip_n,"") && _params.field_names.precip_i != -1) {
			_iPrecip = _params.field_names.precip_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.precip3hr_n))	{
			_iPrecip3hr = i;
    }
		else if(!strcmp(_params.field_names.precip3hr_n,"") && _params.field_names.precip3hr_i != -1) {
			_iPrecip3hr = _params.field_names.precip3hr_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.precip6hr_n))	{
			_iPrecip6hr = i;
    }
		else if(!strcmp(_params.field_names.precip6hr_n,"") && _params.field_names.precip6hr_i != -1) {
			_iPrecip6hr = _params.field_names.precip6hr_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.precip24hr_n))	{
			_iPrecip24hr = i;
    }
		else if(!strcmp(_params.field_names.precip24hr_n,"") && _params.field_names.precip24hr_i != -1) {
			_iPrecip24hr = _params.field_names.precip24hr_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.snow_n))	{
			_iSnow = i;
    }
		else if(!strcmp(_params.field_names.snow_n,"") && _params.field_names.snow_i != -1) {
			_iSnow = _params.field_names.snow_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.vertical_visibility_n))	{
			_iVerticalVisibility = i;
    }
		else if(!strcmp(_params.field_names.vertical_visibility_n,"") && _params.field_names.vertical_visibility_i != -1) {
			_iVerticalVisibility = _params.field_names.vertical_visibility_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.ceiling_low_n))	{
			_iCeilingLow = i;
    }
		else if(!strcmp(_params.field_names.ceiling_low_n,"") && _params.field_names.ceiling_low_i != -1) {
			_iCeilingLow = _params.field_names.ceiling_low_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.metar_type_n))	{
			_iMetarType = i;
    }
		else if(!strcmp(_params.field_names.metar_type_n,"") && _params.field_names.metar_type_i != -1) {
			_iMetarType = _params.field_names.metar_type_i;
		}

		if(!strcmp(arrayTokens[i].c_str(),_params.field_names.raw_n))	{
			_iRawText = i;
    }
		else if(!strcmp(_params.field_names.raw_n,"") && _params.field_names.raw_i != -1) {
			_iRawText = _params.field_names.raw_i;
		}


  }

  if(_iObsTime == MISSING_INDEX)
	{
	  cerr << "WARNING : Required field (time) \"" << _params.field_names.obs_time_n << "\"  not found in header" << endl;
	  cerr << "HEADER LINE = <" << line << ">" << endl;
		return false;
  }

  return true;
}



/////////////////////////////////////
// run()
int MetarCsv2Spdb::run()

{
  int iret = 0;

  char *input_filename;
  int totalMetarLines = 0;
  int successfulMetarDeocdings = 0;
  
  while( _useStdin || (input_filename = _inputPath->next()) != NULL)
  {
    // create SPDB object
    DsSpdb spdbXml;
    DsSpdb spdbRaw;

    if (_params.debug >= Params::DEBUG_VERBOSE) {
	    spdbXml.setDebug();
	    spdbRaw.setDebug();
    }
    if (_params.output_compression == Params::COMPRESSION_GZIP) {
	    spdbXml.setChunkCompressOnPut(Spdb::COMPRESSION_GZIP);
	    spdbRaw.setChunkCompressOnPut(Spdb::COMPRESSION_GZIP);
    } 
    else if (_params.output_compression == Params::COMPRESSION_BZIP2) {
	    spdbXml.setChunkCompressOnPut(Spdb::COMPRESSION_BZIP2);
	    spdbRaw.setChunkCompressOnPut(Spdb::COMPRESSION_BZIP2);
    }


    spdbXml.setAppName(_progName);
    spdbRaw.setAppName(_progName);

    // Process the input file
    // Register with the process mapper
    string procmap_status;
    if( _useStdin )
	  {
      procmap_status = "Input from <STDIN>";
      if (_params.debug) {
        cout << "New data from STDIN" << endl;
      }
    }
    else
	  {
      Path path(input_filename);
      procmap_status = "File <" + path.getFile() + ">";
      if (_params.debug) {
        cout << "New data in file: " << input_filename << endl;
      }
	  }
    PMU_auto_register(procmap_status.c_str());
      


    // 
    // open the file into a fstream
    // use get_line and END_DELIMTER to delimit pireps
    // copy lines to a vector of strings; result is a array of reports
    // use isalpha and toupper to convert all characters to upper case
    // use a filter to convert all '/015' to whitespace
    //
    ifstream in_file;
    if( _useStdin ) {
      in_file.open("/dev/stdin");
    } else {
      in_file.open(input_filename);
	  }
    if(!in_file) {
      cerr << "ERROR: Unable to open file: " 
	   << input_filename << "; continuing ..." << endl;
      continue;
    }

    string line;
    getline(in_file, line); // get header
    if(!_parseHeader(line))
    {
      cerr << "ERROR: _parseHeader failed" << endl;
      if(_useStdin) break;
        continue;
      }

    while(getline(in_file, line)) {
      if(line.size() == 0){ 
	      continue;
      }
      istringstream ss(line);

      //putting all the tokens in the vector
      vector<string> arrayTokens; 

      while(ss) {
        string s;
        if(!getline(ss, s, ',')) break;
        arrayTokens.push_back(s);
      }
      totalMetarLines++;
      WxObs m;
        if(!fillMetarObject(arrayTokens, m)) {

	  verifyMetar(m, line);
	  successfulMetarDeocdings++;
          if(strlen(_params.decoded_spdb_url) != 0) {
            addXmlSpdb(&spdbXml, &m, _params.expire_secs);
    	    }
	      }
        else {
          cerr << "WARNING: Decoder failed on line: " << line  << endl;
        }
    }  
    // Write
    if(strlen(_params.decoded_spdb_url) != 0) {
      _writeSpdb(spdbXml, _params.decoded_spdb_url);
    }

    if(strlen(_params.ascii_spdb_url) != 0) {
      _writeSpdb(spdbRaw, _params.ascii_spdb_url);
    }

    if( _useStdin ) {
	    break;
    }
  } // while((input_filename = _inputPath->next() ...

  cout << "Read " << totalMetarLines << " lines of metars.\n";
  cout << "Processed " << successfulMetarDeocdings << " metars.\n";
  return iret;

}


void MetarCsv2Spdb::verifyMetar(const WxObs& m, const string& line){

  
  
  //test 1:
  // precip type is RA, -RA, +RA, DZ, -DZ, +DZ and T <= -10C.
  // NOTE that -DZ and +DZ are not supported weather types, so we don't check for them.
  if (m.getTempC() <= -10){
    for (int ix = 0; ix < m.getWeatherTypeSize(); ix++){
      if ( (m.getWeatherType(ix) == WxT_RA) ||
	   (m.getWeatherType(ix) == WxT_MRA) ||
	   (m.getWeatherType(ix) == WxT_PRA) ||
	   (m.getWeatherType(ix) == WxT_DZ) ) {
	cerr << "ERROR: Decoded metar has Temp < -10 (" << m.getTempC() << ") and invalid weather (" << m.wxType2Str(m.getWeatherType(ix)) << ")" << endl;
	cerr << "ERROR:         input line: " << line << endl;	  	     
      }
      
    }
      
  }
  //test 2:
  // precip type is UNKNOWN
      for (int ix = 0; ix < m.getWeatherTypeSize(); ix++){
	if ( (m.getWeatherType(ix) == WxT_UNKNOWN) &&
	     ( m.getMetarWx().length() > 0 )){
	  cerr << "ERROR: Decoded metar has Wx Type UNKNOWN.  Weather String was: " << m.getMetarWx() << endl;
          cerr << "ERROR: input line: " << line << endl;

	}
      }

  
}

/////////////////////////////////////////////////////////////////////////
// adds xml pirep to spdb
void MetarCsv2Spdb::addXmlSpdb(DsSpdb* spdb, WxObs* m, int& expire_secs)
{
  time_t obsTime = m->getObservationTime();
  const time_t expireTime = obsTime + expire_secs;
  int stationId = Spdb::hash4CharsToInt32(m->getStationId().c_str());

  spdb->addPutChunk(stationId,
    obsTime,
	  expireTime,
    m->getBufLen(), m->getBufPtr());
}


////////////////////////////////
// write ASCII airep to database
int MetarCsv2Spdb::_writeSpdb(DsSpdb &spdb, string url)
{
  if (spdb.nPutChunks() > 0) {
    if (_params.debug) {
      cout << "Putting decoded metars to URL: " << url << endl;
    }
    spdb.setPutMode(Spdb::putModeAddUnique);
    
    if (spdb.put(url,
                 SPDB_STATION_REPORT_ID,
                  SPDB_STATION_REPORT_LABEL)) {
      cerr << "ERROR - MetarCsv2Spdb::_writeSpdb" << endl;
      cerr << spdb.getErrStr() << endl;
      return -1;
    }
  }
  
  return 0;

}
/////////////////////////////////////////////////////////////////////////
// fillPirepObject
int MetarCsv2Spdb::fillMetarObject(const vector<string>& in, WxObs& out)
{

  float as = _params.altitude_conversions.scale;
  float ab = _params.altitude_conversions.bias;

  float ts = _params.temperature_conversion.scale;
  float tb = _params.temperature_conversion.bias;

  float ws = _params.wind_speed_conversion.scale;
  float wb = _params.wind_speed_conversion.bias;

  float wgs = _params.wind_gust_conversion.scale;
  float wgb = _params.wind_gust_conversion.bias;

  float vs = _params.visibility_conversion.scale;
  float vb = _params.visibility_conversion.bias;

  float cs = _params.cloud_coverage_conversion.scale;
  float cb = _params.cloud_coverage_conversion.bias;

  float ps = _params.precip_conversion.scale;
  float pb = _params.precip_conversion.bias;

  float prs = _params.pressure_conversion.scale;
  float prb = _params.pressure_conversion.bias;

  float prss = _params.pressure_sea_conversion.scale;
  float prsb = _params.pressure_sea_conversion.bias;

  double ceiling = 99999;
  bool foundCeil = false;

  // check if item exists before entering it
  int index = _iIcao;
  if(index < 0 || in[index].size() == 0){
    return -1;
  }
  out.setStationId(in[index].c_str());

  index = _iLat;
  if(index < 0 || in[index].size() == 0){
    return -1;
  }
  out.setLatitude(atof(in[index].c_str()));

  index = _iLon;
  if(index < 0 || in[index].size() == 0){
    return -1;
  }
  out.setLongitude(atof(in[index].c_str()));

  index = _iAlt;
  if(index < 0 || in[index].size() == 0){
    return -1;
  }
  out.setElevationM(atoi(in[index].c_str())*as+ab);

  index = _iObsTime;
  if(index < 0 || in[index].size() == 0){
    return -1;
  }

  time_t obsT = convertStringToTimeT(in[index]);
  if(obsT != 0) {
    out.setObservationTime(obsT);
  }
  else {
    return -1;
  } 

  //  _iReportTime - ignore

  if(_iSurfaceTemp >= 0) {
	  out.setTempC(atof(in[_iSurfaceTemp].c_str())*ts+tb);
  }

  if(_iDewPtTemp >= 0) {
	  out.setDewpointC(atof(in[_iDewPtTemp].c_str())*ts+tb);
  }

  if(_iWindDir >= 0) {
	  out.setWindDirnDegT(atof(in[_iWindDir].c_str()));
  }

  if(_iWindSpeed >= 0) {
	  out.setWindSpeedMps(atof(in[_iWindSpeed].c_str())*ws+wb);
  }

  if(_iWindGust >= 0) {
	  out.setWindGustMps(atof(in[_iWindGust].c_str())*wgs+wgb);
  }

  if(_iHorizVisibility >= 0) {
	  out.setVisibilityKm(atof(in[_iHorizVisibility].c_str())*vs+vb);
  }

  // _iAltimeter

  if(_iSeaLvlPressure >= 0) {
	  out.setSeaLevelPressureMb(atof(in[_iSeaLvlPressure].c_str())*prss+prsb);
  }

  if(_iQcField >= 0) {
	  if((atoi(in[_iQcField].c_str()) & 32)) {
	    out.setMetarRemTsDown(true);
    }
	  if((atoi(in[_iQcField].c_str()) & 64)) {
	    out.setMetarRemFzraDown(true);
    }
  if( (atoi(in[_iQcField].c_str()) & 128)) {
      out.setMetarRemPwiDown(true);
    }
  }

  if(_iPresentWeatherOff >= 0) {
	  if(!strcmp(in[_iPresentWeatherOff].c_str(),"TRUE")) {
		  out.setMetarRemPwiDown(true);
    }
  }

  if(_iFreezingRainOff >= 0) {
	  if(!strcmp(in[_iFreezingRainOff].c_str(),"TRUE")) {
		  out.setMetarRemFzraDown(true);
    }
  }

  if(_iLightningOff >= 0) {
	  if(!strcmp(in[_iLightningOff].c_str(),"TRUE")) {
		  out.setMetarRemTsDown(true);
    }
  }

  if(_iCeilingLow >= 0) {
    if(strcmp(in[_iCeilingLow].c_str(),"")) {
      ceiling = atof(in[_iCeilingLow].c_str());
      foundCeil = true;
    }
  }

  if(_iCloudCoverage1 >= 0 && _iCloudBase1 >= 0){
    double ccFraction = -1;
    if (WxObs::skyObscurationStringToFraction(in[_iCloudCoverage1],ccFraction)){
      out.addSkyObscuration(ccFraction, atof(in[_iCloudBase1].c_str())*cs+cb);
      if( !foundCeil && 
	  ( in[_iCloudCoverage1] == "OVC" || in[_iCloudCoverage1] == "BKN") ) {
	double ceil = atof(in[_iCloudBase1].c_str());
	if(ceil < ceiling) {
	  ceiling = ceil;
	}
      }
    }
  }

  if(_iCloudCoverage2 >= 0 && _iCloudBase2 >= 0){
    double ccFraction = -1;
    if (WxObs::skyObscurationStringToFraction(in[_iCloudCoverage2],ccFraction)){
      out.addSkyObscuration(ccFraction, atof(in[_iCloudBase2].c_str())*cs+cb);
      if( !foundCeil && 
	  ( in[_iCloudCoverage2] == "OVC" || in[_iCloudCoverage2] == "BKN") ) {
	double ceil = atof(in[_iCloudBase2].c_str());
	if(ceil < ceiling) {
	  ceiling = ceil;
	}
      }
    }
  }

  if(_iCloudCoverage3 >= 0 && _iCloudBase3 >= 0){
    double ccFraction = -1;
    if (WxObs::skyObscurationStringToFraction(in[_iCloudCoverage3],ccFraction)){
      out.addSkyObscuration(ccFraction, atof(in[_iCloudBase3].c_str())*cs+cb);
      if( !foundCeil && 
	  ( in[_iCloudCoverage3] == "OVC" || in[_iCloudCoverage3] == "BKN") ) {
	double ceil = atof(in[_iCloudBase3].c_str());
	if(ceil < ceiling) {
	  ceiling = ceil;
	}
      }
    }
  }

  if(_iCloudCoverage4 >= 0 && _iCloudBase4 >= 0){
    double ccFraction = -1;
    if (WxObs::skyObscurationStringToFraction(in[_iCloudCoverage4],ccFraction)){
      out.addSkyObscuration(ccFraction, atof(in[_iCloudBase4].c_str())*cs+cb);
      if( !foundCeil && 
	  ( in[_iCloudCoverage4] == "OVC" || in[_iCloudCoverage4] == "BKN") ) {
	double ceil = atof(in[_iCloudBase4].c_str());
	if(ceil < ceiling) {
	  ceiling = ceil;
	}
      }
    }
  }

  if(_iCloudCoverage5 >= 0 && _iCloudBase5 >= 0){
    double ccFraction = -1;
    if (WxObs::skyObscurationStringToFraction(in[_iCloudCoverage5],ccFraction)){
      out.addSkyObscuration(ccFraction, atof(in[_iCloudBase5].c_str())*cs+cb);
      if( !foundCeil && 
	  ( in[_iCloudCoverage5] == "OVC" || in[_iCloudCoverage5] == "BKN") ) {
	double ceil = atof(in[_iCloudBase5].c_str());
	if(ceil < ceiling) {
	  ceiling = ceil;
	}
      }
    }
  }

  if(_iCloudCoverage6 >= 0 && _iCloudBase6 >= 0){
    double ccFraction = -1;
    if (WxObs::skyObscurationStringToFraction(in[_iCloudCoverage6],ccFraction)){
      out.addSkyObscuration(ccFraction, atof(in[_iCloudBase6].c_str())*cs+cb);
      if( !foundCeil && 
	  ( in[_iCloudCoverage6] == "OVC" || in[_iCloudCoverage6] == "BKN") ) {
	double ceil = atof(in[_iCloudBase6].c_str());
	if(ceil < ceiling) {
	  ceiling = ceil;
	}
      }
    }
  }

  if(ceiling != 99999) {
    out.setCeilingKm(ceiling*0.3048);
  }

  if(_iPresentWeather >= 0) {
	  stringstream ssin(in[_iPresentWeather]);
	  while(ssin.good()) {
		  string temp;
		  ssin >> temp;
		  if(temp[0] == '-') {
			  temp[0] = 'M';
		  }
		  out.addWeatherType(WxObs::str2WxType(temp));
    }
	  out.setMetarWx(in[_iPresentWeather]);
  }
  if(_iPressureTendency >= 0) {
   out.setPressureTendencyMb(atof(in[_iPressureTendency].c_str())*prs+prb,60*60*6);
  }

  if(_iMaxT >= 0) {
	  out.setMaxTempC(atof(in[_iMaxT].c_str())*ts+tb, 60*60*6);
  }

  if(_iMinT >= 0) {
   out.setMinTempC(atof(in[_iMinT].c_str())*ts+tb, 60*60*6);
  }


  if(_iMaxT24hr >= 0) {
	  out.setMaxTempC(atof(in[_iMaxT24hr].c_str())*ts+tb, 60*60*24); 
  }

  if(_iMinT24hr >= 0) {
	  out.setMinTempC(atof(in[_iMinT24hr].c_str())*ts+tb, 60*60*24);
  }

  if(_iPrecip >= 0) {
	  out.setPrecipLiquidMm(atof(in[_iPrecip].c_str())*ps+pb,WxObs::missing); // Liquid precipitation since the last regular METAR
  }

  if(_iPrecip3hr >= 0) {
	  out.setPrecipLiquidMm(atof(in[_iPrecip3hr].c_str())*ps+pb,60*60*3);
  }

  if(_iPrecip6hr >= 0) {
	  out.setPrecipLiquidMm(atof(in[_iPrecip6hr].c_str())*ps+pb,60*60*6);
  }

  if(_iPrecip24hr >= 0) {
	  out.setPrecipLiquidMm(atof(in[_iPrecip24hr].c_str())*ps+pb,60*60*24);
  }

  if(_iSnow >= 0) {
   out.setSnowDepthMm(atof(in[_iSnow].c_str())*ps+pb, WxObs::missing); // accum_secs?
  }

  if(_iVerticalVisibility >= 0) {
	  out.setVertVisKm(atof(in[_iVerticalVisibility].c_str())*vs+vb);
  }

  // metarType

  if(_iRawText >= 0) {
	  if (_params.debug >= Params::DEBUG_VERBOSE) {
		  cout << "\n" << in[_iRawText] << "\n";
	  }
    out.setMetarText(trimWhitespace(in[_iRawText]));
    std::size_t rmkPos = in[_iRawText].find("RMK");
    string remarks = in[_iRawText].substr(rmkPos+3);
    remarks = trimWhitespace(remarks);
    out.setMetarRemarks(remarks);
    // search for (A01|A01A|A02|A02A|AO1|AO1A|AO2|AO2A|AOA|AWOS)
    if(remarks.find("A01A") != std::string::npos) {
	    out.setMetarRemStnIndicator("A01A");
    }
    else if(remarks.find("A01") != std::string::npos) {
	    out.setMetarRemStnIndicator("A01");
    }
    else if(remarks.find("A02A") != std::string::npos) {
	    out.setMetarRemStnIndicator("A02A");
    }
    else if(remarks.find("A02") != std::string::npos) {
	    out.setMetarRemStnIndicator("A02");
    }
    else if(remarks.find("AO1A") != std::string::npos) {
	    out.setMetarRemStnIndicator("AO1A");
    }
    else if(remarks.find("AO1") != std::string::npos) {
	    out.setMetarRemStnIndicator("AO1");
    }
    else if(remarks.find("AO2A") != std::string::npos) {
	    out.setMetarRemStnIndicator("AO2A");
    }
    else if(remarks.find("AO2") != std::string::npos) {
	    out.setMetarRemStnIndicator("AO2");
    }
    else if(remarks.find("AOA") != std::string::npos) {
	    out.setMetarRemStnIndicator("AOA");
    }
    else if(remarks.find("AWOS") != std::string::npos) {
	    out.setMetarRemStnIndicator("AWOS");
    } 
  }

  if (_params.output_report_type == Params::REPORT_PLUS_METAR_XML) {
	  out.assembleAsReport(REPORT_PLUS_METAR_XML);
  } else if (_params.output_report_type == Params::REPORT_PLUS_FULL_XML) {
	  out.assembleAsReport(REPORT_PLUS_FULL_XML);
  } else if (_params.output_report_type == Params::XML_ONLY) {
    out.assembleAsXml();
  } else {
    return -1;
  }

  return 0;
}
/////////////////////////////////////////////////////////////////////////
// convertStringToTimeT
time_t MetarCsv2Spdb::convertStringToTimeT(string in)
{
  int year, month, day, hour, min, sec;
  if (sscanf(in.c_str(), "%4d-%2d-%2dT%2d:%2d:%2d", // 2013-07-15T19:30:00
         &year, &month, &day, &hour, &min, &sec) == 6) {
    DateTime dt(year, month, day, hour, min, sec);
    return dt.utime();
  }

  if (sscanf(in.c_str(), "%4d-%2d-%2d %2d:%2d:%2d", // 2013-07-15 19:30:00
         &year, &month, &day, &hour, &min, &sec) == 6) {
    DateTime dt(year, month, day, hour, min, sec);
    return dt.utime();
  }

  time_t tval;
  if (sscanf(in.c_str(), "%ld", &tval) == 1) {
    return tval;
  }

  return 0;
}

string MetarCsv2Spdb::trimWhitespace(string s)
{
   string temp = s;
   size_t p = temp.find_first_not_of(" \t");
   temp.erase(0, p);

   p = temp.find_last_not_of(" \t");
   if (string::npos != p)
      temp.erase(p+1);
   return temp;
}
