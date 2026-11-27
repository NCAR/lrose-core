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
// Dsr2Mdv.cc
//
// Dsr2Mdv object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1998
//
///////////////////////////////////////////////////////////////
//
// Dsr2Mdv produces a forecast image based on (a) motion data
// provided in the form of (u,v) components on a grid and
// (b) image data on a grid.
//
///////////////////////////////////////////////////////////////

#include "Dsr2Mdv.hh"
#include "Lookup.hh"
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/toolsa_macros.h>
using namespace std;

// Constructor

Dsr2Mdv::Dsr2Mdv(int argc, char **argv)

{

  OK = TRUE;
  Done = FALSE;
  _scanType = -1;

  // set programe name

  _progName = STRdup("Dsr2Mdv");
  ucopyright(_progName);

  // get command line args

  _args = new Args(argc, argv, _progName);
  if (!_args->OK) {
    fprintf(stderr, "ERROR: %s\n", _progName);
    fprintf(stderr, "Problem with command line args\n");
    OK = FALSE;
    return;
  }
  if (_args->Done) {
    Done = TRUE;
    return;
  }

  // get TDRP params

  _Params = new Params(_args->paramsFilePath,
		       &_args->override,
		       _progName,
		       _args->printParams,
		       _args->printShort);
  
  if (!_Params->OK) {
    fprintf(stderr, "ERROR: %s\n", _progName);
    fprintf(stderr, "Problem with TDRP parameters\n");
    OK = FALSE;
    return;
  }
  if (_Params->Done) {
    Done = TRUE;
    return;
  }
  _params = &_Params->p;

  if (!OK) {
    return;
  }

  PMU_auto_init(_progName, _params->instance, PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

Dsr2Mdv::~Dsr2Mdv()

{

  // unregister process

  PMU_auto_unregister();

  // free up

  delete(_Params);
  delete(_args);
  STRfree(_progName);

}

//////////////////////////////////////////////////
// Run

int Dsr2Mdv::Run ()
{

  PMU_auto_register("Dsr2Mdv::Run");

  // Instantiate lookup table

  Lookup lookup(_progName, _params);

  // Instantiate and initialize the DsRadar queue and message

  DsRadarQueue radarQueue;
  DsRadarMsg radarMsg;

  if (_params->seek_to_end_of_input) {
    if (radarQueue.init(_params->input_fmq_url, _progName,
			_params->debug,
			DsFmq::BLOCKING_READ_WRITE, DsFmq::END )) {
      fprintf(stderr, "ERROR - %s:Dsr2Mdv::Run\n", _progName);
      fprintf(stderr, "Could not initialize radar queue '%s'\n",
	      _params->input_fmq_url);
      return(-1);
    }
  } else {
    if (radarQueue.init(_params->input_fmq_url, _progName,
			_params->debug,
			DsFmq::BLOCKING_READ_WRITE, DsFmq::START )) {
      fprintf(stderr, "ERROR - %s:Dsr2Mdv::Run\n", _progName);
      fprintf(stderr, "Could not initialize radar queue '%s'\n",
	      _params->input_fmq_url);
      return(-1);
    }
  }

  // set up the Resample object

  Resample resample(_progName, _params, &lookup);
  if (!resample.OK) {
    fprintf(stderr, "ERROR - %s:Run\n", _progName);
    fprintf(stderr, "Cannot set up Resample object\n");
    return (-1);
  }

   // read beams from the queue and process them

   int start_of_vol = TRUE;
   int end_of_vol;
   int has_beam_data;
   int forever = TRUE;
   time_t lastIntermediateTime = time(NULL);

   while (forever) {

     if (_readMsg(radarQueue, radarMsg, &lookup,
		  has_beam_data, end_of_vol) == 0) {

       // update the lookup table as necessary at the start of a volume

       if (start_of_vol && has_beam_data) {
	 if (lookup.update(radarMsg.getRadarParams().scanType)) {
	   return (-1);
	 }
	 if (resample.prepareVol(radarMsg)) {
	   return (-1);
	 }
	 start_of_vol = FALSE;
       }

       if (has_beam_data) {
	 resample.processBeam(radarMsg);
       }

       // write out intermediate files as required

       if (_params->write_intermediate_files) {

	 time_t now = time(NULL);
	 int dtime = now - lastIntermediateTime;
	 if (dtime > _params->intermediate_file_frequency) {
	   resample.writeIntermediateVol();
	   lastIntermediateTime = now;
	 }

       }

       // at the end of a volume, write out

       if (end_of_vol && !start_of_vol) {
	 resample.writeCompleteVol();
	 start_of_vol = TRUE;
       }

     } //  if (_readBeam ...

   } // while (forever)


   return (0);

 }

 ////////////////////////////////////////////////////////////////////
 // _readMsg()
 //
 // Read a message from the queue, setting the flags about beam_data
 // and end_of_volume appropriately.
 //

 int Dsr2Mdv::_readMsg(DsRadarQueue &radarQueue,
		       DsRadarMsg &radarMsg,
		       Lookup *lookup,
		       int &has_beam_data,
		       int &end_of_vol) 

 {

   int contents;
   int accept_beam;

   PMU_auto_register("Reading radar queue");

   //
   // Read messages until all params have been set
   //

   int scan_type;
   int forever = TRUE;
   while (forever) {

     if (radarQueue.getDsMsg(radarMsg, &contents)) {
	 return (-1);
     }

     // check if parameters have been set, and that the
     // the gate spacing is correct. If so, accept the beam.

     if (radarMsg.allParamsSet()) {

      DsRadarParams radarParams= radarMsg.getRadarParams();
      scan_type = radarParams.scanType;
      radar_scan_table_t *scan_table = lookup->handle.scan_table;

      if ( _params->override_radar_location ) {
        radarParams.latitude = _params->radar_location.latitude;
        radarParams.longitude = _params->radar_location.longitude;
        radarParams.altitude = _params->radar_location.altitude;
      }

      // is this an end of vol?
      end_of_vol = FALSE;

      if (contents & DsRadarMsg::RADAR_FLAGS) {
          const DsRadarFlags &flags = radarMsg.getRadarFlags();
          if (_params->end_of_vol_decision == END_OF_VOL_FLAG) {
             if (flags.endOfVolume) {
	        // end-of-vol flag
	        end_of_vol = TRUE;
             }
          } else if (flags.endOfTilt &&
                     flags.tiltNum == _params->last_tilt_in_vol) {
               // look last tilt in vol
               end_of_vol = TRUE;
          } else if (flags.newScanType) {
               end_of_vol = TRUE;
          }
      }
      if (_scanType >= 0 && _scanType != scan_type) {
        end_of_vol = TRUE;
      }
      _scanType = scan_type;

      // accept beam?
      accept_beam = FALSE;
      if (fabs(radarParams.gateSpacing -
	       scan_table->gate_spacing) < 0.5) {

	 accept_beam = TRUE;

      } else {

	if (_params->debug >= DEBUG_VERBOSE) {
	  fprintf(stderr, "WARNING - rejecting beam because of "
		  "incorrect gate spacing\n");
	  fprintf(stderr, "Lookup table '%s'\n",
		  lookup->handle.file_path);
	  fprintf(stderr, "Lookup gate spacing: %g\n",
		  scan_table->gate_spacing);
	  fprintf(stderr, "Radar data gate spacing: %g\n",
		  radarParams.gateSpacing);
	} // if (_params->debug >= DEBUG_VERBOSE) {

      } // if (fabs(radarParams.gateSpacing ...

      // Check the beam elevation to see if it's within the range
      // we are processing

      if (accept_beam)
      {
	float elevation = radarMsg.getRadarBeam().elevation;
	
	if (elevation < _params->min_elevation ||
	    elevation > _params->max_elevation)
	{
	  accept_beam = FALSE;

	  if (_params->debug >= DEBUG_VERBOSE)
	  {
	    fprintf(stderr,
		    "WARNING - rejecting beam because elevation is out of accepted range.\n");
	    fprintf(stderr,
		    "Beam elevation: %f degrees\n", elevation);
	    fprintf(stderr,
		    "Min elevation: %f degrees\n", _params->min_elevation);
	    fprintf(stderr,
		    "Max elevation: %f degrees\n", _params->max_elevation);
	  }
	}
      } /* endif - accept_beam */
      
      if( accept_beam || end_of_vol )
	 break;

    } // if (radarMsg.allParamsSet()) 
    
  } // while (forever)

  // does it have beam data?  should we use it?
  
  if (contents & DsRadarMsg::RADAR_BEAM && accept_beam) {
    has_beam_data = TRUE;
  } else {
    has_beam_data = FALSE;
  }
  
  return (0);

}

