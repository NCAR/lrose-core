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
//
// GenPoly2Mdv.cc
//
// RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Reads SPDB GenPoly and writes gridded MDV data.
//

#include "GenPoly2Mdv.hh"

using namespace std;

// Constructor

GenPoly2Mdv::GenPoly2Mdv(int argc, char **argv)
  
{
  isOK = true;

 _progName = "GenPoly2Mdv";

 ucopyright((char *) _progName.c_str());

 // get command line args

 if (_args.parse(argc, argv, _progName)) {
   cerr << "ERROR: " << _progName << endl;
   cerr << "Problem with command line args" << endl;
   isOK = false;
   return;
 }

 // get TDRP params

 _paramsPath = (char *) "unknown";
 
 if (_params.loadFromArgs(argc, argv, _args.override.list,
                           &_paramsPath)) {
   cerr << "ERROR: " << _progName << endl;
   cerr << "Problem with TDRP parameters" << endl;
   isOK = false;
 }

 // init process mapper registration
 
 PMU_auto_init((char *) _progName.c_str(),
	       _params.instance,
	       PROCMAP_REGISTER_INTERVAL);
}

// destructor. 

GenPoly2Mdv::~GenPoly2Mdv()
{

}

//////////////////////////////////////////////////
// Run

int GenPoly2Mdv::Run()
{

  
  //
  // Set up trigger
  //
  switch ( _params.triggerMode )
    { 

    case Params::TRIGGER_ARCHIVE :
      {
	//
	// Archive mode.
	//
	DateTime start( _params.start_time);
	
	DateTime end( _params.end_time);
	
	//
	// Setup the first process interval
	//
	time_t time_begin = start.utime();
	
	time_t time_end = end.utime();
	
	do 
	  {
	    _run(time_begin - _params.lookback,  time_begin );
	    
	    time_begin  = time_begin + _params.trigger_interval;
	    
	  } while (time_begin < time_end);
	
	break;
      }
    
    case Params::TRIGGER_INTERVAL:
      {
	//
	// Realtime mode triggered on specified interval. 
	//
	while (true)
	  {
	    //
	    // Run right now.
	    //
	    time_t time_end = time(0);
	    
	    time_t time_begin = time_end - _params.lookback;
	    
	    _run(time_begin, time_end);
	    
	    //
	    // Delay for the TriggerInterval before the next run.
	    // Register every 15 seconds.
	    //
	    for (int i=0; i < _params.trigger_interval; i++)
	      {
		sleep(1);
		
		if ((i % 15) == 0) 
		  PMU_auto_register("GenPoly2Mdv:Waiting to trigger");
	      }
	  } 
	break;
      }

  case Params::TRIGGER_LDATAINFO:
    {
      //
      // Realtime mode triggered by latest_data_info file. 
      //
      // Run forever.
      //
      
      time_t lastTime = 0;
      
      while (1)
	{    
	  LdataInfo ldata(_params.trigger_dir);
	  
	  if (!(ldata.read(_params.max_valid_age)))
	    {
	      time_t dataTime = ldata.getLatestTime();
	      
	      if ( dataTime <= lastTime)
		{
		  sleep(1);
		  
		  PMU_auto_register("GenPoly2Mdv: Waiting for new data.");
		} 
	      else 
		{
		  _run(dataTime - _params.lookback, dataTime);
		  
		  lastTime = dataTime; 
		}
	    }
	}
      break;
    }
    
  case Params::TRIGGER_FMQ:
    {
      NowcastQueue nowcastQueue;
      
      if (nowcastQueue.initReadOnly(_params.trigger_fmq, "GenPoly2Mdv", _params.debug))
	{
	  cerr << "GebPoly2Mdv::_run(): Failed to initialize queue " << _params.trigger_fmq << endl;
	  exit(-1);
	}
      
      int got_request = 0;
      time_t fmqIssueTime;
      size_t count;
      time_t deltaTime;
      string saysWho;
      
      //
      // run forever
      //
      while(true)
	{
	  //
	  // Poll for a request to run
	  //
	  while (!got_request)
	    {
	      sleep(1);
	      
	      PMU_auto_register( "GenPoly2Mdv::Run: Polling for trigger request" );
	      
	      if ( nowcastQueue.nextTrigger( saysWho, &fmqIssueTime, &count, &deltaTime ) == 0) 
		{
		  got_request = 1;
		}
	    } 
	  
	  _run(fmqIssueTime - _params.lookback, fmqIssueTime);
	  
	  got_request = 0;
	  
	} // Endless loop.
      
      break;
      
    } //end case TRIGGER_FMQ
    
    } //end switch

  return 0;

}
      
//////////////////////////////////////////////////
//
// _run
//
int GenPoly2Mdv::_run(time_t time_begin, time_t time_end)
{
  //
  // Read the spdb database of polygons.
  //
  DsSpdb spdbMgr;

  if (spdbMgr.getInterval(_params.spdb_url, time_begin, time_end))
    {
      cerr << "GenPoly2Mdv::_run(): getInterval failed for " << _params.spdb_url << ", [" 
	   << time_begin << "," << time_end << "]" <<  endl;
      return -1;
    }
  
  //
  // Get chunk data and references
  //
  int nChunks = spdbMgr.getNChunks();

  vector <Spdb::chunk_t> chunks  = spdbMgr.getChunks();

  if (_params.debug)
    {
      cerr << "GenPoly2Mdv::_run(): "
	   << nChunks << " polygons found." << endl;
    }


  //
  // Create and initialize MdvMgr object
  //
  MdvMgr *mdvMgr = new MdvMgr(_params, time_begin, time_end);

  mdvMgr->init();

  //
  // Loop through the chunks and disassmeble.
  //
  //
  for (int i = 0; i < nChunks; i++)
   {
     
     //
     // Dissassemble the data into a GenPoly struct.
     //
     GenPoly genPoly;
     
     bool success = genPoly.disassemble((const void*) chunks[i].data, chunks[i].len);

     if (!success)
       {
	 cerr << "ERROR: GenPoly2Mdv::_run()\n";
	 cerr << genPoly.getErrStr() << endl;	
	 continue;
       }
  
     //
     // Add polygon to Mdv file
     //
     int ret = mdvMgr->addPolygon(genPoly);

     if (ret)
       {
	 cerr << "ERROR: GenPoly2Mdv::_run(): error adding polygon to mdv file.\n";
       }
     
   } // End of loop through all GenPoly object.
  
  mdvMgr->write();
  
  delete mdvMgr;
  
  return 0;  
}






