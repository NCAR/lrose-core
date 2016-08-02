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
//  NetCDF class for radar input stream
//
//  Terri Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  September 2001
//
//  $Id: BinetNetCDF.cc,v 1.8 2016/03/07 01:23:07 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////

#include <netcdf.h>
#include <didss/DsInputPath.hh>

#include "Driver.hh"
#include "Params.hh"
#include "BinetNetCDF.hh"
using namespace std;

const struct BinetNetCDF::fieldInfo_t 
             BinetNetCDF::FIELD_INFO[] = { 
                { Params::DM,     "DM",    "dBm" },
                { Params::DZ,     "DZ",    "dBz" },
                { Params::ZDR,    "ZDR",   "dBm" },
                { Params::PHI,    "PHI",   "dBm" },
                { Params::LDR,    "LDR",   "degrees" },
                { Params::RHOHV,  "RHOHV", "none" },
                { Params::VE,     "VE",    "m/s" },
                { Params::SW,     "SW",    "m/s" }};

BinetNetCDF::BinetNetCDF()
 : inputRadarFlags  ( inputRadarMsg.getRadarFlags()   ),
   inputRadarParams ( inputRadarMsg.getRadarParams()  ),
   inputRadarBeam   ( inputRadarMsg.getRadarBeam()    ),
   inputRadarFields ( inputRadarMsg.getFieldParams()  ),

   outputRadarFlags ( outputRadarMsg.getRadarFlags()  ),
   outputRadarParams( outputRadarMsg.getRadarParams() ),
   outputRadarBeam  ( outputRadarMsg.getRadarBeam()   ),
   outputRadarFields( outputRadarMsg.getFieldParams() )
{
   beamData              = NULL;
   beamDataLen           = 0;
   inputParamsChanged    = false;
   outputParamsChanged   = false;
   ncFile                = NULL;
   fileIsOpen            = false;
   newFileRead           = false;
   firstCall             = true;
   currentVol            = -1;
   currentBeam           = -1;
   currentTilt           = -1;
   currentElev           = -1;
}

BinetNetCDF::~BinetNetCDF()
{
   clearScaling();

   if ( beamData )
      ufree( beamData );
}

void
BinetNetCDF::clearScaling()
{
   //
   // Delete the input scaling for the each field
   //
   for( size_t i=0; i < inputScaling.size(); i++ ) {
      delete( inputScaling[i] );
   } 
   inputScaling.erase( inputScaling.begin(), inputScaling.end() );
}

int
BinetNetCDF::init( Params& params, DsInputPath* trigger )
{
   //
   // Initialize field parameters
   //
   DsFieldParams*  field;
   int             fieldType;

   for( int i=0; i < params.output_fields_n; i++ ) {
      //
      // Make sure the tdrp field types match the static constants of the class
      //
      fieldType = params._output_fields[i].type;
      assert( fieldType == FIELD_INFO[fieldType].type );

      //
      // Create the new radar field
      //
      field = new DsFieldParams( FIELD_INFO[fieldType].name,
                                 FIELD_INFO[fieldType].units,
                                 params._output_fields[i].scale,
                                 params._output_fields[i].bias );
      inputRadarFields.push_back( field );
   }

   inputRadarParams.numFields = inputRadarFields.size();

   //
   // Set application-specific radar characteristics that are constant
   // For now we will assume ground type instrument in surveillance mode.
   //
   inputRadarParams.radarType = DS_RADAR_GROUND_TYPE;
   inputRadarParams.scanMode  = DS_RADAR_SURVEILLANCE_MODE;

   //
   // Set user-specified radar characteristics that are constant
   // These characteristics are not available in the radar stream
   //
   inputRadarParams.radarId      = params.radar_id;
   inputRadarParams.polarization = params.polarization_code;
   inputRadarParams.receiverMds  = params.receiver_mds;
   inputRadarParams.scanType     = params.scan_type;
   inputRadarParams.scanTypeName = params.scan_type_name;

   string fullName = params.radar_name;
   fullName += " at ";
   fullName += params.site_name;
   inputRadarParams.radarName = fullName;

   //
   // Set user-specified radar characteristics that are only defaults
   // These values will be overridden if they are available via input stream
   //
   inputRadarParams.horizBeamWidth = params.horiz_beam_width;
   inputRadarParams.vertBeamWidth  = params.vert_beam_width;

   //
   // See if we need to override the radar location
   //
   if ( params.override_radar_location ) {
      overrideRadarLocation = true;
      inputRadarParams.latitude  = params.radar_location.latitude;
      inputRadarParams.longitude = params.radar_location.longitude;
      inputRadarParams.altitude  = params.radar_location.altitude;
   }
   
   //
   // Max tolerance for radar movement
   //
   maxDeltaElev = params.max_elev_change;

   //
   // Input data file triggering
   //
   assert( trigger != NULL );
   fileTrigger = trigger;

   return( 0 );
}

