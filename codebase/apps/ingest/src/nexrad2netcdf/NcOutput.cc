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
////////////////////////////////////////////////////////////////
// NcOutput:  class that handles netCDF output
//
// Jaimi Yee, RAP, NCAR, Boulder, CO, 80307, USA
// August 2004
//
// $Id: NcOutput.cc,v 1.63 2016/03/07 01:23:10 dixon Exp $
//
////////////////////////////////////////////////////////////////
#include <ncf/ncf.hh>
#include <toolsa/pmu.h>
#include <toolsa/MsgLog.hh>
#include <dsserver/DsLdataInfo.hh>
#include "NcOutput.hh"
#include "Driver.hh"
#include "PtrMgr.hh"

//
// Constants associated with radar
//
const float  NcOutput::RADAR_CONSTANT        = 58.4;
const float  NcOutput::RECEIVER_GAIN         = 0.0;    // dB
const float  NcOutput::ANTENNA_GAIN          = 45.0;   // dB
const float  NcOutput::SYSTEM_GAIN           = 0.0;    // dB
const float  NcOutput::WAVELENGTH            = 10.71;  // cm
const float  NcOutput::BEAM_WIDTH            = 0.95;   // deg
const float  NcOutput::XMIT_PEAK_POWER       = 1000;   // kW

//
// Constants associated with file naming
//
const int    NcOutput::MAX_FILE_NAME_LEN     = 100;
const string NcOutput::DIR_DELIM             = "/";

//
// Constants associated with netCDF output
//
const int    NcOutput::SHORT_STR_LEN         = 16;

const string NcOutput::AZIMUTH_NAME          = "Azimuth";
const string NcOutput::ELEV_NAME             = "Elevation";
const string NcOutput::TIME_NAME             = "time_offset";

const string NcOutput::DZ_NAME               = "DZ";
const string NcOutput::VE_NAME               = "VE";
const string NcOutput::SW_NAME               = "SW";
const string NcOutput::SNR_NAME              = "SNR";
const string NcOutput::PR_NAME               = "PowerRatio";

const string NcOutput::DZ_UNITS              = "DBZ";
const string NcOutput::VE_UNITS              = "meters/second";
const string NcOutput::SW_UNITS              = "meters/second";
const string NcOutput::SNR_UNITS             = "dB";
const string NcOutput::PR_UNITS              = "dB";

//
// Other constants
//
const float NcOutput::KM_TO_M               = 1000.0;

NcOutput::NcOutput( Params& tdrpParams ) 
    : params( tdrpParams )
{
   ncFile                = NULL;
   millisecsPastMidnight = 0;
   volumeNum             = -1;
   sweepNum              = -1;
   fileName              = "";
   fieldNames            = NULL;
   volumeStartTime       = 0;
   baseTime              = 0;
}


NcOutput::~NcOutput() 
{
   delete [] fieldNames;
   delete ncFile;
}


Status::info_t NcOutput::init() 
{
   //
   // Set output path
   //
   outputPath = params.outputPath;
   
   //
   // Set up field names
   //
   int numFields = params.momentFieldDefs_n + params.derivedFieldDefs_n;
   
   fieldNames = new char[numFields*SHORT_STR_LEN];

   memset( (void *) fieldNames, (int) ' ', 
           sizeof( char ) * numFields*SHORT_STR_LEN );

   char *fnPtr = fieldNames;

   for( int ii = 0; ii < params.momentFieldDefs_n; ii++ ) {
      switch( params._momentFieldDefs[ii].outputField ) {
          case Params::DZ :

             strncpy( fnPtr, DZ_NAME.c_str(), DZ_NAME.size() );
             break;
             
          case Params::VE :

             strncpy( fnPtr, VE_NAME.c_str(), VE_NAME.size() );
             break;
             
          case Params::SW :

             strncpy( fnPtr, SW_NAME.c_str(), SW_NAME.size() );
             break;
      }
 
      if( ii < numFields-1 ) {
         fnPtr += SHORT_STR_LEN;
      }
   }
   
   for( int ii = 0; ii < params.derivedFieldDefs_n; ii++) {
      switch( params._derivedFieldDefs[ii].outputField ) {
          case Params::SNR :

             strncpy( fnPtr, SNR_NAME.c_str(), SNR_NAME.size() );
             break;
             
          case Params::PR :

             strncpy( fnPtr, PR_NAME.c_str(), PR_NAME.size() );
             break;
      }
 
      if( ii < numFields-1 ) {
         fnPtr += SHORT_STR_LEN;
      }
      
   }


   //
   // Setup range table if necessary
   //
   if( params.useRangeCutoff ) {
      Status::info_t ret = rangeTable.setup( params.rangeCutoffTablePath );
      if( ret != Status::ALL_OK ) {
         POSTMSG( ERROR, "Could not setup range table" );
         return( Status::FAILURE );
      }
   }

   return( Status::ALL_OK );
}


