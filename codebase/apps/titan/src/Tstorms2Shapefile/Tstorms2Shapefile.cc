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
////////////////////////////////////////////////////////////////////////
// Tstorms2Shapefile.cc
// Taken from pieces of Mike Dixon's Tstorms2Shapefile and Rview
//
// Terri L. Betancourt, RAP, NCAR
// June 2003
//
///////////////////////////////////////////////////////////////
//
// Tstorms2Shapefile reads native TITAN data files, 
// converts the data into ESRI shapefile format
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <toolsa/ucopyright.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/DateTime.hh>
#include <didss/RapDataDir.hh>
#include <Mdv/MdvxProj.hh>

#include "Tstorms2Shapefile.hh"
using namespace std;


// Constructor

Tstorms2Shapefile::Tstorms2Shapefile(int argc, char **argv)

{
  isOK = true;
  _input = NULL;

  // set programe name

  _progName = "Tstorms2Shapefile";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
  }

  // check args in ARCHIVE mode
  
  if (_params.mode == Params::ARCHIVE) {
    if (_args.inputFileList.size() == 0) {
      if ((_args.startTime == 0 || _args.endTime == 0)) {
	cerr << "ERROR: " << _progName << endl;
	cerr << "In ARCHIVE mode, you must specify a file list" << endl
	     << "  or start and end times." << endl;
	isOK = FALSE;
	return;
      }
    }
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  // set up input object

  if (_params.mode == Params::ARCHIVE) {
    if (_args.inputFileList.size() > 0) {
      _input = new DsInputPath(_progName,
			       _params.debug >= Params::DEBUG_VERBOSE,
			       _args.inputFileList);
      _input->setSearchExt("th5");
    } else if (_args.startTime != 0 && _args.endTime != 0) {
      string inDir;
      RapDataDir.fillPath(_params.input_dir, inDir);
      if (_params.debug) {
	cerr << "Input dir: " << inDir << endl;
      }
      _input = new DsInputPath(_progName,
			       _params.debug >= Params::DEBUG_VERBOSE,
			       inDir,
			       _args.startTime,
			       _args.endTime);
      _input->setSearchExt("th5");
    }
  } else {
    string inDir;
    RapDataDir.fillPath(_params.input_dir, inDir);
    if (_params.debug) {
      cerr << "Input dir: " << inDir << endl;
    }
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     inDir,
			     _params.max_realtime_valid_age,
			     PMU_auto_register);
  }

  return;

}

// destructor

Tstorms2Shapefile::~Tstorms2Shapefile()

{

  if (_input) {
    delete _input;
  }


  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Tstorms2Shapefile::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  if (_params.mode == Params::ARCHIVE) {
    _input->reset();
  }

  char *inputFilePath;
  while ((inputFilePath = _input->next()) != NULL) {
  
    if (_params.debug) {
      cerr << "Processing input file: " << inputFilePath << endl;
    }

    _processTrackFile(inputFilePath);
    
  }

  return 0;

}

//////////////////////////////////////////////////
// process track file

int Tstorms2Shapefile::_processTrackFile (const char *input_file_path)

{

  TitanTrackFile tFile;
  TitanStormFile sFile;

  // open files

  if (_openFiles(input_file_path, tFile, sFile)) {
    return -1;
  }

  // load up scan times

  vector<time_t> scanTimes;

  if (_loadScanTimes(sFile, scanTimes)) {
    return -1;
  }

  if (_params.mode == Params::REALTIME) {

    // REALTIME mode - find the scan which matches the latest data info
    // and only process that scan

    time_t valid_time = _input->getLdataInfo().getLatestTime();

    for (size_t iscan = 0; iscan < scanTimes.size(); iscan++) {
      if (scanTimes[iscan] == valid_time) {
	time_t expire_time;
	if (iscan == 0) {
	  expire_time = valid_time;
	} else {
	  expire_time = valid_time + (valid_time - scanTimes[iscan - 1]);
	}
	_processScan(sFile, tFile, iscan, valid_time, expire_time);
	break;
      }
    }

  } else {

    // ARCHIVE mode - process all scans
    
    for (size_t iscan = 0; iscan < scanTimes.size(); iscan++) {
      time_t valid_time = scanTimes[iscan];
      time_t expire_time;
      if (scanTimes.size() == 1) {
	expire_time = valid_time;
      } else {
	if (iscan == scanTimes.size() - 1) {
	  expire_time = valid_time + (valid_time - scanTimes[iscan - 1]);
	} else {
	  expire_time = scanTimes[iscan + 1];
	}
      }
      _processScan(sFile, tFile, iscan, valid_time, expire_time);
    }

  }


  return 0;

}

