
#include "PolarSat2Mdv.hh"

#include <Mdv/DsMdvx.hh>
#include <toolsa/pmu.h>

#include <cmath>
#include <math.h>
#include <Ncxx/Ncxx.hh>

using namespace std;

const float PolarSat2Mdv::GEO_MISSING = -999.9;
const float PolarSat2Mdv::SAT_MISSING = -9999.9;
const float PolarSat2Mdv::R_km = 6371;
const float PolarSat2Mdv::MYPI = 3.1415926535897932384626433832;
const float PolarSat2Mdv::EPSILON = .0000000001;
const float PolarSat2Mdv::VERY_LARGE_NUM = 1000000000;
const float PolarSat2Mdv::VERY_SMALL_NUM = -1000000000;
const int   PolarSat2Mdv::NUM_SPARSE_PTS_CROSS_TRACK = 3;
const int   PolarSat2Mdv::NUM_GRING_PTS = 9;
const string PolarSat2Mdv::GRING_LAT_ATT_NAME = string("g_ring_latitude");
const string PolarSat2Mdv::GRING_LON_ATT_NAME = string("g_ring_longitude");

PolarSat2Mdv::PolarSat2Mdv(int argc, char **argv):
_progName("PolarSat2Mdv"),
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
  
  //
  // Initialize data arrays, flags, and other variables
  //
  _latitudes = NULL;

  _longitudes = NULL;

  _satDataVals = NULL;

  _mdvDataVals = NULL;
 
  _haveDataInMdvArray = false;

  _dataTimeSpan = 0;

  return;
}

PolarSat2Mdv::~PolarSat2Mdv()
{
  //
  // Free allocated memory
  //
  _clearMdvData();

  _clearNcfData();

  //
  // unregister process
  //
  PMU_auto_unregister();  
}

void PolarSat2Mdv::_clearNcfData()
{
  if (_latitudes)
  {
    delete[] _latitudes;
  }
  _latitudes = NULL;

  if (_longitudes)
  {
    delete[] _longitudes;
  }
  _longitudes = NULL;

  if (_satDataVals)
  {
    delete[] _satDataVals;
  }
  _satDataVals = NULL;
}

void PolarSat2Mdv::_clearMdvData()
{
   if (_mdvDataVals)
   {
    delete[] _mdvDataVals;
   }
  _mdvDataVals = NULL;

  _haveDataInMdvArray = false;
 
  _dataTimeSpan = 0;
}

int PolarSat2Mdv::Run()
{
  //
  // register with procmap
  //
  PMU_auto_register("Run");

  //
  // Basic header data doesnt change file to file so set it once
  // We will adjust time meta data in master header necessary
  //
  _setMasterHeader();
  
  _setVlevelHdr();
  
  _setProjection();
  
  _setFieldHdr();

  //
  // process data
  //
  char *inputPath;
  
  int timeBetweenFiles = 0;

  //
  // Process indefinitely in real-time or until there arent any files left in archive mode
  //
  while (true) 
  {
    if ((inputPath = _fileTrigger->next(false)) != NULL ||
      (timeBetweenFiles > _params.max_time_between_files)) 
      {
	if ( inputPath )
	{ 
	  if (_processData(inputPath))
	  {
	    cerr << "PolarSat2Mdv::Run(): Errors in processing file or satellite path does not overlap the domain of interest specified by the user: " <<  inputPath << endl;
	     if( _haveDataInMdvArray)
	       {
		 _writeMdv();
	       }
	     timeBetweenFiles = 0;
	  }
	}
	else
	{
	  //
	  // We are in realtime processing mode and too much time has elapsed, write the stored data to disk
	  //
	  if( _haveDataInMdvArray)
	  {
	    if(_params.debug)
	    {
	      cerr << "PolarSat2Mdv::Run(): Too much time has elapsed between files: " 
		   << timeBetweenFiles <<  " Writing data." << endl;
	    }
	    _writeMdv();
	  }
          else
          {
             PMU_auto_register("Sleep");

	     if(_params.debug == Params::DEBUG_NORM)
	       {
                 cerr << " PolarSat2Mdv::Run(): Waiting...Time between files: " << timeBetweenFiles << endl;
	       }

             sleep(_params.sleep_secs);
     
             timeBetweenFiles+=_params.sleep_secs;
          }
	}
      } // if have file or too much time has passed
    else if (_params.mode == Params::REALTIME)
    {
       PMU_auto_register("Run");

      //
      // We do not have a file in realtime so track the time
      //
      sleep(_params.sleep_secs);
      
      timeBetweenFiles+=_params.sleep_secs;
      
      if(_params.debug == Params::DEBUG_NORM)
      {
	cerr << " PolarSat2Mdv::Run(): Waiting...Time between files: " << timeBetweenFiles << endl;
      }
    }
    else 
    {
      //
      // We are done
      //
      if (inputPath == NULL)
      {
	if( _haveDataInMdvArray)
	  {
	    _writeMdv();
	  }
	if(_params.debug)
	{
	  cerr << " PolarSat2Mdv::Run(): No more archive files found. Exiting\n";
	}
	return 0;
      }
    }
  }// while
  
  delete _fileTrigger;
  
  return 0;
}

