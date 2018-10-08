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
// $Id: NcOutput.cc,v 1.29 2018/02/16 20:45:38 jcraig Exp $
//
////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <toolsa/pmu.h>
#include <toolsa/MsgLog.hh>
#include <dsserver/DsLdataInfo.hh>
#include "NcOutput.hh"
#include "Nexrad2Netcdf.hh"
#include "Beam.hh"

//
// Constants associated with radar
//   Note: beam width defined in SweepData class
//
const float  NcOutput::RADAR_CONSTANT        = 58.4;
const float  NcOutput::RECEIVER_GAIN         = 0.0;    // dB
const float  NcOutput::ANTENNA_GAIN          = 45.0;   // dB
const float  NcOutput::SYSTEM_GAIN           = 0.0;    // dB
const float  NcOutput::WAVELENGTH            = 10.71;  // cm
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
const string NcOutput::ZDR_NAME              = "ZDR";
const string NcOutput::PHI_NAME              = "PHI";
const string NcOutput::RHO_NAME              = "RHO";
const string NcOutput::CMAP_NAME             = "ClutterMap";
const string NcOutput::BMAP_NAME             = "BypassMap";
const string NcOutput::SNR_NAME              = "SNR";
const string NcOutput::PR_NAME               = "PR";
const string NcOutput::REC_NAME              = "REC";

const string NcOutput::DZ_UNITS              = "DBZ";
const string NcOutput::VE_UNITS              = "meters/second";
const string NcOutput::SW_UNITS              = "meters/second";
const string NcOutput::ZDR_UNITS             = "meters/second";
const string NcOutput::PHI_UNITS             = "meters/second";
const string NcOutput::RHO_UNITS             = "meters/second";
const string NcOutput::SNR_UNITS             = "dB";
const string NcOutput::PR_UNITS              = "dB";
const string NcOutput::REC_UNITS             = "none";

//
// Other constants
//
const float NcOutput::KM_TO_M               = 1000.0;

NcOutput::NcOutput( Params *tdrpParams ) 
{
  params                = tdrpParams;
  ncFile                = NULL;
  millisecsPastMidnight = 0;
  volumeNum             = -1;
  sweepNum              = -1;
  fileName              = "";
  fieldNames            = NULL;
  volumeStartTime       = 0;
  baseTime              = 0;
  _azimuth              = NULL;
  _elevation            = NULL;
  _dataTime             = NULL;
  _dz                   = NULL;
  _ve                   = NULL;
  _sw                   = NULL;
  _zdr                  = NULL;
  _phi                  = NULL;
  _rho                  = NULL;
  _snr                  = NULL;
  _pr                   = NULL;
  _rec                  = NULL;

  init();
}


NcOutput::~NcOutput() 
{
  if(fieldNames)
    delete [] fieldNames;
  if(ncFile)
    delete ncFile;
  clear();
}

void NcOutput::clear() 
{
  if(_azimuth)
    delete [] _azimuth;
   if(_elevation)
     delete [] _elevation;
   if(_dataTime)
     delete [] _dataTime;
   if(_dz)
     delete [] _dz;
   if(_ve)
     delete [] _ve;
   if(_sw)
     delete [] _sw;
   if(_zdr)
     delete [] _zdr;
   if(_phi)
     delete [] _phi;
   if(_rho)
     delete [] _rho;
   if(_snr)
     delete [] _snr;
   if(_pr)
     delete [] _pr;
   if(_rec)
     delete [] _rec;
   
   _azimuth   = NULL;
   _elevation = NULL;
   _dataTime  = NULL;
   _dz        = NULL;
   _ve        = NULL;
   _sw        = NULL;
   _zdr       = NULL;
   _phi       = NULL;
   _rho       = NULL;
   _snr       = NULL;
   _pr        = NULL;
   _rec       = NULL;
}


void NcOutput::setRadarInfo(char *radarName, double radarHeight, double radarLat, double radarLon,
			    int radarID, char *siteName)
{
  strncpy(_radarName, radarName, 4);
  _radarName[4] = char(0);
  _radarHeight = radarHeight;
  _radarLat = radarLat;
  _radarLon = radarLon;
  _radarID = radarID;
  if(siteName != NULL)
    strncpy(_siteName, siteName, 254);
  else
    strcpy(_siteName, "Unknown");


   //
   // Set output path
   //
   outputPath = params->outputPath;
   if(params->outputPath[strlen(params->outputPath)] != '/')
      outputPath += '/';
   outputPath += _radarName;
}

