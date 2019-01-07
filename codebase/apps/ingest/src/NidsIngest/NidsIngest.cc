/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1992 - 2004 */
/* ** University Corporation for Atmospheric Research(UCAR) */
/* ** National Center for Atmospheric Research(NCAR) */
/* ** Research Applications Program(RAP) */
/* ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA */
/* ** 2004/11/2 10:41:19 */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */

#include "NidsIngest.hh"
#include "lightweightDirSearch.hh"

#include <bzlib.h>
#include <cmath>
#include <map>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <cstdio>
#include <dirent.h>
#include <Spdb/DsSpdb.hh>
#include <toolsa/pmu.h>
#include <didss/DsInputPath.hh>
#include <rapformats/GenPoly.hh>
#include <euclid/PjgFlatCalc.hh>

NidsIngest::NidsIngest(Params *P, int iradar , int ifield){


  //
  // User defined parameters
  //
  _params = P;
    
  //
  // Last radar volume number processed.  
  //
  _prevVolNum = -1;

  //
  // Previous tilt number of volume processed.
  // Will be -1 at start of processing
  //
  _prevTiltNum = -1;
   
  //
  // Previous file suffix will be empty string at start
  // of processing
  //
  _prevFileSuffix = string("");

  //
  // Flag to indicate if plane of data exists for field. 
  //
  _haveData = false;

  //
  // Integer indicator or field or product to be processed
  //
  _ifield= ifield;

  //
  // Boolean first volume flag
  //
  _needFirstVolFirstTilt = false;

  //
  // Boolean star of volume flag
  //
  _startOfVol = false;

  //
  // Boolean end of volume flag
  //
  _endOfVol = false;

  //
  // Boolean for outputting data by last tilt in volume
  //
  _outputLastTiltInVol =  _params->_fields[ifield].outputLastTiltInVol;

  //
  // Last user specified tilt number which will be in the volume
  //
  _lastTiltNum = _params->_fields[ifield].lastTiltNum;

  _radarName =   _params->_radars[iradar];

  //
  // Create path to input data:
  // inputDir/radarName/productName
  //
  string pathDelim = "/";

  string indir = _params->topInDir;

  _inDir = indir + pathDelim + _params->_radars[iradar] + 
    pathDelim + _params->_fields[ifield].inFieldName;

  //
  // Create path to output data
  // ouputDir/radarName/productName/
  //
  string outurl = _params->topOutUrl;

  _outUrl = outurl + pathDelim +  _params->_radars[iradar] + pathDelim +  
     _params->_fields[ifield].outDirName;

  //
  // Create path to output spdb data
  // ouputDir/radarName/productName/
  //
  string outSpdbUrl = _params->topOutSpdbUrl;
  
  _outSpdbUrl = outSpdbUrl + pathDelim +  _params->_radars[iradar];

  //
  // Boolean for outputting point data (NHI TVS NME) in text format
  //
  _outputTextPointData =  _params->outputTextPointData;
  
  //
  // Create path to output text data
  // ouput/radarName/productName/
  //
  _outTextUrl = string(_params->topOutTextUrl) +  pathDelim + 
    _params->_radars[iradar] + pathDelim +   _params->_fields[ifield].outDirName;
  

  //
  // Option to save data as radial data. If false, data is saved in 
  // cartesian format according to user defined parameters
  //
  _saveAsRadial = _params->_fields[ifield].saveAsRadial;

  //
  // Process available files in real-time or archive modes
  //
  if (_params->mode == Params::REALTIME)
  {
    //
    // Boolean flag to indicate we must find the proper lowest 
    // elevation tilt to start processing. Eliminates files containing
    // partial volumes
    //
    _needFirstVolFirstTilt = true;
    
    //
    // Sleep, if we are staggering startup. No
    // need to register with procmap as parent
    // process is doing that.
    //
    if (_params->stagger.staggerProcs)
    {
      sleep(iradar * _params->stagger.procStaggerSecs );
    }

    if (_params->lightweightFileSearch)
    {
      //
      // Dated directory structure is assumed
      //
      lightweightDirSearch L((char *)_inDir.c_str(),  
			     _params->max_realtime_valid_age, 
			     _params->debugRealtimeFileSearch, 
			     _params->dirScanSleep);
      while(1)
      {
        PMU_auto_register("NidsIngest::NidsIngest: Getting next file");

	char *filePath = L.nextFile();

	if (filePath != NULL) 
	{
	  //
	  // process the file-- decode, convert to Mdv
	  //
	  _convertFile(filePath);
	}
      }
    } 
    else 
    {     
      //
      // File search with no assumption on subdirectory structure
      //
      DsInputPath *inPath = new DsInputPath("NidsIngest",
					    _params->debugRealtimeFileSearch,
					    _inDir.c_str(),
					    _params->max_realtime_valid_age,
					    PMU_auto_register,
					    false, true);
      
      inPath->setDirScanSleep( _params->dirScanSleep );
      
      while (1)
      {
	char *filePath = inPath->next();

	//
	// process file-- decode, convert to Mdv
	//
	if (filePath != NULL)
	{
	  _convertFile(filePath);
	}
      }
    }
  } 
  else 
  { 
    //
    //  Archive mode, process all files under _inDir
    //
    _procDir( _inDir );
  }
  
  return;
}

NidsIngest::~NidsIngest()
{
  cerr << "~NidsIngest(): Destructor" << endl;
  //
  // If we have a plane or volume of data, output it,
  // clean up, and reset the data state.
  // 
  if (_haveData && (int) _outOfOrderTilts.size() >0)
  {
    for (int i = 0 ; i < (int) _outOfOrderTilts.size(); i++)
    {
      _createMdvVolume( *(_outOfOrderTilts[i].first),  _outOfOrderTilts[i].second, false);
      
      delete (_outOfOrderTilts[i].first);
    }
    _outOfOrderTilts.clear();
  }
  
  else if (!_haveData && (int) _outOfOrderTilts.size() >0)
  {
    _createMdvVolume( *(_outOfOrderTilts[0].first),  _outOfOrderTilts[0].second, true);

    delete (_outOfOrderTilts[0].first);

    for (int i = 1; i < (int) _outOfOrderTilts.size(); i++)
    {
      _createMdvVolume( *(_outOfOrderTilts[i].first),  _outOfOrderTilts[i].second, false);
      
      delete (_outOfOrderTilts[i].first);
    }
  }
 
  _outOfOrderTilts.clear();
 
  _clearData();
}