int PolarSat2Mdv::_processData(char *inputPath)

{
  if (_params.debug)
  {
    cerr << "PolarSat2Mdv::_processData: Processing file : " << inputPath << endl;
  }
  
  //
  // registration with procmap
  //
  PMU_force_register("Processing data");
  
  //
  // Read input NetCDF file
  //
  if (_readNetcdf(inputPath))
    return 1;

  //
  // Check to see if adding current file to stored Mdv data violates
  // time constraints. If yes, write the stored data to a file before processing.
  //
  if(  _startTime.utime() - _master_hdr.time_begin > _params.max_data_time_span_per_file  && _haveDataInMdvArray)
  {
    if (_params.debug)
    {
      cerr << "PolarSat2Mdv::_processData(): The current file will start a new Mdv file since otherwise the time span of data would be " <<  _startTime.utime() - _master_hdr.time_begin  <<   " seconds which is greater than the maximum time span "  <<  _params.max_data_time_span_per_file << " Writing stored data to disk. " << endl;
    }

    //
    // Write the existing data
    //
    _writeMdv();
  }

  //
  // Configure time members of master header with latest data information
  //
  _masterHdrAdjustTime();
  
  //
  // Map satellite data to user specified projection
  //
  _mapSatData2MdvArray();
  
  //
  // Clean up memory allocation needed for reading NetCDF file
  //
  _clearNcfData();

  //
  // Check time constraints of stored data, write file if necessary
  //
  if(_dataTimeSpan > _params.max_data_time_span_per_file  && _haveDataInMdvArray)
  {
    if (_params.debug)
    {
      cerr << "PolarSat2Mdv::_processData(): The stored data spans " <<  _dataTimeSpan <<   " seconds which is greater than the maximum time span "  <<  _params.max_data_time_span_per_file << " Writing stored data to disk. " << endl;
    }

    //
    // Write the stored data to a file
    //
    _writeMdv();
  }

  //
  // If not collecting granules, write data from each NetCDF file to separate Mdv file
  //
  if ( !_params.collect_granular_data)
  {
    _writeMdv();
  }
 
  return 0;
}

int PolarSat2Mdv::_writeMdv()
{
  DsMdvx mdv;
  
  //
  // Sync headers with projection, add master header, field header to Mdvx object
  //
  _outputProj.syncToHdrs(_master_hdr,_field_hdr);
  
  mdv.setMasterHeader(_master_hdr);
    
  MdvxField *satField = new MdvxField(_field_hdr, _vlevel_hdr, (void *)_mdvDataVals); 

  mdv.addField(satField);

  if (mdv.writeToDir(_params.output_url)) 
  {
    cerr << "PolarSat2Mdv::_writeData(): Error. Cannot write mdv file" << endl;
    
    cerr << mdv.getErrStr() << endl;
    
    return 1;
  }
    
  if (_params.debug) 
  {
    cerr << "PolarSat2Mdv::_writeData(): Wrote file: " << mdv.getPathInUse() << endl;
  }

  //
  // Memory clean up
  //
  _clearMdvData();

  return 0;
}

