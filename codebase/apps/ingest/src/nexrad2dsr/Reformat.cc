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
////////////////////////////////////////////////////////////////////////////////
//
//  Working class for converting a nexrad data buffer to Dsr format
//
//  Terri Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  November 2001
//
//  $Id: Reformat.cc,v 1.17 2016/03/07 01:23:10 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////

#include <math.h>
#include <ctime>
#include <rapformats/swap.h>

#include "Driver.hh"
#include "Reformat.hh"
using namespace std;

const double Reformat::SPEED_OF_LIGHT = 299792458; // m/s
const size_t Reformat::GATE_RATIO     = 4;

Reformat::Reformat()
{
   firstCall          = true;
   noiseAt100km       = 0.0;
   prevScanType       = -1;
   prevTiltNum        = -1;
   referenceTime      = -1;
   numGates           = 0;
   numFields          = 0;
   volumeNum          = 1;
   reformattedDataLen = 0;
   overrideDataTime   = false;
   sequentialTiltNumber = 0;
   specifiedGateSpacing = -1.0;

   return;

}

int
Reformat::init( Params& params )
{
   POSTMSG( DEBUG, "Initializing the reformatter" );

   //
   // Initialize table of NEXRAD scan types, 
   // a.k.a. VCP's, scan strategies, etc.
   //
   if ( scanTable.init( params ) != 0 ) {
      return( -1 );
   }

   //
   // Initialize table of requested NEXRAD data fields
   //
   if ( fieldTable.init( params ) != 0 ) {
      return( -1 );
   }

   //
   // Hang onto other relevant stuff
   //
   noiseAt100km     = params.noise_dbz_at_100km;
   overrideDataTime = params.use_wallclock_time;
   offsetDataTime   = params.offset_sec;
   overrideInputTiltNumbers = params.overrideInputTiltNumbers;
   specifiedGateSpacing = params.specifiedGateSpacing;

   return( 0 );
}

Status::info_t
Reformat::nexrad2dsr( ui08* nexradBuffer, DsRadarMsg& dsrMsg, 
                      bool volTitleSeen )
{
   //
   // We can assume at this point, that we are starting with a
   // DsRadarMsg which should not be cleared out before setting values.
   // The DataMgr has taken care of making sure that the initial state 
   // of the radar message is set properly in DataMgr::shiftRadarMsg().
   //
   RIDDS_data_hdr *nexradData;

   //
   // Hang onto arguments and their derivatives since we will need them
   // all throughout the reformatting
   // 
   volumeTitleSeen = volTitleSeen;

   //
   // Swap the nexradBuffer in place
   // NOTE: the old ridds2mom made a copy first, do we need to?
   //
   nexradData = (RIDDS_data_hdr*)nexradBuffer;
   BE_to_RIDDS_data_hdr( nexradData );

   /*-------------------------------------
   POSTMSG (DEBUG, "Azimuth : %lf", (double(nexradData->azimuth)/8)*180.0/4096.0);
   POSTMSG (DEBUG, "Elevation : %lf", (double(nexradData->elevation)/8)*180.0/4096.0);
   POSTMSG (DEBUG, "VCP : %d", (int)nexradData->vol_coverage_pattern );

   switch (nexradData->radial_status){

   case 0 :
     POSTMSG (DEBUG, "start of new elevation");
     break;

   case 1 :
     POSTMSG (DEBUG, "intermediate radial");
     break;

   case 2 :
     POSTMSG (DEBUG, "end of elevation");
     break;

   case 3 :
     POSTMSG (DEBUG, "start of volume");
     break;

   case 4 :
     POSTMSG (DEBUG, "end of volume");
     break;

   default :
     POSTMSG (DEBUG, "Unknown radial status : %d\n", nexradData->radial_status);
     break;

   }
   ---------------------------------*/

   //
   // First time around, set up the SNR lookup table
   // based on the higher-resolution velocity gate spacing,
   //
   if ( firstCall && fieldTable.isRequested( FieldTable::SNR_DATA ) ) {
      initSnrLut( (double)nexradData->vel_gate_width / 1000. );
   }

   //
   // Determine which data are available in the stream
   //
   setDataInStream( nexradData );

   //
   // Set the Dsr radar parameters, based on inStream data
   //
   setRadarParams( nexradData, dsrMsg );

   //
   // Set the radar beam characteristics based on radar parameters
   // We defer setting the actual beam data until after 
   // the field parameters are set below
   //
   setRadarBeam( nexradData, dsrMsg );

   //
   // Set the radar flags based on radar beam characteristics
   //
   setRadarFlags( nexradData, dsrMsg );

   //
   // Determine which data are stored based on radar flags
   //
   setDataStored( nexradData, dsrMsg );

   //
   // Set the field params based on data availabiliity
   //
   setFieldParams( nexradData, dsrMsg );

   //
   // Now that we have all the information we need,
   // set the actual beam data into the the Dsr radar beam
   //
   setBeamData( nexradData, dsrMsg );

   //
   // That's all folks
   //
   firstCall = false;
   return( Status::ALL_OK );
}