void NidsIngest::_procDir(string dirName)
{
  //
  // Open directory
  //
  DIR *dirp = opendir( dirName.c_str() );
  
  if (dirp == NULL)
  {
    fprintf(stderr, "Cannot open directory %s\n", dirName.c_str());
    perror(dirName.c_str());
    return;
  }

  //
  // Get the the file list
  //
  struct dirent **namelist = NULL;
  
  int numfiles = scandir( dirName.c_str(), &namelist, 0, alphasort);
 
  //
  // We will insert files into a map container according to time 
  // so that files can be processed in time order rather than
  // alphbetical order
  // 
  map< time_t, string > timePathMap;

  for (int n = 0; n < numfiles; n++)
  {
    //
    // Do not process files beginning with '.' or '_'
    // 
    if( namelist[n]->d_name[0] == '.' || 
	namelist[n]->d_name[0] == '_') continue; 
   
    //
    // Create a fully qualified  path for the file
    // 
    string pathDelim = "/";

    string fileName(namelist[n]->d_name);

    string fullName = dirName + pathDelim + fileName;

    struct stat buf;
    
    //
    // Stat the file
    //
    if (stat(fullName.c_str(), &buf))
    {
      // Error occurred 
      continue; 
    }
    //
    // If file is a directory, recursively process
    // 
    if (S_ISDIR(buf.st_mode))
    {
      _procDir( fullName );
    }

    // If regular file, add to map container by time
    
    if (S_ISREG(buf.st_mode))
    { 
      timePathMap[buf.st_mtime]= fullName.c_str();
    }    
  }

  //
  // Process files in the map container
  //
  map<time_t, string >::iterator it;

  for(it = timePathMap.begin(); it != timePathMap.end(); it++)
  {
   _convertFile( (*it).second.c_str());
  }

  //
  // Cleanup the container
  //
  timePathMap.erase(timePathMap.begin(),timePathMap.end());
 
  //
  // Close the directory
  // 
  closedir(dirp);
  
  //
  // Free the memory allocated by scandir
  // 
  for (int n = 0; n < numfiles; n++)
  {
    free(namelist[n]);
  }
  free(namelist); 

  namelist = NULL;
  
  return;
}

void NidsIngest::_convertFile( const char *filePath )
{ 

  PMU_auto_register("Converting file to RAL formats");

  //
  // If we are using threads, make sure we are not exceeding the max number 
  // of threads that are actively processing.
  //
  if ( (!(_params->forkIt)) && (_params->threadLimit.limitNumThreads) )
  {
    while (1)
    {
      // Wait for another thread to finish processing if necessary
     
      if (nt < _params->threadLimit.maxNumThreads) 
	break;
      
      PMU_auto_register("Thread in wait mode while others process files");
      
      sleep(1); 
    }
    
    //
    // Increment threads in use 
    //
    pthread_mutex_lock(&ntMutex);
    
    nt++;
    
    pthread_mutex_unlock(&ntMutex);
    
    if (_params->debug)
    {
      cerr << "NidsIngest::_convertFile(): Acquired "
	   << "active thread number " << nt << " of " 
	   << _params->threadLimit.maxNumThreads << endl;
    }
  } 

  //
  // Read and decode input file
  // 
  NidsFile *nidsFile = new NidsFile(filePath, _params->byteSwap, _params->debug, _params->hasExtraHdr);

  //
  // If decoding fails, then free thread if necessary.
  //
  if ( nidsFile->decode() !=  0)
  {
    cerr << "NidsIngest::_convertFile(): "
	 << "Problem reading file " << filePath << ", skipping." << endl;

    delete(nidsFile);
    
    _freeThread();

    return;
  }

  //
  // In real-time we must start at the beginning of a volume.
  // (scientist requested)
  //
  if (_needFirstVolFirstTilt)
  {    
    cerr << "NidsIngest::_convertFile(): "
	 << "Looking for first first tilt of volume to begin processing.  " << endl;
    
    _setStartOfVolFlag(filePath,  nidsFile->getStartOfVolStr());

    if(!_startOfVol)
    {
      cerr << "File " << filePath << " is not the beginning of a volume, "
	   << "skipping." << endl;  
      
      delete(nidsFile);
      
      _freeThread();
      
      return;
    }
    else
    {
      _needFirstVolFirstTilt = false;
    }

  }

  //
  //  Throw away files that exceed the users specification for the last tilt.
  //  in volume
  // 
  if (_outputLastTiltInVol && nidsFile->getTiltIndex() > _lastTiltNum)
  {
    cerr << "NidsIngest::_convertFile(): "
	 << "Not processing " << filePath << ". Elevation number of product, "
	 << nidsFile->getTiltIndex() << ", exceeds last tilt parameter, " 
	 << _lastTiltNum << endl;

    delete(nidsFile);
    
    _freeThread();

    return;
  }

  DateTime currTime(time(0));

  cerr << "NidsIngest::_convertFile(): Processing " << filePath << "  (current time: " <<  currTime.dtime() << ")" << endl; 
  
  if( nidsFile->isContourData())
  {
    PMU_auto_register("NidsIngest::_convertFile():Creating contours");

    _createSpdbContours(*nidsFile);

    delete (nidsFile);
  
    _freeThread();
  }
  else if (nidsFile->isGenPtData())
  {
    PMU_auto_register("NidsIngest::_convertFile():Creating gen point");

    _createSpdbGenPt(*nidsFile);

    delete (nidsFile);
  
    _freeThread();
  }
  else // we have gridded data
  { 
    PMU_auto_register("NidsIngest::_convertFile(): Handling gridded data");
   
    _handleGriddedData(nidsFile, filePath);

    _freeThread();
  }

  return;
}