int PolarSat2Mdv::_readNetcdf(char *filename)
{
  //
  // registration with procmap
  //
  PMU_force_register("Reading netCDF data");
 
  if (_params.debug) 
  {
    cerr << "PolarSat2Mdv::_readNetcdf(): Reading file: " << filename << endl;
  }

  //
  //  Cross track geolocation data 
  //
  _numPtsCrossTrack = _params.cross_track_dim;

  //
  // Along track geolocation data
  //
  _numPtsAlongTrack = _params.along_track_dim;
 
  //
  // Open the file. 
  // 
  NcxxFile ncFile(filename, NcxxFile::read);


  if (_params.check_domain_before_process)
  {
    //
    // Get g-ring data to determine swath boundaries and processing
    //
    NcxxGroupAtt latGRingAtt =  ncFile.getAtt(GRING_LAT_ATT_NAME);
    
    NcxxGroupAtt lonGRingAtt =  ncFile.getAtt(GRING_LON_ATT_NAME);
    
    float latGRingData [NUM_GRING_PTS];

    float lonGRingData [NUM_GRING_PTS];
    
    latGRingAtt.getValues( latGRingData);
    
    lonGRingAtt.getValues( lonGRingData);
    
    if (!_satPathOverlapsDomain(latGRingData, lonGRingData))
    {
	return 1;
    }
  }

  //
  // Get start time
  //
  NcxxGroupAtt startTimeAtt = ncFile.getAtt("time_coverage_start");
  
  string startTime;
  
  startTimeAtt.getValues (startTime);
  
  _startTime.setFromW3c(startTime.c_str());

  // Get end time

  NcxxGroupAtt stopTimeAtt = ncFile.getAtt("time_coverage_end");
  
  string endTime;
  
  stopTimeAtt.getValues (endTime);
  
  _endTime.setFromW3c(endTime.c_str());

  if (_params.debug)
  {
    cerr << "PolarSat2Mdv::_readNetcdf(): startTime: " << startTime.c_str() << endl;
  
    cerr << "PolarSat2Mdv::_readNetcdf(): endTime: " << endTime.c_str() << endl;
  }

  //
  // Get sparse latitude data
  //
  NcxxVar latData =  ncFile.getVar(_params.sparse_lat_fieldname);

  if(latData.isNull()) 
  {
    cerr << "PolarSat2Mdv::_readNetcdf(): Failure to get sparse latitude data. Exiting" << endl;
   
    return 1;
  }

  float lats[_params.along_track_dim][NUM_SPARSE_PTS_CROSS_TRACK];
  
  latData.getVal(lats);

  //
  // Get sparse longitude data
  //
  NcxxVar lonData =  ncFile.getVar(_params.sparse_lon_fieldname);
 
  if (lonData.isNull())
  {
    cerr << "PolarSat2Mdv::_readNetcdf(): Failure to get sparse longitude data. Exiting" << endl;
   
    return 1;
  }

  float lons [_params.along_track_dim][NUM_SPARSE_PTS_CROSS_TRACK];

  lonData.getVal(lons);
  
  //
  // Get data field
  //
  NcxxVar satData = ncFile.getVar(_params.sat_data_fieldname);
  
  if (satData.isNull())
  {
    cerr << "PolarSat2Mdv::_readNetcdf(): Failure to get reflectance. Exiting." << endl;
   
    return 1;
  }

  unsigned short *satShortVals = new ushort[_numPtsAlongTrack* _numPtsCrossTrack];
   
  satData.getVal(satShortVals);

  //
  //  Get scale and bias
  //
  NcxxVarAtt scaleAtt = satData.getAtt("scale_factor");

  if (scaleAtt.isNull())
  {
    cerr << "PolarSat2Mdv::_readNetcdf(): Failure to get scale factor. Exiting." << endl;
   
    return 1;
  }

  float satFieldScale;

  scaleAtt.getValues(&satFieldScale);
  
  
  NcxxVarAtt biasAtt = satData.getAtt("add_offset");

  if (biasAtt.isNull())
  {
    cerr << "PolarSat2Mdv::_readNetcdf(): Failure to get add_offset. Exiting." << endl;
   
    return 1;
  }

  float satFieldBias;

  biasAtt.getValues(&satFieldBias);

  _satDataVals = new float[ _numPtsAlongTrack * _numPtsCrossTrack];

  
  //
  // Initialize variables for keeping track of min and max values of data field
  //
  float min = VERY_LARGE_NUM;

  float max = VERY_SMALL_NUM;

  //
  // Apply scale and bias and copy to float array (user can decide output data type)
  //
  for (int j = 0; j <  _numPtsAlongTrack; j++)
  {
    for (int i = 0; i <  _numPtsCrossTrack; i++)
    {
      if ( satShortVals[j*_numPtsCrossTrack + i] >= _params.min_sat_missing_or_bad)
      {
  	_satDataVals[j*_numPtsCrossTrack + i] = SAT_MISSING ;
      }
      else
      {
	_satDataVals[j*_numPtsCrossTrack + i] = satShortVals[j*_numPtsCrossTrack + i] * satFieldScale + satFieldBias;
      
	//
	// Record max and min values for debug purposes
	//
	if (_satDataVals[j*_numPtsCrossTrack + i] < min)
	{
	  min = _satDataVals[j*_numPtsCrossTrack + i];
	}
	if (_satDataVals[j*_numPtsCrossTrack + i] > max)
	{
	  max = _satDataVals[j*_numPtsCrossTrack + i];	
	}
      }// end else
    } // end for i
  }// end for j

  delete[] satShortVals;

  if (_params.debug)
  {
    cerr << "PolarSat2Mdv::_readNetcdf(): min data field value: " << min << endl;
    cerr << "PolarSat2Mdv::_readNetcdf(): max data field value: " << max << endl;
  }

  //
  // Create location data for every data field point
  // 
  _sparseToFullGeolocationData(lats,lons);
  
  return 0;
};


