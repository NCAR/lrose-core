
#include "MSG3Sat2Mdv.hh"

#include <Mdv/DsMdvx.hh>
#include <toolsa/pmu.h>
#include "sunae.h"
#include <cmath>
#include <math.h>
#include <Ncxx/Ncxx.hh>
#include <Ncxx/NcxxFile.hh>

using namespace std;

const float MSG3Sat2Mdv::GEO_MISSING = -999.0;
const float MSG3Sat2Mdv::SAT_MISSING = -9999.9;
const float MSG3Sat2Mdv::VERY_LARGE_NUM = 1000000000;
const float MSG3Sat2Mdv::VERY_SMALL_NUM = -1000000000;
const double MSG3Sat2Mdv::EPSILON = 1e-20;

// C1 = 2*Planck*speedOfLight^2
const double MSG3Sat2Mdv::C1 = 1.19104e-5; //mWm^-2sr^-1(cm^-1)^-4
// C2 = Planck*speedOfLight/Boltzmann
const double MSG3Sat2Mdv::C2 = 1.43877; // K(cm^-1)^-1


MSG3Sat2Mdv::MSG3Sat2Mdv(int argc, char **argv):
_progName("MSG3Sat2Mdv"),
_args(_progName)
{
  isOK = true;

  //
  // set programe name
  //
  ucopyright((char *) _progName.c_str());

  //
  // get command line args
  //
  if (_args.parse(argc, argv)) 
    {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Problem with command line args" << endl;
      isOK = FALSE;
      return;
    }

  //
  // get TDRP params
  //
  char *paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &paramsPath)) 
    {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Problem with TDRP parameters" << endl;
      isOK = FALSE;
      return;
    }

  //
  // init process mapper registration
  //
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		300);

  //
  // Initialize and set file trigger
  //
  _fileTrigger = NULL;

  if ( _params.mode == Params::REALTIME ) 
  {
    if (_params.debug)
    {
      cerr << "FileInput::init: Initializing realtime input from " 
	   << _params.input_dir << endl;
    }

    _fileTrigger = new DsInputPath( _progName, _params.debug,
				    _params.input_dir,
				    _params.max_valid_realtime_age_min*60,
				    PMU_auto_register,
				    _params.ldata_info_avail,
				    false );
    //
    // Set wait for file to be written to disk before being served out.
    //
    _fileTrigger->setFileQuiescence( _params.file_quiescence_sec );
    
    //
    // Set time to wait before looking for new files.
    // 
    _fileTrigger->setDirScanSleep( _params.check_input_sec );  
  }
  else if ( _params.mode == Params::FILELIST ) 
  {    
    //
    // Archive mode.
    //
    const vector<string> &fileList =  _args.getInputFileList();
    
    if ( fileList.size() ) 
    {
      if (_params.debug)
      {
	cerr << "FileInput::init: Initializing archive FILELIST mode." << endl;
      }

      _fileTrigger = new DsInputPath( _progName, _params.debug , fileList );
    }
  }
  else if ( _params.mode == Params::TIME_INTERVAL ) 
  { 
    //
    // Set up the time interval
    //
    DateTime start( _params.start_time);
    
    DateTime end( _params.end_time);
    
    time_t time_begin = start.utime();
    
    time_t time_end = end.utime();
       
    if (_params.debug)
    {
      cerr << "FileInput::init: Initializing file input for time interval [" 
	   << time_begin << " , " << time_end << "]." << endl;  
    }
       
    _fileTrigger = new DsInputPath( _progName, _params.debug,
				   _params.input_dir,
				   time_begin,
				   time_end,false);
  }
  if(_fileTrigger == NULL) 
  {
    isOK = false;
  }
  
  return;
}

MSG3Sat2Mdv::~MSG3Sat2Mdv()
{
  //
  // unregister process
  //
  PMU_auto_unregister();  
}

