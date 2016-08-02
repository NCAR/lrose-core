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
#include "MdvCircularFilt.hh"
using namespace std;

//
// Constructor
//
MdvCircularFilt::MdvCircularFilt(int argc, char **argv)
{
  isOK = true;

  //
  // set programe name
  //
  _progName = "MdvCircularFilt";

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
  _paramsPath = "unknown";

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
  
  
  _min = NULL;

  _max = NULL;

  _avg = NULL;

  _std = NULL;

  _ptile = NULL;

  _cov = NULL;

  return;

}

//////////////////////////////////////////////////////
//
// destructor
//
MdvCircularFilt::~MdvCircularFilt()
{
  //
  // unregister process
  //
  PMU_auto_unregister();

  if( _trigger)
    delete _trigger;

  _clear();

}

void MdvCircularFilt::_clear()
{
  if ( _min)
    delete[] _min;
  
  _min = NULL;

  if (_max )
    delete[] _max;
  
  _max = NULL;

  if (_avg)
    delete[] _avg;

  _avg = NULL;

  if (_std)
    delete[] _std;

  _std = NULL;
 
  if (_ptile)
    delete[] _ptile;
  
  _ptile = NULL;
  
  if(_cov)
    delete[] _cov;

  _cov = NULL;

}

//////////////////////////////////////////////////
//
// Run
//
int MdvCircularFilt::Run ()
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
          cerr << "MdvCircularFilt::Run(): Creating ARCHIVE trigger\n"
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
            cerr << "MdvCircularFilt::Run(): Initializing archive FILELIST mode." << endl;

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
          cerr << "MdvCircularFilt::Run(): Creating REALTIME trigger" << endl;
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
      cerr << "MdvCircularFilt::Run" <<"  ERROR: Failed to instantiate trigger\n" << endl;
      
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
	  cerr << "MdvCircularFilt::Run" <<"  Errors in processing time: " <<  triggerInfo.getIssueTime()
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
int MdvCircularFilt::_processData(time_t input_time, int leadTime, const string filepath)
{
  //
  // registration with procmap
  //
  PMU_force_register("Processing data");

  if (_params.debug)
    {
      cerr << "MdvCircularFilt::_processData: Processing time: " << input_time
           << " file trigger: " << filepath << endl;
    }
  
  //
  // Reads field data from DsMdvx objects 
  // 
  if (_readMdv(input_time, leadTime, filepath))
    {
      cerr << "MdvCircularFilt::_readMdv(): ERROR reading mdv file" << endl;
      return 1;
    }

  if(_applyFilt())
    {
      cerr << "MdvCircularFilt::_processData(): ERROR applying filter to data." << endl;
      return 1;
    }

  if (_writeMdv())
    {
      cerr << "MdvCircularFilt::_writeMdv(): ERROR writing data." << endl;
      return 1;
    }

  //
  // Cleanup
  //
  _clear();

  return 0;
}

