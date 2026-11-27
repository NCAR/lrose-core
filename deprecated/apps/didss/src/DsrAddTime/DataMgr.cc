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
// DataMgr - Data Manager
//
// $Id: DataMgr.cc,v 1.12 2016/03/06 23:53:40 dixon Exp $
//
///////////////////////////////////////////////////////////////
#include <string>
#include <unistd.h>
#include <toolsa/utim.h>
#include <toolsa/udatetime.h>
#include <toolsa/MsgLog.hh>

#include "DataMgr.hh"
#include "DsrAddTime.hh"
using namespace std;

//
// Defines
//
#define HDR " Vol Tilt El_tgt El_act     Az Ngat Gspac  PRF     Date     Time"
#define FMT "%4ld %4ld %6.2f %6.2f %6.2f %4ld %5ld %4ld " \
"%.2ld/%.2ld/%.2ld %.2ld:%.2ld:%.2ld"

//
// Constants
//
const string DataMgr::TIME_FIELD_UNITS = "seconds";
const int    DataMgr::MISSING_TIME_VAL = 0;
const float  DataMgr::TIME_BIAS        = -30.0;

DataMgr::DataMgr()
    :outputParams( outputMsg.getRadarParams() ),
     outputFields( outputMsg.getFieldParams() ),
     outputBeam( outputMsg.getRadarBeam() ),
     outputFlags( outputMsg.getRadarFlags() )
{
   paramsSet        = false;
   fieldsSet        = false;
   setReferenceTime = true;
   writeSummary     = false;
   summaryInterval  = 1;
   referenceTime    = 0;
}

DataMgr::~DataMgr()
{
}

int DataMgr::init( Params& params, MsgLog& msgLog )
{
   //
   // Set up the input queue
   //
   if( inputQueue.init( params.inputFmqUrl, PROGRAM_NAME,
                        DEBUG_ENABLED,
                        DsFmq::BLOCKING_READ_ONLY, DsFmq::END )) {
      POSTMSG( ERROR, "Could not initialize fmq %s", 
	       params.inputFmqUrl );
      return( DSR_FAILURE );
   }

   //
   // Set up the output queue
   //
   if( outputQueue.init( params.outputFmqUrl,
		         PROGRAM_NAME,
		         DEBUG_ENABLED,           
		         DsFmq::READ_WRITE, DsFmq::END, 
		         params.outputFmqCompress,
		         params.outputFmqNslots,
		         params.outputFmqSize, 1000,
                         &msgLog )) {
      POSTMSG( ERROR, "Could not initialize fmq %s", 
	       params.outputFmqUrl );
      return( DSR_FAILURE );
   }

   //
   // Misc.
   //
   timeFieldName   = params.timeFieldName;
   timeScale       = (float) params.timeScale;
   writeSummary    = params.printSummary ? true : false;
   summaryInterval = params.summaryInterval;

   return( DSR_SUCCESS );
}

void
DataMgr::processData() 
{
   int        contents = 0;
   int        count    = 0;

   while (true) {

      PMU_auto_register( "Reading input radar queue" );
    
      //
      // Get a message from the input radar queue
      //
      if( inputQueue.getDsMsg( inputMsg, &contents ) ) {

         POSTMSG( ERROR, "Could not get the beam from the radar queue" );
         sleep( 1 );

      } else { 

         //
         // Got a message - now add the time field
         //
         bool processed = addTime( contents );

         //
         // If the radar parameters and field parameters have
         // not been set, do nothing
         //
         if( !paramsSet || !fieldsSet ) 
            continue;

         //
         // If the user wants to print the summary, decide if it
         // is time to do that
         //
         if( writeSummary && processed ) {
            count += 1;
            if( count >= summaryInterval ) {

               //
               // Print the summary - This is based upon the
               // output message.
               //
               printSummary();

               //
               // Reset the count 
               //
               count = 0;
            }
         }
       
         //
         // Write out the beam - Using the same contents here
         // as we got from the input queue.  This means that we
         // are output radar params, field params, beam data
         // and flags only when we receive them.
         //
         if( outputQueue.putDsMsg( outputMsg, contents ) ) {
            POSTMSG( ERROR, "Could not write the radar message to the "
                     "output queue" );
         }
         
      }
   }
}
   