void NidsIngest::_handleGriddedData( NidsFile *nidsFile,  string filePath)
{
  //
  // Tilt number is the VCP scan number-- may not be in sequential order
  // and scan numbers that are in order can differ by more than 1, depending on the VCP 
  // and scans included in NIDS volume.
  //
  int tiltNum = nidsFile->getElevNumber();

  //
  // File suffix is used as "better" indicator for successive tilt index-- 1,2,3,4
  // (eg. N0S,N1S,N2S,N3S) and start of volume and end of volume flags.
  // When tilt number > 1, the suffix can indicate an extra .5 degree
  // scan in SAILS mode.
  //
  string currFileSuffix = nidsFile->getFileSuffix();
 
  int currVolNum =  nidsFile->getVolNum();

  if( currVolNum  == _prevVolNum && _haveData)
  {
    if(_params->debug)
    {
      cerr << "NidsIngest::_handleGriddedData(): We have tilts "
	   << "in the same volume." << endl;  
    }

    _newVolume = false;

    if ( !nidsFile->isNextTilt(_prevFileSuffix) ) 
    {
      if( nidsFile->getTiltIndex() != 1)
      {
	//
	// If this tilt is not in the expected order AND it is not another .5 degree tilt, save it
	//
	cerr << "NidsIngest::_handleGriddedData: WARNING: Tilt " << currFileSuffix.c_str() << " is out of order. "
	     << "Not processing file " << filePath <<". Saving for later." << endl;
	
	_saveOutOfOrderTilt(nidsFile,filePath,tiltNum);
	
	return;
      }
      else
      {
	//
	// This is a SAILS repeated .5 degree tilt. Delete it.
	//
	cerr << "NidsIngest::_handleGriddedData: WARNING: "
	     << "Not processing file " << filePath <<". This is repeated elevation (SAILS) scan." << endl;
	delete (nidsFile);

	return;
      }
    }
  }
  else
  {
    _newVolume = true;

    if (_params -> debug)
    {
      cerr << "NidsIngest::_handleGriddedData(): Starting new volume" << endl;  
    }
    
    // 
    // Write any data related to last volume if necessary
    //
    _dumpOldData(currVolNum);

    //
    // Determine if we have the first tilt of the new vol. Data may arrive
    // out of order.
    //
    _setStartOfVolFlag(filePath,  nidsFile->getStartOfVolStr());

    if(!_startOfVol)
    {
	cerr << "NidsIngest::_convertFile(): File " << filePath << " is not the beginning of a volume, "
	   << "File is out of order. Saving." << endl;

	_saveOutOfOrderTilt( nidsFile, filePath, tiltNum);
	
	return;
      }
    }

  _createMdvVolume(*nidsFile, filePath, _newVolume);

  delete (nidsFile);
  
  if(_haveData)
    _newVolume = false;

  //
  // Try to write any saved tilts if appropriate
  //
  _writeOutOfOrderTiltsCurrVol(currFileSuffix);

  
  return; 
}

void NidsIngest::_createMdvVolume( NidsFile & nidsFile, const string filepath, const bool newVolume)
{

  PMU_auto_register("NidsFile to Mdv or Spdb");
 
  _setEndOfVolFlag(filepath,  nidsFile.getEndOfVolStr(), nidsFile.getTiltIndex());
  
  if (newVolume)
  {
    if (_params->debug)
       cerr << "NidsIngest::_createMdvVolume()" << endl;

    //
    // Use 'memset' to set all the bytes in these structures to 0.
    //
    memset(&_fhdr, 0, sizeof(_fhdr));
    memset(&_vhdr, 0, sizeof(_vhdr));
    memset(&_mhdr, 0, sizeof(_mhdr));
    
    //
    // Set up master header.
    //
    _mhdr.data_collection_type = Mdvx::DATA_MEASURED;
    _mhdr.num_data_times = 1;
    _mhdr.data_ordering = Mdvx::ORDER_XYZ;
    _mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
    _mhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
    _mhdr.vlevel_included = 1;
    _mhdr.sensor_lat = nidsFile.getLat();
    _mhdr.sensor_lon = nidsFile.getLon();
    _mhdr.sensor_alt = nidsFile.getAlt();
    _mhdr.data_dimension = 2;
    _mhdr.vlevel_included = 1;
    sprintf(_mhdr.data_set_info,"%s","Level III data");
    sprintf(_mhdr.data_set_name,"%s","Level III data");
    sprintf(_mhdr.data_set_source,"%s", "Level III data");
    _mhdr.time_gen = nidsFile.getTime();
    _mhdr.time_begin = nidsFile.getTime();
    _mhdr.time_end = nidsFile.getTime();
    _mhdr.time_expire = nidsFile.getTime();
    _mhdr.time_centroid = nidsFile.getTime(); 
    
    //
    // Set up fieldheader
    //
    _fhdr.proj_origin_lat =  nidsFile.getLat();
    _fhdr.proj_origin_lon =  nidsFile.getLon();
    _fhdr.forecast_time = nidsFile.getTime();
    _fhdr.forecast_delta = 0;
    _fhdr.bad_data_value = nidsFile.getMissingFl32(); 
    _fhdr.missing_data_value = nidsFile.getMissingFl32();
    _fhdr.grid_minz = nidsFile.getElevAngle();
    
    _fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
    _fhdr.data_element_nbytes = sizeof(fl32);
    _fhdr.compression_type = Mdvx::COMPRESSION_NONE;
    _fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
    _fhdr.scaling_type = Mdvx::SCALING_NONE;
    sprintf(_fhdr.field_name, "%s", _params->_fields[_ifield].outFieldName);
    sprintf(_fhdr.field_name_long, _params->_fields[_ifield].outFieldNameLong);
    sprintf(_fhdr.units, _params->_fields[_ifield].outFieldUnits);
    sprintf( _fhdr.transform,"%s","none");
    
    if (_saveAsRadial)
    {
      _fhdr.nx = nidsFile.getNgates();
      _fhdr.ny = nidsFile.getNradials();
      _fhdr.grid_dx = nidsFile.getGateSpacing();
      _fhdr.grid_dy = nidsFile.getDelAz();
      _fhdr.grid_minx = nidsFile.getFirstGateDist();
      _fhdr.grid_miny = 0.0;
      _fhdr.proj_type = Mdvx::PROJ_POLAR_RADAR;
      
      //
      // This will change if there are more levels for this field
      //
      _fhdr.nz = 1; 
      _fhdr.grid_dz = 0;
      _fhdr.volume_size = _fhdr.nx * _fhdr.ny * _fhdr.nz * sizeof(fl32);
      _data = new fl32[ _fhdr.nx * _fhdr.ny];
      memcpy((void*)_data, (void*)  nidsFile.getAllRadialFl32s(),  _fhdr.volume_size);
    }
    else
    {
      nidsFile.remapToCart(_params->res.delta, _params->res.dist);
      _fhdr.nx = nidsFile.getNxy();
      _fhdr.ny = nidsFile.getNxy();
      _fhdr.grid_dx = nidsFile.getDxy();
      _fhdr.grid_dy = nidsFile.getDxy();
      _fhdr.grid_minx = -_fhdr.grid_dx*_fhdr.nx/2.0 + _fhdr.grid_dx/2.0;
      _fhdr.grid_miny = -_fhdr.grid_dy*_fhdr.ny/2.0 + _fhdr.grid_dy/2.0;
      _fhdr.proj_type = Mdvx::PROJ_FLAT;
      
      //
      // This will change if there are more levels for this field
      //
      _fhdr.nz = 1; 
      _fhdr.grid_dz = 0;
      _fhdr.volume_size = _fhdr.nx * _fhdr.ny * _fhdr.nz *sizeof(fl32);
      _data =  new fl32[ _fhdr.nx * _fhdr.ny];
      memcpy((void*)_data, (void*)  nidsFile.getCartData(),  _fhdr.volume_size);
    }
    
    //
    // Setup Vlevel header
    //
    _vhdr.type[0] = Mdvx::VERT_TYPE_ELEV;
    _vhdr.level[0] = nidsFile.getElevAngle();
    
    _haveData = true;
  } // end new volume 

  if (!newVolume)
  {

    if(_params->debug)
       cerr << "Adding to existing volume" << endl;
    
    //
    // Add data to existing volume. Modify Mdv header members appropriately
    //
    _fhdr.nz = _fhdr.nz + 1;
    _fhdr.dz_constant = 0;

    _mhdr.time_end = nidsFile.getTime();
    _mhdr.data_dimension = 3;
   
    _vhdr.type[_fhdr.nz -1] = Mdvx::VERT_TYPE_ELEV;
    _vhdr.level[_fhdr.nz -1] = nidsFile.getElevAngle();
   
    //
    // Expand  the _data array
    // Copy the data we have to tmp location
    // 
    fl32 *tmp = new fl32[_fhdr.volume_size/sizeof(fl32)];
    
    if(tmp)
    {
      memcpy((void*)tmp,(void*)_data, _fhdr.volume_size);
    }
    else
    {
      cerr << "NidsIngest::_createMdvVolume(): Memory allocation failed. "
	   << "Exiting." << endl;
      exit(1);
    }

    //
    // delete the old array and create new bigger one
    //
    if (_data)
    {
      delete[] _data; 
    }  
    
    //
    // Allocate space for current number of elements plus the new 
    // field
    //
    _data = new fl32[ _fhdr.volume_size/sizeof(fl32) +  (_fhdr.nx * _fhdr.ny)];

    if(!_data)
    {
      cerr << "NidsIngest::_createMdvVolume(): Memory allocation failed. "
	   << "Exiting." << endl;

	exit(1);
    }

    //
    // Copy the old data to this array
    //
    memcpy((void*)_data, (void*)tmp, _fhdr.volume_size);

    //
    // Clean up
    //
    if(tmp)
    {
      delete[] tmp;
    }

    //
    // Get ptr to space for new data-- _data + number of elements
    // currently stored.
    //
    fl32* newSpacePtr = _data + _fhdr.volume_size/ sizeof(fl32);

    //
    // Copy new data to the volume '_data' array
    //
    if(_saveAsRadial)
    {
      fl32 *radDataPtr = nidsFile.getAllRadialFl32s(); 
      
      _copyDataToVolume(&newSpacePtr, _fhdr.nx, _fhdr.ny ,  
			&radDataPtr, nidsFile.getNgates(), 
			nidsFile.getNradials());
    }
    else
    { 
      nidsFile.remapToCart(_params->res.delta, _params->res.dist);
     
      memcpy((void*) newSpacePtr,(void*) nidsFile.getCartData(), 
	     _fhdr.nx * _fhdr.ny * sizeof(fl32));   
    }

    //
    // Expand the volume size struct member appropriately
    //
    _fhdr.volume_size = _fhdr.volume_size + 
      (_fhdr.nx * _fhdr.ny * sizeof(fl32) );
  } // end add to existing volume

  //
  // Keep track of volume number
  //
  _prevVolNum = nidsFile.getVolNum();

  //
  // Keep track of tilt number, files can be out of order
  //
  _prevTiltNum = nidsFile.getElevNumber();

  _prevFileSuffix = nidsFile.getFileSuffix();

  if( _endOfVol)
  {
    _clearData();
   
  }

  return;
}