void NcOutput::setVolStartTime( int startTime ) 
{
   //
   // If we are setting the volume start time, it must
   // mean that we have a new volume.  Increment the
   // volume number as well.
   //
   volumeStartTime = startTime;
   volumeNum++;
}


void NcOutput::setBaseTime( int startTime, int msPastMidnight ) 
{
   //
   // If we are setting the base time for the sweep,
   // it must mean we have a new sweep.  Increment the
   // sweep number
   //
   baseTime              = startTime;
   millisecsPastMidnight = msPastMidnight;
   sweepNum++;
}


Status::info_t NcOutput::writeFile( SweepData* currentSweep ) 
{
   //
   // Don't do anything if we are skipping this sweep
   //
   if( currentSweep->sweepSkipped() ) {
      return( Status::ALL_OK );
   }

   //
   // Create the file
   //
   if( createFile( currentSweep->getFixedAngle() ) != Status::ALL_OK ) {
      return( Status::FAILURE );
   }

   //
   // Set internal data values
   //
   PMU_auto_register( "Setting file values" );

   Status::info_t ret = setFileVals( currentSweep );
   
   if( ret != Status::ALL_OK ) {
     return( ret );
   }

   //
   // Write the data from the current sweep to the
   // file
   //
   PMU_auto_register( "Adding data top file" );

   int numRays = currentSweep->getNumRays();

   //
   // Set up time offset data
   //
   double *timeOffset = new double[numRays];
   double *timeData   = currentSweep->getTimeData();
   
   for( int i = 0; i < numRays; i++ ) {
      timeOffset[i] = timeData[i] - baseTime;
   }

   //
   // Add one dimensional data arrays
   //
   if( addVar( TIME_NAME.c_str(), timeOffset, numRays ) != Status::ALL_OK ) {
      return( Status::FAILURE );
   }
   
   if( addVar( AZIMUTH_NAME.c_str(), currentSweep->getAzimuth(),
               numRays ) != Status::ALL_OK ) {
      return( Status::FAILURE );
   }
   
   if( addVar( ELEV_NAME.c_str(), currentSweep->getElevation(),
               numRays ) != Status::ALL_OK ) {
      return( Status::FAILURE );
   }

   delete [] timeOffset;

   //
   // Set the number of cells. 
   //
   int numCells = params.maxCells;

   if( params.useRangeCutoff ) {

      float range       = 
         rangeTable.getRange( currentSweep->getFixedAngle() ) * KM_TO_M;
      float cellSpacing = currentSweep->getCellSpacing();
      
      numCells = min( (int) ( range / cellSpacing + 0.5 ), params.maxCells );
   }

   PtrMgr ptrMgr;
   
   ptrMgr.setDataPtrs( params.maxCells, numCells, numRays, currentSweep );

   //
   // Get the "Time" dimension
   //
   NcDim *timeDim = ncFile->get_dim( "Time" );
   if( !timeDim || !timeDim->is_valid() ) {
      POSTMSG( ERROR, "Could not get Time dimension from file %s\n",
               fileName.c_str() );
      return( Status::FAILURE );
   }

   //
   // Set the maxCells dimension in the file
   //
   NcDim *maxCellsDim = ncFile->add_dim( "maxCells", numCells );
   if( !maxCellsDim || !maxCellsDim->is_valid() ) {
      POSTMSG( ERROR, "Could not add dimension maxCells to file %s",
               fileName.c_str() );
      
      return( Status::FAILURE );
   }

   //
   // Get the multiplicative factor for the velocity scale and bias
   //   This should also be applied to the spectrum width
   //
   float velScaleBiasFactor = currentSweep->getVelScaleBiasFactor();

   //
   // Add the variables
   //
   for( int ii = 0; ii < params.momentFieldDefs_n; ii++ ) {

      Status::info_t status = Status::ALL_OK;
      
      switch( params._momentFieldDefs[ii].outputField ) {
          case Params::DZ :

             status = addNewVar( DZ_NAME.c_str(), 
                                 timeDim, 
                                 maxCellsDim, 
                                 params._momentFieldDefs[ii].longName,
                                 DZ_UNITS.c_str(), 
                                 SweepData::DZ_SCALE, 
                                 SweepData::DZ_BIAS,
                                 SweepData::DZ_BAD, 
                                 ptrMgr.getDz(), 
                                 numRays, 
                                 numCells );
             break;
             
          case Params::VE :

             status = addNewVar( VE_NAME.c_str(), 
                                 timeDim, 
                                 maxCellsDim, 
                                 params._momentFieldDefs[ii].longName,
                                 VE_UNITS.c_str(), 
                                 SweepData::VE_SCALE * velScaleBiasFactor,
                                 SweepData::VE_BIAS * velScaleBiasFactor,
                                 SweepData::VE_BAD,
                                 ptrMgr.getVe(), 
                                 numRays, 
                                 numCells );

             break;
             
          case Params::SW :


             status = addNewVar( SW_NAME.c_str(), 
                                 timeDim, 
                                 maxCellsDim, 
                                 params._momentFieldDefs[ii].longName,
                                 SW_UNITS.c_str(), 
                                 SweepData::VE_SCALE * velScaleBiasFactor,
                                 SweepData::VE_BIAS * velScaleBiasFactor,
                                 SweepData::VE_BAD,
                                 ptrMgr.getSw(), 
                                 numRays, 
                                 numCells );
             break;
             

          default:
             continue;
             break;
      }
      
      if( status != Status::ALL_OK ) {
         return( Status::FAILURE );
      }

   } 

   for( int ii = 0; ii < params.derivedFieldDefs_n; ii++ ) {

      Status::info_t status = Status::ALL_OK;
      
      switch( params._derivedFieldDefs[ii].outputField ) {
          case Params::SNR :

             status = addNewVar( SNR_NAME.c_str(), 
                                 timeDim, 
                                 maxCellsDim, 
                                 params._derivedFieldDefs[ii].longName,
                                 SNR_UNITS.c_str(), 
                                 params._derivedFieldDefs[ii].scale, 
                                 params._derivedFieldDefs[ii].bias,
                                 SweepData::SNR_BAD, 
                                 ptrMgr.getSnr(), 
                                 numRays, 
                                 numCells );
             break;
             
          case Params::PR :

             status = addNewVar( PR_NAME.c_str(), 
                                 timeDim, 
                                 maxCellsDim, 
                                 params._derivedFieldDefs[ii].longName,
                                 PR_UNITS.c_str(), 
                                 params._derivedFieldDefs[ii].scale, 
                                 params._derivedFieldDefs[ii].bias, 
                                 SweepData::PR_BAD,
                                 ptrMgr.getPr(), 
                                 numRays, 
                                 numCells );
             break;

          default:
             continue;
             break;
      }
      
      if( status != Status::ALL_OK ) {
         return( Status::FAILURE );
      }

   }

   //
   // Clean up -
   //   Note that freeing the file object will actually
   //   write the file to disk
   //
   delete ncFile;
   ncFile = NULL;

   //
   // Update ldata info
   //
   DsLdataInfo ldata( outputPath );

   string currentPath = filePath.getPath();
   string relPath = "";

   filePath.stripDir( outputPath, currentPath, relPath );
   
   
   ldata.setRelDataPath( relPath.c_str() );
   ldata.setWriter( PROGRAM_NAME );
   ldata.setDataFileExt( "nc" );
   ldata.setDataType( "nc" );
   ldata.write( baseTime );
   
   POSTMSG( DEBUG, "Wrote file %s", fileName.c_str() );
   
   return( Status::ALL_OK );
}