//
// MdvCircularFilt::_readMdv():
// Read mdv field data based on param file specifications.
//
int MdvCircularFilt::_readMdv(time_t requestTime, int leadTime, const string filepath)
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
  
  if ( _params.mode == Params::REALTIME || _params.mode == Params::TIME_INTERVAL)
    _mdv.setReadTime(Mdvx::READ_CLOSEST, _params.data_url, _params.search_margin, 
		     requestTime);
  else if (  _params.mode == Params::REALTIME_FCST_DATA)
     _mdv.setReadTime(Mdvx::READ_SPECIFIED_FORECAST, _params.data_url, _params.search_margin, 
		     requestTime, leadTime);
  else
    _mdv.setReadTime(Mdvx::READ_CLOSEST,filepath , _params.search_margin, 
		     requestTime);

  
  // 
  // Set vertical limits on data
  //
  if(_params.vert_action != Params::VERT_ACTION_NONE && 
     _params.vlevel_type == Params::VLEVEL_DATA_UNITS)
    _mdv.setReadVlevelLimits( _params.vertical_min, _params.vertical_max);
  
  if(_params.vert_action != Params::VERT_ACTION_NONE && 
     _params.vlevel_type == Params::VLEVEL_PLANE_NUM)
    _mdv.setReadPlaneNumLimits( (int)_params.vertical_min, (int)_params.vertical_max);
  
  //
  // Set composite request
  //
  if (_params.vert_action == Params::VERT_ACTION_COMPOSITE)
    _mdv.setReadComposite();
  
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
  
  if (_params.debug) 
    {
      cerr << "MdvCircularFilt::_readMdv() : Reading data for URL: " 
	   << _params.data_url<< endl;
      if (_params.debug == Params::DEBUG_VERBOSE)
	_mdv.printReadRequest(cerr);
    }
  
  //
  // perform the read
  //
  if (_mdv.readVolume()) 
    {
      cerr << "MdvCircularFilt::readMdv(): ERROR: Cannot read data for url: " << _params.data_url<< endl;
      
      cerr << "  " << _mdv.getErrStr() << endl;
      
      return 1;
    }
  
  //
  // Get field data
  //
  MdvxField *field = _mdv.getField(0);
  
  const Mdvx::field_header_t fHdr  =  field->getFieldHeader();
  
  _dataArraySize = fHdr.nx * fHdr.ny * fHdr.nz; 
  
  _nx = fHdr.nx;
  
  _ny = fHdr.ny;
  
  _missing = fHdr.missing_data_value;
      
  _bad = fHdr.missing_data_value;
      
  //
  // Record field header info for writing mdv after linear combination
  //
  _setFieldHeader(fHdr);
  
  _fieldData =  (float*)field->getVol();

  //
  // Get data for filling out master header for output file
  // 
  DsMdvx::master_header_t inMhdr = _mdv.getMasterHeader();
      
  _setMasterHeader(inMhdr);
  
  _gridProjection.init(fHdr);

  return 0;
}