BinetNetCDF::inputStatus_t
BinetNetCDF::readRadarMsg()
{
   inputStatus_t  status;

   //
   // A full radar message is considered to have
   // radarFlags, radarParams, fieldParams and radarBeam
   // so we can't return until we can provide all of these
   //
   PMU_auto_register( "Reading a radar message from NetCDF" );

   //
   // On the first call we load up two radar messages
   // an inputRadarMsg and outputRadarMsg so that we can determine things
   // like start/end of volume.  The data flow looks like:
   //
   //   NetCDF file -> inputRadarMsg -> outputRadarMsg -> outputRadarQueue
   //                       |_________________|                  |
   //                                |                           |
   //                            buffered                     written
   //                       in BinetNetCDF class         from DataMgr class
   //
   // Thus, on subsequent calls (not the first) we have to shift 
   // the inputRadarMsg to the outputRadarMsg position before
   // setting new inputRadarMsg values from the input stream.
   //
   if ( !firstCall ) {
      shiftRadarMsg();
   }

   //
   // Load up an inputRadarMsg
   //
   status = setRadarMsg();
   if ( status != ALL_OK ) {
      return( status );
   }

   //
   // Priming the pump for the first call by loading up a second RadarMsg
   //
   if ( firstCall ) {
      shiftRadarMsg();
      status = setRadarMsg();
      if ( status != ALL_OK ) {
         return( status );
      }
      firstCall = false;
   }

   return( ALL_OK );
}

void
BinetNetCDF::shiftRadarMsg()
{
   //
   // Since the field parameters never change, we only need to shift
   // them once upon the very first call and never again
   //
   if ( firstCall ) {
      shiftFieldParams();
   }

   //
   // The radar parameter change flag should always shift over.
   // But we only have to shift the actual radarParams when they change.
   //
   // NOTE: we never want to fully clear out the input radar parameters 
   // because of the persistent parameters which were
   // previously established in init().
   //
   outputParamsChanged = inputParamsChanged;
   inputParamsChanged  = false;

   if ( outputParamsChanged ) {
      outputRadarParams = inputRadarParams;
   }

   //
   // The radarBeam changes every time, so we always do a shift
   // For efficiency, we don't bother to clear out the radar beam
   // since all members will be neatly overwritten.
   //
   // TODO: wholesale copies of the beam data is perhaps not an efficient
   //       technique.  maybe we could do much better by providing pointers
   //       that switch between the two radar messages.  That would have
   //       to be compared to the extra overhead of memory offset addressing   
   //
   outputRadarBeam = inputRadarBeam;

   //
   // The radarFlags can change every time, so we always do a shift
   // and make sure to completely clear out the input flags.
   // 
   outputRadarFlags = inputRadarFlags;
   inputRadarFlags.clear();
}

BinetNetCDF::inputStatus_t
BinetNetCDF::setRadarMsg()
{
   inputStatus_t  status;
   bool           validData = false;

   //
   // Pass over any invalid beam data
   //
   while ( !validData ) {

      status = seekValidData();
      switch( status ) {

         case BAD_FILE:
         case END_OF_DATA:
              //
              // Bail out
              //
              return( status );
              break;

         case BAD_DATA:
              //
              // Continue onto the next beam
              //
              nextBeam();
              continue;
              break;

         case ALL_OK:
              //
              // Got the good stuff
              //
              validData = true;
              break;
      }
   }

   //
   // We've arrived at a valid beam.  If this is our first valid beam
   // since opening a new file, get a bit of info for later.
   // We will need these for processing each beam, but we only need
   // to establish them once when we open a new file.
   //
   if ( newFileRead ) {
      newFileRead = false;
      currentVol  = ncFile->get_att( "Volume_Number" )->as_int( 0 );
      baseTime    = ncFile->get_var( "base_time" )->as_long( 0 );

      setDataScaling();
      setRadarParams();
   }

   //
   // Set the radar beam data and radar flags every time around
   //
   setRadarBeam();
   setRadarFlags();

   //
   // Set up for the next beam read
   //
   nextBeam();

   return( status );
}

