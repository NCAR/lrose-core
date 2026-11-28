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
/*
------------------------------------------------------------------------------
SstNc2MdvApp.cc - Driver for SstNc2Mdv application

Steve Carson, RAP, NCAR, Boulder, CO, 80307, USA
April 2006
------------------------------------------------------------------------------
*/

using namespace std;

#include "InputVar.hh"
#include "SstNc2MdvApp.hh"

#include "NasaInputHandler.hh"
#include "NasaRalInputHandler.hh"
#include "NoaaInputHandler.hh"

const string
   CLASS_NAME = "SstNc2MdvApp";

/*
------------------------------------------------------------------------------
Constructor - SstNc2MdvApp
------------------------------------------------------------------------------
*/

SstNc2MdvApp::SstNc2MdvApp()
{
}

/*
------------------------------------------------------------------------------
Destructor - ~SstNc2MdvApp
------------------------------------------------------------------------------
*/

SstNc2MdvApp::~SstNc2MdvApp()
{
 FreeMemoryPerRun();
}

/*
------------------------------------------------------------------------------
Init
------------------------------------------------------------------------------
*/

int
SstNc2MdvApp::
Init( int argc, char **argv )

{ // begin Init

 int
    status,
    i;

 const string
    METHOD_NAME = "Init";

 // Initialize facility for obtaining the full-path filename of the
 // SstNc2Mdv executable which is currently running

 mProgPath.setPath( argv[0] );

 //
 // Initialize logging with full-path filename
 // of the current SstNc2Mdv executable
 //

 mMsgLog.setApplication( GetProgramName() );

 //
 // Print a log message
 //

 mMsgLog.enableMsg( INFO, true );
 POSTMSG( INFO, "Starting SstNc2MdvApp::Init");

 //
 // Read the command line args
 //

 mCmdLineArgs = new CmdLineArgs( &mMsgLog );

 status = mCmdLineArgs->ReadArgs( argc, argv );

 switch( status )
    {
    case SUCCESS:
       break;
    case FAILURE:
       mCmdLineArgs->usage();
       return( status );
       break;
    case CMDARG_PRINT_USAGE:
       mCmdLineArgs->usage();
       return( status );
       break;
    case CMDARG_UNKNOWN_ARG:
       mCmdLineArgs->usage();
       return( status );
       break;
    } // end switch

 //
 // Capture start and end times from command line args
 // Default value "DateTime::NEVER" set in CmdLineArgs.cc
 //

 mStartTime = mCmdLineArgs->mStartTime;
 mEndTime   = mCmdLineArgs->mEndTime;

 //
 // Print ucopyright message
 //

 ucopyright( (char *) PROGRAM_NAME );

 //
 // Read the TDRP parameters
 //

 if ( ReadParams( argc, argv ) != SUCCESS ) return( FAILURE );

 //
 // Register with procmap now that we have the instance name (from TDRP)
 //

 PMU_auto_init(
    (char*) PROGRAM_NAME,
    mParams.instance,
    PROCMAP_REGISTER_INTERVAL);

 PMU_auto_register( "Initializing SstNc2MdvApp" );

 //
 // Process the TDRP parameters
 //

 if ( ProcessParams() != SUCCESS ) return( FAILURE );

 //
 // Create DsInputPath object for new SST input file (Netcdf format) 
 // using the constructor appropriate for the current operating mode
 //

 switch( mParams.run_mode )
    {

    case Params::REALTIME:

       POSTMSG(INFO,"Run mode is REALTIME");

       mNewSstNcPath = new DsInputPath
          (
          PROGRAM_NAME,
          (mParams.debug_level >= 1),
          mParams.input_dir,
          mParams.max_inp_sst_age_sec,
          PMU_auto_register,
          mParams.use_ldata_info_file
          );

       break;

    case Params::ARCHIVE_FILE_LIST:
    case Params::FILE_LIST:

       POSTMSG(INFO,"Run mode is ARCHIVE_FILE_LIST");

       mNewSstNcPath = new DsInputPath
          (
          PROGRAM_NAME,
          (mParams.debug_level >= 1),
          mInputPathList
          );

       // Print list of ARCHIVE_FILE_LIST input paths

       // NOTE: this produced a compile error:
       // int
       //    n_input_paths = mInputPathList.size();
       //
       // error was:
       // "error: jump to case label"
       // "error:   crosses initialization of `int n_input_paths'

       int
          n_input_paths;

       n_input_paths = mInputPathList.size();


       // Check to see whether ARCHIVE_FILE_LIST input paths all exist

       struct stat
          file_status;

       for(i = 0; i < n_input_paths; ++i)
          {
          stat(mInputPathList[i].c_str(), &file_status);

          if( !S_ISREG(file_status.st_mode) )
             {

             POSTMSG(WARNING,
                "ARCHIVE_FILE_LIST input file '%s' does not exist",
                mInputPathList[i].c_str());


             } // end if S_ISDIR
          } // end for i

       break;

    case Params::ARCHIVE_START_END_TIMES:
    case Params::TIME_LIST:

       POSTMSG(INFO,"Run mode is ARCHIVE_START_END_TIMES");

       mNewSstNcPath = new DsInputPath
          (
          PROGRAM_NAME,
          (mParams.debug_level >= 1),
          mParams.input_dir,
          mStartTime,
          mEndTime
          );

       break;

    } // end switch mParams.run_mode

 // Initialize the output projection

 InitOutputMdvxProj();

 // Initialize the input handler.  Must be done AFTER initializing the
 // output projection

 InitInputHandler();
 
 return SUCCESS;

} // end init

