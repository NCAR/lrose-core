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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
 *
 * RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/04 02:22:14 $
 *   $Revision: 1.2 $
 *
 *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/******************************************************************************

******************************************************************************/

#include <sys/stat.h>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <dsserver/DsLdataInfo.hh>

#include "asdi2grid.hh"

const char *PROGRAM_NAME  = "asdi2grid";

/*
-----------------------------------------------------------------------------
Constructor - asdi2grid
-----------------------------------------------------------------------------
*/

asdi2grid::asdi2grid(Params *P, const char *programName)
{
  params = P;
  progName = programName;

  //
  // Register with procmap now that we have the instance name
  //
  PMU_auto_init(programName, params->instance, PROCMAP_REGISTER_INTERVAL);
  
  PMU_auto_register( "Initializing" );

  mdvOutput = new MdvOutput( params );
}

/*
-------------------------------------------------------------------------------
Destructor - ~asdi2grid
-------------------------------------------------------------------------------
*/

asdi2grid::~asdi2grid()
{

  if (mdvOutput != (MdvOutput *)NULL)
    delete mdvOutput;
}

/******************************************************************************
 run

 Description:
    Main algorithm starting point. Manages running of the algorithm.

 Inputs: 

    inputFileList : vector<string> :  List of filenames to process.

    startTime : time_t : Start time of data to process.

    startTime : time_t : End time of data to process.

 Outputs: None

 Returns: 
    <status> : int : 0 = SUCCESS, -1 = FAILURE

 Notes: 
    Program only handles input Netcdf files with extension ".nc"
    Runs in one of three modes: REALTIME, ARCHIVE_FILE_LIST,
      ARCHIVE_START_END_TIMES.  In REALTIME mode none of the arguments passed
      in are used. Program watches params->input_dir for incoming files.
      In ARCHIVE_FILE_LIST program only process files in the inputFileList.
      In ARCHIVE_START_END_TIMES program uses params->input_dir and the 
      startTime and endTime to create a list of files to process.

******************************************************************************/
int asdi2grid::run(vector<string> inputFileList, time_t startTime, time_t endTime)
{
  char *input_path = NULL;

  //
  // Create DsInputPath object using the constructor
  // appropriate for the current operating mode
  //
  
  DsInputPath *dsInputPath;
  switch( params->run_mode )
    {
      
    case Params::REALTIME:
      
      dsInputPath = new DsInputPath(PROGRAM_NAME,
				     (params->debug_level >= 1),
				     params->input_dir,
				     params->max_input_valid_age_sec,
				     PMU_auto_register,
				     params->use_ldata_info_file );
      
      cout << "Starting up in Realtime Mode" << endl;
      
      break;

    case Params::ARCHIVE_FILE_LIST:
      
      dsInputPath = new DsInputPath(PROGRAM_NAME,
				     (params->debug_level >= 1),
				     inputFileList );
      
      cout << "Starting up in File List Mode" << endl;
      cout << "Number of Input files: " << inputFileList.size() << endl;

      break;
      
    case Params::ARCHIVE_START_END_TIMES:
      
      dsInputPath = new DsInputPath(PROGRAM_NAME,
				     (params->debug_level >= 1),
				     params->input_dir,
				     startTime,
				     endTime );
      
      cout << "Starting up in Archive Mode" << endl;
      
      break;
      
    } // end switch params->run_mode

  dsInputPath->setSearchExt("nc");

  _initProjection();

  counts = new float[params->grid_info.nx * params->grid_info.ny * params->grid_info.nz];
  memset(counts, 0, params->grid_info.nx * params->grid_info.ny * params->grid_info.nz);
								       
  //
  // Loop over input files processing each
  //
  time_t start_time = 0;
  time_t end_time = 0;
  int mins_per_file = 0;
  while( (input_path = dsInputPath->next()) )
  {

    if(params->debug_level > 0) {
      cout << "Processing file " << input_path << endl;
    }

    PMU_auto_register( "Reading input file" );
    int status = ncInput.readFile(input_path);

    if(status != 0) {
      cout << "Unable to read file " << input_path << endl;
    } else {

      InputData_t *inputData = ncInput.getData();

      if(end_time +1 != inputData->startTime) {
	cout << "New file or files out of order" << endl;
	memset(lastAlt, 0, sizeof(lastAlt));
      }

      end_time = inputData->endTime;
      if(start_time == 0) {
	start_time = inputData->startTime;
	mins_per_file = (end_time - start_time +1) / 60;
      }

      if((end_time - start_time +1) / 60.0 >= params->grid_collection_mins + mins_per_file) {
	status = _writeOutputMdv(start_time, end_time);
	start_time = inputData->startTime;
	if(status != 0)
	  return status;
      }

      PMU_auto_register( "Creating asdi grid" );
      status = _processData( inputData );
      if(status != 0)
	return status;

      if((end_time - start_time +1) / 60.0 >= params->grid_collection_mins) {
	status = _writeOutputMdv(start_time, end_time);
	start_time = 0;
	if(status != 0)
	  return status;
      }
    }

  }

  delete dsInputPath;

  return 0;
}