//////////////////////////////////////////////////
// open track and storm files

int Tstorms2Shapefile::_openFiles(const char *input_file_path,
			     TitanTrackFile &tFile,
			     TitanStormFile &sFile)

{

  char track_file_path[MAX_PATH_LEN];
  STRncopy(track_file_path, input_file_path, MAX_PATH_LEN);
  if (_params.mode == Params::REALTIME) {
    // In Realtime mode the latest data info file has
    // the storm file in it instead of the track file so
    // we need to change the 'sh' to a 'th'.
    char *sh = strstr(track_file_path, "sh");
    if (sh) {
      *sh = 't';
    }
  }

  if (tFile.OpenFiles("r", track_file_path)) {
    cerr << "ERROR - Tstorms2Shapefile::_openFiles" << endl;
    cerr << "  " << tFile.getErrStr() << endl;
    return -1;
  }

  Path stormPath(track_file_path);
  stormPath.setFile(tFile.header().storm_header_file_name);

  if (sFile.OpenFiles("r", stormPath.getPath().c_str())) {
    cerr << "ERROR - Tstorms2Shapefile::_openFiles" << endl;
    cerr << "  " << sFile.getErrStr() << endl;
    return -1;
  }
  
  // lock files

  if (tFile.LockHeaderFile("r")) {
    cerr << "ERROR - Tstorms2Shapefile::_openFiles" << endl;
    cerr << "  " << tFile.getErrStr() << endl;
    return -1;
  }
  if (sFile.LockHeaderFile("r")) {
    cerr << "ERROR - Tstorms2Shapefile::_openFiles" << endl;
    cerr << "  " << sFile.getErrStr() << endl;
    return -1;
  }
  return 0;

}

//////////////////////////////////////////////////
// load up scan times from storm file

int Tstorms2Shapefile::_loadScanTimes(TitanStormFile &sFile,
				 vector<time_t> &scanTimes)

{

  int nScans = sFile.header().n_scans;
  for (int i = 0; i < nScans; i++) {
    // read in scan
    if (sFile.ReadScan(i)) {
      cerr << "ERROR - Tstorms2Shapefile::_loadScanTimes" << endl;
      cerr << "  " << sFile.getErrStr() << endl;
      return -1;
    }
    scanTimes.push_back(sFile.scan().time);
  }

  return 0;

}

int Tstorms2Shapefile::_processScan(TitanStormFile &sFile,
			       TitanTrackFile &tFile,
			       int scan_num,
			       time_t valid_time,
			       time_t expire_time)