/*
-------------------------------------------------------------------------------
ReadParams
-------------------------------------------------------------------------------
*/

int
SstNc2MdvApp::
ReadParams( int argc, char **argv )

{ // begin ReadParams

 const string
    METHOD_NAME = "ReadParams";

 int
    status = 0;

 FILE
    *param_file;

 PMU_auto_register( "Reading the parameter file" );

 //
 // Open the TDRP parameter file
 //

 param_file = fopen( mCmdLineArgs->mParamsFileName.c_str(), "r" );

 if( param_file == NULL )
    {
    mMsgLog.postMsg(
       WARNING,
       "No param file specified: using defaults: check argument '-params'");
    }
 else
    {
    fclose( param_file );
    } // end if param_file == NULL

 //
 // Read the TDRP parameter file
 //

 status =
 mParams.loadFromArgs(
    argc,
    argv,
    mCmdLineArgs->tdrpOverride.list,
    &paramPath);

 //
 // Make sure the read worked
 //

 if ( status == SUCCESS )
    {
    //
    // Set debug level for logging messages
    //

    if ( mParams.debug_level > 0 )
       mMsgLog.enableMsg( DEBUG, true );

    if ( paramPath )
       mMsgLog.postMsg( INFO, "Loaded Parameter file: %s", paramPath );
    }
 else
    {
    mMsgLog.postMsg(ERROR, "Unable to load parameters from '%s'", paramPath);

    if ( paramPath )
       mMsgLog.postMsg(ERROR,"Check syntax of parameter file '%s'", paramPath);

    return status;
    }

 TDRP_free_override( &mCmdLineArgs->tdrpOverride );

 return SUCCESS;

} // end ReadParams

/*
-------------------------------------------------------------------------------
ProcessParams
-------------------------------------------------------------------------------
*/

int
SstNc2MdvApp::
ProcessParams()

{ // begin ProcessParams

 const string
    METHOD_NAME = "ProcessParams";

 //
 // List of full-path filenames of input files
 // (for mode ARCHIVE_FILE_LIST only)
 //

 int
    n_input_files = 0;

 string
    input_dir = mParams.input_dir;

 if( mParams.run_mode == Params::ARCHIVE_FILE_LIST ||
     mParams.run_mode == Params::FILE_LIST )
    {
    n_input_files = mCmdLineArgs->mInputFileList.size();

    for(int i = 0; i < n_input_files; ++i)
       {
       mInputPathList.push_back(
          input_dir + "/" + mCmdLineArgs->mInputFileList[i]);
       }

    }
 else
    {
    mInputPathList.clear();
    }

 return SUCCESS;

} // end ProcessParams

/*
-------------------------------------------------------------------------------
Run
-------------------------------------------------------------------------------
*/

int SstNc2MdvApp::Run()

{ // begin run

 int
    run_status = SUCCESS;

 const string
    METHOD_NAME = "Run";

 switch (mParams.run_mode)
 {
 case Params::REALTIME :
   run_status = RunRealtimeMode();
   break;

 case Params::ARCHIVE_FILE_LIST :
 case Params::FILE_LIST :
 case Params::ARCHIVE_START_END_TIMES :
 case Params::TIME_LIST :
   run_status = RunArchiveMode();
   break;
 } /* endcase - mParams.run_mode */
 
 return run_status;

} // end Run