void NidsIngest::_outputVolume()
{
  PMU_auto_register("NidsIngest::_outputVolume");

  DateTime volTime(_mhdr.time_centroid);

  DateTime currTime(time(0));

  //
  // Declare a DsMdvx object so we can add the fields.
  //
  DsMdvx outMdvx;
  
  outMdvx.setMasterHeader( _mhdr );
  
  //
  // Create MdvxField
  //
  MdvxField *field = new MdvxField(_fhdr, _vhdr,_data);
  
  if (field->convertRounded(Mdvx::ENCODING_INT16,
			    Mdvx::COMPRESSION_ZLIB)){
    fprintf(stderr, "conversion of field failed - I cannot go on.\n");
    return;
  }

  if (field->convertRounded(Mdvx::ENCODING_INT16,
                            Mdvx::COMPRESSION_ZLIB)){
    fprintf(stderr, "conversion of field failed - I cannot go on.\n");
    return;
  }
  
  //
  // Add the field
  //
  outMdvx.addField(field);

  //
  // Write the field
  //
  //
  //
  cerr << "NidsIngest::_outputVolume(): Writing output to "
       << _outUrl.c_str() << ". Volume time: " << volTime.dtime() << " (current time: " <<  currTime.dtime() << ")" << endl;

  if (outMdvx.writeToDir(_outUrl)) {
    cerr << "ERROR - Output::write" << endl;
    cerr << "  Cannot write to url: " << _outUrl << endl;
    cerr << outMdvx.getErrStr() << endl;
  } 
}

void NidsIngest::_copyDataToVolume( fl32** volPtr,  int volNx,  
				    int volNy ,  fl32** planePtr, 
				    int planeNx, int planeNy)
{
  PMU_auto_register("NidsIngest::_copyDataToVolume");

  //
  // If dimensions are the same just copy the new data to the volume
  // all at once 
  //
  if (planeNx == volNx && planeNy == volNy)
  {
    memcpy((void*) (*volPtr), (void*) (*planePtr), volNx * volNy* sizeof(fl32));
  }
  else
  {
    if (_params->debug)
    {
      cerr << "NidsIngest::_copyDataToVolume(): WARNING: the dimensions of the "
	   << "volume and the dimensions of the plane of data to be added to the "
	   << "volume are different." << endl;
      cerr << "volNx: " <<  volNx << " volNy: " << volNy << endl;
      cerr << "planeNx: " << planeNx  << " planeNy: " << planeNy << endl;
    }

    //
    // First set all volume spaces to missing
    //
    for (int j = 0; j  <  volNy; j++)
    {
      for (int i = 0; i <  volNx; i++)
      {
	(*volPtr)[j * volNx + i ] =  _fhdr.missing_data_value;
      }
    }

    //
    // The x and y dimensions of the plane data and volume data are compared.
    // The smaller of each becomes the upperbound in each dimension for data
    // that gets copied from the plane to the volume.
    //
    int yBound;
    
    if( planeNy < volNy)
    {
      yBound =  planeNy;
    }
    else
    {
      yBound = volNy;
    }
    
    int xBound;
   
    if (planeNx < volNx)
      xBound = planeNx;
    else
      xBound = volNx;
    
    if( _params->debug)
    {
      cerr << "Will copy data for y range [0," << yBound-1 << "] and xrange [0," 
	 << xBound-1 << "] from plane to volume" << endl; 
    }

    for (int j = 0; j  < yBound; j++)
    {
      for (int i = 0; i <  xBound; i++)
      {
	(*volPtr)[j * volNx + i ] = (*planePtr)[j * planeNx + i];
      }
    }
  }
}