BinetNetCDF::inputStatus_t
BinetNetCDF::seekValidData()
{
   inputStatus_t  status = ALL_OK;

   //
   // See if we need to open a new NetCDF file
   // 
   if ( !fileIsOpen ) {
      status = openNextFile();
      if ( status == ALL_OK ) {
         //
         // We've got to get the number of beams right away since
         // we test for end-of-file by comparing currentBeam to numBeams
         //
         newFileRead = true;
         currentBeam = 0;
         numBeams    = ncFile->get_var( "Azimuth" )->num_vals();
      }
   }

   if ( status == ALL_OK ) {
      //
      // Make sure the elevation angle is within the specified tolerance
      // of the target elevation.  If it's not the antenna is likely
      // dropping between volumes and we don't want to process the data.
      //
      currentElev = ncFile->get_var( "Elevation" )->as_float( currentBeam );
      targetElev  = ncFile->get_var( "Fixed_Angle" )->as_float( 0 );
      if ( fabs( currentElev - targetElev ) > maxDeltaElev ) {
         status = BAD_DATA;
      }
   }

   return( status );
}

BinetNetCDF::inputStatus_t
BinetNetCDF::openNextFile()
{
   char*  filePath;

   POSTMSG( DEBUG, "Fetching next input data file..." );
   filePath = fileTrigger->next();
   if ( !filePath ) {
      POSTMSG( DEBUG, "Completed processing all input data files." );
      return( END_OF_DATA );
   }

   //
   // Constructor opens file in readOnly mode by default
   //
   POSTMSG( DEBUG, "Attempting to open file '%s'", filePath );
   ncFile = new NcFile( filePath );
   if ( !ncFile->is_valid() ) {
      POSTMSG( ERROR, "Unable to open file '%s'\n%s", filePath, ncErrorMsg());
      return( BAD_FILE );
   }

   //
   // Successfully opened the file
   //
   fileIsOpen = true;

   return( ALL_OK );
}

void
BinetNetCDF::nextBeam()
{
   //
   // If we just processed the last beam in this sweep file,
   // close the file and get rid of it.
   //
   if ( currentBeam >= numBeams - 1 ) {
      ncFile->close();
      delete ncFile;
      fileIsOpen  = false;
      currentBeam = -1;
   }
   else {
      //
      // Increment the beam index for the next time around
      //
      currentBeam++;
   }
}

