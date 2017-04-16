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
/////////////////////////////////////////////////////////
//
// JamesDealias.cc: methods of JamesDealias class
// 
/////////////////////////////////////////////////////////

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include "JamesDealias.hh"
using namespace std;

///////////////////////////////////////////////////////
// 
// Constructor
//
JamesDealias::JamesDealias(int argc, char **argv)
{
  isOK = true;

  //
  // set programe name
  //
  _progName = "JamesDealias";

  ucopyright((char *) _progName.c_str());
  
  jamesCopyright();

  //
  // get command line args
  //
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  //
  // get TDRP params
  //
  _paramsPath = (char *) "unknown";

  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }

  //
  // init process mapper registration
  //
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  //
  // Initialize Dsr2Radar pointers
  //
  _currRadarVol = NULL;
 
  _prevRadarVol = NULL;

  return;
}

//////////////////////////////////////////////////////////////
// 
// destructor
//
JamesDealias::~JamesDealias()

{
  if (_currRadarVol)
    delete (_currRadarVol);

  if (_prevRadarVol)
    delete (_prevRadarVol);

  //
  // unregister process
  //
  PMU_auto_unregister();

}

////////////////////////////////////////////////////////
//
// Run
//
int JamesDealias::Run ()
{

  //
  // register with procmap
  //
  PMU_auto_register("Run");

  while (true) {
    _run();
    cerr << "JamesDealias::Run:" << endl;
    cerr << "  Trying to contact input server at url: "
	 << _params.input_fmq_url << endl;
    sleep(2);
  }

  return 0;
}

//////////////////////////////////////////////////////////////
// 
// _run: initialize fmqs, create Dsr2Radar and FourDD objects
//       for storing beams and dealiasing volumes,
//       start reading messages, processing and writing volumes.  
//
int JamesDealias::_run ()
{
  //
  //
  // Instantiate and initialize the DsRadar queues
  //
  DsRadarQueue radarQueue, outputQueue;

  DsRadarMsg radarMsg;

  //
  // Option to pad the beam data to a constant number of gates
  //
  if ( _params.input_num_gates > 0 ) 
    {
      radarMsg.padBeams( true, _params.input_num_gates );
    }

  if (_params.seek_to_end_of_input) 
    {
      if (radarQueue.init(_params.input_fmq_url, _progName.c_str(),
			  _params.debug,
			  DsFmq::BLOCKING_READ_ONLY, DsFmq::END )) 
	{
	  fprintf(stderr, "ERROR - %s:JamesDealias::_run\n", _progName.c_str());
	  fprintf(stderr, "Could not initialize radar queue '%s'\n",
		  _params.input_fmq_url);
	  return -1;
	}
    } 
  else 
    {
      if (radarQueue.init(_params.input_fmq_url, _progName.c_str(),
			  _params.debug,
			  DsFmq::BLOCKING_READ_ONLY, DsFmq::START )) 
	{
	  fprintf(stderr, "ERROR - %s:JamesDealias::_run\n", _progName.c_str());
	  fprintf(stderr, "Could not initialize radar queue '%s'\n",
		  _params.input_fmq_url);
	  return -1;
	}
    }
  
  if( outputQueue.init( _params.output_fmq_url,
                        _progName.c_str(),
                        _params.debug,
                        DsFmq::READ_WRITE, DsFmq::END,
                        _params.output_fmq_compress,
                        _params.output_fmq_nslots,
                        _params.output_fmq_size, 1000) ) 
    {
      fprintf(stderr, "Error - %s: Could not initialize fmq %s", _progName.c_str(), _params.output_fmq_url );
      return( -1);
    }

  //
  // Create Dsr2Radar objects for reformatting and storing radar data
  //
  _currRadarVol = new Dsr2Radar(_params);
  
  _prevRadarVol = new Dsr2Radar(_params);

  //
  // Create FourDD object for dealiasing
  //
  _fourDD = new FourDD(_params);

  //
  // Read beams from the queue and process them
  //
  while (true) 
    {
      bool end_of_vol;
            
      int contents;
      
      if (_readMsg(radarQueue, radarMsg, end_of_vol, contents) == 0) 
	{ 
	  //
	  // If end of a volume, process volume.
	  //
	  if (end_of_vol) 
	    {      
	      //
	      // process volume
	      //
	      _processVol();
	      
	      //
	      // write the volume 
	      //
	      _writeVol(outputQueue);
	      
	      //
	      // prepare for start next volume
	      //
	      _reset();
	      
	    }
	} //  if (_readMsg()
    } // while (true)
  
  return 0;
}