void NidsIngest::_freeThread()
{
  if ( (!(_params->forkIt)) && (_params->threadLimit.limitNumThreads) )
  
  {
      if (_params->debug) 
	cerr << "NidsIngest::_freeThread(): Releasing active "
	     << "thread number " << nt << endl;
      
      pthread_mutex_lock(&ntMutex);  
      
      nt--;
      
      pthread_mutex_unlock(&ntMutex);

      return;
    }
}
 
void  NidsIngest::_createSpdbContours(  NidsFile &nidsFile)
{
  
  PMU_auto_register("NidsIngest::_createSpdbContours");

  //
  // Initalize object to write data to database
  //
  DsSpdb spdb;

  spdb.setPutMode(Spdb::putModeAddUnique);

  //
  // Flag to indicate if write is necessary. (Otherwise the database 
  // could be filled with "empty" contours).
  //
  bool gotData = false;

  //
  // Initalize a projection object for converting contour
  // coordinates to latitude and longitude 
  // See ICD sec. 3.3.3 for Coordinate systems
  //
  PjgFlatCalc proj( nidsFile.getLat(), nidsFile.getLon(),
                     0, 1024, 1024,1,
                    1, 1, 0,
                   -512, -512, 0);

  //
  // Set the output directory product. Note that the 
  // the specific products have an elevation component:
  // NOM = .5 deg, NAM = .8 deg, N1M = 1.5 deg, NBM = 1.8 deg, 
  // N2M = 2.4 deg, N3M 3.4 deg
  //
  string product;
  
  fl32 elevationAngle =  nidsFile.getElevAngle();
  
  if( 0 < elevationAngle  && elevationAngle < .7 )
    product= "ML/N0M";
  else if  ( .7 <= elevationAngle  && elevationAngle < 1.3 )
    product = "ML/NAM";
  else if ( 1.3 <=  elevationAngle  && elevationAngle < 1.7 )
    product = "ML/N1M";
  else if ( 1.7 <= elevationAngle  && elevationAngle < 2.2  )
    product = "ML/NBM";
  else if( 2.2 <= elevationAngle  && elevationAngle < 3.0 )
    product = "ML/N2M";
  else
    // elevationAngle >= 3.0
    product = "ML/N3M";

  //
  // Construct full output url
  // 
  string url = _outSpdbUrl + "/" + product; 

  cerr << "NidsIngest::_createSpdbContours(): " << url.c_str() << endl;

  //
  // Set the url in the database object
  // 
  spdb.addUrl(url);

  //
  // Write Melting Layer Contours to spdb database
  //
  for (int i = 0; i < 4; i++)
  {
    //
    // Create the contour object
    // 
    GenPoly contour;

    contour.setName("Melting Layer Contour");
    
    //
    // Note that the contours are packed in the NIDS file
    // starting with outermost contour to inner most contour
    // We will reverse the label or id: 1 is innermost,4 is
    // outermost. We skip using 0 as an ID since it has 
    // a specific meaning ("retrieve all")for spdb data retrieval.
    // 
    
    contour.setTime(nidsFile.getTime());
    
    contour.setExpireTime(nidsFile.getTime() + 300);
    
    contour.setNLevels(1);
    
    contour.setClosedFlag(true);

    contour.addVal( nidsFile.getElevAngle() );
   
    //
    // Note that the contours are packed in the NIDS file
    // starting with outermost contour to inner most contour
    //
    if (i == 0)
    {
      //
      // Outer most contour and inner most will have same id
      // since display characteristics are the same
      //
      contour.setId(1);

      contour.setName("Melting Layer Contour Rtt");

      contour.addFieldInfo("Bottom beam intersects Melting Layer top","degrees (Rtt)");
    }
    else if (i == 1)
    {
      //
      // Middle two contours will have same id
      // since display characteristics are the same
      //
      contour.setId(2);
      
      contour.setName("Melting Layer Contour Rt");

      contour.addFieldInfo("Center beam intersects Melting Layer top","degrees (Rt)");
    }
    else if (i == 2)
    {
      contour.setId(2);

      contour.setName("Melting Layer Contour Rb");

      contour.addFieldInfo("Center beam intersects Melting Layer bottom","degrees (Rb)");
    }

    if (i == 3)
    {
      //
      // Inner most contour
      //      
      contour.setId(1);

      contour.setName("Melting Layer Contour Rbb");

      contour.addFieldInfo("Top beam intersects Melting Layer bottom","degrees (Rbb)");
    }
   
    //
    // Flag to indicate non uniform contour values
    //
    bool ptsAreDifferent;

    si16 firstX; 

    si16 firstY; 
    
    nidsFile.getContourPt (0, 0, firstX, firstY);

    //
    // Loop through the contour points, convert to lat, lon
    // and store as a polygon or contour vertex. If contour
    // data is not available, all vertex points are zero. 
    // We will check for this case by checking for
    // a difference in vertex value andnot add the null
    // contours to the database.
    //
    int numPts = nidsFile.getContourSize(i);

    for (int j = 0; j < numPts; j++)
    {
      si16 xCoord;

      si16 yCoord;
      
      double lat, lon;
      
      nidsFile.getContourPt (i, j, xCoord, yCoord);
      
      if( xCoord != firstX && yCoord != firstY)
      {
	ptsAreDifferent = true;
      }

      GenPoly::vertex_t contourPoint;

      proj.xy2latlon(xCoord/4, yCoord/4, lat, lon);
      
      contourPoint.lat = lat;
      
      contourPoint.lon = lon;
      
      contour.addVertex(contourPoint);
    }
         
    if (ptsAreDifferent)
    {
      //
      // the contour is not "null"
      //
      bool success = contour.assemble();
      
      if ( !success)
      {
	cerr << "ERROR: NidsIngest::_createSpdbContours(): GenPoly failed to assemble: "
	     <<  contour.getErrStr() << endl;
	return;
      }
      
      //
      // Add contour to data to be written
      // 
      spdb.addPutChunk(contour.getId(),
		       contour.getTime(),
		       contour.getExpireTime(),
		       contour.getBufLen(),
		       contour.getBufPtr());
      gotData = true;
    }
  }

  if(gotData)
  {
    //
    // Write data
    //
    if (spdb.put(SPDB_GENERIC_POLYLINE_ID, SPDB_GENERIC_POLYLINE_LABEL) != 0)
    {
      cerr << "NidsIngest::_createSpdbContours(): Spdb put failed to " << url.c_str() << endl;
    }
    else
    {
      if(_params->debug)
      {
	 cerr << "NidsIngest::_createSpdbContours(): Writing to " << url.c_str() << endl;
      }
    }
  }
  else
  { 
    //
    // No data (or null contour)
    // 
    if(_params->debug)
    {
      cerr << "NidsIngest::_createSpdbContours(): No Melting Layer data" << endl;
    }
  }  

}