/*
-------------------------------------------------------------------------------
FreeMemoryPerRun
-------------------------------------------------------------------------------
*/

void
SstNc2MdvApp::
FreeMemoryPerRun( void )

{ // begin FreeMemoryPerRun

 delete mNewSstNcPath;
 delete mCmdLineArgs;

} // end FreeMemoryPerRun

/*
-------------------------------------------------------------------------------
InitMdvMasterHeader

NOTE: this function must not be called until AFTER the Mdv output projection
"arOutputProj" has been initialized!
-------------------------------------------------------------------------------
*/

void
SstNc2MdvApp::
InitMdvMasterHeader
   (
   const string    &aInputFilePath, // input: Netcdf input file path
   time_t          aVolumeUnixTime, // input: Unix time for MDV output data
   const MdvxPjg   &arOutputProj,   // input: MdvxProj object for Mdv output

   DsMdvx          *aMdvx           // output: modified DsMdvx object
   )

{
 const string
    METHOD_NAME = "InitMdvMasterHeader";

 Mdvx::master_header_t
    master_hdr;

 // Set entire master header to all zero bytes

 memset(&master_hdr, 0, sizeof(master_hdr));

 // Initialize master header fields as appropriate

 time_t current_unix_time = time(0);

 master_hdr.time_gen             = 0;                 // not forecast data
 master_hdr.user_time            = current_unix_time; // time this prog was run
 master_hdr.time_begin           = aVolumeUnixTime;
 master_hdr.time_end             = aVolumeUnixTime;
 master_hdr.time_centroid        = aVolumeUnixTime;
 master_hdr.time_expire          = aVolumeUnixTime;
 master_hdr.num_data_times       = 1;
 master_hdr.index_number         = 0;
 master_hdr.data_dimension       = 2;
 master_hdr.data_collection_type = Mdvx::DATA_SYNTHESIS;
 master_hdr.user_data            = 0;
 master_hdr.native_vlevel_type   = Mdvx::VERT_TYPE_SURFACE;
 master_hdr.vlevel_type          = Mdvx::VERT_TYPE_SURFACE;
 master_hdr.vlevel_included      = 1;
 master_hdr.grid_orientation     = 1;
 master_hdr.data_ordering        = 0;
 master_hdr.n_fields             = 0;
 master_hdr.n_chunks             = 0;
 master_hdr.field_grids_differ   = 0;

 STRcopy(master_hdr.data_set_info,  aInputFilePath.c_str(),    MDV_INFO_LEN);
 STRcopy(master_hdr.data_set_name,  "Sea Surface Temperature", MDV_NAME_LEN);
 STRcopy(master_hdr.data_set_source,"Generated by SstNc2Mdv",  MDV_NAME_LEN);

 aMdvx->setMasterHeader(master_hdr);

} // end InitMdvMasterHeader

/*
-------------------------------------------------------------------------------
CreateMdvxField
-------------------------------------------------------------------------------
*/

MdvxField*
SstNc2MdvApp::
CreateMdvxField
   (
   MdvxPjg     *aMdvProj,
   time_t      aNetcdfFileUtime,
   fl32        aBadDataValue,
   fl32        aMissingDataValue,
   const char  *aFieldName,
   const char  *aFieldNameLong,
   const char  *aFieldUnits
   )

{ // begin CreateMdvxField

 const string
    METHOD_NAME = "CreateMdvxField";

 // Create the field header

 Mdvx::field_header_t
   field_hdr;

 // Set entire field header to all zero bytes

 memset(&field_hdr, 0, sizeof(field_hdr));

 field_hdr.field_code          = 0;
 field_hdr.forecast_delta      = 0;
 field_hdr.forecast_time       = aNetcdfFileUtime;
 field_hdr.encoding_type       = Mdvx::ENCODING_FLOAT32;
 field_hdr.data_element_nbytes = 4;
 field_hdr.compression_type    = Mdvx::COMPRESSION_NONE;
 field_hdr.transform_type      = Mdvx::DATA_TRANSFORM_NONE;
 field_hdr.scaling_type        = Mdvx::SCALING_NONE;
 field_hdr.native_vlevel_type  = Mdvx::VERT_TYPE_SURFACE;
 field_hdr.vlevel_type         = field_hdr.native_vlevel_type;
 field_hdr.dz_constant         = 1;
 field_hdr.scale               = 1.0;
 field_hdr.bias                = 0.0;
 field_hdr.bad_data_value      = aBadDataValue;
 field_hdr.missing_data_value  = aMissingDataValue;

 STRcopy(field_hdr.field_name,      aFieldName,     MDV_SHORT_FIELD_LEN);
 STRcopy(field_hdr.field_name_long, aFieldNameLong, MDV_LONG_FIELD_LEN);
 STRcopy(field_hdr.units,           aFieldUnits,    MDV_UNITS_LEN);

 aMdvProj->syncToFieldHdr( field_hdr );

 Mdvx::vlevel_header_t  vlevel_hdr;

 memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));

 vlevel_hdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
 vlevel_hdr.level[0] = 0.5;
 
 return new MdvxField(field_hdr, vlevel_hdr, (void *)0, true);

} // end CreateMdvxField