int MSG3Sat2Mdv::Run()
{
  //
  // register with procmap
  //
  PMU_auto_register("Run");

  //
  // Basic header data doesnt change file to file so set it once
  // and reuse. Time and field names in headers are adjusted as necessary
  //
  _setMasterHeader();
  
  _setVlevelHdr();
  
  _setProjection();

  _setFieldHdr();
  //
  // process data
  //
  char *inputPath;

  //
  // Process indefinitely in real-time or until there arent any files left in archive mode
  //
 
  while ( (inputPath = _fileTrigger->next()) != NULL) 
  {
    if (_processData(inputPath))
    {
      cerr << "MSG3Sat2Mdv::Run(): Errors in processing file: " <<  inputPath << endl;
    }
  } 
  delete _fileTrigger;
  
  return 0;
}

int MSG3Sat2Mdv::_processData(char *inputPath)
{
  if (_params.debug)
  {
    cerr << "MSG3Sat2Mdv::_processData: Processing file : " << inputPath << endl;
  }
  
  //
  // registration with procmap
  //
  PMU_force_register("Processing data");
  
  //
  // Read input NetCDF, put in Mdv field objects
  //
  if (_netcdf2MdvFields(inputPath))
    return 1;

  //
  // Write the stored data to a file
  //
  _writeMdv();

  return 0;
}

int MSG3Sat2Mdv::_writeMdv()
{
  //
  // Configure time members of master header with latest data information
  //
  _masterHdrAdjustTime();

  //
  // Sync headers with projection, add master header to Mdvx object
  //
  _outputProj.syncToHdrs(_master_hdr,_field_hdr);
  
  _mdv.setMasterHeader(_master_hdr);

  if (_mdv.writeToDir(_params.output_url)) 
  {
    cerr << "MSG3Sat2Mdv::_writeData(): Error. Cannot write mdv file" << endl;
    
    cerr << _mdv.getErrStr() << endl;
    
    return 1;
  }
    
  if (_params.debug) 
  {
    cerr << "MSG3Sat2Mdv::_writeData(): Wrote file: " << _mdv.getPathInUse() << endl;
  }

  return 0;
}

