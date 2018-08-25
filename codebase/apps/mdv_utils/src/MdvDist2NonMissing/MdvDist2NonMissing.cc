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
#include "MdvDist2NonMissing.hh"
using namespace std;

//
// Constructor
//
MdvDist2NonMissing::MdvDist2NonMissing(int argc, char **argv)
{
  isOK = true;

  //
  // set programe name
  //
  _progName = "MdvDist2NonMissing";

  ucopyright((char *) _progName.c_str());

  //
  // get command line args
  //
  if (_args.parse(argc, argv, _progName))
    {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Problem with command line args" << endl;
      isOK = FALSE;
      return;
    }

  //
  // get TDRP params
  //
  _paramsPath = (char *) "unknown";

  if (_params.loadFromArgs(argc, argv, _args.override.list,
                           &_paramsPath))
    {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Problem with TDRP parameters" << endl;
      isOK = FALSE;
      return;
    }

  // If an additional TDRP file was specified, load that
  // over the existing params.
  if (NULL != _args.additional_tdrp_file){

    if (_params.debug){
      cerr << "Attempting to load additional param file " << _args.additional_tdrp_file << endl;
    }

    if (_params.load(_args.additional_tdrp_file, NULL, TRUE, FALSE)){
      cerr << "ERROR: " << _progName << endl;
      cerr << "Problem with TDRP parameters in file: " << _args.additional_tdrp_file << endl;
      isOK = false;
      return;
    }
  }

  //
  // init process mapper registration
  //
  PMU_auto_init((char *) _progName.c_str(),
                _params.instance,
                PROCMAP_REGISTER_INTERVAL);
  
  
  _dist = NULL;

  return;

}

//////////////////////////////////////////////////////
//
// destructor
//
MdvDist2NonMissing::~MdvDist2NonMissing()
{
  //
  // unregister process
  //
  PMU_auto_unregister();

  if( _trigger)
    delete _trigger;

  _clear();

}

void MdvDist2NonMissing::_clear()
{
  if ( _dist)
    delete[] _dist;
  
  _dist = NULL;

}

//////////////////////////////////////////////////
//
// Run
//
int MdvDist2NonMissing::Run ()
{
  //
  // register with procmap
  //
  PMU_auto_register("Run");

  //
  // Set up triggering mechanisme
  //
  _trigger = NULL;

  if (_params.mode == Params::TIME_INTERVAL)
    {
      //
      // Create archive timelist trigger
      //
      if (_params.debug)
        {
          cerr << "MdvDist2NonMissing::Run(): Creating ARCHIVE trigger\n"
               << "     start time: " << _args.startTime
               << "     end time:   " << _args.endTime << endl;
        }

      DsTimeListTrigger *archive_trigger = new DsTimeListTrigger();

      if (archive_trigger->init(_params.data_url,
                                _args.startTime,
                                _args.endTime) != 0)
        {
          cerr << archive_trigger->getErrStr();
          _trigger = 0;
          return 1;
        }
      else
        {
          _trigger = archive_trigger;
        }
    }
  else if ( _params.mode == Params::FILELIST )
    {
      //
      // Archive filelist mode.
      //
      if ( _args.inputFileList.size() )
        {
          if (_params.debug)
            cerr << "MdvDist2NonMissing::Run(): Initializing archive FILELIST mode." << endl;

          DsFileListTrigger *file_trigger = new DsFileListTrigger();

          if  (file_trigger->init( _args.inputFileList ) )
            {
              cerr << file_trigger->getErrStr();
              _trigger = 0;
              return 1;
            }
          else
            _trigger = file_trigger;
        }
    }
  else if (_params.mode == Params::REALTIME || _params.mode == Params::REALTIME_FCST_DATA)
    {
      if (_params.debug)
        {
          cerr << "MdvDist2NonMissing::Run(): Creating REALTIME trigger" << endl;
        }

      //
      // realtime mode
      //
      DsLdataTrigger *realtime_trigger = new DsLdataTrigger();

      if (realtime_trigger->init(_params.data_url,
                                 _params.max_valid_realtime_age,
                                 PMU_auto_register))
        {
          cerr << realtime_trigger->getErrStr();
          _trigger = 0;
          return 1;
        }
      else
        {
          _trigger = realtime_trigger;
        }
    }

  if ( _trigger == 0 )
    {
      cerr << "MdvDist2NonMissing::Run" <<"  ERROR: Failed to instantiate trigger\n" << endl;
      
      exit (1);
    }

  //
  //
  // process data
  //
  time_t inputTime;
  
  while (!_trigger->endOfData())
    {
      TriggerInfo triggerInfo;
      inputTime = _trigger->next(triggerInfo);
      if (_processData(triggerInfo.getIssueTime(), 
		       triggerInfo.getForecastTime() - triggerInfo.getIssueTime(),
		       triggerInfo.getFilePath()))
	{
	  cerr << "MdvDist2NonMissing::Run" <<"  Errors in processing time: " <<  triggerInfo.getIssueTime()
	       << " input file: " << triggerInfo.getFilePath() << endl;
	  
	  return 1;
	}
    } // while
  
  _clear();

  return 0;

}

