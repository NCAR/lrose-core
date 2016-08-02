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
///////////////////////////////////////////////////////////////
// BasinPrecip.cc
//
// BasinPrecip object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
// Modified by Alex Baia, August 2000
//
///////////////////////////////////////////////////////////////


#include "BasinPrecip.hh"
using namespace std;


/*********************************************************************
 * Constructors
 */

BasinPrecip::BasinPrecip(int argc, char **argv)
{
  const string routine_name = "Constructor";
  
  OK = true;

  // set programe name

  _progName = STRdup("BasinPrecip");
  ucopyright(_progName);

  // get command line args

  _args = new Args(argc, argv, _progName);
  if (!_args->OK)
  {
    cerr << "ERROR: " << _className() << "::" << routine_name << endl;
    cerr << "Problem with command line args" << endl;
    
    OK = false;
    return;
  }

  // get TDRP params

  _params = new Params();
  _paramsPath = "unknown";
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &_paramsPath))
  {
    cerr << "ERROR: " << _className() << "::" << routine_name << endl;
    cerr << "Problem with TDRP parameters" << endl;
    
    OK = false;
    return;
  }

  // check start and end in ARCHIVE mode

  if ((_params->mode == Params::ARCHIVE) &&
      (_args->startTime == -1 || _args->endTime == -1))
  {
    cerr << "ERROR: " << _className() << "::" << routine_name << endl;
    cerr << "In ARCHIVE mode start and end times must be specified." << endl;
    cerr << "Run '" << _progName << " -h' for usage" << endl;
    
    OK = false;
    return;
  }

  // Initialize the object used for finding input files

  switch (_params->mode)
  {
  case Params::REALTIME :
    if (_fileRetriever.setRealtime(_params->input_mdv_url, 600,
				   PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << _className() << "::" << routine_name << endl;
      cerr << "Error creating realtime DsMdvxInput object" << endl;
      
      OK = false;
      return;
    }
    break;
    
  case Params::ARCHIVE :
    if (_fileRetriever.setArchive(_params->input_mdv_url,
				  _args->startTime,
				  _args->endTime) != 0)
    {
      cerr << "ERROR: " << _className() << "::" << routine_name << endl;
      cerr << "Error creating archive DsMdvxInput object" << endl;
      
      OK = false;
      return;
    }
    break;

  } /* endswitch - _params->mode */

  // init process mapper registration

  PMU_auto_init(_progName, _params->instance, PROCMAP_REGISTER_INTERVAL);

  return;

}


/*********************************************************************
 * Destructor
 */

BasinPrecip::~BasinPrecip()
{
  // unregister process

  PMU_auto_unregister();

  // free up

  delete(_params);
  delete(_args);
  STRfree(_progName);
}


/*********************************************************************
 * Run() - Run the BasinPrecip program.
 *
 * Returns the return code for the program (0 if successful, error code
 * otherwise).
 */