int MSG3Sat2Mdv::_netcdf2MdvFields(char *filename)
{

  //
  // Registration with procmap
  //
  PMU_force_register("Reading netCDF data");
 
  if (_params.debug) 
  {
    cerr << "MSG3Sat2Mdv::_readNetcdf(): Reading file: " << filename << endl;
  }
 
  //
  // Open the file. 
  // 
  NcxxFile ncFile(filename, NcxxFile::read);

  //
  // Get dimensions
  //
  NcxxDim xDim = ncFile.getDim(_params.xdim);

  _nx = (int) xDim.getSize();

  NcxxDim yDim = ncFile.getDim(_params.ydim);

  _ny = (int) yDim.getSize();
  
  //
  // Get start time/ end time from filename for now
  //
  string dataPath = filename;
  
  size_t slash = dataPath.find_last_of("/");
  
  string dateTimeStr = dataPath.substr(slash-8,8) +  dataPath.substr(slash+1,6);

  _startTime.set(dateTimeStr);

  _endTime.set(dateTimeStr);

  
  if (_params.debug)
  {
    cerr << "MSG3Sat2Mdv::_readNetcdf(): startTime: " << _startTime.dtime() << endl;
  
    cerr << "MSG3Sat2Mdv::_readNetcdf(): endTime: " << _endTime.dtime() << endl;
  }

  //
  // Get latitude data
  //
  NcxxVar latData =  ncFile.getVar(_params.lat_fieldname);

  if(latData.isNull()) 
  {
    cerr << "MSG3Sat2Mdv::_readNetcdf(): Failure to get latitude data. Exiting" << endl;
   
    return 1;
  }

  float *latitudes = new float[_nx*_ny];
  
  latData.getVal(latitudes);

  //
  // Get longitude data
  //
  NcxxVar lonData =  ncFile.getVar(_params.lon_fieldname);
 
  if (lonData.isNull())
  {
    cerr << "MSG3Sat2Mdv::_readNetcdf(): Failure to get longitude data. Exiting" << endl;
   
    return 1;
  }

  float *longitudes = new float[_nx*_ny];

  lonData.getVal(longitudes);
    
  for (int k = 0; k < _params.fields_n; k++)
  {
    NcxxVar satData = ncFile.getVar(_params._fields[k].ncfFieldName);
  
    if (satData.isNull())
    {
      cerr << "MSG3Sat2Mdv::_readNetcdf(): Failure to get " << _params._fields[k].ncfFieldName
	   << " Exiting." << endl;
      
      return 1;
    }

    short *satShortVals = new short[_nx*_ny];
   
    satData.getVal(satShortVals);
    
    //
    //  Get scale and bias
    //
    NcxxVarAtt scaleAtt = satData.getAtt("scale_factor");
    
    if (scaleAtt.isNull())
    {
      cerr << "MSG3Sat2Mdv::_readNetcdf(): Failure to get scale factor for " << _params._fields[k].ncfFieldName << " Exiting." << endl;
      
      return 1;
    }

    float satFieldScale;

    scaleAtt.getValues(&satFieldScale);
  
  
    NcxxVarAtt biasAtt = satData.getAtt("add_offset");
    
    if (biasAtt.isNull())
    {
      cerr << "MSG3Sat2Mdv::_readNetcdf(): Failure to get add_offset for "  << _params._fields[k].ncfFieldName << " Exiting." << endl;
      
      return 1;
    }
    
    float satFieldBias;

    biasAtt.getValues(&satFieldBias);
  
    //
    // Initialize variables for keeping track of min and max values of data field
    // for debugging only.
    //
    float min = VERY_LARGE_NUM;
    
    float max = VERY_SMALL_NUM;
    
    //
    // Apply scale and bias and copy to float array (user can decide output data type)
    //
    float *satDataVals = new float[_nx*_ny];


    for (int j = 0; j <  _ny; j++)
    {
      for (int i = 0; i <  _nx; i++)
      {
	satDataVals[j*_nx + i] = satShortVals[j*_nx + i] * satFieldScale + satFieldBias;

	if( _params._fields[k].applyWarmChannelAnalyticTempConversion)
	{
	  //The Bidirectional Reflectance Factor (BRF) for the SEVIRI warm channels can be calculated as follows:
	  // r(lambda_i)  = PI * R(lambda_i) * d^2(t)/( I(lambda_i) * cos(Theta(t,x))) 
	  // Where
	  // lambda_i is the channel number (1 = VIS06, 2 = VIS08, 3 = NIR16, 4 = HRV)
	  // r(lambda_i)   is the Bidirectional Reflectance Factor (BRF) for the channel lambda_i
	  // R(lambda_i) is the measured radiance in mW·m-2·sr-1·(cm -1)-1
	  // d^2(t) is the Sun-Earth distance in AU at time t
	  // I(lambda_i)  is the band solar irradiance for the channel #i at 1 AU in mW·m-2 ·(cm-1)-1
	  // Theta(t,x) is the Solar Zenith Angle in Radians at time t and location x
	   
	  float sunEarthDistAU;

	  float azimuthRad;

	  processSolarAngles(_startTime.utime(), latitudes[j*_nx  + i], longitudes[j*_nx  + i],
			      sunEarthDistAU, azimuthRad);

	  float Rlambda = satDataVals[j*_nx + i];

	  float Ilambda = _params._fields[k].nu;

	  satDataVals[j*_nx + i] = satDataVals[j*_nx + i] * (3.14159265358979323 * Rlambda * sunEarthDistAU * sunEarthDistAU)/ (Ilambda * cos(azimuthRad));
	}
	else // we have cold channels 
	{
 	  double R = satDataVals[j*_nx + i];
	  float nu = _params._fields[k].nu;
	  float alpha = _params._fields[k].alpha;
	  float beta = _params._fields[k].beta;

	  double logArg = C1*nu*nu*nu/R +1;

	  if ( C1*nu*nu*nu/R +1  < EPSILON)
	     satDataVals[j*_nx + i] = beta/alpha;
	  else
	    satDataVals[j*_nx + i] = (C2*nu)/(alpha * log(C1*nu*nu*nu/R +1))-beta/alpha; 

	  // if ( satDataVals[j*_nx + i] < 0)
	  //   cerr << "bt = " << satDataVals[j*_nx + i] << " radiance = " << R <<  " shortVal: "  << satShortVals[j*_nx + i] << endl;
	}
	  
	//
	// Record max and min values for debug purposes
	//
	if (satDataVals[j*_nx + i] < min)
	{
	  min = satDataVals[j*_nx + i];
	}
	if (satDataVals[j*_nx + i] > max)
	{
	  max = satDataVals[j*_nx + i];	
	}
      } // end for i
    }// end for j

    //
    // Create mdv data array
    //
    float *mdvDataVals = new float[_params.output_proj.nx * _params.output_proj.ny];
    
    _mapSatData2MdvArray(mdvDataVals, satDataVals, latitudes, longitudes);
    
    _fieldHeaderAdjustNames(_params._fields[k].mdvFieldName, _params._fields[k].mdvFieldName,
			   _params._fields[k].units);
			    
    
    MdvxField *satField = new MdvxField(_field_hdr, _vlevel_hdr, (void *)mdvDataVals);
    
    _mdv.addField(satField);

    if (_params.debug)
    {
      cerr << "MSG3Sat2Mdv::_readNetcdf(): min data field value for field: " 
	   << _params._fields[k].ncfFieldName << " " << min << endl;
      
      cerr << "MSG3Sat2Mdv::_readNetcdf(): max data field value for field: " 
	    << _params._fields[k].ncfFieldName << " " << max << endl;
    }

    delete[] satShortVals;
    delete[] satDataVals;
    delete[] mdvDataVals;   
  }//end for k
 
  delete[] latitudes;
  delete[] longitudes;

  return 0;
};