int PolarSat2Mdv::_sparseToFullGeolocationData(float sparseLats[][3], 
					       float sparseLons[][3])
{
  _latitudes = new float [_numPtsAlongTrack* _numPtsCrossTrack];

  _longitudes = new float[_numPtsAlongTrack* _numPtsCrossTrack];

  //
  // Initialize the location data to missing
  //
  for (int j = 0; j <  _numPtsAlongTrack; j++)
  {
    for (int i = 0; i <  _numPtsCrossTrack; i++)
    {
      _latitudes[ j* _numPtsCrossTrack + i] = GEO_MISSING;

      _longitudes[ j* _numPtsCrossTrack + i] = GEO_MISSING;
    }
  }

  //
  // Put the sparse  latitude data in to the (full) latitude array
  //
  for (int j = 0; j <  _numPtsAlongTrack; j++)
  { 
    //
    // Center ground track
    //
    if ( -90 <= sparseLats[ j][1]  && sparseLats[ j][1] <= 90)
    {
      _latitudes[j* _numPtsCrossTrack + (_numPtsCrossTrack -1)/2] = sparseLats[ j][1];
    }
  }

  //
  // Put the sparse  latitude data in to the (full) latitude array
  //
  for (int j = 0; j <  _numPtsAlongTrack; j++)
  {
    //
    // First element of cross track at along track row j;
    //
    if ( -90 <=  sparseLats[ j][0] && sparseLats[ j][0] <= 90)
    {
      _latitudes[ j* _numPtsCrossTrack] = sparseLats[ j][0];
    }

    //
    // Center ground track
    //
    if ( -90 <= sparseLats[ j][1]  && sparseLats[ j][1] <= 90)
    {
      _latitudes[j* _numPtsCrossTrack + (_numPtsCrossTrack -1)/2] = sparseLats[ j][1];
    }

    //
    // Last element of cross track at along track row j;
    //
    if (  -90 <= sparseLats[ j][2] &&   sparseLats[ j][2] <= 90)
    {
      _latitudes[j* _numPtsCrossTrack + _numPtsCrossTrack -1]= sparseLats[j][2];
    }
  }

  //
  // Put the sparse longitude data in to the (full) longitude array
  //
  for (int j = 0; j <  _numPtsAlongTrack; j++)
  {
    //
    // First element of cross track at along track row j;
    //
    if (  -180 <= sparseLons[ j][0] && sparseLons[ j][0] <= 180)
    {
      _longitudes[ j* _numPtsCrossTrack] = sparseLons[j][0];
    }

    //
    // Center ground track
    //
    if (  -180 <= sparseLons[ j][1]  && sparseLons[ j][1] <= 180)
    {
      _longitudes[j*  _numPtsCrossTrack +  (_numPtsCrossTrack -1)/2] = sparseLons[j][1];
    }

    //
    // Last element of cross track at along track row j;
    //
    if (  -180 <= sparseLons[ j][2]  && sparseLons[ j][2] <= 180)
    {
      _longitudes[j* _numPtsCrossTrack + _numPtsCrossTrack -1] = sparseLons[j][2];
    }
  }

  //
  // Find bearing from ground track location to end points of the cross track
  // 
  float *bearingToPt0 = new float[_params.along_track_dim];
  float *bearingToPt2 = new float [_params.along_track_dim];
  for (int j = 0; j <  _numPtsAlongTrack; j++)
  {
    //
    // Find bearing from ground track loacation to one end point of the cross track
    // 
    if ( ( -90 <= sparseLats[j][1]  && sparseLats[j][1] <= 90) &&
	 ( -90 <= sparseLats[j][0]  && sparseLats[j][0] <= 90) &&
	 (-180 <= sparseLons[j][1]  && sparseLons[j][1] <=180) &&
	 (-180 <= sparseLons[j][0]  && sparseLons[j][0] <=180 ))
    {
      bearingToPt0[j] = _bearing (sparseLats[j][1],
      				  sparseLons[j][1],
      				  sparseLats[j][0],
      				  sparseLons[j][0]);
    }
    else
    {
      bearingToPt0[j] = GEO_MISSING;
    }

    if ( ( -90 <= sparseLats[j][1]  && sparseLats[j][1] <= 90) &&
	 ( -90 <= sparseLats[j][2]  && sparseLats[j][2] <= 90) &&
	 (-180 <= sparseLons[j][1]  && sparseLons[j][1] <=180) &&
	 (-180 <= sparseLons[j][2]  && sparseLons[j][2] <=180 ))
    { 
      bearingToPt2[j] = _bearing (sparseLats[j][1],
      				  sparseLons[j][1],
      				  sparseLats[j][2],
      				  sparseLons[j][2]);
    }
    else
    {
      bearingToPt2[j] = GEO_MISSING;
    }
  }

  //
  // Fill in the rest of latitudes and longitudes arrays using
  // speherical trigonometry and the bearing of cross track 
  // points previously calculated and the fact each cross track point
  // is .375km from its neighboring cross track points 
  // 
  for (int j = 0; j <  _numPtsAlongTrack; j++)
  {
    for (int i = 1; i < (_numPtsCrossTrack -1)/2; i++)
    {
      float distanceKm = .375 * ((_numPtsCrossTrack -1)/2 - i);
      
      if( bearingToPt0[j] != GEO_MISSING)
      {
	_startPtBearingDist2LatLon(sparseLats[j][1], 
				   sparseLons[j][1], 
				   bearingToPt0[j],  
				   distanceKm,
				   _latitudes[j*_numPtsCrossTrack+i],  
				   _longitudes[j*_numPtsCrossTrack+i]);
      }
    } 
   
    for (int i = (_numPtsCrossTrack -1)/2 + 1; i < _numPtsCrossTrack-1; i++)
    {
      float distanceKm = .375 * (i - (_numPtsCrossTrack)/2);
       
      if( bearingToPt2[j] != GEO_MISSING)
      {	
	_startPtBearingDist2LatLon(sparseLats[j][1], 
				   sparseLons[j][1], 
				   bearingToPt2[j],
	 			   distanceKm,
	 			   _latitudes[j*_numPtsCrossTrack+i],  
	 			   _longitudes[j*_numPtsCrossTrack+i]);	
      }
    } // end for i
  }// end for j

  delete[] bearingToPt0;

  delete[] bearingToPt2;

  return 0;  
}

