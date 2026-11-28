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
  _lastVolNum = -1;
   
  //
  // Flag to indicate if plane of data exists for field. 
  //
  _haveData = false;

  //
  // Integer indicator or field or product to be processed
  //
  _ifield= ifield;

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

  string outSpdbUrl = _params->topOutSpdbUrl;
  
  _outSpdbUrl = outSpdbUrl + pathDelim +  _params->_radars[iradar];
  
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
  //
  // If we have a plane or volume of data, output it,
  // clean up, and reset the data state.
  // 
  if (_haveData)
  {
    _outputVolume();
    
    delete[] _data;
    
    _haveData = false;
  }
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
  cerr << " leaving " << dirName.c_str();
  free(namelist); 

  namelist = NULL;
  
  return;
}

void NidsIngest::_convertFile( const char *filePath )
{ 
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
      cerr << "NidsIngest::hiResNidsRadialImgFile(): Acquired "
	   << "active thread number " << nt << " of " 
	   << _params->threadLimit.maxNumThreads << endl;
    }
  } 

  //
  // Read and decode input file
  // 
  hiResRadialFile H(filePath, _params->byteSwap, true, _params->hasExtraHdr);

  //
  // If decoding fails, then free thread if necessary.
  //
  if ( H.decode() !=  0)
  {
    cerr << "NidsIngest::hiResNidsRadialImgFile(): "
	 << "Problem reading file " << filePath << ", skipping." << endl;
    
    _freeThread();

    return;
  }

  if( H.isContourData())
  {
    _createSpdbContours(H);

    _freeThread();

    return;
  }
  else if (H.isGenPtData())
  {
    _createSpdbGenPt(H);
 
    _freeThread();

    return;
  }
  else
  {
    _createMdvVolume(H);

    _freeThread();

    return;
  }
}


