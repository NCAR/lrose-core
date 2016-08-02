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
// Ingester
//
///////////////////////////////////////////////////////////////

#include <toolsa/utim.h>
#include <toolsa/umisc.h>
#include <rapformats/ds_radar.h>
#include <toolsa/MsgLog.hh>
#include <Fmq/DsRadarQueue.hh>
#include <rapformats/DsRadarParams.hh>
#include <rapformats/DsFieldParams.hh>
#include <rapformats/DsRadarElev.hh>
#include <rapformats/DsRadarCalib.hh>
#include <Mdv/MdvxField.hh>
#include "Ingester.hh"
#include "Mdv2Dsr.hh"
using namespace std;

Ingester::Ingester( DsRadarQueue& queue, Params& params ) :
  _params(params),
  radarQueue(&queue),
  radarParams( radarMsg.getRadarParams() ),
  radarCalib( radarMsg.getRadarCalib() ),
  fieldParams( radarMsg.getFieldParams() ),
  radarBeam( radarMsg.getRadarBeam() )
{
   //
   // Set up - copy some stuff we will need out of the params struct,
   // and reset initial counts.
   //
   useCurrent      = params.use_current_time == TRUE ? true : false;
   volumeNum       = -1;
   elevations      = NULL;
   beamWait        = params.beam_wait_msecs;
   volumeWait      = params.vol_wait_secs;
   summaryOn       = params.print_summary == TRUE ? true : false;
   summaryInterval = params.summary_interval;
   summaryCount    = 0;
   useFieldHeaders = params.use_field_headers == TRUE ? true : false;
}


Ingester::~Ingester() 
{
   clearFields();
}

void
Ingester::clearFields() 
{
   vector< DsFieldParams* >::iterator it;
   
   for( it = fieldParams.begin(); it != fieldParams.end(); it++ ) {
      delete (*it);
   }
   fieldParams.erase( fieldParams.begin(), fieldParams.end() );
}

int
Ingester::readPlanes( char* fileName ) 
{

   int nPlanes = 0;
   
   //
   // Read the mdv file
   //
   mdvx.clearRead();
   mdvx.setReadPath( fileName );
   mdvx.setReadEncodingType( (Mdvx::encoding_type_t) _params.output_encoding );
   mdvx.setReadCompressionType( Mdvx::COMPRESSION_NONE );

   if( mdvx.readVolume() != 0 ) {
     POSTMSG( ERROR, "Could not read mdv file" );
     cerr << mdvx.getErrStr() << endl;
     return( 0 );
   }

   if (_params.debug) {
     cerr << "Processing file: " << mdvx.getPathInUse() << endl;
   }

   const Mdvx::master_header_t &masterHeader = mdvx.getMasterHeader();
   int nFields = masterHeader.n_fields;
   if (nFields < 1) {
     cerr << "ERROR - Mdv2Dsr::Ingester" << endl;
     cerr << "  No fields found" << endl;
     return -1;
   }

   bool gotElevations = true;
   
   if (useFieldHeaders) {

     //
     // In this case we are not to use the radar params, so
     // fail outright, causing the field headers to be used.
     //

     gotElevations = false;

   } else {

     //
     // In this case, at least try for the radar params.
     //
     if( mdvxRadar.loadFromMdvx( mdvx )) {
       
       gotElevations = false;
       nElevations = 0;
       
     } else {
       
       // Get the elevation angles
       
       if( mdvxRadar.radarElevAvail() ) {
	 DsRadarElev &radarElev = mdvxRadar.getRadarElev();
	 nElevations = radarElev.getNElev();
	 elevations  = radarElev.getElevArray();
       } else {
	 gotElevations = false;
       }
       
     }

   }

   MdvxField *fld = mdvx.getField(0);
   const Mdvx::field_header_t &fhdr = fld->getFieldHeader();
   const Mdvx::vlevel_header_t &vhdr = fld->getVlevelHeader();
   nPlanes = fhdr.nz;
   if (!gotElevations) {
     nElevations = fhdr.nz;
     elevations = (fl32 *) vhdr.level;
   }

   if (_params.info) {
     cerr << "nPlanes: " << nPlanes << endl;
     cerr << "nElevations: " << nElevations << endl;
     for (int i = 0; i < nElevations; i++) {
       cerr << "elev[" << i << "]: " << elevations[i] << endl;
     }
   }

   //
   // Load the radar parameters from the mdv file
   //

   if(mdvxRadar.radarParamsAvail() ) {
     radarParams = mdvxRadar.getRadarParams();
   } else {
     radarParams.horizBeamWidth = 1.0;
     radarParams.vertBeamWidth = 1.0;
     radarParams.pulseRepFreq = 1000.0;
     radarParams.wavelength = 5.0;
     radarParams.unambigVelocity = 1000.0;
     radarParams.unambigRange = 1000.0;
     radarParams.radarType = DS_RADAR_GROUND_TYPE;
   }

   if (radarParams.prfMode == DS_RADAR_PRF_MODE_NOT_SET) {
     radarParams.prfMode = DS_RADAR_PRF_MODE_FIXED;
   }
   if( mdvxRadar.radarCalibAvail() ) {
     radarCalib = mdvxRadar.getRadarCalib();
   }

   return( nPlanes );
} 