void MSG3Sat2Mdv::_masterHdrAdjustTime()
{
  _master_hdr.time_gen = time(0);

  _master_hdr.time_begin = _startTime.utime();

  _master_hdr.time_end = _endTime.utime();
 
  _master_hdr.time_centroid =
    (_master_hdr.time_begin / 2) + (_master_hdr.time_end / 2);

  _master_hdr.time_expire = _master_hdr.time_end;
}

void MSG3Sat2Mdv::_setMasterHeader()
{
  //
  // Fill out Master Header
  //
  Mdvx::master_header_t _master_hdr;

  memset(&_master_hdr, 0, sizeof(_master_hdr));

  _master_hdr.data_collection_type = Mdvx::DATA_MEASURED;

  _master_hdr.grid_orientation = Mdvx::ORIENT_SN_WE;

  _master_hdr.data_ordering = Mdvx::ORDER_XYZ;

  _master_hdr.vlevel_included = 1;

  strncpy(_master_hdr.data_set_info,
          "EUMETSAT MSG satellite data ingested by MSG3Sat2Mdv", MDV_INFO_LEN);

  strncpy(_master_hdr.data_set_name, "MSG15", MDV_NAME_LEN);

  strncpy(_master_hdr.data_set_source, "EUMETSAT", MDV_NAME_LEN);
}

void MSG3Sat2Mdv::_setVlevelHdr()
{
  //
  // fill out the vertical level header
  //
  memset(&_vlevel_hdr, 0, sizeof(_vlevel_hdr));

  _vlevel_hdr.type[0] = Mdvx::VERT_TYPE_SURFACE;

  _vlevel_hdr.level[0] = 0.0;

  return;
}