void  NidsIngest:: _createSpdbGenPt( NidsFile &nidsFile)
{

  FILE *txtFile = NULL;

  if (_outputTextPointData)
  {
    time_t dataTime = nidsFile.getTime();

    DateTime dateTime(dataTime);

    string dirPath =  _outTextUrl + "/" +  dateTime.getDateStrPlain();

    string mkdirCmd = string("mkdir -p ") + dirPath + string("/");
    
    system( mkdirCmd.c_str());
    
    string textFilePath = dirPath + "/" + dateTime.getTimeStrPlain() + ".txt";
    
    if( (txtFile = fopen(textFilePath.c_str(),"w")) == NULL)
    {
      cerr << "ERROR: NidsIngest:: _createSpdbGenPt(): "
	   << " Additional text file failed to open for writing " << endl;
    }
  }

  PMU_auto_register("NidsIngest::_createSpdbGenPt");

  if ( nidsFile.getNumHailReps() > 0)
  {
    _writeSpdbHail(nidsFile, txtFile);
  }
  else if (  nidsFile.getNumTvsReps() > 0)
  {
    _writeSpdbTvs(nidsFile, txtFile);
  }
  else if (  nidsFile.getNumMesocycReps() > 0)
  {
    _writeSpdbMesocyclone(nidsFile, txtFile);
  }
  
  if ( nidsFile.getNumStormIDs() > 0 )
  {
    _writeSpdbStormIDs(nidsFile);
  }

  if(txtFile)
  {
    fclose(txtFile);
  }
  
}
 
void NidsIngest:: _writeSpdbHail( NidsFile &nidsFile, FILE *txtFile)
{
  
  PMU_auto_register("NidsIngest::_createSpdbHail");
  
  //
  // Write header for optional text output
  //
  if(txtFile != NULL)
  {
    fprintf(txtFile,"#radar,latitude,longitude,probOfHail,probOfSevereHail,maxHailSizeNearestInch\n");
  }
  
  //
  // Initalize object to write data to database
  //
  DsSpdb spdb;

  spdb.setPutMode(Spdb::putModeAddUnique);

  //
  // Initalize a projection object for converting contour
  // coordinates to latitude and longitude 
  // See ICD sec. 3.3.3 for Coordinate systems
  //
  PjgFlatCalc proj( nidsFile.getLat(), nidsFile.getLon(),
		    0, 1024, 1024,1,
		    1, 1, 0, 
		   -512, -512, 0);
  //
  // Set the output directory product,
  //
  string url = _outSpdbUrl + "/" + _params->_fields[_ifield].outDirName; 
 
  //
  // Set the url in the database object
  // 
  spdb.addUrl(url);

  for (int i = 0; i < nidsFile.getNumHailReps(); i++)
  {
    if ( nidsFile.getProbHail(i) != 0)
    {
      int xCoord;
    
      int yCoord;
    
      double lat, lon;
      
      nidsFile.getHailPt(i, xCoord, yCoord);
      
      proj.xy2latlon(xCoord/4, yCoord/4, lat, lon);
      
      GenPt genPt;
      
      genPt.setName("Hail");
      
      genPt.setLat(lat);
      
      genPt.setLon(lon);
      
      genPt.setTime(nidsFile.getTime());
      
      genPt.setId(0);
      
      genPt.setNLevels(1);
      
      genPt.addFieldInfo("ProbOfHail", "percent");
      
      genPt.addFieldInfo("ProbOfSevereHail", "percent");
      
      genPt.addFieldInfo("MaxHailSize", "inches");
      
      genPt.setText( "H" );
      
      //
      // Add field values
      //
      genPt.addVal( nidsFile.getProbHail(i));
      
      genPt.addVal( nidsFile.getProbSevereHail(i));
      
      genPt.addVal( nidsFile.getMaxHailSize(i));
      
      if(txtFile != NULL)
      {
	fprintf(txtFile,"%s,%f,%f,%f,%f,%f\n", _radarName.c_str(), lat, lon,
		nidsFile.getProbHail(i),nidsFile.getProbSevereHail(i), nidsFile.getMaxHailSize(i));
      }

      int errorCheck = genPt.assemble();
      
      if (errorCheck)
      {
	cerr << "ERROR: NidsIngest::_writeSpdbHail: GenPt failed to assemble: "
	     <<  genPt.getErrStr() << endl;
      }
      else
      {
	//
	// Add contour to data to be written
	// 
	spdb.addPutChunk(genPt.getId(),
			 genPt.getTime(),
			 genPt.getTime() + 1,
			 genPt.getBufLen(),
			 genPt.getBufPtr());
	
      }
    }//if POH != 0 end for hail reps
  }

  //
  // Write data for reps (with POH > 0 )
  //
  if (spdb.nPutChunks() >0)
  {
    if (spdb.put(SPDB_GENERIC_POINT_ID, SPDB_GENERIC_POINT_LABEL) != 0)
    {
      cerr << "NidsIngest::_writeSpdbHail(): Spdb put failed to " << url.c_str() << endl;
    }
    else
    {
      if(_params->debug)
      {
	cerr << "NidsIngest::_writeSpdbHail(): Writing to " << url.c_str() << endl;
      }
    }
  }
}