/******************************************************************************
 _initProjection

 Description:
    Initializes the grid projection from the parameter file.
 Outputs: None
 Returns: None

******************************************************************************/
void asdi2grid::_initProjection()
{

  if(params->projection_info.type == Params::PROJ_LAMBERT_CONF)
    proj.initLambertConf(params->projection_info.origin_lat,
			 params->projection_info.origin_lon,
			 params->projection_info.ref_lat_1,
			 params->projection_info.ref_lat_2);
  else if(params->projection_info.type == Params::PROJ_LATLON)
    proj.initLatlon();

  proj.setGrid(params->grid_info.nx, params->grid_info.ny,
	       params->grid_info.dx, params->grid_info.dy,
	       params->grid_info.minx, params->grid_info.miny);

}

/******************************************************************************
 _processData

 Description:
    Processes the input data from the netcdf file on a record by record basis.
 Inputs: InputData_t struct containing input records.
 Outputs: None
 Returns: None

******************************************************************************/
int asdi2grid::_processData(InputData_t *inputData)
{

  for(long int a = 0; a < inputData->records; a++)
  {
    if(inputData->altitude[a] != inputData->alt_missing &&
       inputData->latitude[a] != inputData->lat_missing &&
       inputData->longitude[a] != inputData->lon_missing) 
    {

      char *callsign = &(inputData->callsign[a*inputData->strLen]);
      char callnumStr[5];
      int callnum = 0;
      char id[2];
      if(callsign[1] >= '0' && callsign[1] <= '9') {
	strncpy(callnumStr, &(callsign[1]), 4);
	callnum = atoi(callnumStr);
      } else if(callsign[2] >= '0' && callsign[2] <= '9') {
	strncpy(callnumStr, &(callsign[2]), 4);
	callnum = atoi(callnumStr);
      } else if(callsign[3] >= '0' && callsign[3] <= '9') {
	strncpy(callnumStr, &(callsign[3]), 4);
	callnum = atoi(callnumStr);
      }

      id[0] = callsign[0];
      id[1] = callsign[1];
      if(callsign[strlen(callsign)-1] >= 'A' && callsign[strlen(callsign)-1] <= 'Z') {
	id[1] = callsign[strlen(callsign)-1];
	if(callsign[0] == 'N' && callsign[strlen(callsign)-2] >= 'A' && callsign[strlen(callsign)-2] <= 'Z') 
	  id[0] = callsign[strlen(callsign)-2];
      } else 
	if(callsign[2] >= 'B' && callsign[2] <= 'Z' && callsign[2] != callsign[0])
	  id[1] = callsign[2];
      
      int c1 = int(id[0]) - 48;
      int c2 = int(id[1]) - 48;
      if(c1 >= 0 && c1 < 42 && c2 >= 0 && c2 < 42)
      {
	if(lastAlt[callnum][c1][c2] > 0.0 &&
	   fabs(lastAlt[callnum][c1][c2] - inputData->altitude[a]) < 15000 &&
	   fabs(lastLat[callnum][c1][c2] - inputData->latitude[a]) < 1.0 &&
	   fabs(lastLon[callnum][c1][c2] - inputData->longitude[a]) < 1.0)
	{  

	  _drawline(lastAlt[callnum][c1][c2],
		    lastLat[callnum][c1][c2],
		    lastLon[callnum][c1][c2],
		    inputData->altitude[a], inputData->latitude[a],
		    inputData->longitude[a]);
	  
	}
	
	lastAlt[callnum][c1][c2] = inputData->altitude[a];
	lastLat[callnum][c1][c2] = inputData->latitude[a];
	lastLon[callnum][c1][c2] = inputData->longitude[a];

      }
    }
  }


  return 0;
}