{

  const track_file_forecast_props_t *fprops;
  const storm_file_global_props_t *gprops;

  if (_params.debug) {
    cerr << "Processing scan num: " << scan_num << endl;
    cerr << "  Valid time: " << DateTime::str(valid_time) << endl;
    cerr << "  Expire time: " << DateTime::str(expire_time) << endl;
  }

  // read in track file scan entries

  if (tFile.ReadScanEntries(scan_num)) {
    cerr << "ERROR - Tstorms2Shapefile::_processScan" << endl;
    cerr << "  " << sFile.getErrStr() << endl;
    return -1;
  }

  // read in storm file scan

  if (sFile.ReadScan(scan_num)) {
    cerr << "ERROR - Tstorms2Shapefile::_processScan" << endl;
    cerr << "  " << sFile.getErrStr() << endl;
    return -1;
  }

  // load up scan header

  const storm_file_scan_header_t &scan = sFile.scan();
  int n_entries = tFile.scan_index()[scan_num].n_entries;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  Nentries in scan: " << n_entries << endl;
  }

   //
   // There's nothing to do here if there are no storm entries in the volume
   //
   if ( n_entries == 0 ) {
      POSTMSG( DEBUG, "No entries -- skipping volume" );
      return( 0 );
   }

   //
   // Process each forecast lead time
   //
   time_t leadTimeMin;
   double leadTimeHr;

   for( int i=0; i < _params.forecast_lead_time_n; i++ ) {

      //
      // open shapefile based on lead time (in min)
      //
      leadTimeMin = _params._forecast_lead_time[i];
      if ( openShapefile( valid_time, leadTimeMin ) != 0 ) {
         return( -1 );
      }

      leadTimeHr = ((double)leadTimeMin) / 60.0;

      // get the beginning of the entries for this scan
      const track_file_entry_t *scan_entries = tFile.scan_entries();

      for (int ientry = 0; ientry < n_entries; ientry++, scan_entries++) {

         fprops = &scan_entries->dval_dt;
         gprops = sFile.gprops() + scan_entries->storm_num;
         storm2shape( sFile.params(), *scan_entries, *gprops, *fprops, 
                      scan.grid, leadTimeHr );
      }

      DBFClose( outputDbf );
      SHPClose( outputShp );
      if ( leadTimeHr > 0 ) {
         DBFClose( directionDbf );
         SHPClose( directionShp );
      }
  }

  return 0;

}

int
Tstorms2Shapefile::openShapefile( time_t validTime, time_t leadTimeMin )
{
   //
   // Construct the shapefile structure using basename and validTime
   //
   string fileName = _params.shapefile_basename;
   DateTime when( validTime );

   if ( _params.datetime_structure == Params::DATETIME_IN_FILENAME ) {
      outputPath.setDirectory( _params.output_dir );
      fileName += when.getStrPlain();
   }
   else {
      outputPath.setDirectory( _params.output_dir + 
                               when.getDateStrPlain() +
                               "/" +
                               when.getTimeStrPlain() );
   }

   //
   // Append forecast minutes, if necessary
   //
   if ( leadTimeMin > 0 ) {
      fileName += "_F";
      char leadStr[32];
      sprintf( leadStr, "%d", (int)leadTimeMin );
      fileName += leadStr;
   }

   //
   // Set the filename and make sure the directory exists
   //
   outputPath.setFile( fileName );
   if ( outputPath.makeDirRecurse() != 0 ) {
      POSTMSG( ERROR, "Unable to make directory %s",
               outputPath.getPath().c_str() );
   }
   char* shapepath = (char*)outputPath.getPath().c_str();

   //
   // Open the output shapefile (create mode)
   //
   POSTMSG( DEBUG, "Creating shapefile %s", shapepath );
   outputShp = SHPCreate( shapepath, SHPT_POLYGON );
   outputDbf = DBFCreate( shapepath );

   if ( outputShp == NULL  ||  outputDbf == NULL ) {
      POSTMSG( ERROR, "Unable to open storm shapefile" );
      return( -1 );
   }

   //
   // Create the storm dbf field (area)
   //
   areaField = DBFAddField( outputDbf, "area", FTDouble, 7, 2 );
   massField = DBFAddField( outputDbf, "mass", FTDouble, 7, 2 );
   hailProbField = DBFAddField( outputDbf, "hailProb", FTDouble, 7, 1 );
   hailMassField = DBFAddField( outputDbf, "hailMass", FTDouble, 7, 2 );

   if ( areaField == -1 || massField == -1 || 
        hailProbField == -1 || hailMassField == -1 ) {
      POSTMSG( ERROR, "Unable to create output DBF fields" );
      return( -1 );
   }

   if ( leadTimeMin > 0 ) {

      string directionFileName = fileName;
      directionFileName += "_DIR";

      //
      // Open the direction shapefile
      //
      outputPath.setFile( directionFileName );
      shapepath = (char*)outputPath.getPath().c_str();

      POSTMSG( DEBUG, "Creating shapefile %s", shapepath );
      directionShp = SHPCreate( shapepath, SHPT_ARC );
      directionDbf = DBFCreate( shapepath );

      if ( directionShp == NULL ||  directionDbf == NULL ) {
         POSTMSG( ERROR, "Unable to open direction shapefile" );
         return( -1 );
      }

      speedField = DBFAddField( directionDbf, "speed", FTDouble, 7, 2 );
      directionField = DBFAddField( directionDbf, "direction", FTDouble, 7, 2 );

      if ( speedField == -1 || directionField == - 1 ) {
         POSTMSG( ERROR, "Unable to create direction DBF fields" );
         return( -1 );
      }
   }

   return( 0 );
}