float PolarSat2Mdv::_bearing (float lat0Deg, float lon0Deg, float lat1Deg, float lon1Deg)
{
  if( lon0Deg == lon1Deg && lat1Deg == lat1Deg)
  {
    return 0;
  }
  else
  {
    /*
    *                        B ( angle at pole = lon0 - lon1)
    *                        ^
    *    side c = 90-lat1   / \ side a  = 90- lat0
    *                      /                    \
    *         (lon1,lat1)A ----- C (lon0,lat0)
    *    side b  solve: cos(b) = cos (a) * cos (c) + sin (a) * sin (c) * cos (B)
    *                          
    * Law of Cosines (spherical) cos(c) = cos(a) * cos(b) * + sin(a) * sin(b) * cos(C)
    * Law of Cosines (spherical) cos(b) = cos(a) * cos(c) + sin(a) * sin (c) * cos (B)
    * Law of Sines (spherical) 	 sin(B)/sin(b) = sin(A)/sin(a) = sin(C)/sin(c)
    *
    */
  
    float a = (90 - lat0Deg)*MYPI/180;
    
    float c = (90 - lat1Deg) * MYPI/180;
    
    float B = fabs((lon0Deg-lon1Deg)* MYPI/180);
    
    float b = acos(cos(a) * cos(c) + sin(a) * sin(c) * 
                   cos ((lon1Deg - lon0Deg) * MYPI/180));
    
    // bearing from C to A:
    // cos(C) = (cos(c) - cos(a)*cos(b))/(sin(a)*sin(b))

    float bearing = acos ( (cos(c) - cos(a)*cos(b))/(sin(a)*sin(b)) );
    
    bool pt1IsEast = false;
    
    if ( (0 <= lon1Deg - lon0Deg && lon1Deg - lon0Deg <= 180)  ||
	 (lon1Deg - lon0Deg < 0 &&  lon1Deg - lon0Deg <= -180))
    {
      pt1IsEast = true;
    }
    else
    {
      pt1IsEast = false;
    }
    
    if (pt1IsEast)
    {
      return ( bearing * 180/MYPI);
    }
    else
    {
      return ( 360.0 - bearing * 180/MYPI);
    }
  } 
}
 