bool
DataMgr::addTime( int contents ) 
{  
   
   //
   // Should we update the reference time?
   //
   if( contents & DsRadarMsg::RADAR_FLAGS ) {
      DsRadarFlags &radarFlags = inputMsg.getRadarFlags();
      
      if( radarFlags.startOfVolume ) {
         setReferenceTime = true;
      }
   }

   //
   // If the input radar params are present, set the output
   // params to be a copy of the input, with modifications
   //   
   if( contents & DsRadarMsg::RADAR_PARAMS ) {

      //
      // Read the input radar params
      //
      const DsRadarParams &inputParams = inputMsg.getRadarParams();

      //
      // Copy the input into the output
      //
      outputParams.copy( inputParams );

      //
      // Modify the number of fields in the radar parameters
      //
      outputParams.numFields += 1;

      paramsSet = true;
      
   }
   
   //
   // If the field params came in the input message, copy them
   // to the output message and add the time field
   //
   if( contents & DsRadarMsg::FIELD_PARAMS ) {

      //
      // Clear out the old fields
      //
      vector< DsFieldParams* >::iterator outputIt;
      for( outputIt = outputFields.begin(); 
           outputIt != outputFields.end(); outputIt++ ) {
         delete( *outputIt );
      }
      outputFields.erase( outputFields.begin(), outputFields.end() );

      //
      // Read the input field params
      //
      const vector< DsFieldParams* > &inputFields = inputMsg.getFieldParams();
      
      //
      // Copy the input field params to the output field params
      //
      vector< DsFieldParams* >::const_iterator inputIt;
      for( inputIt = inputFields.begin(); 
           inputIt != inputFields.end(); inputIt++ ) {

         DsFieldParams *fieldParams = new DsFieldParams( *(*inputIt) );
         outputFields.push_back( fieldParams );

      }
      
      //
      // Add the time field params
      //
      DsFieldParams *timeField = new DsFieldParams();

      timeField->scale            = timeScale;
      timeField->bias             = TIME_BIAS;
      timeField->name             = timeFieldName;
      timeField->units            = TIME_FIELD_UNITS;
      timeField->missingDataValue = MISSING_TIME_VAL;
      
      outputFields.push_back( timeField );

      fieldsSet = true;
      
   }
   
   //
   // If the beam data are present, add the time data to
   // the existing field data - as long as the parameters
   // have already been set
   //
   if( contents & DsRadarMsg::RADAR_BEAM && paramsSet && fieldsSet ) {

      //
      // Read the input radar beam
      //
      const DsRadarBeam &inputBeam = inputMsg.getRadarBeam();

      //
      // Copy the input beam
      //
      outputBeam.copy( inputBeam );

      //
      // Set the reference time if necessary
      //
      if( setReferenceTime ) {
         referenceTime    = inputBeam.dataTime;
         setReferenceTime = false;
      }
      
      //
      // Set the reference time in the beam header
      //
      outputBeam.referenceTime = referenceTime;

      //
      // Find the difference from the reference time
      // and round to the nearest ten seconds
      //
      ui08 timeValue = 
         (ui08) ( ((inputBeam.dataTime - referenceTime - TIME_BIAS) / 
                   timeScale) + 0.5 );

      //
      // Tell the user what is going on
      //
      if( DEBUG_ENABLED ) {
         int diffValue = (int) (inputBeam.dataTime - referenceTime + 0.5);
      
         POSTMSG( DEBUG, "Reference time = %ld, difference = %d, "
                  "scaledTime = %d\n", referenceTime, diffValue, timeValue );
      }
      

      //
      // Get the number of gates and the number of fields
      // in order to process the data below
      //
      int nGates  = outputParams.numGates;
      int nFields = outputParams.numFields;
      
      //
      // Allocate space for the output data - the output data
      // should have one extra field.  This means adding 1*nGates
      // slots to the input data length to make room in the output
      // data array.
      //
      int   inputBeamLen  = inputBeam.dataLen();
      ui08 *inputBeamData = inputBeam.data();
      
      int   outputBeamLen  = inputBeamLen + nGates;
      ui08 *outputBeamData = new ui08[outputBeamLen];
      
      //
      // Copy the data from the input fields into the correct
      // slots in the output data array.  Add the new time
      // field data into the correct slot as well.  The data
      // is arranged in a gate-by-gate format, so we add each
      // field in for a single gate and then proceed to the
      // next gate.
      //
      for( int i = 0; i < nGates; i++ ) {
         for( int j = 0; j < nFields-1; j++ ) {
            outputBeamData[i*nFields + j] = inputBeamData[i*(nFields-1) + j];
         }
         outputBeamData[i*nFields + (nFields-1)] = timeValue;
      }
      
      //
      // Load the data into the output beam
      //
      outputBeam.loadData( outputBeamData, outputBeamLen );

      //
      // Clean up
      //
      delete[] outputBeamData;
   }

   //
   // Set the flags if necessary - If the flags were not included
   // in the input message, we don't do anything here.  That means
   // that the flags MUST NOT be written out, unless we receive
   // them as input as well.  Otherwise, we could, for example, 
   // erroneously put out startOfVolume, endOfVolume, startOfTilt 
   // or endOfTilt flags
   //
   if( contents & DsRadarMsg::RADAR_FLAGS ) {

      //
      // Read the input flags
      //
      const DsRadarFlags &inputFlags = inputMsg.getRadarFlags();
      
      //
      // Copy the input flags to the output flags
      //
      outputFlags.copy( inputFlags );
   }

   return( true );

}


void 
DataMgr::printSummary()
{
  
  fprintf(stdout, HDR);
  fprintf(stdout, "\n");
  
  //
  // Parse the time of the beam
  //
  date_time_t  dataTime;
  dataTime.unix_time = outputBeam.dataTime;
  uconvert_from_utime( &dataTime );
  
  fprintf(stdout, FMT,
          (long) outputBeam.volumeNum,
          (long) outputBeam.tiltNum,
          (double) outputBeam.targetElev,
          (double) outputBeam.elevation,
          (double) outputBeam.azimuth,
          (long) outputParams.numGates,
          (long) (outputParams.gateSpacing * 1000),
          (long) outputParams.pulseRepFreq,
          (long) dataTime.year,
          (long) dataTime.month,
          (long) dataTime.day,
          (long) dataTime.hour,
          (long) dataTime.min,
          (long) dataTime.sec);
  fprintf(stdout, "\n");
  fflush(stdout);
}