void  NidsIngest::_writeSpdbTvs( NidsFile &nidsFile, FILE *txtFile)
{
  PMU_auto_register("NidsIngest::_writeSpdbTvs");

  //
  // Write header for optional text output
  //
  if(txtFile != NULL)
  {
    fprintf(txtFile,"#radar,latitude,longitude\n");
  }

  //
  // Initalize object to write data to database
  //
  DsSpdb spdb;

  spdb.setPutMode(Spdb::putModeAddUnique);

  //
  // Initalize a projection object for converting contour
  // coordinates to latitude and longitude 
  // See ICD sec. 3.3.3 for Coordinate systems
  //
  PjgFlatCalc proj( nidsFile.getLat(), nidsFile.getLon(),
		    0, 1024, 1024,1,
		    1, 1, 0, 
		   -512, -512, 0);
  //
  // Set the output directory product,
  //
  string url = _outSpdbUrl + "/" + _params->_fields[_ifield].outDirName; 
  
  
  //
  // Set the url in the database object
  // 
  spdb.addUrl(url);

  for (int i = 0; i < nidsFile.getNumTvsReps(); i++)
  {
    int xCoord;
    
    int yCoord;
    
    double lat, lon;
    
    nidsFile.getTvsPt(i, xCoord, yCoord);
    
    proj.xy2latlon(xCoord/4, yCoord/4, lat, lon);

    if(txtFile != NULL)
    {
      fprintf(txtFile,"%s,%f,%f\n", _radarName.c_str(), lat, lon);
    }
    
    GenPt genPt;
    
    genPt.setName("TVS");
    
    genPt.setLat(lat);
    
    genPt.setLon(lon);
    
    genPt.setTime(nidsFile.getTime());
    
    genPt.setId(0);
    
    genPt.setNLevels(1);
    
    genPt.addFieldInfo("TVS","none");

    genPt.setText("TVS" );

    genPt.addVal(1);

    int errorCheck = genPt.assemble();
    
    if (errorCheck)
    {
      cerr << "ERROR: NidsIngest::_writeSpdbTvs: GenPt failed to assemble: "
	   <<  genPt.getErrStr() << endl;
    }
    else
    {
      //
      // Add genPt to data to be written
      // 
      spdb.addPutChunk(genPt.getId(),
		       genPt.getTime(),
		       genPt.getTime() + 1,
		       genPt.getBufLen(),
		       genPt.getBufPtr());
      
    }
  }// end for tvs reps
  
  //
  // Write data
  //
  if (spdb.put(SPDB_GENERIC_POINT_ID, SPDB_GENERIC_POINT_LABEL) != 0)
  {
    cerr << "NidsIngest::_writeSpdbTvs(): Spdb put failed to " << url.c_str() << endl;
  }
  else
  {
    if(_params->debug)
    {
      cerr << "NidsIngest::_writeSpdbTvs(): Writing to " << url.c_str() << endl;
    }
  }
}

void  NidsIngest:: _writeSpdbMesocyclone( NidsFile &nidsFile, FILE *txtFile)
{
  PMU_auto_register("NidsIngest::_writeSpdbMesocyclone"); 

  if(txtFile != NULL)
  {
    fprintf(txtFile,"#radar,latitude,longitude,radiusKm,typeStr\n");
  }

  //
  // Initalize object to write data to database
  //
  DsSpdb spdb;

  spdb.setPutMode(Spdb::putModeAddUnique);

  //
  // Initalize a projection object for converting contour
  // coordinates to latitude and longitude 
  // See ICD sec. 3.3.3 for Coordinate systems
  //
  PjgFlatCalc proj( nidsFile.getLat(), nidsFile.getLon(),
		    0, 1024, 1024,1,
		    1, 1, 0, 
		   -512, -512, 0);
  //
  // Set the output directory product,
  //
  string url = _outSpdbUrl + "/" + _params->_fields[_ifield].outDirName; 
  
  //
  // Set the url in the database object
  // 
  spdb.addUrl(url); 

  for (int i = 0; i < nidsFile.getNumMesocycReps(); i++)
  {
    int xCoord;
    
    int yCoord;
    
    double lat, lon;
    
    nidsFile.getMesocycPt(i, xCoord, yCoord);
    
    proj.xy2latlon(xCoord/4, yCoord/4, lat, lon);
    
    string typeStr = nidsFile.getMesocycTypeStr(i);

    if(txtFile != NULL && typeStr != string("MesoTypeUnknown"))
    {
      fprintf(txtFile,"%s,%f,%f,%f,%s\n", _radarName.c_str(), lat, lon, nidsFile.getMesocycRadius(i), typeStr.c_str());
    }

    GenPt genPt;
    
    genPt.setName("Mesocyclone");
    
    genPt.setLat(lat);
    
    genPt.setLon(lon);
    
    genPt.setTime(nidsFile.getTime());
    
    genPt.setId(0);
    
    genPt.setNLevels(1);
    
    genPt.addFieldInfo("radius","km");

    genPt.setText("M");
    
    //
    // Add field values
    //
    genPt.addVal(nidsFile.getMesocycRadius(i));
    
    int errorCheck = genPt.assemble();
    
    if (errorCheck)
    {
      cerr << "ERROR: NidsIngest:: _writeSpdbMesocyclone(): GenPt failed to assemble: "
	   <<  genPt.getErrStr() << endl;
    }
    else
    {
      //
      // Add genPt to data to be written
      // 
      spdb.addPutChunk(genPt.getId(),
		       genPt.getTime(),
		       genPt.getTime() + 1,
		       genPt.getBufLen(),
		       genPt.getBufPtr());
      
    }
  }// end for mesocyclone reps
  
  //
  // Write data
  //
  if (spdb.put(SPDB_GENERIC_POINT_ID, SPDB_GENERIC_POINT_LABEL) != 0)
  {
    cerr << "NidsIngest::_writeSpdbMesocyclone(): Spdb put failed to " << url.c_str() << endl;
  }
  else
  {
    if(_params->debug)
    {
      cerr << "NidsIngest::_writeSpdbMesocyclone(): Writing to " << url.c_str() << endl;
    }
  } 
}


void  NidsIngest:: _writeSpdbStormIDs( NidsFile &nidsFile)
{
  PMU_auto_register("NidsIngest::__writeSpdbStormIDs"); 

  //
  // Initalize object to write data to database
  //
  DsSpdb spdb;

  spdb.setPutMode(Spdb::putModeAddUnique);

  //
  // Initalize a projection object for converting contour
  // coordinates to latitude and longitude 
  // See ICD sec. 3.3.3 for Coordinate systems
  //
  PjgFlatCalc proj( nidsFile.getLat(), nidsFile.getLon(),
		    0, 1024, 1024,1,
		    1, 1, 0, 
		   -512, -512, 0);
  //
  // Set the output directory product,
  //
  string url = _outSpdbUrl + "/stormIDs"; 
  
  //
  // Set the url in the database object
  // 
  spdb.addUrl(url);

  for (int i = 0; i < nidsFile.getNumStormIDs(); i++)
  {
    int xCoord;
    
    int yCoord;
    
    double lat, lon;
    
    nidsFile.getStormIDPt(i, xCoord, yCoord);
    
    proj.xy2latlon(xCoord/4, yCoord/4, lat, lon);
    
    GenPt genPt;
    
    genPt.setName("StormID");
    
    genPt.setLat(lat);
    
    genPt.setLon(lon);
    
    genPt.setTime(nidsFile.getTime());
    
    genPt.setId(0);
    
    genPt.setText( nidsFile.getStormID(i) );

    genPt.setNLevels(1);
    
    genPt.addFieldInfo( nidsFile.getStormID(i), "stormId");
    
    genPt.addVal(1);
    
    int errorCheck = genPt.assemble();
    
    if (errorCheck)
    {
      cerr << "ERROR: NidsIngest::_writeSpbdStormIDs: GenPt failed to assemble: "
	   <<  genPt.getErrStr() << endl;
    }
    else
    {
      //
      // Add genPt to data to be written
      // 
      spdb.addPutChunk(genPt.getId(),
		       genPt.getTime(),
		       genPt.getTime() + 1,
		       genPt.getBufLen(),
		       genPt.getBufPtr());
      
    }
  }// end for mesocyclone reps
  
  //
  // Write data
  //
  if (spdb.put(SPDB_GENERIC_POINT_ID, SPDB_GENERIC_POINT_LABEL) != 0)
  {
    cerr << "NidsIngest::_writeSpdbStormIDs(): Spdb put failed to " << url.c_str() << endl;
  }
  else
  {
    if(_params->debug)
    {
      cerr << "NidsIngest::_writeSpdbStormIDs(): Writing to " << url.c_str() << endl;
    }
  }  
}