Status::info_t NcOutput::createFile( float fixedAngle ) 
{
   //
   // Set the file name
   //
   if( setFileName( fixedAngle ) != Status::ALL_OK ) {
      return( Status::FAILURE );
   }
   
   //
   // Use the cdl file to create a new file
   //
   if( NCF_cdl2nc( params.cdlPath, filePath.getPath().c_str() ) != 0 ) {
      POSTMSG( ERROR, "Could not create file %s using cdl file %s", 
               fileName.c_str(), params.cdlPath );
      return( Status::FAILURE );
   }
   
   //
   // Set up the new file
   //
   ncFile = new NcFile( filePath.getPath().c_str(), NcFile::Write );
   if( !ncFile || !ncFile->is_valid() ) {
      POSTMSG( ERROR, "Could not open file %s", fileName.c_str() );
      return( Status::FAILURE );
   }

   return( Status::ALL_OK );
}


Status::info_t NcOutput::setFileName( float fixedAngle ) 
{
   char tmpStr[MAX_FILE_NAME_LEN];

   //
   // Get the time stamp for the current (beam) time.  Set
   // up the date directory if necessary.
   //
   DateTime tStamp( baseTime );

   filePath.setDirectory( outputPath, tStamp.getYear(), tStamp.getMonth(),
			  tStamp.getDay() );

   if( filePath.makeDir() != 0 ) {
     POSTMSG( ERROR, "Could not create date directory" );
     return( Status::FAILURE );
   }

   //
   // Set up the file name
   //
   int millisec = millisecsPastMidnight % 1000;

   sprintf( tmpStr, "%s_%s_%s_%s.%.3d_u%d_s%d_%3.1f_SUR_.nc",
            params.baseName, params.radarName, 
            tStamp.getDateStrPlain().c_str(),
            tStamp.getTimeStrPlain().c_str(),
            millisec, volumeNum, sweepNum, fixedAngle );
   
   fileName = tmpStr;

   //
   // Set the file path
   //
   filePath.setFile( fileName );

   return( Status::ALL_OK );
}