void PolarSat2Mdv::_startPtBearingDist2LatLon(float startLatDeg, float startLonDeg, float bearingDeg, 
					      float distanceKm, float &endLatDeg, float &endLonDeg) 
{
  /*                        B ( angle at pole = lon0 - lon1)
   *                        ^
   *    side c = 90-lat1   / \ side a  = 90- lat0
   *                      /                     \
   *         (lon1,lat1)A ----- C  (lon0,lat0)
   *                      side b 
   *                         
   * Law of Cosines (spherical) 
   * cos(c) = cos(a) * cos(b)  + sin(a) * sin(b) * cos(C)
   */
 
  float a = (90 - startLatDeg)*MYPI/180;
  
  float C;
 
  if ( 0 <= bearingDeg && bearingDeg <= 180)
  {
    C = bearingDeg*MYPI/180;
  }
  else if( 180 < bearingDeg && bearingDeg <= 360)
  {
    C = (360.0 - bearingDeg)*MYPI/180;
  }
  
  float b = distanceKm/R_km; 
  
  float cosc =  cos(a) * cos(b)  + sin(a) * sin(b) * cos(C);
  
  // bounds check for inverse cosine application

  if (cosc < -1.0) 
  {
    cosc = -1.0;
  }
  else if (cosc > 1.0)
  {
    cosc = 1.0;
  }

  float c = acos(cosc);

  // 
  // One output of method: latitude
  // 
  endLatDeg = 90.0 - c*180/MYPI;

 
  float B;

  if ( sin (a)*sin(c) < EPSILON)
  { 
    endLonDeg = startLonDeg;
  }
  else
  {

    // cos(b) = cos(a) * cos(c)  + sin(a) * sin(c) * cos(B)
    // so  cos(B) = (cos(b) - cos(a) * cos(c))/ sin(a) * sin(c)

    float cosB = (cos(b) -  cos(a)*cos(c))/(sin (a)*sin(c));
   
    // bounds check for inverse cosine application

    if (cosB < -1.0)
    {
       cosB = -1.0;
    }

    if (cosB > 1.0)
    {
      cosB = 1.0;
    }

    B = acos(cosB);

    if (  bearingDeg >180) 
    {
      // west
      endLonDeg =  startLonDeg - B*180/MYPI;
    }
    else
    {
      // east
      endLonDeg =  startLonDeg + B*180/MYPI;
    }

    // check bounds
    if ( endLonDeg > 180)
    {
      endLonDeg = 360 - endLonDeg;
    }
    else if ( endLonDeg < -180 )
    {
      endLonDeg = 360 + endLonDeg;
    }
  }
}
void PolarSat2Mdv::_masterHdrAdjustTime()
{
  _master_hdr.time_gen = time(0);

  if( _haveDataInMdvArray)
  {
    // We already have the start time so just record new end time
    _master_hdr.time_end = _endTime.utime();
  }
  else
  {
    _master_hdr.time_begin = _startTime.utime();

    _master_hdr.time_end = _endTime.utime();
  }
  
  _master_hdr.time_centroid =
    (_master_hdr.time_begin / 2) + (_master_hdr.time_end / 2);

  _master_hdr.time_expire = _master_hdr.time_end;

  _dataTimeSpan =  _master_hdr.time_end - _master_hdr.time_begin;

}
void PolarSat2Mdv::_setMasterHeader()
{
  // Fill out Master Header
  Mdvx::master_header_t _master_hdr;

  memset(&_master_hdr, 0, sizeof(_master_hdr));

  _master_hdr.data_collection_type = Mdvx::DATA_MEASURED;

  _master_hdr.grid_orientation = Mdvx::ORIENT_SN_WE;

  _master_hdr.data_ordering = Mdvx::ORDER_XYZ;

  _master_hdr.vlevel_included = 1;

  strncpy(_master_hdr.data_set_info,
          "VIIRS satellite data ingested by PolarSat2Mdv", MDV_INFO_LEN);
  
  strncpy(_master_hdr.data_set_name, "PolarSat2Mdv", MDV_NAME_LEN);
  
  strncpy(_master_hdr.data_set_source,
          "VIIRS-I1-IMG-EDR, VIIRS-IMG-GTM-EDR-GEO", MDV_NAME_LEN);
}