/*
-------------------------------------------------------------------------------
InitInputHandler

Initialize the input handler object.
-------------------------------------------------------------------------------
*/

void
SstNc2MdvApp::
InitInputHandler()
{ // begin InitInputHandler

  const string
    METHOD_NAME = "InitInputHandler";

  // Initialize the base handler object

  switch (mParams.input_filename_type)
  {
  case Params::FNAME_RAL :
    _inputHandler = new NasaRalInputHandler(mOutSstNumDataPts,
					    mOutputMdvxProj,
					    mParams.debug_level >0,
					    &mMsgLog);
    break;
   
  case Params::FNAME_NASA :
    _inputHandler = new NasaInputHandler(mOutSstNumDataPts,
					 mOutputMdvxProj,
					 mParams.debug_level >0,
					 &mMsgLog);
    break;
   
  case Params::FNAME_NOAA :
    _inputHandler = new NoaaInputHandler(mOutSstNumDataPts,
					 mOutputMdvxProj,
					 mParams.debug_level > 0,
					 &mMsgLog);
    break;
  }
 
  // Add the input variables to be processed

  for (int i = 0; i < mParams.input_vars_n; ++i)
  {
    InputVar input_var(mParams._input_vars[i].nc_var_name,
		       mParams._input_vars[i].missing_data_attr_name,
		       mParams._input_vars[i].units_attr_name,
		       mParams._input_vars[i].mdv_field_name,
		       mParams._input_vars[i].mdv_missing_data_value,
		       mParams._input_vars[i].specify_mdv_scaling,
		       mParams._input_vars[i].mdv_scale,
		       mParams._input_vars[i].mdv_bias,
		       mParams.debug_level > 0,
		       &mMsgLog);
    
    _inputHandler->addInputVar(input_var);
    
  } /* endfor i */
  
} // end InitInputHandler

/*
-------------------------------------------------------------------------------
InitOutputMdvxProj

Initializes the projection in SstNc2MdvApp::mOutputMdvxProj according
to the projection type mParams.remap_proj_type and the values in the
sst_output_grid_t struct argument.
-------------------------------------------------------------------------------
*/

void
SstNc2MdvApp::
InitOutputMdvxProj()
{ // begin InitOutputMdvxProj

 const string
    METHOD_NAME = "InitOutputMdvxProj";

 switch( mParams.remap_proj_type )
    {

    case Params::PROJ_LATLON :

       mOutputMdvxProj.initLatlon
          (
          mParams.sst_output_grid.nx,
          mParams.sst_output_grid.ny,
          mParams.sst_output_grid.nz,
          mParams.sst_output_grid.dx,
          mParams.sst_output_grid.dy,
          mParams.sst_output_grid.dz,
          mParams.sst_output_grid.minx,
          mParams.sst_output_grid.miny,
          mParams.sst_output_grid.minz
          );

       break;

    case Params::PROJ_FLAT :

       mOutputMdvxProj.initFlat
          (
          mParams.sst_output_grid.origin_lat,
          mParams.sst_output_grid.origin_lon,
          mParams.sst_output_grid.rotation,
          mParams.sst_output_grid.nx,
          mParams.sst_output_grid.ny,
          mParams.sst_output_grid.nz,
          mParams.sst_output_grid.dx,
          mParams.sst_output_grid.dy,
          mParams.sst_output_grid.dz,
          mParams.sst_output_grid.minx,
          mParams.sst_output_grid.miny,
          mParams.sst_output_grid.minz
          );
       
       break;

    case Params::PROJ_LAMBERT_CONF :

       mOutputMdvxProj.initLc2
          (
          mParams.sst_output_grid.origin_lat,
          mParams.sst_output_grid.origin_lon,
          mParams.sst_output_grid.lat1,
          mParams.sst_output_grid.lat2,
          mParams.sst_output_grid.nx,
          mParams.sst_output_grid.ny,
          mParams.sst_output_grid.nz,
          mParams.sst_output_grid.dx,
          mParams.sst_output_grid.dy,
          mParams.sst_output_grid.dz,
          mParams.sst_output_grid.minx,
          mParams.sst_output_grid.miny,
          mParams.sst_output_grid.minz
          );
       
    } // end switch mParams.remap_proj_type

 mOutSstNumRows    = mParams.sst_output_grid.ny;
 mOutSstNumCols    = mParams.sst_output_grid.nx;
 mOutSstNumDataPts = mOutSstNumRows * mOutSstNumCols;

} // end InitOutputMdvxProj