void  NidsIngest::_setEndOfVolFlag(const string filepath, const string endOfVolStr, const int tiltIndex)
{

  int strLen = endOfVolStr.length();
  
  if(  filepath.compare(filepath.size() -strLen, strLen, endOfVolStr) == 0)
  {
    _endOfVol = true;

    if (_params->debug)
    {
      cerr << "NidsIngest::_setEndOfVolFlag(): End of vol == TRUE. End of vol suffix: " 
	   << endOfVolStr.c_str() << " File processed: " <<  filepath.c_str() << endl;
    }
  }
  else if  (_outputLastTiltInVol && tiltIndex == _lastTiltNum)
  {
    _endOfVol = true;
    
    if (_params->debug)
    {
      cerr << "NidsIngest::_setEndOfVolFlag(): End of vol == TRUE. "
	   << "_outputLastTiltInVol = true and parameter _lastTiltNum = "  
	   << _lastTiltNum << ", index of current tilt is " << tiltIndex << endl;
    }
  }
  else
  {
    if (_params->debug)
    {
      cerr << "NidsIngest::_setEndOfVolFlag(): End of vol == FALSE. End of vol suffix: " 
	   << endOfVolStr.c_str() << " File processed: " << filepath.c_str() << endl;
    }
    _endOfVol = false;
  }
}


void  NidsIngest::_setStartOfVolFlag(const string filepath, const string startOfVolStr)
{

  int strLen = startOfVolStr.length();
  
  if(  filepath.compare(filepath.size() -strLen, strLen, startOfVolStr) == 0)
  {
    _startOfVol = true;

    if (_params->debug)
    {
      cerr << "NidsIngest::_setStartOfVolFlag(): start of vol == TRUE. Start of vol suffix: " 
	   << startOfVolStr.c_str() << " File processed: " <<  filepath.c_str() << endl;
    }
  }
  else
  {
    if (_params->debug)
    {
      cerr << "NidsIngest::_setStartOfVolFlag(): Start of vol == FALSE. Start of vol suffix: " 
	   << startOfVolStr.c_str() << " File processed: " << filepath.c_str() << endl;
    }
    _startOfVol = false;
  }
}

void NidsIngest::_clearData()
{
  if (_haveData)
  {
    _outputVolume();
    
    if(_data)
    {
      delete[] _data;
      
      _data = NULL;
    }
    
    _haveData = false;
    
    _endOfVol = false;
    
    _prevVolNum = -1;
    
    _prevTiltNum = -1;

    _prevFileSuffix = string("");
  }
}

void NidsIngest::_saveOutOfOrderTilt( NidsFile *nidsFile, string &filePath, const int tiltNum)
{
  pair < NidsFile *, string> fileStrPair( nidsFile, filePath );
  bool inserted = false;
  for (int i = 0; i < (int)_outOfOrderTilts.size();i++)
  {
    if (tiltNum < _outOfOrderTilts[i].first->getElevNumber())
    {
      _outOfOrderTilts.insert(_outOfOrderTilts.begin() +i, fileStrPair);
      inserted = true;
      i = (int)_outOfOrderTilts.size();
    }
  }
  if (!inserted)
    _outOfOrderTilts.push_back(fileStrPair);	 
  
  return;
}

void NidsIngest::_dumpOldData( const int currVolNum )
{
  //
  // Dump all data not related to the current volume
  //
  if( (int)_outOfOrderTilts.size() > 0 && 
      _outOfOrderTilts[0].first->getVolNum() !=  currVolNum)
  {
    
    for (int i = 0 ; i < (int) _outOfOrderTilts.size(); i++)
    {
      DateTime currTime(time(0));
      cerr << "NidsIngest::_dumpOldData(): Processing " <<  _outOfOrderTilts[i].second
	   << "  (current time: " <<  currTime.dtime() << ")" << endl;
      
      if(_haveData)
	_createMdvVolume( *(_outOfOrderTilts[i].first),  _outOfOrderTilts[i].second, false);
      else
	_createMdvVolume( *(_outOfOrderTilts[i].first),  _outOfOrderTilts[i].second, true);
      
      delete (_outOfOrderTilts[i].first);
    }
    
    _outOfOrderTilts.clear();
  }

  _clearData();

  return;
}

void NidsIngest::_writeOutOfOrderTiltsCurrVol(const string prevSuffix)
{
  
  int tiltChecks =  (int) _outOfOrderTilts.size() ; 

  string lastSuffix = prevSuffix;

  int i = 0;
  while ( (int) _outOfOrderTilts.size() > 0  && tiltChecks > 0)
  {
    //
    // Sanity check
    //
    if ( (int) _outOfOrderTilts.size() - 1 < i)
    {
      cerr << "NidsIngest::_writeOutOfOrderTiltsCurrVol(): ERROR-- Trying to access tilt which does not exist." << endl;
      return;
    }

    if ( _outOfOrderTilts[i].first->isNextTilt(lastSuffix))
    {
      //
      // We found the next tilt, write it
      //
      DateTime currTime(time(0));

      cerr << "NidsIngest::_writeOutOfOrderTiltsCurrVol(): Processing " <<  _outOfOrderTilts[i].second
	   << "  (current time: " <<  currTime.dtime() << ")" << endl;

      _createMdvVolume( *(_outOfOrderTilts[i].first),  _outOfOrderTilts[i].second, false );

      //
      // update lastSuffix to the last one written
      //
      lastSuffix = _outOfOrderTilts[i].first->getFileSuffix();
      
      delete (_outOfOrderTilts[i].first); 
       
      _outOfOrderTilts.erase(_outOfOrderTilts.begin() + i);

      //
      // Start checking at the beginning of the stored tilts for the next one 
      //
      i = 0;

      tiltChecks =  (int) _outOfOrderTilts.size(); 
    }	  
    else
      i++;

    tiltChecks--;
  }
 
  return;
}