void PolarSat2Mdv::_setVlevelHdr()
{
  //
  // fill out the vertical level header
  //
  memset(&_vlevel_hdr, 0, sizeof(_vlevel_hdr));

  _vlevel_hdr.type[0] = Mdvx::VERT_TYPE_SURFACE;

  _vlevel_hdr.level[0] = 0.0;

  return;
}

void PolarSat2Mdv::_setProjection() 
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

void PolarSat2Mdv::_mapSatData2MdvArray()
{
  //
  //  Intialize variables for tracking max and min latitudes and longitudes for debugging
  //
  float minLat = 90;
  
  float maxLat = -90;
  
  float minLon = 180;
  
  float maxLon = -180;

  if( !_haveDataInMdvArray)
  {
    //
    // Create mdv data array
    //
    _mdvDataVals = new float[_params.output_proj.nx * _params.output_proj.ny];
    
    //
    // Initialize array
    //
    for (int i = 0; i < _params.output_proj.nx * _params.output_proj.ny; i++)
    {
      _mdvDataVals[i] = SAT_MISSING;
    }
  }
  
  //
  // Map the satellite data to the mdv projection
  //
  for (int i = 0; i < _numPtsCrossTrack*_numPtsAlongTrack; i++)
  { 
    int mdv_data_index;

    if ( _latitudes[i] != GEO_MISSING &&  _longitudes[i] != GEO_MISSING )
    {
      //
      // Map sat data location to mdv array postion
      //
      if (_outputProj.latlon2arrayIndex(_latitudes[i], _longitudes[i],
				       mdv_data_index) < 0)
      {
	if (_params.debug == Params::DEBUG_VERBOSE)
	{
	  cerr << "DEBUG: PolarSat2Mdv::_createMdv:" <<  endl;
	  cerr << "Data point outside of output grid." << endl;
	  cerr << "lat = " << _latitudes[i]
	       << ", lon = " << _longitudes[i] << endl;
	}
	
	continue;
      }
    }
    //
    // Update the MDV data value
    //
    _mdvDataVals[mdv_data_index] = _satDataVals[i];
  }// end for

  _haveDataInMdvArray = true;
}

void PolarSat2Mdv::_setFieldHdr()
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
  
  strncpy(_field_hdr.field_name_long, _params.sat_data_fieldname, MDV_LONG_FIELD_LEN);
  
  strncpy(_field_hdr.field_name, _params.sat_data_short_name, MDV_SHORT_FIELD_LEN);
  
  strncpy(_field_hdr.units,  _params.sat_data_units, MDV_UNITS_LEN); 
}
   