/////////////////////////////////////////////////////////////////////////
// convert storm geometry to shape geometry

int
Tstorms2Shapefile::storm2shape( const storm_file_params_t &sparams,
			        const track_file_entry_t &entry,
			        const storm_file_global_props_t &gprops,
			        const track_file_forecast_props_t &fprops,
			        const titan_grid_t &grid,
                                double leadTimeHr )
     
{
   bool       dbfFailure;
   int        recordId;
   double     dirVectorX[2], dirVectorY[2];
   SHPObject *shpObject, *dirObject;

   // initialize projection

   titan_grid_comps_t grid_comps;
   TITAN_init_proj(&grid, &grid_comps);


   // output polygon based of forecast leadTime
  
   double plot_x, plot_y, plot_storm_scale;
   double poly_delta_az, theta, range, dx, dy;

   if ( leadTimeHr == 0.0 ) {
      plot_x = gprops.proj_area_centroid_x;
      plot_y = gprops.proj_area_centroid_y;
      plot_storm_scale = 1.0;
   }
   else {
      plot_x  = gprops.proj_area_centroid_x +
                entry.dval_dt.proj_area_centroid_x * leadTimeHr;
      plot_y  = gprops.proj_area_centroid_y +
                entry.dval_dt.proj_area_centroid_y * leadTimeHr;

      double plot_proj_area = gprops.proj_area + 
                              entry.dval_dt.proj_area * leadTimeHr;

      double plot_volume = gprops.volume +
                           entry.dval_dt.volume * leadTimeHr;

      plot_storm_scale = sqrt(plot_proj_area / gprops.proj_area);

      //
      // Bail out if volume or area is too low
      //
      if ( !entry.forecast_valid || 
            plot_proj_area < 1.0 || plot_volume < 1.0 ) {
         return( 0 );
      }

      //
      // Plot forecast direction vector
      //
      if ( _params.output_projection == Params::FLAT ) {
         dirVectorX[0] = gprops.proj_area_centroid_x;
         dirVectorY[0] = gprops.proj_area_centroid_y;
         dirVectorX[1] = plot_x;
         dirVectorY[1] = plot_y;
         dirObject = SHPCreateSimpleObject( SHPT_ARC, 2,
                                            dirVectorX, dirVectorY, NULL );
      }
      else {
         TITAN_xy2latlon( &grid_comps,
                          gprops.proj_area_centroid_x,
                          gprops.proj_area_centroid_y,
                          &dirVectorX[0], 
                          &dirVectorY[0] );
         TITAN_xy2latlon( &grid_comps,
                          plot_x,
                          plot_y,
                          &dirVectorX[1], 
                          &dirVectorY[1] );
         dirObject = SHPCreateSimpleObject( SHPT_ARC, 2,
                                            dirVectorY, dirVectorX, NULL );
      }

      //
      // Write the direction vector to shapefile
      //
      recordId = SHPWriteObject( directionShp, -1, dirObject );
      SHPDestroyObject( dirObject );

      //
      // Write the corresponding dbf record
      //
      dbfFailure = false;

      double direction = fmod( fprops.smoothed_direction + grid_comps.rotation, 
                               360.0 );
      if (direction < 0.0) {
        direction += 360.0;
      }
 
      if ( DBFWriteDoubleAttribute( directionDbf, recordId,
                                    directionField, direction ) == -1 ) {
         dbfFailure = true;
      }
      if ( DBFWriteDoubleAttribute( directionDbf, recordId,
                                   speedField, fprops.smoothed_speed ) == -1 ) {
         dbfFailure = true;
      }
   
      if ( dbfFailure ) {
         POSTMSG( ERROR, "Unable to write to direction DBF fields" );
         return( -1 );
      }
   }

   poly_delta_az = sparams.poly_delta_az * DEG_TO_RAD;
   theta = sparams.poly_start_az * DEG_TO_RAD;
   dx = grid.dx;
   dy = grid.dy;

   double rayX[sparams.n_poly_sides];
   double rayY[sparams.n_poly_sides];
   double polygonX[sparams.n_poly_sides + 1];
   double polygonY[sparams.n_poly_sides + 1];

   for( int iray=0; iray < sparams.n_poly_sides; iray++ ) {
      range = gprops.proj_area_polygon[iray] * plot_storm_scale;
      rayX[iray] = range * sin(theta) * dx;
      rayY[iray] = range * cos(theta) * dy;

      if ( _params.output_projection == Params::FLAT ) {
         //
         // Leave the polygon in flat coords...
         //
         polygonX[iray] = plot_x + rayX[iray];
         polygonY[iray] = plot_y + rayY[iray];
      }
      else {
         //
         // Convert from ray in grid coords ---> polygon in lat/lon coords
         //
         TITAN_xy2latlon( &grid_comps,
                          plot_x + rayX[iray],
                          plot_y + rayY[iray],
                          &polygonX[iray], 
                          &polygonY[iray] );
      }

      theta += poly_delta_az;
   }

   polygonX[sparams.n_poly_sides] = polygonX[0];
   polygonY[sparams.n_poly_sides] = polygonY[0];

   //
   // Create a simple polygon object to be used for writing shapefile records
   // The coordinate ording differes for LatLon and Flat
   //
   if ( _params.output_projection == Params::LATLON ) {
     shpObject = SHPCreateSimpleObject( SHPT_POLYGON, 
                                        sparams.n_poly_sides+1, 
                                        polygonY, polygonX, NULL );
   }
   else {
     shpObject = SHPCreateSimpleObject( SHPT_POLYGON, 
                                        sparams.n_poly_sides+1, 
                                        polygonX, polygonY, NULL );
   }

   //
   // Write the shapefile polygon location record
   //
   recordId = SHPWriteObject( outputShp, -1, shpObject );
   SHPDestroyObject( shpObject );

   //
   // Write the corresponding dbf record
   //
   dbfFailure = false;
   if ( DBFWriteDoubleAttribute( outputDbf, recordId, areaField, 
                                 gprops.proj_area ) == -1 ) {
      dbfFailure = true;
   }
   if ( DBFWriteDoubleAttribute( outputDbf, recordId, massField, 
                                 gprops.mass ) == -1 ) {
      dbfFailure = true;
   }
   if ( DBFWriteDoubleAttribute( outputDbf, recordId, hailProbField, 
                                 gprops.add_on.hail_metrics.waldvogelProbability ) == -1 ) {
      dbfFailure = true;
   }
   if ( DBFWriteDoubleAttribute( outputDbf, recordId, hailMassField, 
                                 gprops.add_on.hail_metrics.hailMassAloft ) == -1 ) {
      dbfFailure = true;
   }

   if ( dbfFailure ) {
      POSTMSG( ERROR, "Unable to write to DBF fields" );
      return( -1 );
   }

   return( 0 );
}
