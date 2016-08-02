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
// Dsr2Rsl.cc
//
//  Calls methods to read an input radar FMQ, puts the data 
//  into an RSL Radar struct, dealias the data, write to an fmq.
//
///////////////////////////////////////////////////////////////////////

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include "Dsr2Rsl.hh"
using namespace std;

//
// Constructor
//
Dsr2Rsl::Dsr2Rsl(int argc, char **argv)

{

  isOK = true;

  //
  // set programe name
  //
  _progName = "Dsr2Rsl";
  ucopyright((char *) _progName.c_str());

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
  _paramsPath = "unknown";
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

  currRadarVol = NULL;
  prevRadarVol = NULL;

  return;

}

//
// destructor
//
Dsr2Rsl::~Dsr2Rsl()

{
  if (currRadarVol)
    delete (currRadarVol);

  if (prevRadarVol)
    delete (prevRadarVol);

  //
  // unregister process
  //
  PMU_auto_unregister();

}


//////////////////////////////////////////////////
//
// Run
//
int Dsr2Rsl::Run ()
{

  //
  // register with procmap
  //
  PMU_auto_register("Run");


  while (true) {
    _run();
    cerr << "Dsr2Rsl::Run:" << endl;
    cerr << "  Trying to contact input server at url: "
	 << _params.input_fmq_url << endl;
    sleep(2);
  }

  return 0;

}

//////////////////////////////////////////////////
// 
// _run
//
int Dsr2Rsl::_run ()
{

  //
  // register with procmap
  //
  PMU_auto_register("Run");

  //
  // Instantiate and initialize the DsRadar queue (input and output )and message
  //
  DsRadarQueue radarQueue, outputQueue;
  DsRadarMsg radarMsg;

  if (_params.seek_to_end_of_input) 
    {
      if (radarQueue.init(_params.input_fmq_url, _progName.c_str(),
			  _params.debug,
			  DsFmq::BLOCKING_READ_ONLY, DsFmq::END )) 
	{
	  fprintf(stderr, "ERROR - %s:Dsr2Rsl::_run\n", _progName.c_str());
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
	  fprintf(stderr, "ERROR - %s:Dsr2Rsl::_run\n", _progName.c_str());
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
                        _params.output_fmq_size, 1000) ) {
    fprintf(stderr, "Error - %s: Could not initialize fmq %s", _progName.c_str(), _params.output_fmq_url );
      return( -1);
   }

  //
  // Create Dsr2Radar objects
  //
  currRadarVol = new Dsr2Radar(_progName, _params);
  
  prevRadarVol = new Dsr2Radar(_progName, _params);

  //
  // create FourDD object
  //
  fourDD = new FourDD(_params);

  //
  // read beams from the queue and process them
  //
  while (true) 
    {
      bool end_of_vol;
            
      int contents;
      
      if (_readMsg(radarQueue, radarMsg, end_of_vol, contents) == 0) 
	{ 
	  //
	  // If end of a volume, process volume
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
	      // reset
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
// Read a message from the queue, setting the flags about beam_data
// and end_of_volume appropriately.
//

int Dsr2Rsl::_readMsg(DsRadarQueue &radarQueue, DsRadarMsg &radarMsg,
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
      // Reformat beam and store in Radar struct
      //
      currRadarVol->reformat(radarMsg, contents);
      
      //
      // check for end of volume.
      // Note the end of volume decision is based 
      // param file params or a change in scan type.
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
	  else if (flags.newScanType) 
	    {
	      end_of_vol = true;
	    }
	} // end if (contents & DsRadarMsg::RADAR_FLAGS)
      
    } // end if (contents ! = 0);
  
  return 0;

}

////////////////////////////////////////////////////////////////
// 
// process the volume
//
void Dsr2Rsl::_processVol()

{

  if (_params.debug) {
      cerr << "_processVol(): " << endl;
  }

  Volume *currDbzVol = currRadarVol->getDbzVolume();
  
  Volume *currVelVol = currRadarVol->getVelVolume();

  Volume *prevVelVol = prevRadarVol->getVelVolume();
    
  time_t volTime = currRadarVol->getVolTime();

  //
  // If there is a previous volume , make 
  // sure the previous and the current volumes are the same
  // size before dealiasing. The James Dealiaser presently 
  // requires it.
  //
  if ( prevVelVol != NULL ) 
    {
      if( prevVelVol->h.nsweeps == currVelVol->h.nsweeps)
	{
	  fourDD->Dealias(prevVelVol, currVelVol, currDbzVol, volTime);
	}      
      else
	{
	  if (_params.debug)
	    fprintf(stderr, "Cannot dealias current velocity volume. Previous and current volume are of different sizes. \n");
	}
    }
  else 
    fourDD->Dealias(prevVelVol, currVelVol, currDbzVol, volTime);
}

void Dsr2Rsl::_writeVol(DsRadarQueue &outputQueue)
{
  
  if (_params.debug) {
      cerr << "_writeVol(): writing processed beams to fmq " << endl;
  }
 
  currRadarVol->writeVol(outputQueue);

}



/////////////////////////////////////////////////////////////////
//  
// reset method
//
//
void Dsr2Rsl::_reset()

{

  //
  // clean out prevRadarVol for recycling:
  // currRadarVol is set to cleaned out prevRadarVol,
  // prevRadarVol gets set to currRadarVol
  //
 
  prevRadarVol->clearData();
     
  Dsr2Radar *tmpRadarVol = prevRadarVol;

  prevRadarVol = currRadarVol;

  currRadarVol = tmpRadarVol;
   

  if (_params.debug) {
    cerr << "=========== Start of volume ==================" << endl;
  }
}