bool PolarSat2Mdv::_satPathOverlapsDomain(float *latGRingData, float *lonGRingData)
{
  // 
  // initialize variables for determining min and max g-ring values for 
  // latitude and longitude
  //
  float gRingMinLat = 90;
  float gRingMaxLat = -90;
  float gRingMinLon = 180;
  float gRingMaxLon = -180;
  float gRingMaxNegLon = -180;
  
  if (_params.debug)
  {
    cerr << "PolarSat2Mdv::_satPathOverlapsDomain:G-ring lon and lat pairs " << endl;
  }

  //
  // Determine max and min lats and lons of sat track
  //
  if (_params.debug)
    {
      cerr << "PolarSat2Mdv::_satPathOverlapsDomain: lat,lon pairs: " << endl;
    }
  for (int i = 0; i < 9; i++)
  {
    if ( lonGRingData[i] < gRingMinLon)
      gRingMinLon = lonGRingData[i];
    if(  lonGRingData[i] > gRingMaxLon)
      gRingMaxLon = lonGRingData[i];
    
    if ( latGRingData[i] < gRingMinLat)
      gRingMinLat = latGRingData[i];
    if(  latGRingData[i] > gRingMaxLat)
      gRingMaxLat = latGRingData[i];
    
    if (lonGRingData[i] < 0 && lonGRingData[i] >  gRingMaxNegLon)
      gRingMaxNegLon = lonGRingData[i]; 

    if (_params.debug)
    {
      cerr << latGRingData[i] <<"," << lonGRingData[i] << endl;
    }
  }
  
  //
  // Find the max and min lon of the domain of interest specified by the user
  //
  double lowerLeftLat, lowerLeftLon, upperRightLat, upperRightLon;

  _outputProj.getLL(lowerLeftLat, lowerLeftLon);

  _outputProj.getUR(upperRightLat, upperRightLon);

  if (_params.debug)
  {
    cerr << "PolarSat2Mdv::_satPathOverlapsDomain:domain of interest min and max lat lon pairs:" << endl;
    cerr << lowerLeftLat << "," << lowerLeftLon << endl; 
    cerr << upperRightLat << "," <<  upperRightLon << endl;
  }
  
  //
  // If the min latitude of domain of interest > max latitude of the satellite path
  // or if the max latitude of the domain of interest < min latitude of the satellite path 
  // then the regions of satellite path and domain of interest will not intersect.
  //
  if( gRingMaxLat < lowerLeftLat  || gRingMinLat > upperRightLat)
  {
    if (_params.debug)
    {
      cerr << "PolarSat2Mdv::_satPathOverlapsDomain: Latitudes of domain of interest and satellite "
	   << "path do not overlap" << endl;
    }
    
    return false;
  }
  else // check longitudes
  {

    float gRingLonDiff = gRingMaxLon - gRingMinLon;
    if ( 0 < gRingLonDiff && gRingLonDiff < 180 )
    {
      if (  gRingMaxLon < lowerLeftLon ||  gRingMinLon > upperRightLon)
      {	
	if (_params.debug)
	  {
	    cerr << "PolarSat2Mdv::_satPathOverlapsDomain: Longitudes of domain of interest and satellite "
		 << "path do not overlap" << endl;
	  }
	return false;
      }
      //
      // If satellite is away from the poles and spans the -180 meridian
      // sat path spans 180th meridian therefore satellite path longitudes are 
      // the union of  gRingMaxLon<lon<180  
      // and -180<lon<gRingMaxNegLon
      //
      else if( (-80 <  gRingMinLat && gRingMaxLat < 80) &&
	       ((0 <  upperRightLon &&  upperRightLon <  gRingMaxLon) || 
		(0 > lowerLeftLon &&  lowerLeftLon > gRingMaxNegLon)))
	{
	  if (_params.debug)
	    {
	      cerr << "PolarSat2Mdv::_satPathOverlapsDomain: Longitudes of domain of interest and satellite "
		   << "path do not overlap" << endl;
	    }
	  return false;
	}
    }
  }
  
  if(_params.debug)
    {
      cerr << "PolarSat2Mdv::_satPathOverlapsDomain: Domains overlap" << endl;
    }
  
  return true;
}