void
Reformat::setDataInStream( RIDDS_data_hdr* nexradData )
{
   //
   // Clear out the previous list of available field data
   //
   fieldTable.clearAvailable();

   //
   // Check for velocity data in the stream
   //
   if ( nexradData->vel_ptr > 0 ) {

      fieldTable.setInStream( FieldTable::VEL_DATA );

      //
      // Set the scale and bias properly
      //
      if ( nexradData->velocity_resolution == 2 ) {
         //
         // 0.5 m/s resolution which is the default
         //
         fieldTable.setScaleBias( FieldTable::VEL_DATA, 1.0 );
      }
      else {
         //
         // 1.0 m/s resolution which is double the default
         //
         fieldTable.setScaleBias( FieldTable::VEL_DATA, 2.0 );
      }

      //
      // We should get spectrum width whenever we get velocity data.
      // Check that this is the case.
      //
      if (!( nexradData->sw_ptr > 0 )){
	POSTMSG(ERROR,"Encountered velocity data sans spectral width");
	exit(-1);
      }
      fieldTable.setInStream( FieldTable::SPW_DATA );
   }

   //
   // Check for reflectivity data in the stream
   //
   if ( nexradData->ref_ptr > 0 ) {
      fieldTable.setInStream( FieldTable::DBZ_DATA );

      //
      // We will derive signal to noise data and effectively
      // add it to the stream whenever we have reflectivity data
      //
      fieldTable.setInStream( FieldTable::SNR_DATA );
   }

   return;

}

void
Reformat::setRadarParams( RIDDS_data_hdr* nexradData, DsRadarMsg& dsrMsg )
{
   //
   // Transfer from the input stream only transient radar parameters.
   // All others are either user-specified or application-specific
   // and were already initialized in the DataMgr.
   //
   DsRadarParams& p = dsrMsg.getRadarParams();

   //
   // Nyquist velocity: si16 scaled to m/s
   //
   p.unambigVelocity = ((float)nexradData->nyquist_vel) / 100;  

   //
   // Unambiguous range: si16 scaled to km
   //
   p.unambigRange = ((float)nexradData->unamb_range_x10) / 10;

   //
   // PRF: derived from unambiguous range prf = c/2r
   //      where prf:(Hz), c:speed of light(m/s), r:unambig range(km -> m)
   //
   p.pulseRepFreq   = (float)(SPEED_OF_LIGHT / (2 * p.unambigRange * 1000));

   //
   // Scan Type: a.k.a. VCP
   //
   p.scanType     = nexradData->vol_coverage_pattern;
   p.scanTypeName = scanTable.getName( p.scanType );

   //
   // Pulse width: varies depending upon scan type
   //
   p.pulseWidth = scanTable.getPulseWidth( p.scanType );
   //
   //
   // Use the higher resolution velocity gate spacing, if available
   //
   if ( fieldTable.isInStream( FieldTable::VEL_DATA )) {
      p.numGates    = nexradData->vel_num_gates;
      p.gateSpacing = nexradData->vel_gate_width / 1000.0;
      p.startRange  = nexradData->vel_gate1 / 1000.0;
   }
   else {
      //
      // No velocity data, use the reflectivity gate characteristics
      //
      p.numGates    = nexradData->ref_num_gates;
      p.gateSpacing = nexradData->ref_gate_width / 1000.0;
      p.startRange  = nexradData->ref_gate1 / 1000.0;
   }

   // If the user has specified a gate spacing, use that instead
   if (specifiedGateSpacing > 0)
     p.gateSpacing = specifiedGateSpacing;

   //
   // Hang onto the number of gates, we use it a lot later on
   //
   numGates  = p.numGates;

   if (fieldTable.isInStream( FieldTable::DBZ_DATA ))
     refNumGates = nexradData->ref_num_gates;

   return;
   
}