int BasinPrecip::Run ()
{

  // Create the BasinList object

  BasinList basin_list(_params->basin_debug);


  // Read in the basin information

  for (int i = 0; i < _params->shape_file_bases_n; ++i)
  {
    if (!basin_list.addFromShapeFile(_params->_shape_file_bases[i]))
    {
      cerr << "ERROR: " << _className() << endl;
      cerr << "Error reading basin information from shape files: " <<
	_params->_shape_file_bases[i] << endl;
      cerr << "--- Skipping file ---" << endl;
      
      continue;
    }
  }


  // Print all of the basin information

  cerr << endl << endl;
  
  basin_list.print(cerr);

  cerr << endl << endl;

  // Create a statistics data stream

  ofstream sds;
  
  // Open the statistics output file for writing
  
  sds.open(out_file);


  char pmu_message[BUFSIZ];
  
  Basin *basin;
//Basin *basin = new Basin(false);


  int basin_num;
  
   
  // Process each mdv file

  while (true)
    {
      DsMdvx input_file;

      if (!_readNextFile(input_file))
	{
	  if (_params->mode == Params::REALTIME)
	    continue;
	  else
	    {
	      if (_params->debug >= Params::DEBUG_WARNINGS)
		cerr << "No more files to process -- exiting" << endl;

	      break;
	    }  
	}
      
      sprintf(pmu_message, "Processing data for time %s",
	      utimstr(input_file.getMasterHeader().time_centroid));
      PMU_force_register(pmu_message);
    

      // Now loop through the basins in the list and print them individually

      for (basin = basin_list.getFirstBasin(), basin_num = 0;
	   basin != 0;
	   basin = basin_list.getNextBasin(), ++basin_num)
	{
	 
	  if (_params->basin_debug)
	    {
	      cerr << "Basin #" << basin_num << ":" << endl;
	      basin->print(cerr, false);
	      cerr << endl << endl;
	    }

	  // Create a mask for the basin

	  Mdvx::field_header_t field_hdr =
	    input_file.getFieldByNum(0)->getFieldHeader();
	  MdvxProj projection(field_hdr);
  
	  int min_x, max_x;
	  int min_y, max_y;

	  unsigned char *mask = basin->createMask(projection,
						  min_x, max_x,
						  min_y, max_y);
  
	  // Run statistics methods

	  MdvxField *field = input_file.getFieldByNum(0);
	  fl32 *data = (fl32 *)field->getVol();
	  
//	  PrintStats(data,
//		     field_hdr.bad_data_value,
//		     field_hdr.missing_data_value,
//		     mask, sds, min_x, max_x, 
//		     min_y, max_y, projection.getCoord().nx, 
//		     input_file, basin_num);


	  // Finally, write out an SPDB database for displaying with
	  // the given MDV data. 

	  //mdv_file.setReadPath(_mdvDataFile.c_str());
	  input_file.readAllHeaders();
  
	  Mdvx::master_header_t master_hdr = input_file.getMasterHeaderFile();
  
	  time_t valid_time = master_hdr.time_centroid;
	  time_t expire_time = master_hdr.time_expire;
  
	  DsSpdb spdb;
  
	  spdb.setPutMode(Spdb::putModeAdd);

	  GenPt point;
	  WorldPoint2D outflow = basin->getOutflowLocation();
    
	  point.setId(basin->getId());
	  point.setTime(valid_time);
	  point.setLat(outflow.lat);
	  point.setLon(outflow.lon);
	  point.setNLevels(1);
    
	  point.addFieldInfo("avg", "mm");
	  point.addVal(BasinAverage(data,
				    field_hdr.bad_data_value,
				    field_hdr.missing_data_value,
				    mask, min_x, max_x, 
				    min_y, max_y, projection.getCoord().nx));
    
	  point.addFieldInfo("std", "mm");
	  point.addVal(BasinSDeviation(data,
				       field_hdr.bad_data_value,
				       field_hdr.missing_data_value,
				       mask, min_x, max_x, 
				       min_y, max_y, projection.getCoord().nx));
    
	  if (!point.check())
	    {
	      cerr << "Error in point:" << endl;
	      cerr << point.getErrStr() << endl;
	      cerr << "*** Skipping point ***" << endl;
      
	      continue;
	    }
	  
	  point.assemble();
    
	  // Add the point to the SPDB buffer

	  spdb.addPutChunk(basin->getId(),
			   valid_time,
			   expire_time,
			   point.getBufLen(),
			   point.getBufPtr());
    

	  // Write the SPDB data to the database

	  if (spdb.put(_params->output_spdb_url,
		   SPDB_GENERIC_POINT_ID,
		   SPDB_GENERIC_POINT_LABEL) != 0)
	    {
	      cerr << "ERROR: " << _className() << endl;
	      cerr << "Error writing data to SPDB database:" << endl;
	      cerr << spdb.getErrStr() << endl;
	  
	      return -1;
	    }
     

            // free the mask
            ufree(mask);

 

	} /* endfor - basin */ 


    } /* endwhile - true */


  // Close the statistics output file
  
  sds.close();

  return (0);
}

      
/**************************************************************
 * PRIVATE MEMBER FUNCTIONS
 **************************************************************/

/**********************************************************************
 * _readNextFile() - Read the next file to be processed.  In realtime
 *                   mode, blocks until a new file is available.
 *
 * Returns true if successful, false otherwise.
 */

