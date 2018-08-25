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
#include "Ltg2RampClosures.hh"
#include <euclid/Pjg.hh>
#include <toolsa/DateTime.hh>
using namespace std;

//
// Constructor
//
Ltg2RampClosures::Ltg2RampClosures(int argc, char **argv)
{
  isOK = true;

  //
  // set programe name
  //
  _progName = "Ltg2RampClosures";

  ucopyright((char *) _progName.c_str());

  //
  // get command line args
  //
  if (_args.parse(argc, argv, _progName)) 
    {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Problem with command line args" << endl;
      isOK = FALSE;
      return;
    }

  //
  // get TDRP params
  //
  _paramsPath = (char *) "unknown";
  
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) 
    {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Problem with TDRP parameters" << endl;
      isOK = FALSE;
      return;
    }

  //
  // init process mapper registration
  //
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;
}

//////////////////////////////////////////////////////
//
// destructor
//
Ltg2RampClosures::~Ltg2RampClosures()

{
  _clear();

  //
  // unregister process
  //
  PMU_auto_unregister();  
}

void Ltg2RampClosures::_clear()
{

}


//////////////////////////////////////////////////
// 
// Run
//
int Ltg2RampClosures::Run ()
{

  DateTime dTime;

  //
  // register with procmap
  //
  PMU_auto_register("Run");

  //
  // set up triggering
  //
  if (_params.mode == Params::ARCHIVE)
    {
      //
      // Create archive timelist trigger
      //
      if (_params.debug)
	{
	  cerr << "Ltg2RampClosures::Run(): Creating ARCHIVE trigger\n"  
	       << "     start time: " << dTime.strn(_args.startTime) 
	       << "     end time:   " << dTime.strn(_args.endTime) 
	       << endl;
	}      
    }
  else
    {
      cerr << "Presently there is only an ARCHIVE mode. Sorry.\n";
      return 1;
    }
  
  _initRampRegions();


  //
  //
  // process data
  //
  if ( _processData(_args.startTime, _args.endTime))
    {
      cerr << "Ltg2RampClosures::Run(): Errors in processing time: [" 
	   <<  dTime.strn(_args.startTime) << ", " 
	   <<  dTime.strn(_args.endTime) << "] " << endl; 
      return 1;
    }

  return 0; 
}


///////////////////////////////////
//
//  process data 
//
int Ltg2RampClosures::_processData(time_t start_time, time_t end_time)
{
  DateTime dTime;

  //
  // registration with procmap
  //
  PMU_force_register("Processing data");
 
  for (int j = 0; j < _rampRegions.size(); j++)
  {
    _rampRegions[j]->setIntervalStart(start_time); 
     _rampRegions[j]->setIntervalEnd(end_time); 
  }
  

  //
  // Process in time intervals of  _params.processingInterval or less
  //
  time_t beginT = start_time;
  time_t endT;
  if (beginT + _params.processingInterval <  end_time)
    endT = beginT + _params.processingInterval;
  else
    endT = end_time;

  do
  {
    if (_readSpdb( beginT, endT) )
    {
      cerr << "Ltg2RampClosures::_processData(): ERROR: Failure to read " 
	   << "database! " << " (Check URL). Time interval:  [" 
	   << dTime.strn(_args.startTime) << ", " 
	   << dTime.strn(_args.endTime) << "] " << endl;
    }
    
    beginT = endT + 1;
    
    if (beginT + _params.processingInterval <  end_time)
      endT = beginT +  _params.processingInterval ;
    else
      endT = end_time;
  }
  while (beginT < end_time);
  
  for (int j = 0; j < _rampRegions.size(); j++)
  {
    _rampRegions[j]->computeClosures();    
  }
  
  //
  // Cleanup
  //
  _clear();
  
  return 0;
}

/////////////////////////////////////////////////////////////////
//
// Read ltg database for time interval and store relevant strike time 
// information.
//
int Ltg2RampClosures::_readSpdb(time_t time_begin, time_t time_end)
{
  DateTime dTime;

  DsSpdb spdbMgr;

  //
  // Process in increments of an hour
  //


  if (spdbMgr.getInterval(_params.ltgUrl, time_begin, time_end))
  {
    cerr << "Ltg2RampClosures::_processData(): getInterval failed for " 
	 << _params.ltgUrl << ", [" <<  dTime.strn(time_begin) << "," 
	 <<  dTime.strn(time_end) << "]" <<  endl;
    return 1;
  }

  //
  // Get chunk data
  //
  int nChunks = spdbMgr.getNChunks();
    
  vector <Spdb::chunk_t> chunks  = spdbMgr.getChunks();
  
  if (_params.debug)
  {
    cerr << "Ltg2RampClosures::readSpdb(): processing interval ["
	 << dTime.strn(time_begin) << ", "  << dTime.strn(time_end) << "]" 
	 << endl;
  }
  
  //
  // Loop through the chunks,disassmeble, and save. 
  //  
  for (int i = 0; i < nChunks; i++)
  {
    int data_len = chunks[i].len;
    
    // check for extended data
    
    if (data_len < (int) sizeof(ui32)) 
    {
      return 1;
    }
    
    ui32 cookie;
    
    memcpy(&cookie, chunks[i].data, sizeof(cookie));
    
    if (cookie == LTG_EXTENDED_COOKIE) 
    {
      int num_strikes = data_len / sizeof(LTG_extended_t);
      
      LTG_extended_t *ltg_data = (LTG_extended_t*) (chunks[i].data);
      
      for (int strike = 0; strike < num_strikes; strike++) 
      {
	LTG_extended_from_BE(&ltg_data[strike]);
	
	for (int j = 0; j < _rampRegions.size(); j++)
	{
	  _rampRegions[j]->process(ltg_data[strike]);
	}
      }
    } 
    else 
    {  
      int num_strikes = data_len / sizeof(LTG_strike_t); 
      
      LTG_strike_t *ltg_data = (LTG_strike_t *)(chunks[i].data);
      
      for (int strike = 0; strike < num_strikes; strike++) 
      {	   
	LTG_from_BE(&(ltg_data[strike]));
	
	for (int j = 0; j < _rampRegions.size(); j++)
	{
	  _rampRegions[j]->process(ltg_data[strike]);
	}
      }
    }
  }
  
  return 0;
}

void Ltg2RampClosures::_initRampRegions()
{
  for(int i = 0; i < _params.rampRegions_n; i ++)
    {
      cerr << "_initRampRegions :Initializing region " <<  _params._rampRegions[i].regionName << endl;

      Pjg proj;

      proj.initLatlon(3661,1837,1,0.01912, 0.017964, 0, -130,20,0);

      RampRegion *rampReg = new RampRegion(_params._rampRegions[i].regionName,
					   _params._rampRegions[i].lat,
					   _params._rampRegions[i].lon,
					   _params._rampRegions[i].radius,
					   _params._rampRegions[i].closureTime,
					   _params.asciiOutdir,
					   proj);

      _rampRegions.push_back(rampReg);
    }

}