void
Reformat::setRadarBeam( RIDDS_data_hdr* nexradData, DsRadarMsg& dsrMsg )
{
   DsRadarBeam& b = dsrMsg.getRadarBeam();

   if ( overrideDataTime ) {
      b.dataTime = time( NULL );
   }
   else {
      b.dataTime = ((nexradData->julian_date - 1) * 86400 +
                     nexradData->millisecs_past_midnight / 1000);
   }

   b.dataTime += offsetDataTime;

   //
   // Time at beginning of volume established in setRadarFlags()
   //
   b.referenceTime = referenceTime;

   //
   // 1-based index [1-100,000] established in setRadarFlags()
   //
   b.volumeNum = volumeNum;

   //
   // 1-based index [1-25]
   //
   if (overrideInputTiltNumbers){
     b.tiltNum = sequentialTiltNumber;
   } else {
     b.tiltNum = nexradData->elev_num;
   } 
   //
   // Degrees [0-359.95]
   //
   b.azimuth = (nexradData->azimuth / 8.) * (180. / 4096.);

   //
   // Actual elevation
   //
   b.elevation = (nexradData->elevation / 8.) * (180. / 4096.);

   //
   // The target elevation is derived from the VCP and the tiltNum
   //
   b.targetElev = scanTable.getElev( nexradData->vol_coverage_pattern,
                                     nexradData->elev_num );

   return;

}

void
Reformat::setRadarFlags( RIDDS_data_hdr* nexradData, DsRadarMsg& dsrMsg )
{
   DsRadarParams& p = dsrMsg.getRadarParams();
   DsRadarBeam&   b = dsrMsg.getRadarBeam();
   DsRadarFlags&  f = dsrMsg.getRadarFlags();

   //
   // Get some basic info from the input radar beam and params
   // NOTE: The radar beam and radar parameters must be established
   //       first in order to set the flags properly.
   //
   f.time      = b.dataTime;
   f.tiltNum   = b.tiltNum;
   f.volumeNum = b.volumeNum;
   f.scanType  = p.scanType;

   //
   // Check for change of tilt condition
   // NOTE: The nexradData radial_status should indicate the start of a new
   //       volume and new tilt, but in some sample data the radial_status 
   //       has not been a reliable trigger, thus we also check tilt number
   //       which does seem to be reliable.
   //
   if ( f.tiltNum != prevTiltNum  ||  
        nexradData->radial_status == START_OF_NEW_ELEVATION ) {
      f.startOfTilt = true;
      prevTiltNum = b.tiltNum;
      sequentialTiltNumber ++;
   }

   //
   // Check for change of volume condition
   // Again, the radial_status is fairly unreliable, so we need other checks
   //
   if ( firstCall  ||  (f.startOfTilt && f.tiltNum == 1) 
                   ||  nexradData->radial_status == BEGINNING_OF_VOL_SCAN 
                   ||  volumeTitleSeen ) {

      f.startOfVolume = true;
      sequentialTiltNumber = 0;

      //
      // Volume number is not included in the nexrad data stream
      // so we generate it ourselves and make it 1-based for consistency
      // with the nexrad elevation indexing.  Let's arbitraily set the upper
      // limit of the range to a tidy 100,000.
      //
      if ( firstCall  ||  volumeNum > 100000 ) {
         volumeNum = 1;
      }
      else {
         volumeNum++;
         f.volumeNum = b.volumeNum = volumeNum;
      }

      //
      // Start of a new volume implies start of a new tilt
      //
      f.startOfTilt = true;
      prevTiltNum = b.tiltNum;

      //
      // Beam reference time is set to the data time at the beginning
      // of each new volume, i.e., the reference time is constant for a volume
      //
      referenceTime = b.dataTime;
   }

   //
   // Check for change of scan strategy
   //
   if ( f.scanType != prevScanType ) {
      f.newScanType = true;
      prevScanType = f.scanType;
   }

   return;

}