status_t NcOutput::init() 
{
   //
   // Set up field names
   //
   int numFields = params->momentFieldDefs_n + params->derivedFieldDefs_n;
   
   fieldNames = new char[numFields*SHORT_STR_LEN];

   memset( (void *) fieldNames, (int) ' ', 
           sizeof( char ) * numFields*SHORT_STR_LEN );

   char *fnPtr = fieldNames;

   for( int ii = 0; ii < params->momentFieldDefs_n; ii++ ) {
      switch( params->_momentFieldDefs[ii].outputField ) {
          case Params::DZ :
             strncpy( fnPtr, DZ_NAME.c_str(), DZ_NAME.size() );
             break;
             
          case Params::VE :
             strncpy( fnPtr, VE_NAME.c_str(), VE_NAME.size() );
             break;
             
          case Params::SW :
             strncpy( fnPtr, SW_NAME.c_str(), SW_NAME.size() );
             break;

          case Params::ZDR :
             strncpy( fnPtr, ZDR_NAME.c_str(), ZDR_NAME.size() );
             break;

          case Params::PHI :
             strncpy( fnPtr, PHI_NAME.c_str(), PHI_NAME.size() );
             break;

          case Params::RHO :
             strncpy( fnPtr, RHO_NAME.c_str(), RHO_NAME.size() );
             break;

          case Params::CMAP :
             strncpy( fnPtr, CMAP_NAME.c_str(), CMAP_NAME.size() );
             break;

          case Params::BMAP :
             strncpy( fnPtr, BMAP_NAME.c_str(), BMAP_NAME.size() );
             break;
      }
 
      if( ii < numFields-1 ) {
         fnPtr += SHORT_STR_LEN;
      }
   }
   
   for( int ii = 0; ii < params->derivedFieldDefs_n; ii++) {
      switch( params->_derivedFieldDefs[ii].outputField ) {
          case Params::SNR :

             strncpy( fnPtr, SNR_NAME.c_str(), SNR_NAME.size() );
             break;
             
          case Params::PR :

             strncpy( fnPtr, PR_NAME.c_str(), PR_NAME.size() );
             break;

          case Params::REC :

             strncpy( fnPtr, REC_NAME.c_str(), REC_NAME.size() );
             break;

      }
 
      if( ii < numFields-1 ) {
         fnPtr += SHORT_STR_LEN;
      }
      
   }


   //
   // Setup range table if necessary
   //
   if( params->useRangeCutoff ) {
      status_t ret = rangeTable.setup( params->rangeCutoffTablePath );
      if( ret != ALL_OK ) {
         POSTMSG( ERROR, "Could not setup range table" );
         return( FAILURE );
      }
   }

   return( ALL_OK );
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


status_t NcOutput::writeFile( SweepData* currentSweep ) 
{
  Nc3Error ncError( Nc3Error::silent_nonfatal );

   //
   // Don't do anything if we are skipping this sweep
   //
   if( currentSweep->sweepSkipped() ) {
      return( ALL_OK );
   }

   clear();

   assemble( *currentSweep);

   if(_numCellsVel < 1) {
     POSTMSG( ERROR, "Cannot create Netcdf file, numCellsVel = %d\n",
	      _numCellsVel );
     return( FAILURE );
   }
   if(_numCellsRefl < 1) {
     POSTMSG( ERROR, "Cannot create Netcdf file, numCellsRefl = %d\n",
	      _numCellsRefl );
     return( FAILURE );
   }


   //
   // Create the file
   //
   if( createFile( currentSweep->getFixedAngle() ) != ALL_OK ) {
      return( FAILURE );
   }

   //
   // Set internal data values
   //
   PMU_auto_register( "Setting file values" );

   status_t ret = setFileVals( currentSweep );
   
   if( ret != ALL_OK ) {
     delete ncFile;
     ncFile = NULL;
     deleteCurrentFileName();
     return( ret );
   }

   //
   // Write the data from the current sweep to the
   // file
   //
   PMU_auto_register( "Adding data to file" );
   
   if(_numRays < 1) {
     POSTMSG( ERROR, "Cannot create Netcdf file, numRays = %d\n",
	      _numRays );
     delete ncFile;
     ncFile = NULL;
     deleteCurrentFileName();
     return( FAILURE );
   }

   //
   // Add one dimensional data arrays
   //
   if( addVar( TIME_NAME.c_str(), _dataTime, _numRays ) != ALL_OK ) {
     delete ncFile;
     ncFile = NULL;
     deleteCurrentFileName();
     return( FAILURE );
   }
   
   if( addVar( AZIMUTH_NAME.c_str(), _azimuth,
               _numRays ) != ALL_OK ) {
     delete ncFile;
     ncFile = NULL;
     deleteCurrentFileName();
     return( FAILURE );
   }
   
   if( addVar( ELEV_NAME.c_str(), _elevation,
               _numRays ) != ALL_OK ) {
     delete ncFile;
     ncFile = NULL;
     deleteCurrentFileName();
     return( FAILURE );
   }

   if( addVar( "horiz_noise", _hNoise,
               _numRays ) != ALL_OK ) {
     delete ncFile;
     ncFile = NULL;
     deleteCurrentFileName();
     return( FAILURE );
   }

   if( addVar( "vert_noise", _vNoise,
               _numRays ) != ALL_OK ) {
     delete ncFile;
     ncFile = NULL;
     deleteCurrentFileName();
     return( FAILURE );
   }

   //
   // Get the "Time" dimension
   //
   Nc3Dim *timeDim = ncFile->get_dim( "Time" );
   if( !timeDim || !timeDim->is_valid() ) {
      POSTMSG( ERROR, "Could not get Time dimension from file %s\n",
               fileName.c_str() );
      delete ncFile;
      ncFile = NULL;
      deleteCurrentFileName();
      return( FAILURE );
   }

   //
   // Set the maxCellsVel dimension in the file
   //
   Nc3Dim *maxCellsVelDim = NULL;
   if(_numCellsVel != 0) {
     maxCellsVelDim = ncFile->add_dim( "maxCells_Dop", _numCellsVel );
     if( !maxCellsVelDim || !maxCellsVelDim->is_valid() ) {
       POSTMSG( ERROR, "Could not add dimension maxCellsDop to file %s",
		fileName.c_str() );
       delete ncFile;
       ncFile = NULL;
       return( FAILURE );
     }
   }

   //
   // Set the maxCellsRefl dimension in the file
   //
   Nc3Dim *maxCellsReflDim = NULL;
   if(_numCellsRefl != 0) {
     maxCellsReflDim = ncFile->add_dim( "maxCells_Surv", _numCellsRefl );
     if( !maxCellsReflDim || !maxCellsReflDim->is_valid() ) {
       POSTMSG( ERROR, "Could not add dimension maxCellsSurv to file %s",
		fileName.c_str() );
       delete ncFile;
       ncFile = NULL;
       return( FAILURE );
     }
   }

   //
   // Set the radials dimension in the file
   //
   Nc3Dim *radialDim = ncFile->add_dim( "radials", 360 );
   if( !radialDim || !radialDim->is_valid() ) {
      POSTMSG( ERROR, "Could not add dimension radials to file %s",
               fileName.c_str() );
      delete ncFile;
      ncFile = NULL;
      deleteCurrentFileName();
      return( FAILURE );
   }

   //
   // Set the maxZones dimension in the file
   //
   Nc3Dim *twentyDim = ncFile->add_dim( "maxZones", 20 );
   if( !twentyDim || !twentyDim->is_valid() ) {
      POSTMSG( ERROR, "Could not add dimension twenty to file %s",
               fileName.c_str() );
      delete ncFile;
      ncFile = NULL;
      deleteCurrentFileName();
      return( FAILURE );
   }

   //
   // Set the zoneAttributes dimension in the file
   //
   Nc3Dim *twoDim = ncFile->add_dim( "zoneAttributes", 2 );
   if( !twoDim || !twoDim->is_valid() ) {
      POSTMSG( ERROR, "Could not add dimension two to file %s",
               fileName.c_str() );
      delete ncFile;
      ncFile = NULL;
      deleteCurrentFileName();
      return( FAILURE );
   }

   //
   // Set the rangeBitMap dimension in the file
   //
   Nc3Dim *thirtytwoDim = ncFile->add_dim( "rangeBitMap", 32 );
   if( !thirtytwoDim || !thirtytwoDim->is_valid() ) {
      POSTMSG( ERROR, "Could not add dimension thirtytwo to file %s",
               fileName.c_str() );
      delete ncFile;
      ncFile = NULL;
      deleteCurrentFileName();
      return( FAILURE );
   }

   //
   // Get the multiplicative factor for the velocity scale and bias
   //   This should also be applied to the spectrum width
   //
   float velScaleBiasFactor = currentSweep->getVelScaleBiasFactor();

   ClutterSegment_t *clutterSeg = currentSweep->getClutterSegment();
   BypassSegment_t *bypassSeg = currentSweep->getBypassSegment();

   //
   // Add the variables
   //
   for( int ii = 0; ii < params->momentFieldDefs_n; ii++ ) {

      status_t status = ALL_OK;
      
      switch( params->_momentFieldDefs[ii].outputField ) {
          case Params::DZ :

	    if(_haveDz && _numCellsRefl != 0) {
             status = addNewVar( DZ_NAME.c_str(), 
                                 timeDim, 
                                 maxCellsReflDim, 
                                 params->_momentFieldDefs[ii].longName,
                                 DZ_UNITS.c_str(), 
                                 currentSweep->getDbzScale(), 
                                 currentSweep->getDbzBias(),
                                 currentSweep->getDbzBad(), 
				 currentSweep->getRangeToFirstReflGate(),
				 currentSweep->getCellSpacingRefl(),
                                 _dz, 
                                 _numRays, 
                                 _numCellsRefl );
	    } else {
	      POSTMSG( ERROR, "No %s data avaialable for file %s",
		       DZ_NAME.c_str(), fileName.c_str());
	    }
	    break;
             
          case Params::VE :

	    if(_haveVe && _numCellsVel != 0) {
             status = addNewVar( VE_NAME.c_str(), 
                                 timeDim, 
                                 maxCellsVelDim, 
                                 params->_momentFieldDefs[ii].longName,
                                 VE_UNITS.c_str(), 
                                 currentSweep->getVelScale() * velScaleBiasFactor, 
                                 currentSweep->getVelBias() * velScaleBiasFactor,
                                 currentSweep->getVelBad(), 
				 currentSweep->getRangeToFirstVelGate(),
				 currentSweep->getCellSpacingVel(),
                                 _ve, 
                                 _numRays, 
                                 _numCellsVel );
	    } else {
	      POSTMSG( ERROR, "No %s data avaialable for file %s",
		       VE_NAME.c_str(), fileName.c_str());
	    }
	    break;
             
          case Params::SW :

	    if(_haveSw && _numCellsVel != 0) {
             status = addNewVar( SW_NAME.c_str(), 
                                 timeDim, 
                                 maxCellsVelDim, 
                                 params->_momentFieldDefs[ii].longName,
                                 SW_UNITS.c_str(), 
                                 currentSweep->getSwScale() * velScaleBiasFactor, 
                                 currentSweep->getSwBias() * velScaleBiasFactor,
                                 currentSweep->getSwBad(), 
				 currentSweep->getRangeToFirstVelGate(),
				 currentSweep->getCellSpacingVel(),
                                 _sw, 
                                 _numRays, 
                                 _numCellsVel );
	    } else {
	      POSTMSG( ERROR, "No %s data avaialable for file %s",
		       SW_NAME.c_str(), fileName.c_str());
	    }
	    break;

          case Params::ZDR :

	    if(_haveZdr && _numCellsVel != 0) {
             status = addNewVar( ZDR_NAME.c_str(), 
                                 timeDim, 
                                 maxCellsVelDim, 
                                 params->_momentFieldDefs[ii].longName,
                                 ZDR_UNITS.c_str(), 
                                 currentSweep->getZdrScale(), 
                                 currentSweep->getZdrBias(),
                                 currentSweep->getZdrBad(), 
				 currentSweep->getRangeToFirstVelGate(),
				 currentSweep->getCellSpacingVel(),
                                 _zdr, 
                                 _numRays, 
                                 _numCellsVel );
	    } else {
	      POSTMSG( INFO, "No %s data avaialable for file %s",
		       ZDR_NAME.c_str(), fileName.c_str());
	    }
	    break;
             
          case Params::PHI :

	    if(_havePhi && _numCellsVel != 0) {
             status = addNewVar( PHI_NAME.c_str(), 
                                 timeDim, 
                                 maxCellsVelDim, 
                                 params->_momentFieldDefs[ii].longName,
                                 PHI_UNITS.c_str(), 
                                 currentSweep->getPhiScale(), 
                                 currentSweep->getPhiBias(),
                                 -999.99, 
				 currentSweep->getRangeToFirstVelGate(),
				 currentSweep->getCellSpacingVel(),
                                 _phi, 
                                 _numRays, 
                                 _numCellsVel );
	    } else {
	      POSTMSG( INFO, "No %s data avaialable for file %s",
		       PHI_NAME.c_str(), fileName.c_str());
	    }
	    break;
             
          case Params::RHO :

	    if(_haveRho && _numCellsVel != 0) {
             status = addNewVar( RHO_NAME.c_str(), 
                                 timeDim, 
                                 maxCellsVelDim, 
                                 params->_momentFieldDefs[ii].longName,
                                 RHO_UNITS.c_str(), 
                                 currentSweep->getRhoScale(), 
                                 currentSweep->getRhoBias(),
                                 currentSweep->getRhoBad(), 
				 currentSweep->getRangeToFirstVelGate(),
				 currentSweep->getCellSpacingVel(),
                                 _rho, 
                                 _numRays, 
                                 _numCellsVel );
	    } else {
	      POSTMSG( INFO, "No %s data avaialable for file %s",
		       RHO_NAME.c_str(), fileName.c_str());
	    }
	    break;

          case Params::CMAP :


	    if(clutterSeg == NULL) {
	      POSTMSG( ERROR, "No %s data avaialable for file %s",
		       CMAP_NAME.c_str(), fileName.c_str());
	    } else {
	      status = addClutterVar( CMAP_NAME.c_str(), 
				  radialDim, 
				  twentyDim, 
				  twoDim,
				  (short*) clutterSeg,
				  360, 20, 2 );
	    }
	      break;

          case Params::BMAP :

	    if(bypassSeg == NULL) {
	      POSTMSG( ERROR, "No %s data avaialable for file %s",
		       BMAP_NAME.c_str(), fileName.c_str());
	    } else {
	      status = addBypassVar( BMAP_NAME.c_str(), 
				  radialDim, 
				  thirtytwoDim, 
				  (short*) bypassSeg,
				  360, 32 );
	    }
	    break;

          default:
             continue;
             break;
      }
      
      if( status != ALL_OK ) {
	delete ncFile;
	ncFile = NULL;
	deleteCurrentFileName();
	return( FAILURE );
      }

   } 

   for( int ii = 0; ii < params->derivedFieldDefs_n; ii++ ) {

      status_t status = ALL_OK;
      
      switch( params->_derivedFieldDefs[ii].outputField ) {
          case Params::SNR :

	    if(_haveSnr && _numCellsRefl != 0) {
	      status = addNewVar( SNR_NAME.c_str(), 
                                 timeDim, 
                                 maxCellsReflDim, 
                                 params->_derivedFieldDefs[ii].longName,
                                 SNR_UNITS.c_str(), 
                                 currentSweep->getSnrScale(), 
                                 currentSweep->getSnrBias(),
                                 currentSweep->getSnrBad(), 
				 currentSweep->getRangeToFirstReflGate(),
				 currentSweep->getCellSpacingRefl(),
                                 _snr, 
                                 _numRays, 
                                 _numCellsRefl );
	    } else {
	      POSTMSG( ERROR, "No %s data avaialable for file %s",
		       SNR_NAME.c_str(), fileName.c_str());
	    }
	    break;
             
          case Params::PR :

	    if(_havePr && _numCellsVel != 0) {
	      status = addNewVar( PR_NAME.c_str(), 
				 timeDim, 
                                 maxCellsReflDim, 
                                 params->_derivedFieldDefs[ii].longName,
                                 PR_UNITS.c_str(), 
                                 currentSweep->getPrScale(), 
                                 currentSweep->getPrBias(),
                                 currentSweep->getPrBad(), 
				 currentSweep->getRangeToFirstReflGate(),
				 currentSweep->getCellSpacingRefl(),
                                 _pr, 
                                 _numRays, 
                                 _numCellsRefl );
	    } else {
	      POSTMSG( ERROR, "No %s data avaialable for file %s",
		       PR_NAME.c_str(), fileName.c_str());
	    }
	    break;
             
          case Params::REC :

	    if(_haveRec && _numCellsVel != 0) {
             status = addNewVar( REC_NAME.c_str(), 
                                 timeDim, 
                                 maxCellsVelDim, 
                                 params->_derivedFieldDefs[ii].longName,
                                 REC_UNITS.c_str(), 
                                 currentSweep->getRecScale(), 
                                 currentSweep->getRecBias(),
                                 currentSweep->getRecBad(), 
				 currentSweep->getRangeToFirstVelGate(),
				 currentSweep->getCellSpacingVel(),
                                 _rec, 
                                 _numRays, 
                                 _numCellsVel );
	    } else {
	      POSTMSG( ERROR, "No %s data avaialable for file %s",
		       REC_NAME.c_str(), fileName.c_str());
	    }
	    break;
             

          default:
             continue;
             break;
      }
      
      if( status != ALL_OK ) {
	delete ncFile;
	ncFile = NULL;
	deleteCurrentFileName();
	return( FAILURE );
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
   ldata.setWriter( "Nexrad2Netcdf" );
   ldata.setDataFileExt( "nc" );
   ldata.setDataType( "nc" );
   ldata.write( baseTime );
   
   POSTMSG( DEBUG, "Wrote file %s", fileName.c_str() );
   
   return( ALL_OK );
}

void NcOutput::deleteCurrentFileName()
{
  remove( filePath.getPath().c_str() );
}

status_t NcOutput::createFile( float fixedAngle ) 
{
   //
   // Set the file name
   //
   if( setFileName( fixedAngle ) != ALL_OK ) {
      return( FAILURE );
   }
   
   //
   // Use the cdl file to create a new file
   //
   char *command_string = new char[1000];
   sprintf(command_string, "ncgen -b -o %s %s", filePath.getPath().c_str(), params->cdlPath);
   int ret = system(command_string);
   delete [] command_string;
   if( ret != 0 ) {
      POSTMSG( ERROR, "Could not create file %s using cdl file %s", 
               fileName.c_str(), params->cdlPath );
      return( FAILURE );
   }
   /*
   if( NCF_cdl2nc( params->cdlPath, filePath.getPath().c_str() ) != 0 ) {
      POSTMSG( ERROR, "Could not create file %s using cdl file %s", 
               fileName.c_str(), params->cdlPath );
      return( FAILURE );
   }
   */
   //
   // Set up the new file
   //
   ncFile = new Nc3File( filePath.getPath().c_str(), Nc3File::Write );
   if( !ncFile || !ncFile->is_valid() ) {
      POSTMSG( ERROR, "Could not open file %s", fileName.c_str() );
      return( FAILURE );
   }

   return( ALL_OK );
}


status_t NcOutput::setFileName( float fixedAngle ) 
{
   char tmpStr[MAX_FILE_NAME_LEN];

   //
   // Get the time stamp for the current (beam) time.  Set
   // up the date directory if necessary.
   //
   DateTime tStamp( baseTime );

   filePath.setDirectory(outputPath , tStamp.getYear(), tStamp.getMonth(),
			  tStamp.getDay() );

   //
   // Set up the file name
   //
   int millisec = millisecsPastMidnight % 1000;

   sprintf( tmpStr, "%s_%s_%s_%s.%.3d_%3.1f_SUR_.nc",
            params->baseName, _radarName, 
            tStamp.getDateStrPlain().c_str(),
            tStamp.getTimeStrPlain().c_str(),
            millisec, fixedAngle );
   
   fileName = tmpStr;

   //
   // Set the file path
   //
   filePath.setFile( fileName );

   if( filePath.makeDir() != 0 ) {
     if( filePath.makeDirRecurse() != 0 ) {
       POSTMSG( ERROR, "Could not create date directory %s", filePath.getDirectory().c_str());
       return( FAILURE );
     }
   }

   return( ALL_OK );
}


status_t NcOutput::setFileVals( SweepData* currentSweep ) 
{
   //
   // Add the fields dimension to the file
   //
   int numFields = params->momentFieldDefs_n + params->derivedFieldDefs_n;
   Nc3Dim *fieldsDim = ncFile->add_dim( "fields", numFields );

   if( !fieldsDim || !fieldsDim->is_valid() ) {
      POSTMSG( ERROR, "Could not add fields dimension to file %s",
               fileName.c_str() );
      return( FAILURE );
   }

   //
   // Add the short string dimension to the file
   //
   Nc3Dim *shortStrDim = ncFile->add_dim( "short_string", SHORT_STR_LEN );
   if( !shortStrDim || !shortStrDim->is_valid() ) {
      POSTMSG( ERROR, "Could not add short_string dimension to file %s",
               fileName.c_str() );
      return( FAILURE );
   }

   //
   // Set variables in the netcdf file...
   //
   if( addVar( "volume_start_time", volumeStartTime ) != 0 ) {
      return( FAILURE );
   }
   
   if( addVar( "base_time", baseTime ) != 0 ) {
      return( FAILURE );
   }

   if( addNewVar( "field_names", fieldsDim, shortStrDim, fieldNames, 
                  numFields, SHORT_STR_LEN ) != 0 ) {
      return( FAILURE );
   }

   if( addVar( "Fixed_Angle", currentSweep->getFixedAngle() ) != 0 ) {
      return( FAILURE );
   }

   if( addVar( "Nyquist_Velocity", 
               currentSweep->getNyquistVelocity() ) != 0 ) {
      return( FAILURE );
   }

   if( addVar( "Unambiguous_Range", 
               currentSweep->getUnambiguousRange() ) != 0 ) {
      return( FAILURE );
   }

   if( addVar( "Latitude", _radarLat ) != 0 ) {
      return( FAILURE ); 
   }
   
   if( addVar( "Longitude", _radarLon ) != 0 ) {
      return( FAILURE );
   }

   if( addVar( "Altitude", _radarHeight ) != 0 ) {
      return( FAILURE );
   }
   
   if( addVar( "Radar_Constant", RADAR_CONSTANT ) != 0 ) {
      return( FAILURE );
   }
   
   if( addVar( "rcvr_gain", RECEIVER_GAIN ) != 0 ) {
      return( FAILURE );
   }

   if( addVar( "ant_gain", ANTENNA_GAIN ) != 0 ) {
      return( FAILURE );
   }
   
   if( addVar( "sys_gain", SYSTEM_GAIN ) != 0 ) {
      return( FAILURE );
   }
   
   if( addVar( "bm_width", SweepData::BEAM_WIDTH ) != 0 ) {
      return( FAILURE );
   }
   
   if( addVar( "peak_pwr", XMIT_PEAK_POWER ) != 0 ) {
      return( FAILURE );
   }

   if( addVar( "Wavelength", WAVELENGTH) != 0 ) {
      return( FAILURE );
   }
   
   if( addVar( "PRF", currentSweep->getPrf() ) != 0 ) {
      return( FAILURE );
   }

   if( currentSweep->merged() ) {
      if( addVar( "sur_PRF", currentSweep->getSurPrf() ) != 0 ) {
         return( FAILURE );
      }
   }

   // Variables only found in Msg 31
   if(currentSweep->getDbz0() != -999.0) {
     if( addVar( "dbz0", currentSweep->getDbz0() ) != 0 ) {
       return( FAILURE );
     }

     //if( addVar( "horiz_noise", currentSweep->getHorizNoise(), _numRays) != 0 ) {
     //  return( FAILURE );
     //}

     //if( addVar( "vert_noise", currentSweep->getVertNoise(), _numRays) != 0 ) {
     //  return( FAILURE );
     //}
   }

   RIDDS_VCP_hdr *vcpHdr = currentSweep->getVcpHdr();
   RIDDS_elevation_angle *vcpElev = currentSweep->getVcpElev();
   if(vcpHdr && vcpElev) {
     if( addVar( "channel_config", vcpElev->channel_config ) != 0 ) {
       return( FAILURE );
     }

     if( addVar( "waveform_type", vcpElev->waveform_type ) != 0 ) {
       return( FAILURE );
     }

     if( addVar( "super_res_control", vcpElev->super_res_control ) != 0 ) {
       return( FAILURE );
     }

     if( addVar( "sur_prf_num", vcpElev->surveillance_prf_num ) != 0 ) {
       return( FAILURE );
     }

     if( addVar( "sur_prf_pulse_count", vcpElev->surveillance_prf_pulse_count ) != 0 ) {
       return( FAILURE );
     }

     if( addVar( "dop_prf_num", vcpElev->doppler_prf_number1 ) != 0 ) {
       return( FAILURE );
     }

     if( addVar( "dop_prf_pulse_count", vcpElev->doppler_prf_pulse_count1 ) != 0 ) {
       return( FAILURE );
     }

     if( addVar( "azimuth_rate", (vcpElev->azimuth_rate/8.)*(22.5/2048) ) != 0 ) {
       return( FAILURE );
     }
   } else {

     if( addVar( "sur_prf_pulse_count", currentSweep->getSurPulseCount() ) != 0 ) {
       return( FAILURE );
     }

     if( addVar( "dop_prf_pulse_count", currentSweep->getPulseCount() ) != 0 ) {
       return( FAILURE );
     }

   }

   RIDDS_status_data *statusData = currentSweep->getStatusData();
   if(statusData) {
     if( addVar( "RDA_build_num", statusData->rda_build_num ) != 0 ) {
       return( FAILURE );
     }
   }

   //
   // Write in the global attributes
   //
   addGlobalAtt( currentSweep->getVCP(), currentSweep->merged() );
      
   return( ALL_OK );
}


void NcOutput::addGlobalAtt( int vcp, bool merged ) 
{
  char cbuf[1024];

  //
  // Radar identification
  //
  ncFile->add_att( "Content", "Radar sweep file" );
  ncFile->add_att( "Conventions", "NCAR_ATD-NOAA_ETL/Scanning_Remote_Sensor" );
  ncFile->add_att( "Instrument_Name", _radarName );
  ncFile->add_att( "Site_Name", _siteName );
  ncFile->add_att( "Radar_Id", _radarID );
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
  ncFile->add_att( "Software", "Nexrad2Netcdf" );
  ncFile->add_att( "Range_Segments", "All data has fixed cell spacing" );
  
}

status_t NcOutput::addVar( const char* varName, int value ) 
{
   //
   // Get a pointer to the variable and check it
   //
   Nc3Var *ncVar = ncFile->get_var( varName );
   if( !ncVar || !ncVar->is_valid() ) {
      POSTMSG( ERROR, "Could not get variable %s from file %s", 
               varName, fileName.c_str() );
      return( FAILURE );
   }
   
   //
   // Put the value to that variable and check that
   //
   int status = ncVar->put( &value, 1 );
   if( status == 0 ) {
      POSTMSG( ERROR, "Could not write value to variable %s to file %s",
               varName, fileName.c_str() );
      return( FAILURE );
   } 

   return( ALL_OK );
}


status_t NcOutput::addVar( const char* varName, float value ) 
{
   //
   // Get a pointer to the variable and check it
   //
   Nc3Var *ncVar = ncFile->get_var( varName );
   if( !ncVar || !ncVar->is_valid() ) {
      POSTMSG( ERROR, "Could not get variable %s from file %s", 
               varName, fileName.c_str() );
      return( FAILURE );
   }
   
   //
   // Put the value to that variable and check that
   //
   int status = ncVar->put( &value, 1 );
   if( status == 0 ) {
      POSTMSG( ERROR, "Could not write value to variable %s to file %s",
               varName, fileName.c_str() );
      return( FAILURE );
   } 

   return( ALL_OK );
}


status_t NcOutput::addVar( const char* varName, double value ) 
{
   //
   // Get a pointer to the variable and check it
   //
   Nc3Var *ncVar = ncFile->get_var( varName );
   if( !ncVar || !ncVar->is_valid() ) {
      POSTMSG( ERROR, "Could not get variable %s from file %s", 
               varName, fileName.c_str() );
      return( FAILURE );
   }
   
   //
   // Put the value to that variable and check that
   //
   int status = ncVar->put( &value, 1 );
   if( status == 0 ) {
      POSTMSG( ERROR, "Could not write value to variable %s to file %s",
               varName, fileName.c_str() );
      return( FAILURE );
   } 

   return( ALL_OK );
}


status_t NcOutput::addVar( const char* varName, float* values, long c0 ) 
{
   //
   // Get a pointer to the variable and check it
   //
   Nc3Var *ncVar = ncFile->get_var( varName );
   if( !ncVar || !ncVar->is_valid() ) {
      POSTMSG( ERROR, "Could not get variable %s from file %s", 
               varName, fileName.c_str() );
      return( FAILURE );
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
      return( FAILURE );
   } 

   return( ALL_OK );
}


status_t NcOutput::addVar( const char* varName, double* values, 
                                 long c0 ) 
{
   //
   // Get a pointer to the variable and check it
   //
   Nc3Var *ncVar = ncFile->get_var( varName );
   if( !ncVar || !ncVar->is_valid() ) {
      POSTMSG( ERROR, "Could not get variable %s from file %s", 
               varName, fileName.c_str() );
      return( FAILURE );
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
      return( FAILURE );
   } 

   return( ALL_OK );
}


status_t NcOutput::addNewVar( const char* varName, Nc3Dim* dim1,
                                    Nc3Dim* dim2, char* values, 
                                    long c0, long c1 ) 
{
   //
   // Create the variable
   //
   Nc3Var *newVar = ncFile->add_var( varName, nc3Char, dim1, dim2 );
   if( !newVar || !newVar->is_valid() ) {
      POSTMSG( ERROR, "Could not create %s variable in %s",
               varName, fileName.c_str() );
      return( FAILURE );
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
      return( FAILURE );
   } 

   return( ALL_OK );
}

status_t NcOutput::addNewVar( const char* varName, Nc3Dim* dim1,
                                    Nc3Dim* dim2, ui08* values, 
                                    long c0, long c1 ) 
{
   Nc3Var *newVar = ncFile->add_var( varName, nc3Byte, dim1, dim2 );
   if( !newVar || !newVar->is_valid() ) {
      POSTMSG( ERROR, "Could not create %s variable in %s",
               varName, fileName.c_str() );
      return( FAILURE );
   }

   int status = newVar->put( (ncbyte*)values, c0, c1 );
   if( status == 0 ) {
      POSTMSG( ERROR, "Could not write value to variable %s to file %s",
               varName, fileName.c_str() );
      return( FAILURE );
   } 

   return( ALL_OK );
}

status_t NcOutput::addBypassVar( const char* varName, Nc3Dim* dim1,
                                    Nc3Dim* dim2, short* values, 
                                    long c0, long c1 ) 
{
   Nc3Var *newVar = ncFile->add_var( varName, nc3Short, dim1, dim2 );
   if( !newVar || !newVar->is_valid() ) {
      POSTMSG( ERROR, "Could not create %s variable in %s",
               varName, fileName.c_str() );
      return( FAILURE );
   }

   int ranges[2];
   ranges[0] = 0;
   ranges[1] = 511;

   newVar->add_att( "Range", 2, ranges);

   int status = newVar->put( values, c0, c1 );
   if( status == 0 ) {
      POSTMSG( ERROR, "Could not write value to variable %s to file %s",
               varName, fileName.c_str() );
      return( FAILURE );
   } 

   return( ALL_OK );
}

status_t NcOutput::addClutterVar( const char* varName, Nc3Dim* dim1,
                                    Nc3Dim* dim2, Nc3Dim* dim3, short* values, 
                                    long c0, long c1, long c2 ) 
{
   Nc3Var *newVar = ncFile->add_var( varName, nc3Short, dim1, dim2, dim3 );
   if( !newVar || !newVar->is_valid() ) {
      POSTMSG( ERROR, "Could not create %s variable in %s",
               varName, fileName.c_str() );
      return( FAILURE );
   }

   newVar->add_att( "Attribute1", "op_code");
   newVar->add_att( "Attribute2", "end_range");

   int status = newVar->put( values, c0, c1, c2 );
   if( status == 0 ) {
      POSTMSG( ERROR, "Could not write value to variable %s to file %s",
               varName, fileName.c_str() );
      return( FAILURE );
   } 

   return( ALL_OK );
}

status_t NcOutput::addNewVar( const char* varName, Nc3Dim* dim1,
			      Nc3Dim* dim2, const char* longName,
			      const char* units, double scale,
			      double bias, ui08 badValue,
			      float rangeToFirst, float cellSpacing, 
			      ui08* values, long c0, long c1 ) 
{
   //
   // Get a pointer to the variable and check it
   //
   Nc3Var *ncVar = ncFile->add_var( varName, nc3Byte, dim1, dim2 );
   if( !ncVar || !ncVar->is_valid() ) {
      POSTMSG( ERROR, "Could not add variable %s to file %s", 
               varName, fileName.c_str() );
      return( FAILURE );
   }

   //
   // Add attributes
   //
   ncVar->add_att( "long_name", longName );
   ncVar->add_att( "units", units );
   ncVar->add_att( "scale_factor", scale );
   ncVar->add_att( "add_offset", bias );
   ncVar->add_att( "missing_value", badValue );
   ncVar->add_att( "_FillValue", (ncbyte) badValue );
   ncVar->add_att( "Range_to_First_Cell",  rangeToFirst);
   ncVar->add_att( "Cell_Spacing", cellSpacing);
   
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
   int status = ncVar->put( (ncbyte*)values, c0, c1 );
   if( status == 0 ) {
      POSTMSG( ERROR, "Could not write value to variable %s of file %s",
               varName, fileName.c_str() );
      return( FAILURE );
   } 

   return( ALL_OK );
}

status_t NcOutput::addNewVar( const char* varName, Nc3Dim* dim1,
			      Nc3Dim* dim2, const char* longName,
			      const char* units, double scale,
			      double bias, si16 badValue,
			      float rangeToFirst, float cellSpacing, 
			      si16* values, long c0, long c1 ) 
{
   //
   // Get a pointer to the variable and check it
   //
   Nc3Var *ncVar = ncFile->add_var( varName, nc3Short, dim1, dim2 );
   if( !ncVar || !ncVar->is_valid() ) {
      POSTMSG( ERROR, "Could not add variable %s to file %s", 
               varName, fileName.c_str() );
      return( FAILURE );
   }

   //
   // Add attributes
   //
   ncVar->add_att( "long_name", longName );
   ncVar->add_att( "units", units );
   ncVar->add_att( "scale_factor", scale );
   ncVar->add_att( "add_offset", bias );
   ncVar->add_att( "missing_value", badValue );
   ncVar->add_att( "_FillValue", (short) badValue );
   ncVar->add_att( "Range_to_First_Cell",  rangeToFirst);
   ncVar->add_att( "Cell_Spacing", cellSpacing);
   
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
   int status = ncVar->put( (short*)values, c0, c1 );
   if( status == 0 ) {
      POSTMSG( ERROR, "Could not write value to variable %s of file %s",
               varName, fileName.c_str() );
      return( FAILURE );
   } 

   return( ALL_OK );
}

void quicksort(float *a, int *b, int lo, int hi)
{
//  lo is the lower index, hi is the upper index
//  of the region of array a that is to be sorted
    int i=lo, j=hi, h;
    float g;
    float x = a[(lo+hi)/2];

    //  partition
    do
    {    
        while (a[i] < x) i++; 
        while (a[j] > x) j--;
        if (i <= j)
        {
            g = a[i];
	    a[i] = a[j];
	    a[j] = g;
	    h = b[i];
	    b[i] = b[j];
	    b[j] = h;
            i++;
	    j--;
        }
    } while (i <= j);

    //  recursion
    if (lo < j) quicksort(a, b, lo, j);
    if (i < hi) quicksort(a, b, i, hi);
}

void NcOutput::assemble( SweepData& currentSweep) 
{
  vector< Beam* >& beams = currentSweep.getBeams();

  _numCellsVel  = currentSweep.getNVelGates();
  _numCellsRefl = currentSweep.getNReflGates();

  bool calcSnr = currentSweep.getCalcSnr();
  bool calcPr = currentSweep.getCalcPr();
  bool calcRec = currentSweep.getCalcRec();

  if( params->useRangeCutoff ) {
    
    float range       = 
      rangeTable.getRange( currentSweep.getFixedAngle() ) * KM_TO_M;
    float cellSpacingVel  = currentSweep.getCellSpacingVel();
    float cellSpacingRefl = currentSweep.getCellSpacingRefl();
    
    if(cellSpacingVel != 0)
      _numCellsVel = min( (int) ( range / cellSpacingVel + 0.5 ), _numCellsVel );
    if(cellSpacingRefl != 0)
      _numCellsRefl = min( (int) ( range / cellSpacingRefl + 0.5 ), _numCellsRefl );
  }

  _numRays  = (int) beams.size();

  clear();  // Deletes all data arrays, sets to NULL

  //
  // Create data arrays that we will need
  int *index = new int[_numRays];
  _azimuth   = new float[_numRays];
  _elevation = new float[_numRays];
  _dataTime  = new double[_numRays];

  _hNoise    = new float[_numRays];
  _vNoise    = new float[_numRays];

  for( int ii = 0; ii < params->momentFieldDefs_n; ii++ ) {
    
    if(params->_momentFieldDefs[ii].outputField == Params::DZ && _numCellsRefl != 0)
      _dz      = new ui08[_numRays*_numCellsRefl];
    
    if(_numCellsVel != 0) {
      switch( params->_momentFieldDefs[ii].outputField ) {
      case Params::VE :
	_ve    = new ui08[_numRays*_numCellsVel];
	break;
      case Params::SW :
	_sw    = new ui08[_numRays*_numCellsVel];
	break;
      case Params::ZDR :
	_zdr   = new ui08[_numRays*_numCellsVel];
	break;
      case Params::PHI :
	_phi   = new si16[_numRays*_numCellsVel];
	break;
      case Params::RHO :
	_rho   = new ui08[_numRays*_numCellsVel];
	break;
      case Params::DZ :
      case Params::CMAP :
      case Params::BMAP :
	break;
      }
    }

  }
  for( int ii = 0; ii < params->derivedFieldDefs_n; ii++ ) {
    
    if(params->_derivedFieldDefs[ii].outputField == Params::REC && _numCellsVel != 0 && calcRec)
      _rec      = new ui08[_numRays*_numCellsVel];
    
    if(_numCellsRefl != 0) {
      if(params->_derivedFieldDefs[ii].outputField == Params::SNR && calcSnr)
	  _snr  = new ui08[_numRays*_numCellsRefl];
      if(params->_derivedFieldDefs[ii].outputField ==  Params::PR && calcPr)
	  _pr   = new ui08[_numRays*_numCellsRefl];
    }
    
  }

  ui08 *dzPtr    = _dz;
  ui08 *vePtr    = _ve;
  ui08 *swPtr    = _sw;
  ui08 *zdrPtr   = _zdr;
  si16 *phiPtr   = _phi;
  ui08 *rhoPtr   = _rho;
  ui08 *snrPtr   = _snr;
  ui08 *prPtr    = _pr;
  ui08 *recPtr   = _rec;
  const ui08 *dataPtr;

  //
  // Sort the Azimuths so we can save the data in order
  for( int i = 0; i < _numRays; i++ ) {
    _azimuth[i]   = beams[i]->getAzimuth();
    index[i] = i;
  }
  quicksort(_azimuth, index, 0, _numRays -1);

  _haveDz  = false;
  _haveVe  = false;
  _haveSw  = false;
  _haveDz  = false;
  _haveZdr = false;
  _havePhi = false;
  _haveRho = false;
  _havePr  = false;
  _haveRec = false;
  
  //
  // Copy the data over one beam at a time
  for( int i = 0; i < _numRays; i++ ) {
    _azimuth[i]   = beams[index[i]]->getAzimuth();
    _elevation[i] = beams[index[i]]->getElevation();
    _dataTime[i]  = beams[index[i]]->getTime() - baseTime;

    _hNoise[i] = beams[index[i]]->getHorizNoise();
    _vNoise[i] = beams[index[i]]->getVertNoise();

    if(dzPtr) {
      dataPtr = beams[index[i]]->getDbz();
      if(dataPtr) {
	memcpy((void *) dzPtr, (void *) dataPtr, sizeof(ui08) * _numCellsRefl );
	_haveDz = true;
      } else {
	memset((void *) dzPtr, Beam::DBZ_BAD, _numCellsRefl);
      }
      dzPtr    += _numCellsRefl;
    }

    if(vePtr) {
      dataPtr = beams[index[i]]->getVel();
      if(dataPtr) {
	memcpy((void *) vePtr, (void *) dataPtr, sizeof(ui08) * _numCellsVel );
	_haveVe = true;
      } else {
	memset((void *) vePtr, Beam::VEL_BAD, _numCellsVel);
      }
      vePtr    += _numCellsVel;
    }

    if(swPtr) {
      dataPtr = beams[index[i]]->getWidth();
      if(dataPtr) {
	memcpy((void *) swPtr, (void *) dataPtr, sizeof(ui08) * _numCellsVel );
	_haveSw = true;
      } else {
	memset((void *) swPtr, Beam::SW_BAD, _numCellsVel);
      }
      swPtr    += _numCellsVel;
    }

    if(zdrPtr) {
      dataPtr = beams[index[i]]->getZdr();
      if(dataPtr) {
	memcpy((void *) zdrPtr, (void *) dataPtr, sizeof(ui08) * _numCellsVel );
	_haveZdr = true;
      } else {
	memset((void *) zdrPtr, Beam::ZDR_BAD, _numCellsVel);
      }
      zdrPtr    += _numCellsVel;
    }

    if(phiPtr) {
      const si16 *dataPtr = beams[index[i]]->getPhi();
      if(dataPtr) {
	memcpy((void *) phiPtr, (void *) dataPtr, sizeof(si16) * _numCellsVel );
	_havePhi = true;
      } else {
	memset((void *) phiPtr, Beam::PHI_BAD, _numCellsVel*2);
      }
      phiPtr    += _numCellsVel;
    }

    if(rhoPtr) {
      dataPtr = beams[index[i]]->getRho();
      if(dataPtr) {
	memcpy((void *) rhoPtr, (void *) dataPtr, sizeof(ui08) * _numCellsVel );
	_haveRho = true;
      } else {
	memset((void *) rhoPtr, Beam::RHO_BAD, _numCellsVel);
      }
      rhoPtr    += _numCellsVel;
    }

    if(calcSnr && snrPtr) {
      dataPtr = beams[index[i]]->getSnr();
      if(dataPtr) {
	memcpy((void *) snrPtr, (void *) dataPtr, sizeof(ui08) * _numCellsRefl );
	_haveSnr = true;
      } else {
	memset((void *) snrPtr, Beam::SNR_BAD, _numCellsRefl);
      }
      snrPtr   += _numCellsRefl;
    }

    if(calcPr && prPtr) {
      dataPtr = beams[index[i]]->getPr();
      if(dataPtr) {
	memcpy((void *) prPtr, (void *) dataPtr, sizeof(ui08) * _numCellsRefl );
	_havePr = true;
      } else {
	memset((void *) prPtr, Beam::PR_BAD, _numCellsRefl);
      }
      prPtr    += _numCellsRefl;
    }

    if(calcRec && recPtr) {
      dataPtr = beams[index[i]]->getRec();
      if(dataPtr) {
	memcpy((void *) recPtr, (void *) dataPtr, sizeof(ui08) * _numCellsVel );
	_haveRec = true;
      } else {
	memset((void *) recPtr, Beam::REC_BAD, _numCellsVel);
      }
      recPtr   += _numCellsVel;
    }

  }

  delete[] index;
}