void MSG3Sat2Mdv::_setProjection() 
{
  //
  // Initialize the user defined projection
  //
  switch (_params.output_proj.proj_type)
  {
  case Params::PROJ_FLAT :
    _outputProj.initFlat(_params.output_proj.origin_lat,
			_params.output_proj.origin_lon,
			_params.output_proj.rotation,
			_params.output_proj.nx,
			_params.output_proj.ny,
			1,
			_params.output_proj.dx,
			_params.output_proj.dy,
			1.0,
			_params.output_proj.minx,
			_params.output_proj.miny,
			0.0);
    break;

  case Params::PROJ_LATLON :
    _outputProj.initLatlon(_params.output_proj.nx,
			  _params.output_proj.ny,
			  1,
			  _params.output_proj.dx,
			  _params.output_proj.dy,
			  1.0,
			  _params.output_proj.minx,
			  _params.output_proj.miny,
			  0.0);
    break;
    
  case Params::PROJ_LC2 :
    _outputProj.initLc2(_params.output_proj.origin_lat,
		       _params.output_proj.origin_lon,
		       _params.output_proj.lat1,
		       _params.output_proj.lat2,
		       _params.output_proj.nx,
		       _params.output_proj.ny,
		       1,
		       _params.output_proj.dx,
		       _params.output_proj.dy,
		       1.0,
		       _params.output_proj.minx,
		       _params.output_proj.miny,
		       0.0);
    break;
  }

  return;
}

void MSG3Sat2Mdv::_mapSatData2MdvArray(float* mdvDataVals, float *satDataVals, 
				       float *latitudes, float* longitudes)
{
  //
  //  Intialize variables for tracking max and min latitudes and longitudes for debugging
  //
  float minLat = 90;
  
  float maxLat = -90;
  
  float minLon = 180;
  
  float maxLon = -180;
  
  //
  // Initialize array
  //
  for (int i = 0; i < _params.output_proj.nx * _params.output_proj.ny; i++)
  {
    mdvDataVals[i] = SAT_MISSING;
  }
  
  //
  // Map the satellite data to the mdv projection
  //
  for (int i = 0; i < _nx*_ny; i++)
  { 
    int mdv_data_index;

    if ( latitudes[i] != GEO_MISSING &&  longitudes[i] != GEO_MISSING )
    {
      // for debuggubg 
      if ( latitudes[i] < minLat)
      {
	minLat = latitudes[i];
      }
      if ( latitudes[i] > maxLat)
      {
	maxLat =  latitudes[i];
      }
      if ( longitudes[i] < minLon)
      {
	minLon = longitudes[i];
      }
      if ( longitudes[i] > maxLon)
      {
	maxLon =  longitudes[i];
      }

      //
      // Map sat data location to mdv array postion
      //
      if (_outputProj.latlon2arrayIndex(latitudes[i], longitudes[i],
				       mdv_data_index) < 0)
      {
	if (_params.debug == Params::DEBUG_VERBOSE)
	{
	  cerr << "WARNING: MSG3Sat2Mdv::_createMdv:" <<  endl;
	  cerr << "Data point outside of output grid." << endl;
	  cerr << "lat = " << latitudes[i]
	       << ", lon = " << longitudes[i] << endl;
	}
	
	continue;
      }
    }
    //
    // Update the MDV data value
    //
    mdvDataVals[mdv_data_index] = satDataVals[i];
  }// end for

  if (_params.debug)
  {
    cerr  << "MSG3Sat2Mdv::_createMdv: field minLat: " << minLat << endl;
   
    cerr  << "MSG3Sat2Mdv::_createMdv: field maxLat: " << maxLat << endl;
    
    cerr  << "MSG3Sat2Mdv::_createMdv: field minLon: " << minLon << endl;
    
    cerr  << "MSG3Sat2Mdv::_createMdv: field maxLon: " << maxLon << endl;
  }
}