///////////////////////////////////
//
//  process data at trigger time
//
int MdvDist2NonMissing::_processData(time_t input_time, int leadTime, const string filepath)
{
  //
  // registration with procmap
  //
  PMU_force_register("Processing data");

  if (_params.debug)
    {
      cerr << "MdvDist2NonMissing::_processData: Processing time: " << input_time
           << " file trigger: " << filepath << endl;
    }
  
  //
  // Reads field data from DsMdvx objects 
  // 
  if (_readMdv(input_time, leadTime, filepath))
    {
      cerr << "MdvDist2NonMissing::_readMdv(): ERROR reading mdv file" << endl;
      return 1;
    }

  if(_findDistances())
    {
      cerr << "MdvDist2NonMissing::_processData(): ERROR in method finding "
	   << "distances to nonmissing data." << endl;
      return 1;
    }

  if (_writeMdv())
    {
      cerr << "MdvDist2NonMissing::_writeMdv(): ERROR writing data." << endl;
      return 1;
    }

  //
  // Cleanup
  //
  _clear();

  return 0;
}

//
// MdvDist2NonMissing::_readMdv():
// Read mdv field data based on param file specifications.
//
int MdvDist2NonMissing::_readMdv(time_t requestTime, int leadTime, const string filepath)
{
 
  //
  // Set up for reading mdv data, reinitialize DsMdvx object
  //
  _mdv.clearRead();
  
  _mdv.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  
  _mdv.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  
  _mdv.setReadScalingType(Mdvx::SCALING_NONE);

  if ( _params.use_field_name)
    _mdv.addReadField(_params.field_name);
  else 
    _mdv.addReadField(_params.field_number);	
  
  if (_params.debug == Params::DEBUG_VERBOSE)
    _mdv.setDebug(_params.debug);
 
  //
  // Remap data
  //
  if (_params.remap_grid)
    {
      switch (_params.grid_projection)
        {
          case Params::PROJ_FLAT:
            _mdv.setReadRemapFlat(_params.grid_params.nx,
                                  _params.grid_params.ny,
                                  _params.grid_params.minx,
                                  _params.grid_params.miny,
                                  _params.grid_params.dx,
                                  _params.grid_params.dy,
                                  _params.origin_lat,
                                  _params.origin_lon,
                                  _params.flat_rotation);
            break;

          case Params::PROJ_LATLON :

            _mdv.setReadRemapLatlon(_params.grid_params.nx,
                                    _params.grid_params.ny,
                                    _params.grid_params.minx,
                                    _params.grid_params.miny,
                                    _params.grid_params.dx,
                                    _params.grid_params.dy);
            break;

         case Params::PROJ_LAMBERT :

           _mdv.setReadRemapLc2(_params.grid_params.nx,
                                _params.grid_params.ny,
                                _params.grid_params.minx,
                                _params.grid_params.miny,
                                _params.grid_params.dx,
                                _params.grid_params.dy,
                                _params.origin_lat,
                                _params.origin_lon,
                                _params.lambert_lat1,
                                _params.lambert_lat2);
           break;

          default:

            break;
        } // end switch
    } // end remap
 
  if ( _params.mode == Params::REALTIME || _params.mode == Params::TIME_INTERVAL)
    _mdv.setReadTime(Mdvx::READ_CLOSEST, _params.data_url, _params.search_margin, 
		     requestTime);
  else if (  _params.mode == Params::REALTIME_FCST_DATA)
     _mdv.setReadTime(Mdvx::READ_SPECIFIED_FORECAST, _params.data_url, _params.search_margin, 
		     requestTime, leadTime);
  else
    _mdv.setReadTime(Mdvx::READ_CLOSEST,filepath , _params.search_margin, 
		     requestTime);

  if (_params.debug) 
    {
      cerr << "MdvDist2NonMissing::_readMdv() : Reading data for URL: " 
	   << _params.data_url<< endl;
      if (_params.debug == Params::DEBUG_VERBOSE)
	_mdv.printReadRequest(cerr);
    }
  
  //
  // perform the read
  //
  if (_mdv.readVolume()) 
    {
      cerr << "MdvDist2NonMissing::readMdv(): ERROR: Cannot read data for url: " << _params.data_url<< endl;
      
      cerr << "  " << _mdv.getErrStr() << endl;
      
      return 1;
    }
  
  //
  // Get field data
  //
  MdvxField *field = _mdv.getField(0);
  
  const Mdvx::field_header_t fHdr  =  field->getFieldHeader();
  
  _nx = fHdr.nx;
  
  _ny = fHdr.ny;

  _grid_dx = fHdr.grid_dx;
  
  _dataArraySize = fHdr.nx * fHdr.ny; 
   
  _missing = fHdr.missing_data_value;
      
  _bad = fHdr.missing_data_value;
      
  //
  // Record field header info for writing mdv after distance calculation
  //
  _setFieldHeader(fHdr);
  
  _fieldData = (float*)field->getVol();

  //
  // Get data for filling out master header for output file
  // 
  DsMdvx::master_header_t inMhdr = _mdv.getMasterHeader();
      
  _setMasterHeader(inMhdr);
  
  _gridProjection.init(fHdr);

  return 0;
}