//
// MdvCircularFilt::_applyFilt()
//
int MdvCircularFilt::_applyFilt()
{
  if (_params.debug)
    {
      cerr << "MdvCircularFilt::_applyFilt(): Filtering data." << endl;
    }

  //
  // Allocate arrays
  //
  _max = new float[_dataArraySize];
  _min = new float[_dataArraySize];
  _avg = new float[_dataArraySize];
  _std = new float[_dataArraySize];
  _cov = new float[_dataArraySize];
  _ptile = new float[_dataArraySize];

  //
  // Initialize arrays
  //
  for (int i = 0; i < _dataArraySize; i++)
    {
      _max[i] = 0;
      _min[i] = 0;
      _avg[i] = 0;
      _std[i] = 0;
      _cov[i] = 0;
      _ptile[i] = 0;
    }

  if (_max == NULL || _min == NULL || _avg == NULL 
      || _std == NULL || _cov == NULL || _ptile == NULL )
    {
      cerr << "MdvCircularFilt::_applyFilt(): Trouble allocating memory for arrays.";
      return 1;
    }

  //
  // Instantiate circular region template
  //
  double radiusInGridSqr = _gridProjection.km2xGrid( _params.radius );
  
  CircularTemplate circTemplate(radiusInGridSqr);
  
  //
  // Step through grid and apply circular filter
  //
  if (_params.debug == Params::DEBUG_VERBOSE)
    cerr << "There are "  << _ny << " rows of data to filter." << endl;
  for (int j = 0; j < _ny; j++)
    {
       if (_params.debug == Params::DEBUG_VERBOSE)
	 if ( j%10 == 0)
	   cerr << "Filtering row " << j << endl;

      for (int i = 0; i < _nx; i++)
	{ 
	  //
	  // Get the array index of the data point about which 
	  // we filter data.
	  //
	  int index = j * _nx + i;

	  //
	  // Initialize variables for computing stats on the circular
	  // region.
	  //
	  bool isFirstPt = true;

	  int numGoodPts = 0;

	  list <float> circRegionVals;

	  circRegionVals.clear();

	  circRegionVals.erase( circRegionVals.begin(), circRegionVals.end());

	  //
	  // Loop through all points of grid within the circle centered
	  // at (i,j) 
	  //
	  for( GridPoint *point = circTemplate.getFirstInGrid(i,j, _nx,_ny);
	       point != NULL; point = circTemplate.getNextInGrid())
	    {	      
	      //
	      // Get index of point in circle about center (i,j)
	      //
	      int pIndex = point->getIndex(_nx,_ny);

	      //
	      // If data val is not missing or bad, record some stats
	      //
	      if (fabs(_fieldData[pIndex] -_bad) > MDVCIRCULARFILT_EPSILON 
		  && fabs (_fieldData[pIndex] - _missing)  > MDVCIRCULARFILT_EPSILON)
		{
		  numGoodPts++;

		  if (isFirstPt)
		    {
		      _max[index] = _fieldData[pIndex];
		      
		      _min[index] = _fieldData[pIndex];

		      isFirstPt = false;
		    }
		  else
		    {
		      if (_fieldData[pIndex] > _max[index])
			_max[index] = _fieldData[pIndex];
		      
		      if (_fieldData[pIndex] < _min[index])
			_min[index] = _fieldData[pIndex];
		    }
		  
		  //
		  // For avg: we'll divide later by number of vals
		  // 
		  _avg[index] += _fieldData[pIndex];
		  
		  //
		  // For standard deviation we'll just keep track
		  // of sum of squares for now since
		  // std = sum( [x(i)]^2 + n(mean)^2/(n-1)
		  // 
		  _std[index] += (_fieldData[pIndex] * _fieldData[pIndex]);
		  
		  //
		  // Keep the list of vals for %cov and percentile
		  // calculations
		  //
		  circRegionVals.push_back(_fieldData[pIndex]);
					   
		} // end if( GridPoint ...
	    }

	  //
	  // Compute Statistics
	  //
	  if (numGoodPts > 0)
	    {
	      //
	      // Compute avg
	      //
	      _avg[index] = _avg[index]/numGoodPts;
	      
	      //
	      // Compute Std deviation
	      //
	      if (numGoodPts > 1)
		{
		  _std[index] =  (_std[index] -  _avg[index] *
				  _avg[index] *( numGoodPts))/ (numGoodPts -1);
		  _std[index] = sqrt( _std[index]);
		}
	      else
		_std[index] = 0;
	      
	      //
	      // Calculate percent coverage and record requested 
	      // percentile of data.
	      //
	      int ptileIndex = (int)round( float( _params.ptile/100) 
					   * (int)(circRegionVals.size()-1));
	      
	      
	      if ( circRegionVals.size() >1)
		circRegionVals.sort();
	      
	      float numAboveThresh = 0;
	      
	      int listElementCount = 0;
	      
	      list <float>::iterator iter;
	      
	      for ( iter = circRegionVals.begin(); iter != circRegionVals.end(); 
		    iter++)
		{
		  
		  if (*iter >= _params.percent_cov_thresh)
		    numAboveThresh++;
		  
		  if (listElementCount ==  ptileIndex)
		    _ptile[index] = *iter;
		  
		  listElementCount++;
		}
	      
	      _cov[index] = (float)numAboveThresh/numGoodPts *100;
	      
	    }
	  else
	    {
	      //
	      // No non missing or bad data in circular region.
	      // Set to missing.
	      //
	      _min[index] = _missing;
	      _max[index] = _missing;
	      _avg[index] = _missing;
	      _ptile[index] = _missing;
	      _std[index] = _missing;
	      _cov[index] = _missing;
	      
	    }
	}
    }
  
  return 0;
}