void
Reformat::setDataStored( RIDDS_data_hdr* nexradData, DsRadarMsg& dsrMsg )
{
   //
   // Update the state of the stored data whenever 
   // we have a new tilt with only one type of data availability
   //
   DsRadarFlags& flags = dsrMsg.getRadarFlags();
   if ( flags.startOfTilt  ) {
      if ( fieldTable.reflectivityOnly() ) {
         kmTilt.startOfDbz( numGates );
      }
      else if ( fieldTable.velocityOnly() ) {
         kmTilt.startOfVel();
      }
   }

   //
   // Now that the state of the stored data has been adjusted as necessary,
   // check to see if we have stored data available
   //
   if ( kmTilt.isAvailable() ) {
      fieldTable.setStored( FieldTable::DBZ_DATA );

      //
      // signal to noise data is stored whenerver we have reflectivity data   
      //
      fieldTable.setStored( FieldTable::SNR_DATA );
   }

   return;

}

void
Reformat::setFieldParams( RIDDS_data_hdr* nexradData, DsRadarMsg& dsrMsg )
{
   FieldType *fieldType;
   int        fieldPosition = 0;
   vector<DsFieldParams*> &dsrFields = dsrMsg.getFieldParams();

   //
   // We add to the dsrMsg only those fields which are writable,
   // i.e., fields which are both available and have been requested
   //
   for( size_t i=0; i < fieldTable.size(); i++ ) {
      fieldType = fieldTable[i];
      if ( fieldType->isWritable() ) {
         dsrFields.push_back( fieldType->getFieldParams() );
         fieldType->setPosition( fieldPosition );
         fieldPosition++;
      }
   }

   //
   // Update the dsr msg to reflect the number of fields we have
   // and hang onto that number, we use it a lot when setting
   // the beam data
   //
   dsrMsg.getRadarParams().numFields = numFields = dsrFields.size();

   return;

}

void
Reformat::setBeamData( RIDDS_data_hdr* nexradData, DsRadarMsg& dsrMsg )
{
   //
   // Create our beam data from the nexrad buffer
   //
   convertData( nexradData, dsrMsg );

   //
   // Now, load up the DsRadarBeam from our reformatted data
   //
   DsRadarBeam& beam = dsrMsg.getRadarBeam();
   beam.loadData( reformattedData, reformattedDataLen );

   //   POSTMSG (DEBUG, "Beam data length : %d", reformattedDataLen );

   return;

}

void
Reformat::convertData( RIDDS_data_hdr* nexradData, DsRadarMsg& dsrMsg )
{
   //
   // The length of our reformatted data is the number of gates
   // for each of the fields that we intend to send out with this message
   //
   reformattedDataLen = numFields * numGates;

   //
   // We have to handle the conversion differently depending on
   // the presence of data in the nexrad stream
   //
   if ( fieldTable.reflectivityOnly() ) {
     //     POSTMSG(DEBUG, "Converting DBZ only");
     convertDbz( nexradData, dsrMsg );
   }
   else if ( fieldTable.velocityOnly() ) {
     //     POSTMSG(DEBUG, "Converting VEL only");
     convertVel( nexradData, dsrMsg );
   }
   else {
     //     POSTMSG(DEBUG, "Converting all data");
     convertAll( nexradData );
   }

   return;

}