/*
-------------------------------------------------------------------------------
RunRealtimeMode

RunRealtimeMode has two sub-modes of operation: with and without
reading a background file. One instance of SstNc2Mdv will watch the
NOAA input directory, while another will watch the NASA satellite
input directory. Both instances will write Mdv files on the same grid
to the OW SST output directory. The instance that watches the NOAA
input directory will *not* use a background file. Thus whenever a NOAA
file comes in, we will "start over" with only analysis data and
without any satellite data. The instance that watches the NASA input
directory *will* use a background file. The background file will be
the latest OW SST MDV file in the OW output directory. Thus with two
instances of SstNc2Mdv, the OW realtime system should be able to
ingest both NOAA global analysis data and NASA satellite swath SST
data, and use the NOAA data as bckground while overlaying the latest
NASA satellite SST on top of the NOAA data, wherever the NASA data is
newer.
-------------------------------------------------------------------------------
*/

int
SstNc2MdvApp::
RunRealtimeMode( void )

{ // begin RunRealtimeMode

 char
    *inp_sst_nc_path = NULL;

 const string
    METHOD_NAME = "RunRealtimeMode";

 // Watch input SST directory; when a new file comes, process it

 while( (inp_sst_nc_path = mNewSstNcPath->next()) != NULL )
   ProcessFile(inp_sst_nc_path);

 // Should never get here

 return SUCCESS;

} // end RunRealtimeMode


/*
-------------------------------------------------------------------------------
ProcessFile

Process the given raw data file.
-------------------------------------------------------------------------------
*/

int SstNc2MdvApp::ProcessFile( const string &inp_sst_nc_path )
{
 const string METHOD_NAME = "SstNc2MdvApp::ProcessFile()";

 // Read in new SST data

 POSTMSG( INFO, "Processing SST input file '%s'", inp_sst_nc_path.c_str());

 time_t data_time;
 
 if ((data_time =
      _inputHandler->readData(inp_sst_nc_path,
			      mParams.fld_name_inp_lat,
			      mParams.att_name_inp_lat_miss,
			      mParams.fld_name_inp_lon,
			      mParams.att_name_inp_lon_miss,
			      mParams.dim_name_inp_rows,
			      mParams.dim_name_inp_cols)) == -1)
 {
   POSTMSG( ERROR, "Error reading in SST netCDF file");
   return FAILURE;
 }
 
 POSTMSG( INFO, "Read in SST netCDF file");
 
 // Initialize master header of Mdv output object
 // (Sets "time_centroid" in Master Header to data_time)

 InitMdvMasterHeader(inp_sst_nc_path,
		     data_time,
		     mOutputMdvxProj,
		     &mMdvxOutSst);

 // Create the MDV fields and add them to the MDV file

 if (!_inputHandler->createMdvFields(mMdvxOutSst))
   return FAILURE;
 
 // Write out the Mdv file

 POSTMSG( INFO, "About to write SST MDV file");
 
 mMdvxOutSst.setWriteLdataInfo();

 if (mMdvxOutSst.writeToDir( mParams.output_url ) != 0)
 {
   return FAILURE;
 }

 mMdvxOutSst.clearFields();

 POSTMSG( INFO, "Wrote SST MDV file");
 
 return SUCCESS;

} // end ProcessFile


/*
-------------------------------------------------------------------------------
RunArchiveMode
-------------------------------------------------------------------------------
*/

int
SstNc2MdvApp::
RunArchiveMode( void )

{ // begin RunArchiveMode

 char *inp_sst_nc_path = NULL;

 const string METHOD_NAME = "RunArchiveMode";

 while( (inp_sst_nc_path = mNewSstNcPath->next()) != NULL )
   ProcessFile(inp_sst_nc_path);

 return SUCCESS;

} // end RunArchiveMode