//
// MdvDist2NonMissing::_findDistances()
//
int MdvDist2NonMissing::_findDistances()
{
  if (_params.debug)
    {
      cerr << "MdvDist2NonMissing::_findDist(): Finding distance from point "
	   << "to closest non missing point." << endl;
    }

  //
  // Allocate array
  //
  _dist = new float[_dataArraySize];

  if (_dist == NULL )
    {
      cerr << "MdvDist2NonMissing::_findDist(): Trouble allocating memory for arrays.";
      return 1;
    }
 
  //
  // Initialize array
  //
  memset((void*)_dist, 0, sizeof(fl32) * _dataArraySize);
 
  
  for (int i = 0; i < _nx; i++)
    { 
      
      for (int j = 0; j < _ny; j++)
	{
	
	  fl32 minDist = _params.max_radius; 

	  bool distFound = false;
	  
	  //
	  // Check right away if point is not missing or bad
	  // since we need not go further.
	  //
	  if (_fieldData[j*_nx + i] != _bad &&
	      _fieldData[j*_nx + i] != _missing)
	    { 
	      distFound = true;
	      
	      minDist = 0.0;
	    }
	  else
	    {
	      int  r = 1;
	      
	      double xDist; 	      

	      if(  _gridProjection.getProjType() == Mdvx::PROJ_LATLON)
		xDist = r*_grid_dx * 111.7;
	      else
		xDist = r*_grid_dx;
	    
	      while(!distFound && xDist < _params.max_radius)
		{
		  for (int y = j - r; y<= j + r; y = y + 2*r )
		    { 
		     for (int x = i - r; x<= i + r; x++)
		       {
			  if (x > 0 && x < _nx && y > 0 && y < _ny)
			    {
			      int index = y * _nx + x;
			      
			      if (_fieldData[index] != _bad && 
				  _fieldData[index] != _missing)
				{
				  if (xDist < minDist)
				    minDist = xDist;
				  distFound = true;
				}
			    } //end if		
		       }
		    }
		  for ( int x = i-r ; x <= i + r ; x = x + 2*r)
		    {
		      for (int y = j - r + 1 ;  y <= j + r -1; y++)
			{
			  if (x > 0 && x < _nx && y > 0 && y < _ny)
			    {
			      int index = y * _nx + x;

			      if (_fieldData[index] != _bad && 
				  _fieldData[index] != _missing)
				{
                                  if (xDist < minDist) 
				      minDist = xDist;
                                  distFound = true;
				}
			    } //end if		 
			}
		    }
		  r++;
		  if( _gridProjection.getProjType() == Mdvx::PROJ_LATLON)
		    xDist = r*_grid_dx * 111.7;
		  else
		    xDist = r*_grid_dx; 
		}// end while
	    }// end else
	  if (!distFound)
	    _dist[j*_nx + i] = _params.max_radius;
	  else
	    _dist[j*_nx + i] = minDist;
	      
	} // end for
    }

  
  return 0;
}


//
// _writeMdv: Write linear combination field to disk.
//
int MdvDist2NonMissing::_writeMdv()
{

  if (_params.debug)
    cerr << "MdvDist2NonMissing::_writeMdv(): Writing data to " << _params.output_url << endl;
    
  DsMdvx mdvFile;
 
  //
  // Set master header
  //
  mdvFile.setMasterHeader(_masterHdr);

  //
  // Zero out the vlevel header
  //
  DsMdvx::vlevel_header_t vlevelHdr;

  memset(&vlevelHdr, 0, sizeof(vlevelHdr));
  
  vlevelHdr.type[0] =  Mdvx::VERT_TYPE_UNKNOWN;
      
  vlevelHdr.level[0] = 0;      
      
  //
  // Add field
  //
  MdvxField *field = new MdvxField(_fieldHdr, vlevelHdr, (void*)_dist);
  
  field->convertType(Mdvx::ENCODING_INT8,
		     Mdvx::COMPRESSION_GZIP,
		     Mdvx::SCALING_DYNAMIC);
  
  mdvFile.addField(field);
 
  mdvFile.writeToDir(_params.output_url);

  return 0;
} 