int
Ingester::processFile( char* fileName, int startT, int endT ) 
{
   //
   // The following are set to 0 simply to avoid compiler warnings.
   //
   int             nGates=0, nAz=0, nPlanes;
   float           gateSpacing=0.0, deltaAz=0.0;
   float           startRange=0.0, startAz=0.0;
   time_t          startTime, endTime;
   vector<const void *>  fieldData;
   
   //
   // Read the file
   //   
   nPlanes = readPlanes( fileName );
   if( nPlanes <= 0 ) {
      return( FAILURE );
   }

   //
   // Get the master header
   //
   const Mdvx::master_header_t &masterHeader = mdvx.getMasterHeader();

   //
   // If the field grids differ in any way, this
   // processing won't work
   //
   if( masterHeader.field_grids_differ != 0 ) {
      POSTMSG( ERROR, "Field grids differ" );
      return( FAILURE );
   }

   //
   // Get time info
   //
   if( startT == -1 || endT == -1 ) {
      startTime = (time_t) masterHeader.time_begin;
      endTime   = (time_t) masterHeader.time_end;
   }
   else {
      startTime = (time_t) startT;
      endTime   = (time_t) endT;
   }

   //
   // Get the fields and field data
   //
   clearFields();
   int nFields = masterHeader.n_fields;

   // set scan mode

   MdvxField *field0 = mdvx.getFieldByNum(0);
   const Mdvx::field_header_t &fhdr0 = field0->getFieldHeader();
   Mdvx::projection_type_t projType = (Mdvx::projection_type_t) fhdr0.proj_type;
   // Set geometry info
   nGates  = fhdr0.nx;
   gateSpacing = fhdr0.grid_dx;
   startRange  = fhdr0.grid_minx;
   
   nAz = fhdr0.ny;
   deltaAz = fhdr0.grid_dy;
   startAz = fhdr0.grid_miny;
   
   int scanMode = DS_RADAR_SURVEILLANCE_MODE;
   if (fhdr0.proj_type == Mdvx::PROJ_RHI_RADAR) {
     scanMode = DS_RADAR_RHI_MODE;
   } else {
     double scanWidth = nAz * deltaAz;
     if (scanWidth < 340) {
       scanMode = DS_RADAR_SECTOR_MODE;
     }
   }

   for( int i = 0; i < nFields; i++ ) {
      MdvxField *mdvField = mdvx.getFieldByNum( i );

      const Mdvx::field_header_t &fieldHdr = 
         mdvField->getFieldHeader();

      //
      // Set up field params
      //
      DsFieldParams *fparams = 
	new DsFieldParams( fieldHdr.field_name,
			   fieldHdr.units,
			   fieldHdr.scale,
			   fieldHdr.bias, fieldHdr.data_element_nbytes,
			   (int) fieldHdr.missing_data_value );

      fieldParams.push_back( fparams );

      //
      // Get field data
      //
      fieldData.push_back(mdvField->getVol());
      
   }

   //
   // Override some of the radar parameters
   //
   radarParams.numFields   = nFields;
   radarParams.numGates    = nGates;
   radarParams.altitude    = masterHeader.sensor_alt;
   radarParams.latitude    = masterHeader.sensor_lat;
   radarParams.longitude   = masterHeader.sensor_lon;
   radarParams.gateSpacing = gateSpacing;
   radarParams.startRange  = startRange;
   radarParams.scanMode = scanMode;
   if (radarParams.prt == 0) {
     radarParams.prt = 1.0 / radarParams.pulseRepFreq;
   }
   
   if (_params.override_radar_params) {

     radarParams.radarId = _params.radar_params.radarId;
     radarParams.radarType = _params.radar_params.radarType;
     radarParams.samplesPerBeam = _params.radar_params.samplesPerBeam;
     radarParams.scanType = _params.radar_params.scanType;
     radarParams.polarization = _params.radar_params.polarization;
     radarParams.radarConstant = _params.radar_params.radarConstant;
     radarParams.altitude = _params.radar_params.altitude;
     radarParams.latitude = _params.radar_params.latitude;
     radarParams.longitude = _params.radar_params.longitude;
     radarParams.gateSpacing = _params.radar_params.gateSpacing;
     radarParams.startRange = _params.radar_params.startRange;
     radarParams.horizBeamWidth = _params.radar_params.horizBeamWidth;
     radarParams.vertBeamWidth = _params.radar_params.vertBeamWidth;
     radarParams.pulseWidth = _params.radar_params.pulseWidth;
     radarParams.pulseRepFreq = _params.radar_params.pulseRepFreq;
     radarParams.wavelength = _params.radar_params.wavelength;
     radarParams.xmitPeakPower = _params.radar_params.xmitPeakPower;
     radarParams.receiverMds = _params.radar_params.receiverMds;
     radarParams.receiverGain = _params.radar_params.receiverGain;
     radarParams.antennaGain = _params.radar_params.antennaGain;
     radarParams.systemGain = _params.radar_params.systemGain;
     radarParams.unambigVelocity = _params.radar_params.unambigVelocity;
     radarParams.unambigRange = _params.radar_params.unambigRange;
     radarParams.radarName = _params.radar_params.radarName;
     radarParams.scanTypeName = _params.radar_params.scanTypeName;

   }
   

   //
   // Increment volume number
   //

   volumeNum++;

   if( summaryOn ) {
     fprintf( stderr, "\n-------------------> start of volume %d\n", 
	       volumeNum );
   }

   //
   // Set up and write out messages
   //
   int content = 0;
   int beamCount = 0;
   double deltaTime = ((double) (endTime - startTime) / 
		       (double) (nPlanes * nAz));
   double dataTime = startTime;

   //
   // Initialize the radar flags
   //
   DsRadarFlags &radarFlags = radarMsg.getRadarFlags();
   radarFlags.startOfVolume = true;
   radarFlags.startOfTilt   = true;
   radarFlags.endOfTilt     = false;
   radarFlags.endOfVolume   = false;

   radarQueue->putStartOfVolume(volumeNum);
      
   for( int iz = 0; iz < nPlanes; iz++ ) {

      //
      // start a new tilt
      //
      
      if( summaryOn ) {
	 fprintf( stderr, "\n-------------------> start of tilt %d\n", iz );
      }

      radarQueue->putStartOfTilt(iz);

      // put radar params

      content = (int) DsRadarMsg::RADAR_PARAMS;
      content |= (int) DsRadarMsg::RADAR_CALIB;
      content |= (int) DsRadarMsg::FIELD_PARAMS;
      POSTMSG( DEBUG, "=========>>>> Sending params to FMQ");
      if( radarQueue->putDsMsg( radarMsg, content ) != 0 ) {
        POSTMSG( ERROR, "Could not write params to output FMQ" );
        return( FAILURE );
      }
      content = 0;
        
      for( int iy = 0; iy < nAz; iy++ ) {
	
        PMU_auto_register("Handling beam");
     
	if( useCurrent ) 
	  dataTime = time(0);
	 else
	   dataTime = startTime + beamCount*deltaTime;
	
	time_t beamTime = (time_t) dataTime;
	int nanoSecs = (int) ((dataTime - beamTime) * 1.0e9 + 0.5);
	
	radarBeam.dataTime   = beamTime;
	radarBeam.nanoSecs   = nanoSecs;

	radarBeam.volumeNum  = volumeNum;
	radarBeam.tiltNum    = iz;
	
	radarBeam.scanMode = radarParams.scanMode;
	radarBeam.antennaTransition = false;
	radarBeam.nSamples = radarParams.samplesPerBeam;
	radarBeam.beamIsIndexed = true;
	radarBeam.angularResolution = deltaAz;

	if (projType == Mdvx::PROJ_RHI_RADAR) {
	  radarBeam.elevation = startAz + iy * deltaAz;
	  radarBeam.azimuth = elevations[iz];
	  radarBeam.targetAz = elevations[iz];
	} else {
	  radarBeam.azimuth = startAz + iy * deltaAz;
	  radarBeam.elevation = elevations[iz];
	  radarBeam.targetElev = elevations[iz];
	}

	// Add the offset, put on 0..360
	radarBeam.azimuth +=  _params.az_offset;
	do {
	  if (radarBeam.azimuth < 0.0) radarBeam.azimuth += 360.0;
	  if (radarBeam.azimuth > 360.0) radarBeam.azimuth -= 360.0;
	} while ((radarBeam.azimuth < 0.0) || (radarBeam.azimuth > 360.0));

	switch (_params.output_encoding) {

	case Params::ENCODING_INT8: {

	  ui08 *beamData = new ui08[ nFields*nGates ];
	  int fieldNum = 0;
	  vector<const void *>::iterator it;
	  for(it = fieldData.begin(); it != fieldData.end(); it++) {
	    ui08 *data = (ui08 *) *it;
	    ui08 *fieldRow = data + iz*nAz*nGates + iy*nGates;
	    ui08 *beamPtr  = beamData + fieldNum;
	    for(int ix = 0; ix < nGates; ix++) {
	      *beamPtr = *fieldRow;
	      beamPtr += nFields;
	      fieldRow++;
	    }
	    fieldNum++;
	  }

	  radarBeam.loadData(beamData, nFields*nGates, 1);
	  delete[] beamData;

	} break;
	  
	case Params::ENCODING_INT16: {

	  ui16 *beamData = new ui16[ nFields*nGates ];
	  int fieldNum = 0;
	  vector<const void *>::iterator it;
	  for(it = fieldData.begin(); it != fieldData.end(); it++) {
	    ui16 *data = (ui16 *) *it;
	    ui16 *fieldRow = data + iz*nAz*nGates + iy*nGates;
	    ui16 *beamPtr  = beamData + fieldNum;
	    for(int ix = 0; ix < nGates; ix++) {
	      *beamPtr = *fieldRow;
	      beamPtr += nFields;
	      fieldRow++;
	    }
	    fieldNum++;
	  }

	  radarBeam.loadData(beamData, nFields*nGates*2, 2);
	  delete[] beamData;

	} break;
	  
	case Params::ENCODING_FLOAT32: {

 	  fl32 *beamData = new fl32[ nFields*nGates ];
	  int fieldNum = 0;
	  vector<const void *>::iterator it;
	  for(it = fieldData.begin(); it != fieldData.end(); it++) {
	    fl32 *data = (fl32 *) *it;
	    fl32 *fieldRow = data + iz*nAz*nGates + iy*nGates;
	    fl32 *beamPtr  = beamData + fieldNum;
	    for(int ix = 0; ix < nGates; ix++) {
	      *beamPtr = *fieldRow;
	      beamPtr += nFields;
	      fieldRow++;
	    }
	    fieldNum++;
	  }

	  radarBeam.loadData(beamData, nFields*nGates*4, 4);
	  delete[] beamData;

	} break;

	} // switch

	//
	// Send out the message
	//
        POSTMSG( INFO, "====>>>> Sending beam to FMQ");
	content = (int) DsRadarMsg::RADAR_BEAM;
	if( radarQueue->putDsMsg( radarMsg, content ) != 0 ) {
	  POSTMSG( ERROR, "Could not write beam to output FMQ" );
	  return( FAILURE );
	}
	
	if( summaryOn ) {
	  summaryCount++;
	  if( summaryCount == summaryInterval ) {
	    printSummary();
	    summaryCount = 0;
	  }
	}
	
	umsleep( beamWait );
	
	beamCount++;
	radarFlags.startOfTilt = false;
	radarFlags.startOfVolume = false;
	
      } // iy
      
      radarQueue->putEndOfTilt(iz);
      
      //
      // End of tilt
      //
      
      if( summaryOn ) {
	fprintf( stderr, "\n-------------------> end of tilt %d\n", iz );
      }
      
   } // iz
   
   radarQueue->putEndOfVolume(volumeNum);
      
   //
   // End of volume
   //
   if( useCurrent )
      dataTime = (int) (time(0));
   
   if( summaryOn ) {
      fprintf( stderr, "\n-------------------> end of volume %d\n", 
	       volumeNum );
   }

   sleep( volumeWait );
   return( SUCCESS );

}

void
Ingester::printSummary() 
{
   //
   // Print the header
   //
   fprintf( stderr, " Vol Tilt El_tgt El_act     Az Ngat Gspac  PRF     "
	    "Date     Time\n" );
   
   //
   // Parse the time of the beam
   //
   date_time_t  dataTime;
   dataTime.unix_time = radarBeam.dataTime;
   uconvert_from_utime( &dataTime );

   //
   // Print the beam summary
   //
   fprintf(stderr, "%4ld %4ld %6.2f %6.2f %6.2f %4ld %5ld %4ld "
           "%.2ld/%.2ld/%.2ld %.2ld:%.2ld:%.2ld",
           (long) radarBeam.volumeNum,
           (long) radarBeam.tiltNum,
           (double) radarBeam.targetElev,
           (double) radarBeam.elevation,
           (double) radarBeam.azimuth,
           (long) radarParams.numGates,
           (long) (radarParams.gateSpacing * 1000),
           (long) radarParams.pulseRepFreq,
           (long) dataTime.year,
           (long) dataTime.month,
           (long) dataTime.day,
           (long) dataTime.hour,
           (long) dataTime.min,
           (long) dataTime.sec);
   fprintf(stderr, "\n");
   
}