void
BinetNetCDF::setRadarParams()
{
   //
   // Transfer from the input stream only transient radar parameters.
   // All others are either user-specified or application-specific
   // and were set in init().
   //
   // p is just a shorhand notation for the long lines which follow
   //
   DsRadarParams& p = inputRadarParams;
   p.numGates       = ncFile->get_dim( "maxCells" )->size();
   p.samplesPerBeam = ncFile->get_att( "Num_Samples" )->as_int( 0 );
   p.radarConstant  = ncFile->get_var( "Radar_Constant" )->as_float( 0 );

   p.gateSpacing    = ncFile->get_var( "Cell_Spacing" )->as_float( 0 ) 
                                       / 1000;   // meters to km

   p.startRange     = ncFile->get_var( "Range_to_First_Cell" )->as_float( 0 ) 
                                       / 1000;   // meters to km

   p.pulseWidth     = ncFile->get_var( "pulse_width" )->as_float( 0 )
                                       * 1.0e6;  // seconds to microseconds

   p.pulseRepFreq   = ncFile->get_var( "PRF" )->as_float( 0 );

   p.wavelength     = ncFile->get_var( "Wavelength" )->as_float( 0 )
                                       * 100;    // meters to cm

   p.xmitPeakPower   = ncFile->get_var( "peak_pwr" )->as_float( 0 );
   p.receiverGain    = ncFile->get_var( "rcvr_gain" )->as_float( 0 );
   p.antennaGain     = ncFile->get_var( "ant_gain" )->as_float( 0 );
   p.systemGain      = ncFile->get_var( "sys_gain" )->as_float( 0 );
   p.unambigVelocity = ncFile->get_var( "Nyquist_Velocity" )->as_float( 0 );

   p.unambigRange    = ncFile->get_var( "Unambiguous_Range" )->as_float( 0 )
                                        / 1000;  // meters to km

   //
   // The following radar parameters appear to have undefined values 
   // in the sample files so for these we will use parameter defaults
   // TODO: we should consider checking for missing values for all
   //       of the radar parameters, eh?
   //
   char           *infoName;
   NcVar          *infoVariable;
   float           infoValue, missingValue;

   infoName = "bm_width";
   infoVariable = ncFile->get_var( infoName );
   missingValue = infoVariable->get_att( "missing_value" )->as_float(0);
   infoValue    = infoVariable->as_float( 0 );
   if ( infoValue != missingValue ) {
      p.horizBeamWidth = infoValue;
      p.vertBeamWidth  = infoValue;
   }

   //
   // Set the radar location, if necessary
   //
   if ( !overrideRadarLocation ) {
      p.altitude       = ncFile->get_var( "Altitude" )->as_float( 0 ) 
                                          / 1000;
      p.latitude       = ncFile->get_var( "Latitude" )->as_float( 0 );
      p.longitude      = ncFile->get_var( "Longitude" )->as_float( 0 );
   }

   //
   // See if there's been any change in the radar parameter settings
   //
   if ( inputRadarParams == outputRadarParams ) {
      inputParamsChanged = false;
    }
   else {
      inputParamsChanged = true;
   }
}

void
BinetNetCDF::shiftFieldParams()
{
   DsFieldParams *inputField, *outputField;

   //
   // Copy the field parameters
   // from the input to the output position
   //
   for( int i=0; i < inputRadarParams.numFields; i++ ) {
      inputField = inputRadarMsg.getFieldParams( i );
      outputField = new DsFieldParams( *inputField );

      //
      // Here we're going to do something a little sneeky.
      // If the user is requesting reflectivity or velocity data,
      // we are going to change the input fields from their NetCDF names to
      // output field names consistent with our other radar ingest applications
      //
      if ( outputField->name == "DZ" ) {
         outputField->name = "DBZ";
      }
      else if ( outputField->name == "VE" ) {
         outputField->name = "VEL";
      }

      outputRadarFields.push_back( outputField );
   }
}

void
BinetNetCDF::setDataScaling()
{
   //
   // Hang onto each field's input scale and bias
   //
   DsFieldParams  *fieldParams;
   char           *fieldName;
   NcVar          *fieldVariable;
   float           scale, bias;
   ScaleBiasPair  *scaleBias;

   clearScaling();

   for( int i=0; i < inputRadarParams.numFields; i++ ) {
      fieldParams   = inputRadarMsg.getFieldParams( i );
      fieldName     = (char*)(fieldParams->name.c_str());
      fieldVariable = ncFile->get_var( fieldName );
      scale = fieldVariable->get_att( "scale_factor" )->as_float(0);
      bias  = fieldVariable->get_att( "add_offset" )->as_float(0);
      scaleBias = new ScaleBiasPair( scale, bias );
      inputScaling.push_back( scaleBias );
   }
}