/******************************************************************************
 _drawline

 Description:
    3D pixel line drawing algorithm.
 Inputs: 2 points to be connected by line, in lat/lon and alt.
 Outputs: None
 Returns: None

******************************************************************************/
void asdi2grid::_drawline(float alt, float lat, float lon, float alt2, float lat2, float lon2)
{

  double xx0, xx1, yy0, yy1;
  int ingrid = proj.latlon2xyIndex(lat, lon, xx0, yy0);
  if(ingrid == -1)
    return;
  ingrid = proj.latlon2xyIndex(lat2, lon2, xx1, yy1);
  if(ingrid == -1)
    return;

  int x0, x1, y0, y1, z0 , z1;
  x0 = (int)(xx0+.5);
  y0 = (int)(yy0+.5);
  z0 = floor( ( (float)( (alt / 100) - params->grid_info.minz) / params->grid_info.dz) + .5);
  x1 = (int)(xx1+.5);
  y1 = (int)(yy1+.5);
  z1 = floor( ( (float)( (alt2 / 100) - params->grid_info.minz) / params->grid_info.dz) + .5);

  if(x0 == x1 and y0 == y1) {
    return;
  }

  int steepy = 0;
  int steepz = 0;
  int flip = 0;
  // Make longest dimension the x dimension
  if(abs(y1 - y0) > abs(x1 - x0)) {
    steepy = 1;
    int t = y0;
    y0 = x0;
    x0 = t;
    t = y1;
    y1 = x1;
    x1 = t;
  }
  if(abs(z1 - z0) > abs(x1 - x0)) {
    steepz = 1;
    int t = z0;
    z0 = x0;
    x0 = t;
    t = z1;
    z1 = x1;
    x1 = t;
  }

  int deltax = abs(x1 - x0);
  int deltay = abs(y1 - y0);
  int deltaz = abs(z1 - z0);
  float yerror = 0.0;
  float zerror = 0.0;
  float dyerr = float(deltay) / float(deltax);
  float dzerr = float(deltaz) / float(deltax);
  int y = y0;
  int z = z0;
  int xstep = -1;
  int ystep = -1;
  int zstep = -1;
  if(x0 < x1)
    xstep = 1;
  if(y0 < y1)
    ystep = 1;
  if(z0 < z1)
    zstep = 1;
  int x = x0;
  // end point does not get counted till next time
  while(x != x1)
  {
    int zz, xx, yy;
    if(steepz == 1) {
      zz = x;
      if(steepy == 1) {
	yy = z;
	xx = y;
      } else {
	yy = y;
	xx = z;
      }
    } else {
      zz = z;
      if(steepy == 1) {
	yy = x;
	xx = y;
      } else {
	yy = y;
	xx = x;
      }
    }
    
    if(zz >= 0 && zz < params->grid_info.nz) {
      counts[(zz * params->grid_info.nx * params->grid_info.ny) + (yy * params->grid_info.nx) + xx]++;
    }
    
    yerror += dyerr;
    if(yerror >= 0.5) {
      y += ystep;
      yerror -= 1.0;
    }
    zerror += dzerr;
    if(zerror >= 0.5) {
      z += zstep;
      zerror -= 1.0;
    }
    x += xstep;
  }

}