Status::info_t NcOutput::setFileVals( SweepData* currentSweep ) 
{
   //
   // Add the fields dimension to the file
   //
   int numFields = params.momentFieldDefs_n + params.derivedFieldDefs_n;
   NcDim *fieldsDim = ncFile->add_dim( "fields", numFields );

   if( !fieldsDim || !fieldsDim->is_valid() ) {
      POSTMSG( ERROR, "Could not add fields dimension to file %s",
               fileName.c_str() );
      return( Status::FAILURE );
   }

   //
   // Add the short string dimension to the file
   //
   NcDim *shortStrDim = ncFile->add_dim( "short_string", SHORT_STR_LEN );
   if( !shortStrDim || !shortStrDim->is_valid() ) {
      POSTMSG( ERROR, "Could not add short_string dimension to file %s",
               fileName.c_str() );
      return( Status::FAILURE );
   }

   //
   // Set variables in the netcdf file...
   //
   if( addVar( "volume_start_time", volumeStartTime ) != 0 ) {
      return( Status::FAILURE );
   }
   
   if( addVar( "base_time", baseTime ) != 0 ) {
      return( Status::FAILURE );
   }

   if( addNewVar( "field_names", fieldsDim, shortStrDim, fieldNames, 
                  numFields, SHORT_STR_LEN ) != 0 ) {
      return( Status::FAILURE );
   }

   if( addVar( "Fixed_Angle", currentSweep->getFixedAngle() ) != 0 ) {
      return( Status::FAILURE );
   }

   if( addVar( "Range_to_First_Cell", 
               currentSweep->getRangeToFirstGate() ) != 0 ) {
      return( Status::FAILURE );
   }

   if( addVar( "Cell_Spacing", currentSweep->getCellSpacing() ) != 0 ) {
      return( Status::FAILURE );
   }
   
   if( addVar( "Nyquist_Velocity", 
               currentSweep->getNyquistVelocity() ) != 0 ) {
      return( Status::FAILURE );
   }

   if( addVar( "Unambiguous_Range", 
               currentSweep->getUnambiguousRange() ) != 0 ) {
      return( Status::FAILURE );
   }

   if( addVar( "Latitude", params.radarLocation.latitude ) != 0 ) {
      return( Status::FAILURE );
   }
   
   if( addVar( "Longitude", params.radarLocation.longitude ) != 0 ) {
      return( Status::FAILURE );
   }

   if( addVar( "Altitude", params.radarLocation.altitude ) != 0 ) {
      return( Status::FAILURE );
   }
   
   if( addVar( "Radar_Constant", RADAR_CONSTANT ) != 0 ) {
      return( Status::FAILURE );
   }
   
   if( addVar( "rcvr_gain", RECEIVER_GAIN ) != 0 ) {
      return( Status::FAILURE );
   }

   if( addVar( "ant_gain", ANTENNA_GAIN ) != 0 ) {
      return( Status::FAILURE );
   }
   
   if( addVar( "sys_gain", SYSTEM_GAIN ) != 0 ) {
      return( Status::FAILURE );
   }
   
   if( addVar( "bm_width", BEAM_WIDTH ) != 0 ) {
      return( Status::FAILURE );
   }
   
   if( addVar( "peak_pwr", XMIT_PEAK_POWER ) != 0 ) {
      return( Status::FAILURE );
   }

   if( addVar( "Wavelength", WAVELENGTH) != 0 ) {
      return( Status::FAILURE );
   }
   
   if( addVar( "PRF", currentSweep->getPrf() ) != 0 ) {
      return( Status::FAILURE );
   }

   if( currentSweep->merged() ) {
      if( addVar( "sur_PRF", currentSweep->getSurPrf() ) != 0 ) {
         return( Status::FAILURE );
      }
   }

   //
   // Write in the global attributes
   //
   addGlobalAtt( currentSweep->getVCP(), currentSweep->merged() );
      
   return( Status::ALL_OK );
}