void
BinetNetCDF::setRadarBeam()
{
   //
   // Get some basic information related to the beam
   // NOTE:  this section depends on the radarParams being set first
   //
   // b is just a shorhand notation for the long lines which follow
   //
   DsRadarBeam& b = inputRadarBeam;

   b.dataTime   = baseTime
                + ncFile->get_var( "time_offset" )->as_long( currentBeam );

   b.azimuth    = ncFile->get_var( "Azimuth" )->as_float( currentBeam );
   b.volumeNum  = currentVol;
   b.tiltNum    = currentTilt;
   b.elevation  = currentElev;
   b.targetElev = targetElev;

   //
   // Make sure our data buffer is big enough
   //
   int numFields    = inputRadarParams.numFields;
   int inputDataLen = inputRadarParams.numGates * numFields;
   allocateBeamData( inputDataLen );

   //
   // Fetch the NetCDF record for each field
   //
   int            i, inIndex, outIndex;
   DsFieldParams *fieldParams;
   char          *fieldName;
   NcValues      *fieldValues;
   float          fieldScale, fieldBias;
   double         inValue;
   ui08           outValue;

   for( i=0; i < numFields; i++ ) {
      fieldParams = inputRadarMsg.getFieldParams( i );
      fieldName   = (char*)(fieldParams->name.c_str());
      fieldValues = ncFile->get_var( fieldName )->get_rec( currentBeam );
      fieldScale  = inputScaling[i]->first;
      fieldBias   = inputScaling[i]->second;

      //
      // Assign the field values into our beam data buffer
      // The beam buffer ordering is: gate1:f1 f2 f3, gate2:f1 f2 f3 ...
      //
      for( outIndex=0, inIndex=0; 
           outIndex < (int) beamDataLen; 
           outIndex += numFields, inIndex++ ) {

         //
         // scaling on the fly (short->double->byte)
         //
         inValue = fieldValues->as_short(inIndex) * fieldScale + fieldBias;
         outValue = (ui08)((inValue - fieldParams->bias) / fieldParams->scale);
         beamData[outIndex] = outValue;
      }
   }

   //
   // Now, load up the DsRadarBeam from our beam data buffer
   //
   b.loadData( beamData, beamDataLen );
}

void
BinetNetCDF::setRadarFlags()
{
   //
   // Get some basic info from the input radar beam and params
   // NOTE: The radar beam and radar parameters must be established
   //       first in order to set the flags properly.
   //
   inputRadarFlags.time      = inputRadarBeam.dataTime;
   inputRadarFlags.volumeNum = inputRadarBeam.volumeNum;
   inputRadarFlags.tiltNum   = inputRadarBeam.tiltNum;
   inputRadarFlags.scanType  = inputRadarParams.scanType;

   //
   // If this is the first beam that we've seen,
   // don't even bother to do the remaining checks for 
   // change of volume/tilt/scan since there are
   // no previous flags to compare against. 
   //
   if ( firstCall ) {
      return;
   }

   //
   // Check for change of volume
   // 
   if ( inputRadarFlags.volumeNum != outputRadarFlags.volumeNum ) {
      //
      // Change of volume condition also signals a change of tilt
      //
      outputRadarFlags.endOfVolume  = true;
      outputRadarFlags.endOfTilt    = true;

      inputRadarFlags.startOfVolume = true;
      inputRadarFlags.startOfTilt   = true;
   }

   //
   // Check for change of tilt
   // TODO: We should also know about the change of tilts whenever a new file
   //       is opened -- perhaps we should make an assertion to that effect
   //
   if ( inputRadarBeam.targetElev != outputRadarBeam.targetElev ) {

      inputRadarFlags.startOfTilt = true;
      outputRadarFlags.endOfTilt  = true;

      if ( inputRadarFlags.startOfVolume ) {
         //
         // Reset the tilt numbering for the new volume
         //
         currentTilt = 0;
         inputRadarFlags.tiltNum = currentTilt;
         inputRadarBeam.tiltNum  = currentTilt;
      }
      else {
         //
         // Increment the tilt number
         // but only if we have figured out where we are in the scan sequence
         //
         if ( currentTilt != -1 ) {
            currentTilt++;
            inputRadarBeam.tiltNum++;
            inputRadarFlags.tiltNum++;
         }
      }
   }

   //
   // Check for change of scan strategy
   //
   if ( inputRadarFlags.scanType != outputRadarFlags.scanType ) {
      inputRadarFlags.newScanType = true;
   }
}

void
BinetNetCDF::allocateBeamData( int inputDataLen )
{
   //
   // TODO: Wonder what the equilavent of urealloc is in C++ ?
   //       If you change it here, don't forget the destructor
   //
   if ( inputDataLen != (int) beamDataLen ) {
      beamDataLen = inputDataLen;
      beamData = (ui08*)urealloc( beamData, beamDataLen );
   }
}