void
Reformat::convertDbz( RIDDS_data_hdr* nexradData, DsRadarMsg& dsrMsg )
{
   //
   // In this beam we have only reflectivity data at 1000m gate spacing.
   // We will send this data out but also save it for later to be 
   // synthesized into 250m gate spacing.  This allows us to send out 
   // a volume of reflectivity at a constant gate spacing which 
   // is required by some processes downstream
   //
   int fieldPosition;

   if ( fieldTable.isRequested( FieldTable::DBZ_DATA )  ||
        fieldTable.isRequested( FieldTable::SNR_DATA )) {

      //
      // Perform data quality correction on the reflectivity
      // if it's needed for either DBZ or SNR
      //
      ui08* dbzData = ((ui08*)nexradData + nexradData->ref_ptr);
      correctData( dbzData );

      //
      // Reflectivity data at 1000m
      //
      if ( fieldTable.isRequested( FieldTable::DBZ_DATA, &fieldPosition )) {
         interlaceData( dbzData, fieldPosition );
      }

      //
      // Derived signal to noise data at 1000m
      //
      if ( fieldTable.isRequested( FieldTable::SNR_DATA, &fieldPosition )) {
         deriveSnr( dbzData );
         interlaceData( snrData, fieldPosition );
      }

      //
      // Save the 1km DBZ & SNR data for later
      //
      float azimuth = dsrMsg.getRadarBeam().azimuth;
      kmTilt.setData( azimuth, dbzData, snrData );
   }

   return;

}

void
Reformat::convertVel( RIDDS_data_hdr* nexradData, DsRadarMsg& dsrMsg )
{
   //
   // In this beam we have velocity and spw data at 250m gate spacing.
   // If reflectivity and/or snr are needed, 250m data will be duplicated
   // from the previously save 1000m data arrays.
   //
   int fieldPosition;

   //
   // Velocity data at 250m
   //
   if ( fieldTable.isRequested( FieldTable::VEL_DATA, &fieldPosition )) {
      ui08* velData = ((ui08*)nexradData + nexradData->vel_ptr);
      correctData( velData );
      interlaceData( velData, fieldPosition );
   }

   //
   // Spectrum width data at 250m
   //
   if ( fieldTable.isRequested( FieldTable::SPW_DATA, &fieldPosition )) {
      ui08* spwData = ((ui08*)nexradData + nexradData->sw_ptr);
      correctData( spwData );
      interlaceData( spwData, fieldPosition );
   }

   //
   // Make sure we have previously received the 1000m reflectivity data
   // There's always the possibility that the application starts up
   // at a velocityOnly tilt and missed the previous reflectivityOnly tilt.
   // NOTE: kmTilt data is corrected before being stored so it doesn't have
   //       to be corrected but merely replicated at this point.
   //
   if ( kmTilt.isAvailable() ) {

      //
      // We will need the azimuth from the current velocity radar beam
      // to lookup the previously stored 1000m data
      //
      DsRadarBeam& beam = dsrMsg.getRadarBeam();

      //
      // Reflectivity data at 1000m
      //
      if ( fieldTable.isRequested( FieldTable::DBZ_DATA, &fieldPosition )) {
         ui08* dbzData = kmTilt.getDbz( beam.azimuth );
         replicateData( dbzData, fieldPosition );
      }

      //
      // Signal to noise data at 1000m
      //
      if ( fieldTable.isRequested( FieldTable::SNR_DATA, &fieldPosition )) {
         ui08* snr1km = kmTilt.getSnr( beam.azimuth );
         replicateData( snr1km, fieldPosition );
      }

      //
      // If we are using the 1000m data, we override the velocity azimuth
      // with the azimuth from the stored data
      //
      beam.azimuth = kmTilt.getStoredAzimuth( beam.azimuth );
   }
   else if ( dsrMsg.getRadarFlags().startOfTilt ) {
      POSTMSG( DEBUG, "Beginning velocity tilt without associated "
                      "reflectivity data" );
   }

   return;

}

void
Reformat::convertAll( RIDDS_data_hdr* nexradData )
{
   //
   // In this beam we have all of the nexrad data in the stream.
   // The reflectivity is at 1000m the velocity is at 250m gate spacing.
   // So the only difference in how we handle these is whether we
   // interlace (250m) or replicate (1000m) data.
   //
   int fieldPosition;

   //
   // Velocity data
   //
   if ( fieldTable.isRequested( FieldTable::VEL_DATA, &fieldPosition )) {
      ui08* velData = ((ui08*)nexradData + nexradData->vel_ptr);
      correctData( velData );
      interlaceData( velData, fieldPosition );
   }

   //
   // Spectrum width data
   //
   if ( fieldTable.isRequested( FieldTable::SPW_DATA, &fieldPosition )) {
      ui08* spwData = ((ui08*)nexradData + nexradData->sw_ptr);
      correctData( spwData );
      interlaceData( spwData, fieldPosition );
   }

   //
   // Perform data quality correction on the reflectivity 
   // if it's needed for either DBZ or SNR
   //
   ui08* dbzData = ((ui08*)nexradData + nexradData->ref_ptr);

   if ( fieldTable.isRequested( FieldTable::DBZ_DATA )  ||
        fieldTable.isRequested( FieldTable::SNR_DATA )) {
      correctData( dbzData );
   }

   //
   // Reflectivity data
   //
   if ( fieldTable.isRequested( FieldTable::DBZ_DATA, &fieldPosition )) {
      replicateData( dbzData, fieldPosition );
   }

   //
   // Signal to noise data
   //
   if ( fieldTable.isRequested( FieldTable::SNR_DATA, &fieldPosition )) {
      deriveSnr( dbzData );
      replicateData( snrData, fieldPosition );
   }

   return;

}