void NcOutput::addGlobalAtt( int vcp, bool merged ) 
{
  char cbuf[1024];

  //
  // Radar identification
  //
  ncFile->add_att( "Content", "Radar sweep file" );
  ncFile->add_att( "Conventions", "NCAR_ATD-NOAA_ETL/Scanning_Remote_Sensor" );
  ncFile->add_att( "Instrument_Name", params.radarName );
  ncFile->add_att( "Site_Name", params.siteName );
  ncFile->add_att( "Radar_Id", params.radarId );
  ncFile->add_att( "Instrument_Type", "GROUND" );
  ncFile->add_att( "Scan_Mode", "SUR" );

  //
  // Did we combine two sweeps to get this file?
  //
  if( merged ) {
     ncFile->add_att( "Combined_Sweep", "true" );
  }
  else {
     ncFile->add_att( "Combined_Sweep", "false" );
  }

  // 
  // Volume time information
  //
  DateTime vtime( volumeStartTime );
  sprintf( cbuf, "%.4d/%.2d/%.2d %.2d:%.2d:%2d",
           vtime.getYear(), vtime.getMonth(), vtime.getDay(),
           vtime.getHour(), vtime.getMin(), vtime.getSec() );
  ncFile->add_att( "Volume_Start_Time", cbuf );

  //
  // Sweep time information
  //
  DateTime stime( baseTime );
  ncFile->add_att( "Year", stime.getYear() );
  ncFile->add_att( "Month", stime.getMonth() );
  ncFile->add_att( "Day", stime.getDay() );
  ncFile->add_att( "Hour", stime.getHour() );
  ncFile->add_att( "Minute", stime.getMin() );
  ncFile->add_att( "Second", stime.getSec() );
  ncFile->add_att( "Volume_Number", volumeNum );
  ncFile->add_att( "Scan_Number", sweepNum );
  ncFile->add_att( "VCP", vcp );

  //
  // Current time information
  //
  DateTime now( time(NULL) );
  sprintf( cbuf, "%.4d/%.2d/%.2d %.2d:%.2d:%2d",
           now.getYear(), now.getMonth(), now.getDay(),
           now.getHour(), now.getMin(), now.getSec() );
  ncFile->add_att( "Production_Date", cbuf );

  //
  // Software information
  //
  ncFile->add_att( "Producer_Name", "NSF/UCAR/NCAR/RAP" );
  ncFile->add_att( "Software", PROGRAM_NAME );
  ncFile->add_att( "Range_Segments", "All data has fixed cell spacing" );
  
}

