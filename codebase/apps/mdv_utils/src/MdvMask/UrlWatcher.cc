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


#include <tdrp/tdrp.h>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <Mdv/DsMdvxTimes.hh>
#include <dsdata/DsTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsMultipleTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <sys/stat.h>

#include <iostream>
#include <strings.h>

#include "Params.hh"
#include "UrlWatcher.hh"
#include "Process.hh"
using namespace std;

// define any constants
const string UrlWatcher::_className = "MdvMask";

//
// Constructor
//
UrlWatcher::UrlWatcher(){
}
//////////////////////////////////////////////  
//
// Destructor
//
UrlWatcher::~UrlWatcher(){
}

//////////////////////////////////////////////////
// Main method - run.
//
int UrlWatcher::init( int argc, char **argv ){

  const string methodName = _className + string( "::init()" );

  //
  // Parse command line args. Pretty minimal for this program.
  //

  tdrp_override_t tdrp_override;
  TDRP_init_override(&tdrp_override);
  
  if (ParseArgs(argc,argv, tdrp_override)) return -1;

  //
  // Get TDRP parameters.
  //
  if (P.loadFromArgs(argc,argv,tdrp_override.list,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }                       

  //
  // Register with the process mapper.
  //

  PMU_auto_init("MdvMask", P.Instance,
                PROCMAP_REGISTER_INTERVAL);     

  //
  // Check input parameters for vailidity.
  //
  if (P.InFieldName_n != P.OutFieldName_n){
    cerr << "Invalid number of elements specified in OutFieldName array.";
    cerr << endl;
    cerr << "Check parameter file." << endl;
    exit(-1);      
  }

  switch(P.Mode)
  {
    case Params::COMMAND_LINE :
    {

        if (outUrl.size() > 0) {
            P.OutUrl = (char*) outUrl.c_str();
        }

        // Determine the trigger time based on the input file name.
        size_t f_loc = inputFile.rfind("/f_");
        if (f_loc == string::npos) {
	      cerr << "Could not determine forecast seconds for inputFile:" << endl;
	      cerr << inputFile << endl;
          exit(-1);
        }

        size_t g_loc = inputFile.rfind("/g_");
        if (g_loc == string::npos) {
	      cerr << "Could not determine generation time for inputFile:" << endl;
	      cerr << inputFile << endl;
          exit(-1);
        }

        size_t d_loc = inputFile.rfind("/", (g_loc - 1));
        if (d_loc == string::npos) {
	      cerr << "Could not determine YMD for inputFile:" << endl;
	      cerr << inputFile << endl;
          exit(-1);
        }

        // Drop the slash on these substring locations.
        g_loc++;
        d_loc++;

        string dayString( inputFile, d_loc, (g_loc - d_loc - 1) );
        if ( P.Debug == TRUE )
          cerr << "Got dayString: " << dayString << endl;

        string genString( inputFile, g_loc, (f_loc - g_loc) );
        if ( P.Debug == TRUE )
          cerr << "Got genString: " << genString << endl;

        string hourString( genString, 2, string::npos );
        if ( P.Debug == TRUE )
          cerr << "Got hourString: " << hourString << endl;

        string fcstString( inputFile, (f_loc + 1), string::npos );
        if ( P.Debug == TRUE )
          cerr << "Got fcstString: " << fcstString << endl;

        string secsString( fcstString, 2, 8 );
        if ( P.Debug == TRUE )
          cerr << "Got secsString: " << secsString << endl;

		string dateString(dayString + hourString);
        if ( P.Debug == TRUE )
          cerr << "Got dateString: " << dateString << endl;

        Process S;
        DateTime triggerTime(dateString);
        
        int leadSecs = atoi(secsString.c_str());
        if ( P.Debug == TRUE )
          cerr << "Got leadSecs: " << leadSecs << endl;
        triggerTime.setLeadSecs(leadSecs);

        if ( P.Debug == TRUE )
          cerr << "Got input time: " << triggerTime << endl;

        if ( P.Debug == TRUE )
          cerr << "Got forecastUtime: " << triggerTime.forecastUtime() << endl;

        string impliedInputPath(inputFile, 0, (d_loc - 1));
        if ( P.Debug == TRUE )
          cerr << "Got impliedInputPath: " << impliedInputPath << endl;

        // 
        // Set the input path, based on the location of the input file.
        // 
        P.InputUrl = (char *) impliedInputPath.c_str();

        if ( S.Derive( &P, triggerTime.forecastUtime() ) )  {
            cerr << "Error processing command-line data for file: " << inputFile << endl;
            exit(-1);
        }

    exit(0);
    break;

    }

    case Params::REALTIME :
    case Params::LATEST_DATA :
    case Params::REALTIME_FCST_DATA :
    {
	if(P.Debug) {
	  cerr << "Initializing LATEST_DATA trigger using url: " <<
		P.TriggerUrl << endl;
	} // endif -- _params->Debug
	
	DsLdataTrigger *trigger = new DsLdataTrigger();
	if(trigger->init(P.TriggerUrl, 
			 P.MaxRealtimeValidAge,
			 PMU_auto_register) != 0) {
	    cerr << "ERROR: " << methodName << endl;
	    cerr << "Error initializing trigger using url: " <<
		P.TriggerUrl << endl;
	    return false;
	} // endif -- trigger->init ...
	
	_dataTrigger = trigger;
    
	break;
    }
    
    case Params::TIME_LIST :
    case Params::ARCHIVE :
    {
	time_t start_time =
	    DateTime::parseDateTime(P.time_list_trigger.start_time);
	time_t end_time
	    = DateTime::parseDateTime(P.time_list_trigger.end_time);
    
	if(start_time == DateTime::NEVER) {
	    cerr << "ERROR: " << methodName << endl;
	    cerr << "Error parsing start_time string for TIME_LIST trigger: " <<
		P.time_list_trigger.start_time << endl;
      
	    return false;
	} // endif -- start_time == DateTime::NEVER
    
	if(end_time == DateTime::NEVER) {
	    cerr << "ERROR: " << methodName << endl;
	    cerr << "Error parsing end_time string for TIME_LIST trigger: " <<
		P.time_list_trigger.end_time << endl;
      
	    return false;
	} // endif -- start_time == DateTime::NEVER
    
	if(P.Debug) {
	    cerr << "Initializing TIME_LIST trigger: " << endl;
	    cerr << "   url: " << P.TriggerUrl << endl;
	    cerr << "   start time: " << start_time << endl;
	    cerr << "   end time: " << end_time << endl;
	} // endif -- P.Debug
    
    
	DsTimeListTrigger *trigger = new DsTimeListTrigger();
	if(trigger->init(P.TriggerUrl,
			 start_time, end_time) != 0) {
	    cerr << "ERROR: " << methodName << endl;
	    cerr << "Error initializing TIME_LIST trigger for url: " <<
		P.TriggerUrl << endl;
	    cerr << "    Start time: " << P.time_list_trigger.start_time <<
		endl;
	    cerr << "    End time: " << P.time_list_trigger.end_time << endl;
      
	    return false;
	} // endif -- trigger->init
    
	_dataTrigger = trigger;
    
	break;
    }

    case Params::MULTIPLE_URL :
    {
	if(P.Debug) {
	    cerr << "Initializing MULTIPLE_URL trigger using urls: " << endl;
	    for (int i = 0; i < P.multiple_url_trigger_n; ++i) {
		cerr << "    " << P._multiple_url_trigger[i] << endl;
	    }
	} // endif -- P.Debug
    
	DsMultipleTrigger *trigger = new DsMultipleTrigger();
	
	if(!trigger->initRealtime(-1, PMU_auto_register)) {
	    cerr << "ERROR: " << methodName << endl;
	    cerr << "Error initializing MULTIPLE_URL trigger using urls: " << endl;
	    for (int i = 0; i < P.multiple_url_trigger_n; ++i)
		cerr << "    " << P._multiple_url_trigger[i] << endl;
      
	    return false;
	} // endif -- !trigger->initRealtime

	if(P.trigger_type == Params::TRIGGER_ALL)
	    trigger->setTriggerType(DsMultipleTrigger::TRIGGER_ALL);
	else
	    trigger->setTriggerType(DsMultipleTrigger::TRIGGER_ANY_ONE);

	for (int i = 0; i < P.multiple_url_trigger_n; ++i) {
	    trigger->add(P._multiple_url_trigger[i]);
	}
	trigger->set_debug(P.Debug);
    
	_dataTrigger = trigger;
    
	break;
    }

  }
  return 0;
  
}

int UrlWatcher::run()
{
    const string methodName = _className + string( "::run()" );
    
// register with procmap
    PMU_auto_register("Running");

    if(P.Debug) {
	cerr << "Running:" << endl;
    } // endif -- _params->Debug_mode != Params::DEBUG_OFF

    DateTime triggerTime;
    
// Do something
    while (!_dataTrigger->endOfData()) {
      /*
	if (_dataTrigger->nextIssueTime(triggerTime) != 0)  {
	    cerr << "ERROR: " << methodName << endl;
	    cerr << "Error getting next trigger time" << endl;
      
	    continue;
	}
      */
	TriggerInfo triggerInfo;
	_dataTrigger->next(triggerInfo);
	int fcst_lead_time = 0;
	if (P.Mode == Params::REALTIME_FCST_DATA ||
	    P.Mode == Params::COMMAND_LINE)
	  fcst_lead_time = triggerInfo.getForecastTime() - triggerInfo.getIssueTime();
	
	Process S;
	if (S.Derive(&P,triggerInfo.getIssueTime(), fcst_lead_time))  {
	  cerr << "Error processing data for issue time: "
	       << DateTime::str(triggerInfo.getIssueTime()) << ", lead secs: " << fcst_lead_time << endl;
      
	    continue;
	}
    
    } // endwhile - !_dataTrigger->endOfData()

    return 0;
}

///////////////////////////////////////////////
// Parse command line args.
//
int UrlWatcher::ParseArgs(int argc,char *argv[], tdrp_override_t &override){

  string override_str;
  DateTime start_time = DateTime::NEVER;
  DateTime end_time = DateTime::NEVER;
  
  for (int i=1; i<argc; i++){
 
    if ( !strcmp(argv[i], "-h") ||
	 !strcmp(argv[i], "-help") ||
         !strcmp(argv[i], "--") ||
         !strcmp(argv[i], "-?") ||
	 !strcmp(argv[i], "-man")) {
      _printUsage(cout);
      return -1;
    }
    
    if (!strcmp(argv[i], "-d") ||
	!strcmp(argv[i], "-debug")){
      override_str = "Debug = true;";
      TDRP_add_override(&override, override_str.c_str());
    }
      
    if (!strcmp(argv[i], "-if")){
      i++;
      if (i == argc) {
        cerr << "Must specify input file with -if" << endl;
        exit(-1);
      }

      // Set the mode parameter to COMMAND_LINE

      override_str = "Mode = COMMAND_LINE;";
      TDRP_add_override(&override, override_str.c_str());
      
      // Make sure the input file exists

      struct stat file_stat;
      inputFile = argv[i];
      if ( stat(inputFile.c_str(), &file_stat) ) {
        cerr << "Specified input file doesn't exist: "
             << inputFile << "." << endl;
        exit(-1);
      }
      
    }
      
    if (!strcmp(argv[i], "-out_url")){
      i++;
      if (i == argc) {
        cerr << "Must specify output url with -out_url option" << endl;
        exit(-1);
      }
      
      outUrl = argv[i];
      
    }
      
    if (!strcmp(argv[i], "-interval")){
      i++;
      if (i == argc) {
        cerr << "Must specify start and end times with -interval" << endl;
        exit(-1);
      }
      
      start_time.set(argv[i]);
      if (start_time == DateTime::NEVER){
        cerr << "Invalid start time specified: " << argv[i] << endl;
        return -1;
      }
    
      i++;
      if (i == argc) {
        cerr << "Must specify end time with -interval" << endl;
        return -1;
      }

      end_time.set(argv[i]);
      
      if (end_time == DateTime::NEVER){
        cerr << "Invalid end time specified: " << argv[i] << endl;
        exit(-1);
      }

      // Set the mode parameter to TIME_LIST

      override_str = "Mode = TIME_LIST;";
      TDRP_add_override(&override, override_str.c_str());
      
    }

    if (!strcmp(argv[i], "-start") ||
	!strcmp(argv[i], "-startTime")){
      i++;
      if (i == argc) {
        cerr << "Must specify time with -start" << endl;
        exit(-1);
      }
      
      start_time.set(argv[i]);
      if (start_time == DateTime::NEVER){
        cerr << "Invalid start time specified: " << argv[i] << endl;
        return -1;
      }

      // Set the mode parameter to TIME_LIST

      override_str = "Mode = TIME_LIST;";
      TDRP_add_override(&override, override_str.c_str());
      
    }

    if (!strcmp(argv[i], "-end")){
      i++;
      if (i == argc) {
        cerr << "Must specify time with -end" << endl;
        exit(-1);
      }
      
      end_time.set(argv[i]);
      
      if (end_time == DateTime::NEVER){
        cerr << "Invalid end time specified: " << argv[i] << endl;
        exit(-1);
      }

      // Set the mode parameter to TIME_LIST

      override_str = "Mode = TIME_LIST;";
      TDRP_add_override(&override, override_str.c_str());
      
    }

    // If times were specified on the command line, update the time_list_trigger
    // parameter override

    if (start_time != DateTime::NEVER ||
	end_time != DateTime::NEVER)
    {
      override_str = "time_list_trigger = {\"" + start_time.getStrn() +
	"\", \"" + end_time.getStrn() + "\"};";
      TDRP_add_override(&override, override_str.c_str());
    }
    

  }

  return 0; // All systems go.
  
}

void UrlWatcher::_printUsage(ostream &out)
{
  out << "Usage:" << endl;
  out << endl;
  out << "MdvMask [options] as below:" << endl;
  out << endl;
  out << "     [ --, -help, -man ] produce this list" << endl;
  out << "     [ -debug ] debugging on" << endl;
  out << "     [ -interval YYYYMMDDhhmmss YYYYMMDDhhmmss ] set time interval (TIME_LIST mode)" << endl;
  out << "     [ -start \"YYYY MM DD hh mm ss\" ] specify the start time (TIME_LIST mode)" << endl;
  out << "     [ -end \"YYYY MM DD hh mm ss\" ] specify the end time (TIME_LIST mode)" << endl;
  out << "     [ -if input_file ] sets the trigger time based on the name of the given input file." << endl;
  out << "                        The file name must be a forecast file and the file path" << endl;
  out << "                        is used to determine the trigger time to use." << endl;
  out << "     [ -out_url url ] specifies output location in command_line mode" << endl;
}