/******************************************************************************
 _writeOutputMdv

 Description:
    Write out the counts grid to a mdv file.
 Inputs: Data start and end times.
 Outputs: None
 Returns: 0 on Success

******************************************************************************/
int asdi2grid::_writeOutputMdv(time_t fileStartT, time_t fileEndT)
{
  Mdvx::field_header_t fieldHeader;
  Mdvx::vlevel_header_t vlevelHeader;
  memset( (void *) &fieldHeader, (int) 0, sizeof(Mdvx::field_header_t) );
  memset( (void *) &vlevelHeader, (int) 0, sizeof(Mdvx::vlevel_header_t) );
  vlevelHeader.struct_id = Mdvx::VLEVEL_HEAD_MAGIC_COOKIE;

  // fill out the field header
  fieldHeader.record_len1         = sizeof( Mdvx::field_header_t );
  fieldHeader.struct_id           = Mdvx::FIELD_HEAD_MAGIC_COOKIE;
  fieldHeader.field_code          = 0;
  fieldHeader.forecast_delta      = 0;
  fieldHeader.forecast_time       = fileEndT+1;
  fieldHeader.data_element_nbytes = sizeof(float);
  fieldHeader.encoding_type       = Mdvx::ENCODING_FLOAT32;    
  fieldHeader.field_data_offset   = 0;
  fieldHeader.compression_type    = Mdvx::COMPRESSION_ASIS;
  fieldHeader.transform_type      = Mdvx::DATA_TRANSFORM_NONE;
  fieldHeader.scaling_type        = Mdvx::SCALING_NONE;
  fieldHeader.native_vlevel_type  = Mdvx::VERT_FLIGHT_LEVEL;
  fieldHeader.vlevel_type         = Mdvx::VERT_FLIGHT_LEVEL;
  fieldHeader.dz_constant         = 1;

  fieldHeader.scale               = 1.0;
  fieldHeader.bias                = 0.0;
  fieldHeader.missing_data_value  = -999.00;
  fieldHeader.bad_data_value      = -998.00;

  fieldHeader.proj_type = params->projection_info.type;
  fieldHeader.ny        = params->grid_info.ny;
  fieldHeader.nx        = params->grid_info.nx;
  fieldHeader.nz        = params->grid_info.nz;
  fieldHeader.grid_miny = params->grid_info.miny;
  fieldHeader.grid_minx = params->grid_info.minx;
  fieldHeader.grid_minz = params->grid_info.minz;
  fieldHeader.grid_dy   = params->grid_info.dy;
  fieldHeader.grid_dx   = params->grid_info.dx;
  fieldHeader.grid_dz   = params->grid_info.dz;
  fieldHeader.proj_origin_lat     = params->projection_info.origin_lat;
  fieldHeader.proj_origin_lon     = params->projection_info.origin_lon;
  fieldHeader.proj_rotation       = params->projection_info.rotation;
  fieldHeader.proj_param[0]       = params->projection_info.ref_lat_1;
  fieldHeader.proj_param[1]       = params->projection_info.ref_lat_2;
  fieldHeader.volume_size = fieldHeader.nx * fieldHeader.ny *
    fieldHeader.nz * fieldHeader.data_element_nbytes;
  
  // fill out the vlevel header
  for(int z = 0; z < params->grid_info.nz; z++) {
    vlevelHeader.type[z] = Mdvx::VERT_FLIGHT_LEVEL;
    vlevelHeader.level[z] = params->grid_info.minz + (z * params->grid_info.dz);
  }

  MdvxField *countField = new MdvxField(fieldHeader, vlevelHeader, counts );
  countField->setFieldName ("flight_count");
  countField->setFieldNameLong ("ASDI flight count");
  countField->setUnits ("count");
  
  mdvOutput->setVerticalType(Mdvx::VERT_FLIGHT_LEVEL);
  mdvOutput->addField( countField );

  memset(counts, 0, sizeof(float) * params->grid_info.nx * params->grid_info.ny * params->grid_info.nz);

  if ( mdvOutput->writeVol( fileEndT+1 ) != 0 )
    return 1;

  return 0;
}