//
// _writeMdv: Write linear combination field to disk.
//
int MdvCircularFilt::_writeMdv()
{

  if (_params.debug)
    cerr << "MdvCircularFilt::_writeMdv(): Writing data to " << _params.output_url << endl;
    
  DsMdvx mdvFile;
 
  char fieldNameLong[512];

  char fieldNameShort[512];

  sprintf(fieldNameLong, "%s", _fieldHdr.field_name_long);

  sprintf(fieldNameShort, "%s", _fieldHdr.field_name);

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

  if (_params.doMax)
    {
      //
      // Edit the field header for the linear combination
      // to be appropriate for this field.
      //
      char newFieldNameLong[512];
      
      sprintf(newFieldNameLong,"%sMaxFilt", fieldNameLong);

      STRcopy(_fieldHdr.field_name_long, newFieldNameLong , MDV_LONG_FIELD_LEN);
      
      char newFieldNameShort[512];

      sprintf(newFieldNameShort,"%sMaxFilt", fieldNameShort);

      STRcopy(_fieldHdr.field_name,newFieldNameShort , MDV_SHORT_FIELD_LEN);
      
      //
      // Add field
      //
      MdvxField *field = new MdvxField(_fieldHdr, vlevelHdr, (void*)_max);

      field->convertType(Mdvx::ENCODING_INT8,
			  Mdvx::COMPRESSION_GZIP,
			  Mdvx::SCALING_DYNAMIC);

      mdvFile.addField(field);
    }

  if (_params.doMin)
    {
      //
      // Edit the field header for the linear combination
      // to be appropriate for this field.
      //
      char newFieldNameLong[512];
      
      sprintf(newFieldNameLong,"%sMinFilt", fieldNameLong);

      STRcopy(_fieldHdr.field_name_long, newFieldNameLong , MDV_LONG_FIELD_LEN);
      
      char newFieldNameShort[512];

      sprintf(newFieldNameShort,"%sMinFilt", fieldNameShort);

      STRcopy(_fieldHdr.field_name,newFieldNameShort , MDV_SHORT_FIELD_LEN);
      
      //
      // Add field
      //
      MdvxField *field = new MdvxField(_fieldHdr, vlevelHdr, (void*)_min);

      field->convertType(Mdvx::ENCODING_INT8,
			  Mdvx::COMPRESSION_GZIP,
			  Mdvx::SCALING_DYNAMIC);

      mdvFile.addField(field);
    }

  if (_params.doAvg)
    {
      //
      // Edit the field header for the linear combination
      // to be appropriate for this field.
      //
      char newFieldNameLong[512];
      
      sprintf(newFieldNameLong,"%sAvgFilt", fieldNameLong);

      STRcopy(_fieldHdr.field_name_long, newFieldNameLong , MDV_LONG_FIELD_LEN);
      
      char newFieldNameShort[512];

      sprintf(newFieldNameShort,"%sAvgFilt", fieldNameShort);

      STRcopy(_fieldHdr.field_name,newFieldNameShort , MDV_SHORT_FIELD_LEN);
      
      //
      // Add field
      //
      MdvxField *field = new MdvxField(_fieldHdr, vlevelHdr, (void*)_avg);

      field->convertType(Mdvx::ENCODING_INT8,
			  Mdvx::COMPRESSION_GZIP,
			  Mdvx::SCALING_DYNAMIC);

      mdvFile.addField(field);
    }

  if (_params.doStd)
    {
      //
      // Edit the field header for the linear combination
      // to be appropriate for this field.
      //
      char newFieldNameLong[512];
      
      sprintf(newFieldNameLong,"%sStdFilt", fieldNameLong);

      STRcopy(_fieldHdr.field_name_long, newFieldNameLong , MDV_LONG_FIELD_LEN);
      
      char newFieldNameShort[512];

      sprintf(newFieldNameShort,"%sStdFilt", fieldNameShort);

      STRcopy(_fieldHdr.field_name,newFieldNameShort , MDV_SHORT_FIELD_LEN);
      
      //
      // Add field
      //
      MdvxField *field = new MdvxField(_fieldHdr, vlevelHdr, (void*)_std);

      field->convertType(Mdvx::ENCODING_INT8,
			  Mdvx::COMPRESSION_GZIP,
			  Mdvx::SCALING_DYNAMIC);

      mdvFile.addField(field);
    }

  if (_params.doPtile)
    {
      //
      // Edit the field header for the linear combination
      // to be appropriate for this field.
      //
      char newFieldNameLong[512];
      
      sprintf(newFieldNameLong,"%sPtile%dFilt", fieldNameLong,
	      (int)_params.ptile);

      STRcopy(_fieldHdr.field_name_long, newFieldNameLong , MDV_LONG_FIELD_LEN);
      
      char newFieldNameShort[512];

      sprintf(newFieldNameShort,"%sPtile%dFilt", fieldNameShort,
	      (int)_params.ptile);

      STRcopy(_fieldHdr.field_name,newFieldNameShort , MDV_SHORT_FIELD_LEN);
      
      //
      // Add field
      //
      MdvxField *field = new MdvxField(_fieldHdr, vlevelHdr, (void*)_ptile);

      field->convertType(Mdvx::ENCODING_INT8,
			  Mdvx::COMPRESSION_GZIP,
			  Mdvx::SCALING_DYNAMIC);

      mdvFile.addField(field);
    }
  if (_params.doPercentCov)
    {
      //
      // Edit the field header for the linear combination
      // to be appropriate for this field.
      //
      char newFieldNameLong[512];
      
      sprintf(newFieldNameLong,"%sPctCov%dFilt", fieldNameLong,
	      (int)_params.percent_cov_thresh);

      STRcopy(_fieldHdr.field_name_long, newFieldNameLong , MDV_LONG_FIELD_LEN);
      
      char newFieldNameShort[512];

      sprintf(newFieldNameShort,"%sPctCov%dFilt", fieldNameShort,
	      (int)_params.percent_cov_thresh);

      STRcopy(_fieldHdr.field_name,newFieldNameShort , MDV_SHORT_FIELD_LEN);
      
      //
      // Add field
      //
      MdvxField *field = new MdvxField(_fieldHdr, vlevelHdr, (void*)_cov);

      field->convertType(Mdvx::ENCODING_INT8,
			  Mdvx::COMPRESSION_GZIP,
			  Mdvx::SCALING_DYNAMIC);

      mdvFile.addField(field);
    }

  mdvFile.writeToDir(_params.output_url);

  return 0;
} 