Status::info_t NcOutput::addVar( const char* varName, int value ) 
{
   //
   // Get a pointer to the variable and check it
   //
   NcVar *ncVar = ncFile->get_var( varName );
   if( !ncVar || !ncVar->is_valid() ) {
      POSTMSG( ERROR, "Could not get variable %s from file %s", 
               varName, fileName.c_str() );
      return( Status::FAILURE );
   }
   
   //
   // Put the value to that variable and check that
   //
   int status = ncVar->put( &value, 1 );
   if( status == 0 ) {
      POSTMSG( ERROR, "Could not write value to variable %s to file %s",
               varName, fileName.c_str() );
      return( Status::FAILURE );
   } 

   return( Status::ALL_OK );
}


Status::info_t NcOutput::addVar( const char* varName, float value ) 
{
   //
   // Get a pointer to the variable and check it
   //
   NcVar *ncVar = ncFile->get_var( varName );
   if( !ncVar || !ncVar->is_valid() ) {
      POSTMSG( ERROR, "Could not get variable %s from file %s", 
               varName, fileName.c_str() );
      return( Status::FAILURE );
   }
   
   //
   // Put the value to that variable and check that
   //
   int status = ncVar->put( &value, 1 );
   if( status == 0 ) {
      POSTMSG( ERROR, "Could not write value to variable %s to file %s",
               varName, fileName.c_str() );
      return( Status::FAILURE );
   } 

   return( Status::ALL_OK );
}


Status::info_t NcOutput::addVar( const char* varName, double value ) 
{
   //
   // Get a pointer to the variable and check it
   //
   NcVar *ncVar = ncFile->get_var( varName );
   if( !ncVar || !ncVar->is_valid() ) {
      POSTMSG( ERROR, "Could not get variable %s from file %s", 
               varName, fileName.c_str() );
      return( Status::FAILURE );
   }
   
   //
   // Put the value to that variable and check that
   //
   int status = ncVar->put( &value, 1 );
   if( status == 0 ) {
      POSTMSG( ERROR, "Could not write value to variable %s to file %s",
               varName, fileName.c_str() );
      return( Status::FAILURE );
   } 

   return( Status::ALL_OK );
}