//
// setMasterHeader(): Fill out the master header fields
//
int MdvDist2NonMissing::_setMasterHeader(const DsMdvx::master_header_t &inMhdr)
{
  if (_params.debug)
    cerr << "MdvDist2NonMissing::_setMasterHeader(): Setting master header." << endl;

  memset(&_masterHdr, 0, sizeof(_masterHdr));

  _masterHdr.grid_orientation = 1;
 
  _masterHdr.time_gen = inMhdr.time_gen;
  
  _masterHdr.time_begin = inMhdr.time_begin;

  _masterHdr.time_end = inMhdr.time_end;

  _masterHdr.time_centroid = inMhdr.time_centroid;

  _masterHdr.data_dimension = inMhdr.data_dimension;
 
  _masterHdr.n_fields = 1;
  
  _masterHdr.time_written = time(0);

  STRcopy(_masterHdr.data_set_info,"MdvDist2NonMissing" , MDV_INFO_LEN);

  STRcopy(_masterHdr.data_set_name, "MdvDist2NonMissing", MDV_NAME_LEN);

  STRcopy(_masterHdr.data_set_source, "MdvDist2NonMissing", MDV_NAME_LEN);

  return 0;
}

//
// setFieldHeader(): Fill in the field header information
//
int MdvDist2NonMissing::_setFieldHeader(const DsMdvx::field_header_t &inFhdr)
{
  if (_params.debug)
    cerr << "MdvDist2NonMissing::_setFieldHeader(): Setting field header." << endl;

  //
  // Initialize the field header
  //
  memset(&_fieldHdr, 0, sizeof(_fieldHdr));

  //
  // Set relevant field header members
  //
  _fieldHdr.forecast_delta = inFhdr.forecast_delta;

  _fieldHdr.forecast_time = inFhdr.forecast_time;

  _fieldHdr.nx = inFhdr.nx;

  _fieldHdr.ny = inFhdr.ny;

  _fieldHdr.nz = inFhdr.nz;

  _fieldHdr.proj_type = inFhdr.proj_type;
  
  _fieldHdr.encoding_type = Mdvx::ENCODING_FLOAT32;

  _fieldHdr.data_element_nbytes = 4;

  _fieldHdr.volume_size = inFhdr.nx  * inFhdr.ny *  inFhdr.nz * 
                          _fieldHdr.data_element_nbytes;

  _fieldHdr.compression_type = Mdvx::COMPRESSION_NONE;

  _fieldHdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  
  _fieldHdr.scaling_type = Mdvx::SCALING_DYNAMIC;
  
  _fieldHdr.data_dimension = inFhdr.data_dimension; 
   
  _fieldHdr.proj_origin_lat = inFhdr.proj_origin_lat;

  _fieldHdr.proj_origin_lon = inFhdr.proj_origin_lon;
 
  _fieldHdr.grid_dx = inFhdr.grid_dx;
  
  _fieldHdr.grid_dy = inFhdr.grid_dy;

  _fieldHdr.grid_dz = inFhdr.grid_dz;
  
  _fieldHdr.grid_minx = inFhdr.grid_minx;
  
  _fieldHdr.grid_miny = inFhdr.grid_miny;
  
  _fieldHdr.grid_minz = inFhdr.grid_minz;
  
  _fieldHdr.bad_data_value = _bad;
  
  _fieldHdr.missing_data_value = _missing;
  
  _fieldHdr.proj_rotation = inFhdr.proj_rotation;
  
  _fieldHdr.proj_param[0] =  inFhdr.proj_param[0];

  _fieldHdr.proj_param[1] =  inFhdr.proj_param[1];
  
  _fieldHdr.proj_param[6] =  inFhdr.proj_param[6];

  _fieldHdr.proj_param[7] =  inFhdr.proj_param[7];
 
  STRcopy(_fieldHdr.units, "km", MDV_SHORT_FIELD_LEN);
  
  STRcopy(_fieldHdr.field_name_long,"DistanceToNonMissingData", MDV_LONG_FIELD_LEN);
  
  STRcopy(_fieldHdr.field_name, "Distance", MDV_SHORT_FIELD_LEN);

  return 0;
}