void MSG3Sat2Mdv::_setFieldHdr( )
{
  //
  // Set the field header
  //
  memset(&_field_hdr, 0, sizeof(_field_hdr));
   
  _field_hdr.nz = 1;
  
  _field_hdr.nx = _params.output_proj.nx;
  
  _field_hdr.ny = _params.output_proj.ny;

  _field_hdr.grid_minx = _params.output_proj.minx;
  
  _field_hdr.grid_miny = _params.output_proj.miny;

  _field_hdr.grid_dx = _params.output_proj.dx;
  
  _field_hdr.grid_dy = _params.output_proj.dy;

  switch (_params.output_proj.proj_type)
  {
  case Params::PROJ_FLAT :

    _field_hdr.proj_type = Mdvx::PROJ_FLAT;

    break;

  case Params::PROJ_LATLON :
    _field_hdr.proj_type = Mdvx::PROJ_LATLON;

    _field_hdr.proj_origin_lat =  _params.output_proj.origin_lat;

    _field_hdr.proj_origin_lon =  _params.output_proj.origin_lon;

    break;

  case Params::PROJ_LC2 :

    _field_hdr.proj_type = Mdvx::PROJ_LAMBERT_CONF;

    _field_hdr.proj_origin_lat =  _params.output_proj.origin_lat;

    _field_hdr.proj_origin_lon =  _params.output_proj.origin_lon;

    _field_hdr.proj_param[0] = 	_params.output_proj.lat1;      

    _field_hdr.proj_param[1] = _params.output_proj.lat2;

    break;
  }
  
  _field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  
  _field_hdr.data_element_nbytes = 4;
  
  _field_hdr.volume_size =
    _field_hdr.nx * _field_hdr.ny * _field_hdr.nz * _field_hdr.data_element_nbytes;
  
  _field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  
  _field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
 
  _field_hdr.scaling_type = Mdvx::SCALING_DYNAMIC;
 
  _field_hdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  
  _field_hdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  
  _field_hdr.dz_constant = 1;
  
  _field_hdr.data_dimension = 2;
    
  _field_hdr.grid_dz = 0;
  
  _field_hdr.grid_minz = 0;
  
  _field_hdr.bad_data_value = SAT_MISSING;
  
  _field_hdr.missing_data_value = SAT_MISSING;
}
   
void MSG3Sat2Mdv::_fieldHeaderAdjustNames( char *fieldNameLong, 
					   char *fieldNameShort, 
					   char *units )
{
  strncpy(_field_hdr.field_name_long, fieldNameLong, MDV_LONG_FIELD_LEN);
  
  strncpy(_field_hdr.field_name, fieldNameShort, MDV_SHORT_FIELD_LEN);
  
  strncpy(_field_hdr.units,  units, MDV_UNITS_LEN);
}


void MSG3Sat2Mdv::processSolarAngles(const time_t satTime, const float latitude, const float longitude,
				     float &sunEarthDist, float &azimuth) 

{
  //
  // Convert unix time to struct tm
  //
  struct tm uTimeStruct = *gmtime(&satTime);

  //
  // Define sun azimuth elevation struct from sunae.h
  //
  ae_pack sunaeIO;

  //
  // Fill in inputs to struct
  //
  sunaeIO.year = uTimeStruct.tm_year + 1900;

  sunaeIO.doy =  getJulianDay(satTime);

  sunaeIO.hour = uTimeStruct.tm_hour;

  sunaeIO.lat = latitude;

  sunaeIO.lon = longitude;

  //
  // call sunae code
  //
  double someRes = sunae(&sunaeIO);

  //
  // Get az and distance to sun as output from struct
  //
  azimuth = sunaeIO.az * 3.14159265358979/180;

  sunEarthDist =  sunaeIO.soldst;
}

int MSG3Sat2Mdv::getJulianDay(const time_t uTime)
{
  //
  // Convert unix time to struct tm
  //
  struct tm uTimeStruct = *gmtime(&uTime);

  //
  // Create start of the year struct tm.
  // Retain the year but fill in struct for January 1
  //
  struct tm startOfYear;

  startOfYear.tm_year = uTimeStruct.tm_year;

  startOfYear.tm_mon = 0;

  startOfYear.tm_hour = 0;

  startOfYear.tm_mday = 1;

  startOfYear.tm_min = 0;

  startOfYear.tm_sec = 0;

  startOfYear.tm_yday = 0;

  startOfYear.tm_isdst = -1;

  //
  // Convert startOfYear to time_t
  //
  time_t startOfYearSecs = timegm(&startOfYear);

  int delta = uTime - startOfYearSecs;

  //
  // julian day is in range [1,366]
  //
  int jday = 1 + delta/86400;

  return jday;
}