bool BasinPrecip::_readNextFile(DsMdvx &mdv_file)
{
  const string routine_name = "_readNextFile()";
  
  // Set up the read request

  mdv_file.clearRead();
  mdv_file.clearReadFields();
  mdv_file.addReadField(_params->precip_field);
  mdv_file.clearReadVertLimits();
  mdv_file.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  mdv_file.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  mdv_file.setReadScalingType(Mdvx::SCALING_NONE);

  if (_params->debug >= Params::DEBUG_VERBOSE)
    mdv_file.printReadRequest(cerr);

  // Read the next file -- blocks in realtime.

  PMU_force_register("Reading volume...");

  if (_fileRetriever.readVolumeNext(mdv_file) != 0)
  {
    cerr << "ERROR: " << _className() << "::" << routine_name << endl;
    cerr << "Error reading in new input file: " <<
      _fileRetriever.getErrStr() << endl;
    
    return false;
  }
  
  if (_params->debug >= Params::DEBUG_NORM)
    cerr << "Read in input file: " << mdv_file.getPathInUse() << endl;
  
  return true;
}


// Basin Average Method:

double BasinPrecip::BasinAverage(fl32 *data, 
				 const fl32 bad_data_value,
				 const fl32 missing_data_value,
				 unsigned char *mask, 
  		                 int min_x, int max_x,
			         int min_y, int max_y, int nx)
{

  double sum = 0;              // sum of values within basin  
  int n_points = 0;            // number of points used to compute sum 
  double average = 0;          // average of values

  for (int x = min_x; x <= max_x; x++)
    {
      for (int y = min_y; y <= max_y; y++)
	{
	  
	  int index = x + (y * nx);  
		  
	  if (mask[index] != 0)
	    {
	      // If the data point is bad or missing, treat it as
	      // a data value of 0.0 (no precipitation) so don't
	      // increase the sum, but do increase the number of
	      // points.

	      if (data[index] != bad_data_value &&
		  data[index] != missing_data_value)
		sum += data[index];

	      n_points++;
	    }

	}
    }


  if (n_points == 0)  // prevents bunk average and
    return -9999;     // division by zero

  average = sum / n_points;

  return average;

}


// Basin Standard Deviation Method:

double BasinPrecip::BasinSDeviation(fl32 *data, 
				    const fl32 bad_data_value,
				    const fl32 missing_data_value,
				    unsigned char *mask, 
       			            int min_x, int max_x, 
			            int min_y, int max_y, int nx)
{

  int n_points = 0;                         // # of data points
  double x_bar = BasinAverage(data,         // basin average
			      bad_data_value,
			      missing_data_value,
			      mask,
	      		      min_x, max_x,
			      min_y, max_y, nx); 
  double sum_diff = 0;                      // summation of squared differences
  double variance = 0;                      // sample variance
  double std_dev = 0;                       // s. dev of basin set


 for (int x = min_x; x <= max_x; x++)
    {
      for (int y = min_y; y <= max_y; y++)
	{

	int index = x + (y * nx); 
	
	if (mask[index] != 0)
	   {
	     // If the data point is bad or missing, treat it as
	     // a data value of 0.0 (no precipitation) so don't
	     // increase the sum, but do increase the number of
	     // points.

	     if (data[index] != bad_data_value &&
		 data[index] != missing_data_value)
	       sum_diff += pow (data[index] - x_bar, 2);                          
	     n_points++;
	   }                                                 
	
	}
    }


  if (n_points == 0 || n_points == 1)  // prevents bunk s_dev and
    return -9999;                      // division by zero                 

  variance = sum_diff / (n_points - 1);

  std_dev = sqrt (variance);
 
  return std_dev;

}


// Print Statistics Method  

void BasinPrecip::PrintStats(fl32 *data,
			     const fl32 bad_data_value,
			     const fl32 missing_data_value,
			     unsigned char *mask, ofstream &sds,
       			     int min_x, int max_x, 
			     int min_y, int max_y, int nx, 
			     Mdvx input_file, int basin_num)

{

  double avg = BasinAverage(data,
			    bad_data_value, missing_data_value,
			    mask, min_x, max_x, 
			    min_y, max_y, nx);

  double sdev = BasinSDeviation(data,
				bad_data_value, missing_data_value,
				mask, min_x, max_x, 
			        min_y, max_y, nx);

  sds << endl;
  sds << "Mdv Path: " << input_file.getPathInUse() << endl;
  sds << "Statistics for Basin#:  "  << basin_num << endl;
  sds << "----------------------------" << endl;
  sds << "Average (mean):  " << avg << endl;
  sds << "Standard Deviation:  " << sdev << endl << endl;

}
