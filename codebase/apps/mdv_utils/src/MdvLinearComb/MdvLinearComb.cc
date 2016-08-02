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
#include "MdvLinearComb.hh"
using namespace std;

//
// Constructor
//
MdvLinearComb::MdvLinearComb(int argc, char **argv)
{
  isOK = true;

  //
  // set programe name
  //
  _progName = "MdvLinearComb";

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
  
  _linearComb = NULL;

  _fieldOfMaxValue = NULL;

  return;

}

//////////////////////////////////////////////////////
//
// destructor
//
MdvLinearComb::~MdvLinearComb()
{
  //
  // unregister process
  //
  PMU_auto_unregister();

  if( _trigger)
    delete _trigger;

  _clear();

}

void MdvLinearComb::_clear()
{  
  for (int i = 0; i < (int)_fieldData.size(); i++)
    delete _fieldData[i];

  _fieldData.erase(_fieldData.begin(), _fieldData.end());

  _bad.erase(_bad.begin(), _bad.end());

  _missing.erase(_missing.begin(), _missing.end());

  if (_linearComb)
    delete[] _linearComb;

  _linearComb = NULL;

  if (_fieldOfMaxValue)
    delete [] _fieldOfMaxValue;

  _fieldOfMaxValue = NULL;
}

//////////////////////////////////////////////////
//
// Run
//
int MdvLinearComb::Run ()
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
          cerr << "MdvLinearComb::Run(): Creating ARCHIVE trigger\n"
               << "     start time: " << _args.startTime
               << "     end time:   " << _args.endTime << endl;
        }

      DsTimeListTrigger *archive_trigger = new DsTimeListTrigger();

      if (archive_trigger->init(_params.trigger_url,
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
  else  if ( _params.mode == Params::FILELIST )
    {
      //
      // Archive filelist mode.
      //
      if ( _args.inputFileList.size() )
        {
          if (_params.debug)
            cerr << "MdvLinearComb::Run(): Initializing archive FILELIST mode." << endl;

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
  else if (_params.mode == Params::REALTIME)
    {
      if (_params.debug)
        {
          cerr << "MdvLinearComb::Run(): Creating REALTIME trigger" << endl;
        }

      //
      // realtime mode
      //
      DsLdataTrigger *realtime_trigger = new DsLdataTrigger();

      if (realtime_trigger->init(_params.trigger_url,
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
  else if (_params.mode == Params::REALTIME_MULT_URL)
    {
      if (_params.debug)
        {
          cerr << "MdvLinearComb::Run(): Creating REALTIME_MULT_URL trigger" << endl;
        }

      //
      // realtime mode
      //
      DsMultipleTrigger *realtime_mult_url_trigger = new DsMultipleTrigger();

      if (realtime_mult_url_trigger->initRealtime(_params.max_valid_realtime_age,
						  PMU_auto_register))
        {
	  for (int i = 0; i < _params.mult_trigger_url_n; ++i)
	    realtime_mult_url_trigger->add(_params._mult_trigger_url[i]);

          _trigger = realtime_mult_url_trigger;
        }
      else
        {
          cerr << realtime_mult_url_trigger->getErrStr();
          _trigger = 0;
          return 1;
        }
    }

  if ( _trigger == 0 )
    {
      cerr << "MdvLinearComb::Run" <<"  ERROR: Failed to instantiate trigger\n" << endl;
      
      exit (1);
    }

  //
  //
  // process data
  //
 
  while (!_trigger->endOfData())
    {
      TriggerInfo triggerInfo;
      _trigger->next(triggerInfo);
      if (_processData(triggerInfo.getIssueTime(), 
		       triggerInfo.getForecastTime() - triggerInfo.getIssueTime(),
		       triggerInfo.getFilePath()))
	{
	  cerr << "MdvLinearComb::Run" <<"  Errors in processing time: " <<  triggerInfo.getIssueTime()
	       << " input file: " << triggerInfo.getFilePath() << endl;
	  
	  return 1;
	}

      _clear();
      
    } // while
  
  return 0;

}

///////////////////////////////////
//
//  process data at trigger time
//
int MdvLinearComb::_processData(time_t input_time, int lead_time, const string filepath)
{
  //
  // registration with procmap
  //
  PMU_force_register("Processing data");

  if (_params.debug)
    {
      cerr << "MdvLinearComb::_processData: Processing time: " << input_time
           << " file trigger: " << filepath << endl;
    }
  
  //
  // Reads field data from DsMdvx objects 
  // 
  if (_readMdv(input_time, lead_time, filepath))
    {
      cerr << "MdvLinearComb::_readMdv(): ERROR reading mdv file" << endl;
      return 1;
    }

  if(_doLinearComb())
    {
      cerr << "MdvLinearComb::_processData(): ERROR doing linear combination of fields." << endl;
      return 1;
    }

  if (_params.output_field_of_max_val)
    {
      if (_recordFieldOfMaxVal())
	{
	   cerr << "MdvLinearComb::_processData(): ERROR determining field of max value." << endl;
	   return 1;
	}
    }

  if (_writeMdv())
    {
      cerr << "MdvLinearComb::_writeMdv(): ERROR writing data." << endl;
      return 1;
    }

  //
  // Cleanup
  //
  _clear();

  return 0;
}

//
// MdvLinearComb::_readMdv():
// Read mdv field data based on param file specifications.
// For each field, stuff field data, bad, and missing data values into 
// into a vector. If it is the first field, get field header and master header
// information for the output file. 
//
int MdvLinearComb::_readMdv(time_t requestTime, int leadTime, const string filepath)
{

  bool isFirstField = true;

  //
  // Foreach specified field in the param file, read
  // data from the mdv file
  //
  for (int i = 0; i < _params.field_data_n; i++)
    { 

      DsMdvx mdv;

      //
      // Set up for reading mdv data, reinitialize DsMdvx object
      //
      mdv.clearRead();
      
      mdv.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
      
      mdv.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  
      mdv.setReadScalingType(Mdvx::SCALING_NONE);

      if ( _params._field_data[i].use_field_name)
	mdv.addReadField(_params._field_data[i].field_name);
      else 
	mdv.addReadField(_params._field_data[i].field_number);	
  
      if (_params.debug == Params::DEBUG_VERBOSE)
	mdv.setDebug(_params.debug);
      
      int timeOffset = _params._field_data[i].time_offset;
      
      if (_params.is_forecast_data) {
        leadTime += timeOffset;
	mdv.setReadTime(Mdvx::READ_SPECIFIED_FORECAST, _params._field_data[i].url,
			_params._field_data[i].search_margin, requestTime, leadTime);
      } else {
        if ( _params.mode == Params::FILELIST )
        {
          mdv.setReadPath(filepath);
        } else {
          requestTime += timeOffset;
          mdv.setReadTime(Mdvx::READ_CLOSEST, _params._field_data[i].url,
                          _params._field_data[i].search_margin, requestTime);
        }
      }
	
      // 
      // Set vertical limits on data
      //
      if(_params._field_data[i].vert_action != Params::VERT_ACTION_NONE && 
	 _params._field_data[i].vlevel_type == Params::VLEVEL_DATA_UNITS)
	mdv.setReadVlevelLimits( _params._field_data[i].vertical_min, _params._field_data[i].vertical_max);
      
      if(_params._field_data[i].vert_action != Params::VERT_ACTION_NONE && 
	 _params._field_data[i].vlevel_type == Params::VLEVEL_PLANE_NUM)
	mdv.setReadPlaneNumLimits( (int)_params._field_data[i].vertical_min, (int)_params._field_data[i].vertical_max);
      
      //
      // Set composite request
      //
      if (_params._field_data[i].vert_action == Params::VERT_ACTION_COMPOSITE)
	mdv.setReadComposite();

          
      //
      // Remap data
      //
      if (_params.remap_grid)
	{
	  switch (_params.grid_projection)
	    {
	    case Params::PROJ_FLAT:
	      mdv.setReadRemapFlat(_params.grid_params.nx,
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
	      
	      mdv.setReadRemapLatlon(_params.grid_params.nx,
				     _params.grid_params.ny,
				     _params.grid_params.minx,
				     _params.grid_params.miny,
				     _params.grid_params.dx,
				     _params.grid_params.dy);
	      break;
	      
	    case Params::PROJ_LAMBERT :
	      
	      mdv.setReadRemapLc2(_params.grid_params.nx,
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
  
      if (_params.debug) 
	{
	  cerr << "MdvLinearComb::_readMdv() : Reading data for URL: " 
	       << _params._field_data[i].url << endl;
	  if (_params.debug == Params::DEBUG_VERBOSE)
	    mdv.printReadRequest(cerr);
	}
  
      //
      // perform the read
      //
      if (mdv.readVolume()) 
	{
	  cerr << "MdvLinearComb::readMdv(): ERROR: Cannot read data for url: "
               << _params._field_data[i].url << endl;
	  cerr << "  " << mdv.getErrStr() << endl;
	  return 1;
	}
      
      if (_params.debug) 
      {
        cerr << "Read file: " 
             << mdv.getPathInUse() << endl;
      }
  
      //
      // Get field data
      //
      MdvxField *field = mdv.getField(0);

      const Mdvx::field_header_t fHdr  =  field->getFieldHeader();

      //
      // If this is the first field, we will copy general field header
      // info and pass to new field header.
      //
      if (isFirstField) {

        _nx = fHdr.nx;
        _ny = fHdr.ny;
        _nz = fHdr.nz;
        
        _dataArraySize = _nx * _ny * _nz; 
        _combMissing = -9999.0;
        _combBad = -9999.0;
        
        //
        // Record field header info for writing mdv after linear combination
        //
        _setFieldHeader(fHdr);
        
      } else {
        
        if (_dataArraySize != fHdr.nx * fHdr.ny * fHdr.nz) {
          cerr << "ERROR - MdvLinearComb::_readMdv" << endl;
          cerr << "  Data grid has changed size" << endl;
          cerr << "  Prev nx, ny, nz: " << _nx << ", " << _ny << ", " << _nz << endl;
          cerr << "  Latest nx, ny, nz: " << fHdr.nx << ", " << fHdr.ny << ", " << fHdr.nz << endl;
          return -1;
        }

      }

      fl32 *data =  (fl32*)field->getVol();

      fl32 *fieldData = new fl32[_dataArraySize];

      memcpy(fieldData, data, _dataArraySize *sizeof(fl32));

      _fieldData.push_back(fieldData);
      
      _bad.push_back(fHdr.bad_data_value);

      _missing.push_back(fHdr.missing_data_value);

      //
      // Get data for filling out master header for output file
      // 
      if (isFirstField)
	{
	  DsMdvx::master_header_t inMhdr = mdv.getMasterHeader();
	  
	  _setMasterHeader(inMhdr);
	} 
      
      isFirstField = false;
    }

  return 0;
}

//
// MdvLinearComb::_doLinearComb()
// Do a linear combination of fields specified in the param file.
// Foreach data point int the grid do a1*x1 + a2*x2 + an*xn + c
// where x1,x2,..,xn are the field values at a given point,
// and a1,a2,...,an are the coefficients listed in the param file for
// each field and c is the constant listed in the param file.
//
int MdvLinearComb::_doLinearComb()
{
  if (_params.debug)
    {
      cerr << "MdvLinearComb::_doLinearComb(): Combining data." << endl;
    }

  _linearComb = new fl32[_dataArraySize];

  if (_linearComb == NULL)
    {
      cerr << "MdvLinearComb::_doLinearComb(): Trouble allocating memory for array.";
      
      return 1;
    }

  for (int j = 0; j < _dataArraySize; j++)
    {
      //
      // initialize the sum, initialize field of max val
      //
      _linearComb[j] = 0;

      for (int i = 0; i < (int)_fieldData.size(); i++)
      {
        fl32 val = _fieldData[i][j];
        if ( fabs(val - _missing[i]) > MDVLINEARCOMB_EPSILON && 
             fabs(val -  _bad[i]) > MDVLINEARCOMB_EPSILON)
        {
          _linearComb[j] +=
            _params._field_data[i].scale * val + _params._field_data[i].offset;
//           cerr << "i, j, data, scale, offset, result: "
//                <<  i << ", " << j << ", "
//                << _fieldData[i][j] << ", "
//                << _params._field_data[i].scale << ", "
//                << _params._field_data[i].offset << ", "
//                << _linearComb[j] << endl;
        }
        else
        {
          //
          // data is missing for any fields, set the linear comb to missing
          //
          _linearComb[j] = _combMissing;
          
          break;
        }
      }
    }
  return 0;
}

//
// MdvLinearComb::_recordFieldOfMaxVal(): At each grid point, record the field
//                                        number of the field with the max value.
//                                        Set field number to -1 if unable to compute. 
//
int MdvLinearComb::_recordFieldOfMaxVal()
{
  if (_params.debug)
    {
      cerr << "MdvLinearComb::_recordFieldOfMaxVal()" << endl;
    }

  //
  // Allocate memory for array.
  //
  _fieldOfMaxValue =  new fl32[_dataArraySize];

  if (_fieldOfMaxValue == NULL)
    {
      cerr << "MdvLinearComb::_recordFieldOfMaxVal(): Trouble allocating memory for array.";
      
      return 1;
    }

  //
  // Record the field which produces the max value at each grid point.
  // Record -1 if data is missing or bad for any of the fields.
  //
  for (int j = 0; j < _dataArraySize; j++)
    {
      fl32 maxValue = 0;
      bool maxValInitialized  = false;

      for (int i = 0; i < (int)_fieldData.size(); i++)
	{
	  if ( fabs(_fieldData[i][j] - _missing[i]) > MDVLINEARCOMB_EPSILON && 
	       fabs(_fieldData[i][j] -  _bad[i]) > MDVLINEARCOMB_EPSILON)
	    {
	      if (!maxValInitialized)
		{
		  maxValue = _fieldData[i][j];
		  
		  _fieldOfMaxValue[j] = i;

		  maxValInitialized = true;
		} 
	      else
		{
		  if ( _fieldData[i][j] > maxValue)
		    {
		      maxValue = _fieldData[i][j];
		      
		      _fieldOfMaxValue[j] = i;
		    }
		}
	    }
	  else
	    {
	      //
	      // data is missing for any fields, set the linear comb to missing
	      //

	      _fieldOfMaxValue[j] = -1;

	      break;
	    }
	}
    } 

  return 0;
}


//
// Write linear combination field to disk.
//
int MdvLinearComb::_writeMdv()
{

  if (_params.debug)
    cerr << "MdvLinearComb::_writeMdv(): Writing data to " << _params.output_url << endl;
    
  DsMdvx mdvFile;
 
  //
  // Set master header
  //
  mdvFile.setMasterHeader(_masterHdr);

  //
  // Zero out the vlevel header since we dont know anything 
  // about the combination fields vertical levels.
  //
  DsMdvx::vlevel_header_t vlevelHdr;

  memset(&vlevelHdr, 0, sizeof (vlevelHdr));

  for (int i = 0; i < _fieldHdr.nz; i++)
    {
      vlevelHdr.type[i] =  Mdvx::VERT_TYPE_UNKNOWN;
      
      vlevelHdr.level[i] = 0;      
    }

  //
  // Add fields
  //
  MdvxField *field = new MdvxField(_fieldHdr, vlevelHdr, (void*)_linearComb);
  
  field->convertType(Mdvx::ENCODING_INT16,
		     Mdvx::COMPRESSION_GZIP,
		     Mdvx::SCALING_DYNAMIC);
  

  mdvFile.addField(field);

  if (_params.output_field_of_max_val)
    {
      //
      // Edit the field header for the linear combination
      // to be appropriate for this field.
      //
      _fieldHdr.nz = 1;
      
      _fieldHdr.bad_data_value = -1;
      
      _fieldHdr.missing_data_value = -1;

      _fieldHdr.volume_size = _nx  * _ny * _fieldHdr.data_element_nbytes;
      
      STRcopy(_fieldHdr.field_name_long, "FieldOfMaxVal" , MDV_LONG_FIELD_LEN);
 
      STRcopy(_fieldHdr.field_name, "FieldOfMaxVal", MDV_SHORT_FIELD_LEN);
      
      STRcopy(_fieldHdr.units, "none", MDV_SHORT_FIELD_LEN);    

      //
      // Add field
      //
      MdvxField *field2 = new MdvxField(_fieldHdr, vlevelHdr, (void*)_fieldOfMaxValue);

      field2->convertType(Mdvx::ENCODING_INT16,
			  Mdvx::COMPRESSION_GZIP,
			  Mdvx::SCALING_DYNAMIC);

      mdvFile.addField(field2);
    }


  if (_params.write_as_forecast)
   mdvFile.setWriteAsForecast(); 

  mdvFile.writeToDir(_params.output_url);

  return 0;
} 

//
// setMasterHeader(): Fill out the master header fields
//
int MdvLinearComb::_setMasterHeader(const DsMdvx::master_header_t &inMhdr)
{
  if (_params.debug)
    cerr << "MdvLinearComb::_setMasterHeader(): Setting master header." << endl;

  

  memset(&_masterHdr, 0, sizeof(_masterHdr));

 _masterHdr.grid_orientation = 1;
 
 _masterHdr.time_gen = inMhdr.time_gen;
  
  _masterHdr.time_begin = inMhdr.time_begin;

  _masterHdr.time_end = inMhdr.time_end;

  _masterHdr.time_centroid = inMhdr.time_centroid;

  _masterHdr.data_dimension = inMhdr.data_dimension;
  
  if (_params.output_field_of_max_val)
    _masterHdr.n_fields = 2;
  else
    _masterHdr.n_fields = 1;

  _masterHdr.time_written = time(0);

  STRcopy(_masterHdr.data_set_info,"MdvLinearComb" , MDV_INFO_LEN);

  STRcopy(_masterHdr.data_set_name, "MdvLinearComb", MDV_NAME_LEN);

  STRcopy(_masterHdr.data_set_source, "MdvLinearComb", MDV_NAME_LEN);

  return 0;
}

//
// setFieldHeader(): Fill in the field header information
//
int MdvLinearComb::_setFieldHeader(const DsMdvx::field_header_t &inFhdr)
{
  if (_params.debug)
    cerr << "MdvLinearComb::_setFieldHeader(): Setting field header." << endl;

  //
  // Initialize the field header
  //
  memset(&_fieldHdr, 0, sizeof(_fieldHdr));

  //
  // Is it sufficient to have start time and valid time to get
  // the forecast delta ? Must all be on the same page as
  // far as defining these times
  //
  _fieldHdr.forecast_delta = inFhdr.forecast_delta;

  _fieldHdr.forecast_time = inFhdr.forecast_time;

  _fieldHdr.nx = inFhdr.nx;

  _fieldHdr.ny = inFhdr.ny;

  _fieldHdr.nz = inFhdr.nz;

  _fieldHdr.proj_type = inFhdr.proj_type;
  
  _fieldHdr.encoding_type = Mdvx::ENCODING_FLOAT32;

  _fieldHdr.data_element_nbytes = 4;

  _fieldHdr.volume_size = inFhdr.nx  * inFhdr.ny *  inFhdr.nz * _fieldHdr.data_element_nbytes;

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
  
  _fieldHdr.grid_minz = 0;
  
  _fieldHdr.bad_data_value = _combBad;
  
  _fieldHdr.missing_data_value = _combMissing;
  
  _fieldHdr.proj_rotation = inFhdr.proj_rotation;
 
  STRcopy(_fieldHdr.field_name_long, _params.output_field_name , MDV_LONG_FIELD_LEN);
 
  STRcopy(_fieldHdr.field_name, _params.output_field_name, MDV_SHORT_FIELD_LEN);
  
  STRcopy(_fieldHdr.units, _params.output_field_units, MDV_SHORT_FIELD_LEN);    

  return 0;
}