void
Reformat::correctData( ui08* nexradData ) 
{
   size_t  i;
   ui08    value;

   // POSTMSG (DEBUG, "NUMBER OF GATES : %d", (int)numGates);

   //
   // Correct in place the incoming nexrad data
   //
   for( i=0; i < (size_t)numGates; i++ ) {
      //
      // 0: below SNR, 1: ambiguous range
      //
      value = nexradData[i];
      if ( value == 0  ||  value == 1 ) {
         nexradData[i] = 0;
      }
      else {
         //
         // Subtract 2 to get the unscaled data value
         // since 0 and 1 were used for data quality indicators
         //
         nexradData[i] = value - 2;
      }
   }

   return;

}

void
Reformat::interlaceData( ui08* nexradData, size_t fieldIndex )
{
   //
   // Interlace the nexrad data values into our reformatted data buffer
   // i.e., switch from nexrad field-by-field to Dsr gate-by-gate ordering.
   //
   // Nexrad field-by-field --  field1: g1 g2 g3...  field2: g1 g2 g3...
   // Dsr gate-by-gate      --  gate1:  f1 f2 f3...  gate2:  f1 f2 f3...
   //
   size_t   inIndex, outIndex;

   //
   // Transfer the data from the input buffer to the output buffer
   //
   for( outIndex = fieldIndex, inIndex = 0;
        outIndex < reformattedDataLen;
        outIndex += numFields, inIndex++ ) {

      reformattedData[outIndex] = nexradData[inIndex];
   }

   return;

}

void
Reformat::replicateData( ui08* kmData, size_t fieldIndex )
{
   size_t   inIndex, outIndex, dupCount;
   //size_t   kmDataLen = kmTilt.numGates();
   size_t   kmDataLen = refNumGates;
    
   //
   // Assign the nexrad data values into our reformatted data buffer
   // The incoming nexradData is a continguous array of a single field at
   // 1000m gate spacing which will be duplicated into the outgoing data array
   //
   for( outIndex = fieldIndex, inIndex = 0; 
        outIndex < reformattedDataLen  &&  inIndex < kmDataLen; 
        inIndex++ ) {

      for( dupCount = 0; dupCount < GATE_RATIO; dupCount++ ) {
        reformattedData[outIndex] = kmData[inIndex];
        outIndex += numFields;
      }
   }

   return;

}

void
Reformat::initSnrLut( double gateWidth )
{
   size_t   gateNum;
   double   gateNoise;
   double   logOf100 = log10(100.);
   double   radialDistance;
   float    dbzScale;

   POSTMSG( DEBUG, "Initializing signal to noise lookup table" );

   dbzScale = fieldTable.getField( FieldTable::DBZ_DATA )->getFieldParams()->
                                                           scale;

   for( gateNum=0; gateNum < NEX_MAX_GATES; gateNum++ ) {
      radialDistance = (gateNum + 1) * gateWidth;
      gateNoise = noiseAt100km + 20. * (log10(radialDistance) - logOf100);
      noise[gateNum] = (short) ((gateNoise * 100) / dbzScale);
   }

   return;

}

void
Reformat::deriveSnr( ui08* dbzData )
{
   //
   // Calculate signal to noise from reflectivity
   //
   for( int i=0; i < numGates; i++ ) {
      snrData[i] = dbzData[i] - noise[i];
   }

   return;

}