////////////////////////////////////////////////////////////////////
// _readMsg()
//
// Read a message from the queue. If appropriate, reformat and store 
// data, set end of vol flag.
//

int JamesDealias::_readMsg(DsRadarQueue &radarQueue, DsRadarMsg &radarMsg,
		      bool &end_of_vol, int &contents) 
  
{
  
  PMU_auto_register("Reading radar queue");

  end_of_vol = false;
  
  if (radarQueue.getDsMsg(radarMsg, &contents)) 
    {
      return -1;
    }

  if (contents != 0)
    {
      //
      // Reformat beam and store in RSL structs
      //
      _currRadarVol->reformat(radarMsg, contents);
      
      //
      // Check for end of volume.
      //
      if (contents & DsRadarMsg::RADAR_FLAGS) 
	{      
	  const DsRadarFlags &flags = radarMsg.getRadarFlags();
        
	  if (_params.end_of_vol_decision == Params::END_OF_VOL_FLAG) 
	    {
	      if (flags.endOfVolume) 
		{
		  end_of_vol = true;
		}
	    } 
	  else if (flags.endOfTilt &&
		   flags.tiltNum == _params.last_tilt_in_vol) 
	    {
	      end_of_vol = true;
	    } 
	} // end if (contents & DsRadarMsg::RADAR_FLAGS)
      
    } // end if (contents ! = 0);
  return 0;

}

////////////////////////////////////////////////////////////////
// 
// _processVol(): Dealias the volume if possible.
//              The James dealiaser requires that the previous
//              radar volume (if there is one) is the same 
//              size (ie. same number of tilts) as the current
//              volume. If previous and current are not the same
//              size we dont do the dealiasing. 
//
void JamesDealias::_processVol()

{

  if (_params.debug) {
      cerr << "_processVol(): " << endl;
  }

  if (_currRadarVol->isEmpty())
      {
	cerr << "Current volume is empty. No processing.\n";
	return;
      }
	
  Volume *currDbzVol = _currRadarVol->getDbzVolume();
  
  Volume *currVelVol = _currRadarVol->getVelVolume();

  Volume *prevVelVol = _prevRadarVol->getVelVolume();
    
  time_t volTime = _currRadarVol->getVolTime();

  //
  // If there is a previous volume , make 
  // sure the previous and the current volumes are the same
  // size before dealiasing. The James dealiaser presently 
  // requires it.
  //
  if ( prevVelVol != NULL ) 
    {
      if( prevVelVol->h.nsweeps == currVelVol->h.nsweeps)
	{
	  _fourDD->Dealias(prevVelVol, currVelVol, currDbzVol, volTime);
	}      
      else
	{
	  if (_params.debug)
	    fprintf(stderr, "Cannot dealias current velocity volume. Previous and current volume are of different sizes. Time associated with current volume %ld\n", volTime);
	}
    }
  else 
    _fourDD->Dealias(prevVelVol, currVelVol, currDbzVol, volTime);

}

//////////////////////////////////////////////////////////////////////
//
//  _writeVol(): write current radar volume to output fmq.
//
void JamesDealias::_writeVol(DsRadarQueue &outputQueue)
{
  
  if (_params.debug) 
    {
      cerr << "_writeVol(): writing processed beams to fmq " << endl;
    }
 
  _currRadarVol->writeVol(outputQueue);

}


/////////////////////////////////////////////////////////////////
//  
// _reset(): clean out _prevRadarVol for recycling,
//           _currRadarVol is set to _prevRadarVol,
//           _prevRadarVol gets set to _currRadarVol
//
void JamesDealias::_reset()

{
  if ( ! _currRadarVol->isEmpty() )
    {
      _prevRadarVol->clearData();
      
      Dsr2Radar *tmpRadarVol = _prevRadarVol;
      
      _prevRadarVol = _currRadarVol;
      
      _currRadarVol = tmpRadarVol;
    }
  else 
    _currRadarVol->clearData();


  if (_params.debug) {
    cerr << "=========== Start of volume ==================" << endl;
  }
}

void JamesDealias::jamesCopyright()
{
   cerr << "** JamesDealias executes 4DD which is the Four Dimensional Dealiasing algorithm\n"
        << "** developed by the  Mesoscale Group, Department of Atmospheric Sciences,\n"
        << "** University of Washington, Seattle, WA, USA. Copyright 2001.\n\n";

}




