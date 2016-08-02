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

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/*
 *  $Id: MdvThin.cc,v 1.18 2016/03/04 02:22:15 dixon Exp $
 */

/////////////////////////////////////////////////////////////////////////
//
// Class:	MdvThin
//
// Author:	G M Cunning
//
// Date:	Thu Mar  2 11:38:25 2000
//
// Description: This class thins out and synchronizes a 
//		data stream. The data stream is supportted 
//		through MDV files.
//

// C++ include files
#include <cassert>

// System/RAP include files
#include <dsdata/DsIntervalTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxChunk.hh>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>

// Local include files
#include "Args.hh"
#include "Params.hh"
#include "MdvThin.hh"
using namespace std;


// the singleton itself
MdvThin *MdvThin::_instance = 0;


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Constructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

MdvThin::MdvThin(int argc, char **argv) :
  _isOK(true),
  _progName(""),
  _errStr(""),
  _args(0),
  _params(0)
{
  static const string methodName = "MdvThin::Constructor";
  _errStr = string("ERROR: ") + methodName;

 // Make sure the singleton wasn't already created.
  assert(_instance == 0);

  // Set the singleton instance pointer
  _instance = this;

  // set programe name
  Path pathParts(argv[0]);
  _progName = pathParts.getBase();

  // display ucopyright message and RCSid
  ucopyright(const_cast<char*>(_progName.c_str()));

  // get command line args
  _args = new Args(argc, const_cast<char**>(argv), _progName);
  if (!_args->isOK) {
    cerr << _errStr << endl;
    cerr << "Problem with command line arguments." << endl;
    _isOK = false;
  }

  // get TDRP params
  _params = new Params();
  char *paramsPath = (char *)"unknown";
  if (_params->loadFromArgs(argc, argv, _args->override.list, 
			    &paramsPath)) {
    cerr << _errStr << endl;
    cerr << "Problem with TDRP parameters" << endl;
    _isOK = false;
  }


  switch (_params->runMode)
  {
  case Params::REALTIME :
  {
    if (_params->debug)
    {
      cerr << "Initializing REALTIME trigger:" << endl;
      cerr << "    URL: " << _params->inputUrl << endl;
    }
    
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->inputUrl,
		      _params->maxValidAge,
		      PMU_auto_register) != 0)
    {
      cerr << _errStr << endl;
      cerr << "Error initializing LDATA trigger" << endl;
      cerr << "    URL: " << _params->inputUrl << endl;

      _isOK = false;
    }
    else
    {
      _trigger = trigger;
    }
  }
  break;
  
  case Params::ARCHIVE :
  {
    if (_params->debug)
    {
      cerr << "Initializing ARCHIVE trigger:" << endl;
      cerr << "   URL: " << _params->inputUrl << endl;
      cerr << "   Start time: "
	   << DateTime::str(_args->getArchiveStartTime()) << endl;
      cerr << "   End time: "
	   << DateTime::str(_args->getArchiveEndTime()) << endl;
    }
    
    DsTimeListTrigger *trigger = new DsTimeListTrigger();
    if (trigger->init(_params->inputUrl,
		      _args->getArchiveStartTime(),
		      _args->getArchiveEndTime()) != 0)
    {
      cerr << _errStr << endl;
      cerr << "Error initializing TIME_LIST trigger" << endl;
      cerr << "   URL: " << _params->inputUrl << endl;
      cerr << "   Start time: "
	   << DateTime::str(_args->getArchiveStartTime()) << endl;
      cerr << "   End time: "
	   << DateTime::str(_args->getArchiveEndTime()) << endl;
      
      _isOK = false;
    }
    else
    {
      _trigger = trigger;
    }
  }
  break;
  
  case Params::INTERVAL_REALTIME :
  {
    if (_params->debug)
    {
      cerr << "Initializing INTERVAL_REALTIME trigger:" << endl;
      cerr << "   Interval secs: " << _params->intervalSecs << endl;
      cerr << "   Interval start secs: " << _params->intervalStartSecs << endl;
    }
    
    DsIntervalTrigger *trigger = new DsIntervalTrigger();
    if (trigger->init(_params->intervalSecs,
		      _params->intervalStartSecs,
		      _params->sleepSecs,
		      PMU_auto_register) != 0)
    {
      cerr << _errStr << endl;
      cerr << "Error initializing INTERVAL_REALTIME trigger" << endl;
      cerr << "   Interval secs: " << _params->intervalSecs << endl;
      cerr << "   Interval start secs: " << _params->intervalStartSecs << endl;
      
      _isOK = false;
    }
    else
    {
      _trigger = trigger;
    }
  }
  break;
  
  case Params::INTERVAL_ARCHIVE :
  {
    if (_params->debug)
    {
      cerr << "Initializing INTERVAL_ARCHIVE trigger:" << endl;
      cerr << "   Interval secs = " << _params->intervalSecs << endl;
      cerr << "   Start time = "
	   << DateTime::str(_args->getArchiveStartTime()) << endl;
      cerr << "   End time = "
	   << DateTime::str(_args->getArchiveEndTime()) << endl;
    }
    
    DsIntervalTrigger *trigger = new DsIntervalTrigger();
    if (trigger->init(_params->intervalSecs,
		      _args->getArchiveStartTime(),
		      _args->getArchiveEndTime()) != 0)
    {
      cerr << _errStr << endl;
      cerr << "Error initializing INTERVAL_ARCHIVE trigger" << endl;
      cerr << "   Interval secs: " << _params->intervalSecs << endl;
      cerr << "   Start time: "
	   << DateTime::str(_args->getArchiveStartTime()) << endl;
      cerr << "   End time: "
	   << DateTime::str(_args->getArchiveEndTime()) << endl;
      
      _isOK = false;
    }
    else
    {
      _trigger = trigger;
    }
  }
  break;
  
  case Params::TRIGGER_REALTIME :
  {
    if (_params->debug)
    {
      cerr << "Initializing TRIGGER_REALTIME trigger:" << endl;
      cerr << "    URL: " << _params->triggerUrl << endl;
    }

    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->triggerUrl,
		      _params->maxValidAge,
		      PMU_auto_register) != 0)
    {
      cerr << _errStr << endl;
      cerr << "Error initializing LDATA trigger" << endl;
      cerr << "    URL: " << _params->triggerUrl << endl;

      _isOK = false;
    }
    else
    {
      _trigger = trigger;
    }
  }

  break;

  case Params::TRIGGER_ARCHIVE :
  {
    if (_params->debug)
    {
      cerr << "Initializing TRIGGER_ARCHIVE trigger:" << endl;
      cerr << "   URL: " << _params->triggerUrl << endl;
      cerr << "   Start time: "
	   << DateTime::str(_args->getArchiveStartTime()) << endl;
      cerr << "   End time: "
	   << DateTime::str(_args->getArchiveEndTime()) << endl;
    }

    DsTimeListTrigger *trigger = new DsTimeListTrigger();
    if (trigger->init(_params->triggerUrl,
		      _args->getArchiveStartTime(),
		      _args->getArchiveEndTime()) != 0)
    {
      cerr << _errStr << endl;
      cerr << "Error initializing TIME_LIST trigger" << endl;
      cerr << "   URL: " << _params->triggerUrl << endl;
      cerr << "   Start time: "
	   << DateTime::str(_args->getArchiveStartTime()) << endl;
      cerr << "   End time: "
	   << DateTime::str(_args->getArchiveEndTime()) << endl;
      
      _isOK = false;
    }
    else
    {
      _trigger = trigger;
    }
  }

  break;
  } /* endswitch - _params->runMode */
  
  // init process mapper registration
  PMU_auto_init((char *) _progName.c_str(),
		_params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Destructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
  
MdvThin::~MdvThin()
{
  // unregister process

  PMU_auto_unregister();
  delete _args;
  delete _params;
  delete _trigger;
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Public Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	MdvThin::instance
//
// Description:	Retrieves the singleton instance.
//
// Returns:	
//
// Globals:	
//
// Notes:
//
//

MdvThin* 
MdvThin::instance( int argc, 
			   char **argv )
{
  if ( _instance == 0 ) {
    _instance = new MdvThin(argc, argv);
  }
  
  return(_instance);
}

MdvThin* 
MdvThin::instance()
{
  assert(_instance != 0 );
  return( _instance );
}



//////////////////////////////////////////////////
// run
//
// Returns 0 if everything worked fine, non-zero if there
// was an error.

int MdvThin::run ()
{
  static const string methodName = "MdvThin::run";
  cout << methodName << endl;


  // register with procmap
  PMU_auto_register("Run");

  // Process the files

  time_t prev_data_time = 0;
  
  while (!_trigger->endOfData())
  {
    DateTime trigger_time;
    

    if (_trigger->nextIssueTime(trigger_time) != 0)
    {
      cerr << _errStr << endl;
      cerr << "Trigger error -- trying again" << endl;
      _clearErrStr();

      continue;
    }
    
    if (trigger_time.utime() - prev_data_time < _params->minDataInterval)
      continue;
      
    if (_processMessage(trigger_time))
    {
      prev_data_time = trigger_time.utime();
    }
    else
    {
      cerr << _errStr << endl;
      _clearErrStr();
    }
      
  }  /* endwhile - true */
  
  return 0;
  
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Private Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	MdvThin::_processMessage
//
// Description:	handles proceesing of message. processing includes 
//		reading, writing and timestamp modification, if 
//		requested.
//
// Returns:	
//
// Globals:	
//
// Notes:
//
//

bool 
MdvThin::_processMessage(const DateTime &trigger_time)
{
  static const string methodName = "MdvThin::_processMessage";
  string statusStr;

  statusStr = string("Processing time: ") + trigger_time.getStr();

  if (_params->debug)
    cerr << endl << statusStr << endl;
  PMU_auto_register((char*)(statusStr.c_str()));

  // read message -- use the default to get all fields and chunks
  DsMdvx mdvx;
  
  mdvx.setDebug(_params->debug);
  mdvx.clearRead();
  //  mdvx.setReadTime(Mdvx::READ_CLOSEST,
  mdvx.setReadTime((Mdvx::read_search_mode_t)_params->readSearchMode,
		   _params->inputUrl,
		   _params->maxValidAge, trigger_time.utime());

  // read the data
  if (mdvx.readVolume() < 0) {
    _errStr += string("ERROR: ") + methodName + string("\n");
    _errStr += mdvx.getErrStr() + string("\n");

    return false;
  }

  // Update times in the file
  _updateTimes(mdvx, trigger_time);
  
  // write message
  mdvx.clearWrite();
  if ( mdvx.writeToDir(_params->outputUrl) < 0 ) {
    _errStr = string("ERROR: ") + methodName + string("\n");
    _errStr += mdvx.getErrStr() + string("\n");

    return false;
  }
  
  return true;
  
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Helper Functions
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	MdvThin::_updateTimes
//
// Description:	Updates the times in the input file.
//
// Returns:	
//
// Globals:	
//
// Notes:
//
//

void MdvThin::_updateTimes(Mdvx &mdvx, const DateTime &trigger_time) const
{
  // Update the generation time in the master header.  This is always
  // updated.
  Mdvx::master_header_t master_hdr = mdvx.getMasterHeader();
  //  master_hdr.time_gen = time(0);
  master_hdr.time_written = time(0);
  mdvx.setMasterHeader(master_hdr);
  
  // Do the optional time updates
  if (_params->updateOutputFileTime)
  {
    int time_change = trigger_time.utime() - master_hdr.time_centroid;
    
    // Update the master header times

    master_hdr.time_gen += time_change;
    master_hdr.time_centroid += time_change;
    master_hdr.time_begin += time_change;
    master_hdr.time_end += time_change;
    
    mdvx.setMasterHeader(master_hdr);
    
    if (_params->debug)
    {
      cerr << "  Time change: " << time_change << " secs" << endl;
      cerr << "  New centroid time: " << master_hdr.time_centroid << endl;
    }
    
    // Update each of the fields

    for (int i = 0; i < master_hdr.n_fields; ++i)
    {
      MdvxField *field = mdvx.getField(i);
      
      Mdvx::field_header_t field_hdr = field->getFieldHeader();
      
      field_hdr.forecast_time += time_change;
      
      field->setFieldHeader(field_hdr);
    } /* endfor - i */
    
  } /* endif - _params->updateOutputFileTime */
  
}