void NidsIngest::_createMdvVolume( hiResRadialFile & H)
{
  //
  // Determine if we are adding to existing Mdv data volume or starting a new
  // one
  //
  bool newVolume;
  
  if( H.getVolNum() == _lastVolNum)
  {
    cerr << "NidsIngest::hiResNidsRadialImgFile(): We have tilts "
	 << "in the same volume. We will add to the current Mdv volume." 
	 << endl;

    newVolume = false;
  }
  else
  {
    newVolume = true;

    //
    // Volume numbers of last file and current file differ
    //
    cerr << "NidsIngest::hiResNidsRadialImgFile(): We are starting"
	 << " a new volume" << endl;   
    //
    // Output last volume if we are still hanging on to it, clean up memory
    // reset data flag.
    //
    if (_haveData)
    {
      _outputVolume();
   
      delete[] _data;

      _haveData = false;
    }
  }
  
  if (newVolume)
  {
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
    _mhdr.sensor_lat = H.getLat();
    _mhdr.sensor_lon = H.getLon();
    _mhdr.sensor_alt = H.getAlt();
    _mhdr.data_dimension = 2;
    _mhdr.vlevel_included = 1;
    sprintf(_mhdr.data_set_info,"%s","Level III data");
    sprintf(_mhdr.data_set_name,"%s","Level III data");
    sprintf(_mhdr.data_set_source,"%s", "Level III data");
    _mhdr.time_gen = H.getTime();
    _mhdr.time_begin = H.getTime();
    _mhdr.time_end = H.getTime();
    _mhdr.time_expire = H.getTime();
    _mhdr.time_centroid = H.getTime(); 
    
    //
    // Set up fieldheader
    //
    _fhdr.proj_origin_lat =  H.getLat();
    _fhdr.proj_origin_lon =  H.getLon();
    _fhdr.forecast_time = H.getTime();
    _fhdr.forecast_delta = 0;
    _fhdr.bad_data_value = H.getMissingFl32(); 
    _fhdr.missing_data_value = H.getMissingFl32();
    _fhdr.grid_minz = H.getElevAngle();
    
    _fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
    _fhdr.data_element_nbytes = sizeof(fl32);
    _fhdr.compression_type = Mdvx::COMPRESSION_NONE;
    _fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
    _fhdr.scaling_type = Mdvx::SCALING_NONE;
    sprintf(_fhdr.field_name, _params->_fields[_ifield].outFieldName);
    sprintf(_fhdr.field_name_long, _params->_fields[_ifield].outFieldNameLong);
    sprintf(_fhdr.units, _params->_fields[_ifield].outFieldUnits);
    sprintf( _fhdr.transform,"%s","none");
    
    if (_saveAsRadial)
    {
      _fhdr.nx = H.getNgates();
      _fhdr.ny = H.getNradials();
      _fhdr.grid_dx = H.getGateSpacing();
      _fhdr.grid_dy = H.getDelAz();
      _fhdr.grid_minx = H.getFirstGateDist();
      _fhdr.grid_miny = 0.0;
      _fhdr.proj_type = Mdvx::PROJ_POLAR_RADAR;
      
      //
      // This will change if there are more levels for this field
      //
      _fhdr.nz = 1; 
      _fhdr.grid_dz = 0;
      _fhdr.volume_size = _fhdr.nx * _fhdr.ny * _fhdr.nz * sizeof(fl32);
      _data = new fl32[ _fhdr.nx * _fhdr.ny];
      memcpy((void*)_data, (void*)  H.getAllRadialFl32s(),  _fhdr.volume_size);
    }
    else
    {
      H.remapToCart(_params->res.delta, _params->res.dist);
      _fhdr.nx = H.getNxy();
      _fhdr.ny = H.getNxy();
      _fhdr.grid_dx = H.getDxy();
      _fhdr.grid_dy = H.getDxy();
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
      memcpy((void*)_data, (void*)  H.getCartData(),  _fhdr.volume_size);
    }
    
    //
    // Setup Vlevel header
    //
    _vhdr.type[0] = Mdvx::VERT_TYPE_ELEV;
    _vhdr.level[0] = H.getElevAngle();
    
    _haveData = true;
  } // end new volume 

  if (!newVolume)
  {
    //
    // Add data to existing volume. Modify Mdv header members appropriately
    //
    _fhdr.nz = _fhdr.nz + 1;
    _fhdr.dz_constant = 0;

    _mhdr.time_end = H.getTime();
    _mhdr.data_dimension = 3;
   
    _vhdr.type[_fhdr.nz -1] = Mdvx::VERT_TYPE_ELEV;
    _vhdr.level[_fhdr.nz -1] = H.getElevAngle();
   
    //
    // Expand  the _data array
    // Copy the data we have to tmp location
    // 
    fl32 *tmp = new fl32[_fhdr.volume_size/sizeof(fl32)];

    memcpy((void*)tmp,(void*)_data, _fhdr.volume_size);

    //
    // delete the old array and create new bigger one
    //
    delete[] _data; 
    
    //
    // Allocate space for current number of elements plus the new 
    // field
    //
    _data = new fl32[ _fhdr.volume_size/sizeof(fl32) +  (_fhdr.nx * _fhdr.ny)];

    //
    // Copy the old data to this array
    //
    memcpy((void*)_data, (void*)tmp, _fhdr.volume_size);


    // Clean up
    delete[] tmp;

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
      fl32 *radDataPtr = H.getAllRadialFl32s(); 
      
      _copyDataToVolume(&newSpacePtr, _fhdr.nx, _fhdr.ny ,  
			&radDataPtr, H.getNgates(), H.getNradials());
    }
    else
    { 
      H.remapToCart(_params->res.delta, _params->res.dist);
      cerr << "nxny " <<  H.getNxy() << endl;
      memcpy((void*) newSpacePtr,(void*) H.getCartData(), 
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
  _lastVolNum = H.getVolNum();

  return;

}

void NidsIngest::_outputVolume()
{
  
  cerr << "NidsIngest::_outputVolume(): Writing output to " 
       << _outUrl.c_str() << endl;

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
    cerr << "WARNING: the dimensions of the volume and the dimensions of the "
	 << "plane of data to be added to the volume are different." << endl;
    cerr << "volNx: " <<  volNx << " volNy: " << volNy << endl;
    cerr << "planeNx: " << planeNx  << " planeNy: " << planeNy << endl;

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
    
   cerr << "Will copy data for y range [0," << yBound-1 << "] and xrange [0," 
	<< xBound-1 << "] from plane to volume" << endl; 
 
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
 
void  NidsIngest::_createSpdbContours(  hiResRadialFile &H)
{
  
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
  PjgFlatCalc proj( H.getLat(), H.getLon(),
		    0, 2048, 2048,1,
		    .25, .25, 0, 
		   -512, -512, 0);
  //
  // Set the output directory product. Note that the 
  // the specific products have an elevation component:
  // NOM = .5 deg, NAM = .8 deg, N1M = 1.5 deg, NBM = 1.8 deg, 
  // N2M = 2.4 deg, N3M 3.4 deg
  //
  string product;
  
  fl32 elevationAngle =  H.getElevAngle();
  
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
    
    contour.setTime(H.getTime());
    
    contour.setExpireTime(H.getTime() + 300);
    
    contour.setNLevels(1);
    
    contour.setClosedFlag(true);

    contour.addVal( H.getElevAngle() );
   
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
    
    H.getContourPt (0, 0, firstX, firstY);

    //
    // Loop through the contour points, convert to lat, lon
    // and store as a polygon or contour vertex. If contour
    // data is not available, all vertex points are zero. 
    // We will check for this case by checking for
    // a difference in vertex value andnot add the null
    // contours to the database.
    //
    int numPts = H.getContourSize(i);

    for (int j = 0; j < numPts; j++)
    {
      si16 xCoord;

      si16 yCoord;
      
      double lat, lon;
      
      H.getContourPt (i, j, xCoord, yCoord);
      
      if( xCoord != firstX && yCoord != firstY)
      {
	ptsAreDifferent = true;
      }

      GenPoly::vertex_t contourPoint;

      proj.xy2latlon(xCoord, yCoord, lat, lon);
      
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

void  NidsIngest:: _createSpdbGenPt( hiResRadialFile &H)
{
  if ( H.getNumHailReps() > 0)
  {
    _writeSpdbHail(H);
  }
  else if (  H.getNumTvsReps() > 0)
  {
    _writeSpdbTvs(H);
  }
  else if (  H.getNumMesocycReps() > 0)
  {
    _writeSpdbMesocyclone(H);
  }
  
  if ( H.getNumStormIDs() > 0 )
  {
    _writeSpdbStormIDs(H);
  }
  
}
 
void  NidsIngest:: _writeSpdbHail( hiResRadialFile &H)
{
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
  PjgFlatCalc proj( H.getLat(), H.getLon(),
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

  for (int i = 0; i < H.getNumHailReps(); i++)
  {
    int xCoord;
    
    int yCoord;
    
    double lat, lon;
    
    H.getHailPt(i, xCoord, yCoord);
    
    proj.xy2latlon(xCoord/4, yCoord/4, lat, lon);
    
    GenPt genPt;
    
    genPt.setName("Hail");
    
    genPt.setLat(lat);
    
    genPt.setLon(lon);
    
    genPt.setTime(H.getTime());
    
    genPt.setId(0);
    
    genPt.setNLevels(1);
    
    genPt.addFieldInfo("ProbOfHail", "percent");
      
    genPt.addFieldInfo("ProbOfSevereHail", "percent");

    genPt.addFieldInfo("MaxHailSize", "inches");
    
    genPt.setText( "H" );
    
    //
    // Add field values
    //
    genPt.addVal( H.getProbHail(i));
    
    genPt.addVal( H.getProbSevereHail(i));
    
    genPt.addVal( H.getMaxHailSize(i));
    
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
  }// end for hail reps
 
  //
  // Write data
  //
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

void  NidsIngest:: _writeSpdbTvs( hiResRadialFile &H)
{
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
  PjgFlatCalc proj( H.getLat(), H.getLon(),
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

  for (int i = 0; i < H.getNumTvsReps(); i++)
  {
    int xCoord;
    
    int yCoord;
    
    double lat, lon;
    
    H.getTvsPt(i, xCoord, yCoord);
    
    proj.xy2latlon(xCoord/4, yCoord/4, lat, lon);
    
    GenPt genPt;
    
    genPt.setName("TVS");
    
    genPt.setLat(lat);
    
    genPt.setLon(lon);
    
    genPt.setTime(H.getTime());
    
    genPt.setId(0);
    
    genPt.setNLevels(1);
    
    genPt.addFieldInfo("TVS","none");

    genPt.setText("V" );
    

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

void  NidsIngest:: _writeSpdbMesocyclone( hiResRadialFile &H)
{
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
  PjgFlatCalc proj( H.getLat(), H.getLon(),
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

  for (int i = 0; i < H.getNumMesocycReps(); i++)
  {
    int xCoord;
    
    int yCoord;
    
    double lat, lon;
    
    H.getMesocycPt(i, xCoord, yCoord);
    
    proj.xy2latlon(xCoord/4, yCoord/4, lat, lon);
    
    GenPt genPt;
    
    genPt.setName("Mesocyclone");
    
    genPt.setLat(lat);
    
    genPt.setLon(lon);
    
    genPt.setTime(H.getTime());
    
    genPt.setId(0);
    
    genPt.setNLevels(1);
    
    genPt.addFieldInfo("radius","km");

    genPt.setText("M");
    
    //
    // Add field values
    //
    genPt.addVal(H.getMesocycRadius(i));
    
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

void  NidsIngest:: _writeSpdbStormIDs( hiResRadialFile &H)
{
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
  PjgFlatCalc proj( H.getLat(), H.getLon(),
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

  for (int i = 0; i < H.getNumStormIDs(); i++)
  {
    int xCoord;
    
    int yCoord;
    
    double lat, lon;
    
    H.getStormIDPt(i, xCoord, yCoord);
    
    proj.xy2latlon(xCoord/4, yCoord/4, lat, lon);
    
    GenPt genPt;
    
    genPt.setName("StormID");
    
    genPt.setLat(lat);
    
    genPt.setLon(lon);
    
    genPt.setTime(H.getTime());
    
    genPt.setId(0);
    
    genPt.setText( H.getStormID(i) );

    genPt.setNLevels(1);
    

    genPt.addFieldInfo( H.getStormID(i), "stormId");
    
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