//
// setMasterHeader(): Fill out the master header fields
//
int MdvCircularFilt::_setMasterHeader(const DsMdvx::master_header_t &inMhdr)
{
  if (_params.debug)
    cerr << "MdvCircularFilt::_setMasterHeader(): Setting master header." << endl;

  memset(&_masterHdr, 0, sizeof(_masterHdr));

  _masterHdr.grid_orientation = 1;
 
  _masterHdr.time_gen = inMhdr.time_gen;
  
  _masterHdr.time_begin = inMhdr.time_begin;

  _masterHdr.time_end = inMhdr.time_end;

  _masterHdr.time_centroid = inMhdr.time_centroid;

  _masterHdr.data_dimension = inMhdr.data_dimension;
  
  int fieldNum = (int)_params.doMax + (int)_params.doMin + (int)_params.doAvg
    + (int)_params.doStd + (int)_params.doPtile;
 
  _masterHdr.n_fields = fieldNum;
  
  _masterHdr.time_written = time(0);

  STRcopy(_masterHdr.data_set_info,"MdvCircularFilt" , MDV_INFO_LEN);

  STRcopy(_masterHdr.data_set_name, "MdvCircularFilt", MDV_NAME_LEN);

  STRcopy(_masterHdr.data_set_source, "MdvCircularFilt", MDV_NAME_LEN);

  return 0;
}

//
// setFieldHeader(): Fill in the field header information
//
int MdvCircularFilt::_setFieldHeader(const DsMdvx::field_header_t &inFhdr)
{
  if (_params.debug)
    cerr << "MdvCircularFilt::_setFieldHeader(): Setting field header." << endl;

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
  
  _fieldHdr.grid_minz = 0;
  
  _fieldHdr.bad_data_value = _bad;
  
  _fieldHdr.missing_data_value = _missing;
  
  _fieldHdr.proj_rotation = inFhdr.proj_rotation;
  
  STRcopy(_fieldHdr.units, inFhdr.units, MDV_SHORT_FIELD_LEN);
  STRcopy(_fieldHdr.field_name_long, inFhdr.field_name_long, MDV_LONG_FIELD_LEN);
  STRcopy(_fieldHdr.field_name, inFhdr.field_name, MDV_SHORT_FIELD_LEN);

  
  return 0;
}