Status::info_t NcOutput::addVar( const char* varName, float* values, long c0 ) 
{
   //
   // Get a pointer to the variable and check it
   //
   NcVar *ncVar = ncFile->get_var( varName );
   if( !ncVar || !ncVar->is_valid() ) {
      POSTMSG( ERROR, "Could not get variable %s from file %s", 
               varName, fileName.c_str() );
      return( Status::FAILURE );
   }
   
   //
   // Put the array of data values to that variable
   // and check it.  Note that c0 is the size of the
   // array.
   //
   int status = ncVar->put( values, c0 );
   if( status == 0 ) {
      POSTMSG( ERROR, "Could not write value to variable %s to file %s",
               varName, fileName.c_str() );
      return( Status::FAILURE );
   } 

   return( Status::ALL_OK );
}


Status::info_t NcOutput::addVar( const char* varName, double* values, 
                                 long c0 ) 
{
   //
   // Get a pointer to the variable and check it
   //
   NcVar *ncVar = ncFile->get_var( varName );
   if( !ncVar || !ncVar->is_valid() ) {
      POSTMSG( ERROR, "Could not get variable %s from file %s", 
               varName, fileName.c_str() );
      return( Status::FAILURE );
   }
   
   //
   // Put the array of data values to that variable
   // and check it.  Note that c0 is the size of that
   // array.
   //
   int status = ncVar->put( values, c0 );
   if( status == 0 ) {
      POSTMSG( ERROR, "Could not write value to variable %s to file %s",
               varName, fileName.c_str() );
      return( Status::FAILURE );
   } 

   return( Status::ALL_OK );
}


Status::info_t NcOutput::addNewVar( const char* varName, NcDim* dim1,
                                    NcDim* dim2, char* values, 
                                    long c0, long c1 ) 
{
   //
   // Create the variable
   //
   NcVar *newVar = ncFile->add_var( varName, ncChar, dim1, dim2 );
   if( !newVar || !newVar->is_valid() ) {
      POSTMSG( ERROR, "Could not create %s variable in %s",
               varName, fileName.c_str() );
      return( Status::FAILURE );
   }
   
   //
   // Put the array of data values to the variable and
   // check it.  Note that the values array was declared
   // as a one dimensional array, but in reality it is
   // a two dimensional array.  We have been keeping track
   // of those dimensions as we wrote the data.  The ncVar
   // put function will use the sizes of those dimensions
   // and the one dimensional array to put things in the
   // right place in the file.
   //
   int status = newVar->put( values, c0, c1 );
   if( status == 0 ) {
      POSTMSG( ERROR, "Could not write value to variable %s to file %s",
               varName, fileName.c_str() );
      return( Status::FAILURE );
   } 

   return( Status::ALL_OK );
}


Status::info_t NcOutput::addNewVar( const char* varName, NcDim* dim1,
                                    NcDim* dim2, const char* longName,
                                    const char* units, double scale,
                                    double bias, short badValue,
                                    short* values, long c0, long c1 ) 
{
   //
   // Get a pointer to the variable and check it
   //
   NcVar *ncVar = ncFile->add_var( varName, ncShort, dim1, dim2 );
   if( !ncVar || !ncVar->is_valid() ) {
      POSTMSG( ERROR, "Could not add variable %s to file %s", 
               varName, fileName.c_str() );
      return( Status::FAILURE );
   }

   //
   // Add attributes
   //
   ncVar->add_att( "long_name", longName );
   ncVar->add_att( "units", units );
   ncVar->add_att( "scale_factor", scale );
   ncVar->add_att( "add_offset", bias );
   ncVar->add_att( "missing_value", badValue );
   ncVar->add_att( "_FillValue", badValue );
   
   //
   // Put the array of data values to the variable and
   // check it.  Note that the values array was declared
   // as a one dimensional array, but in reality it is
   // a two dimensional array.  We have been keeping track
   // of those dimensions as we wrote the data.  The ncVar
   // put function will use the sizes of those dimensions
   // and the one dimensional array to put things in the
   // right place in the file.
   //
   int status = ncVar->put( values, c0, c1 );
   if( status == 0 ) {
      POSTMSG( ERROR, "Could not write value to variable %s of file %s",
               varName, fileName.c_str() );
      return( Status::FAILURE );
   } 

   return( Status::ALL_OK );
}


  
