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
// EventMan.cc
//
// EventMan object
//
// F. Hage 
//
// Sept 2005
//
///////////////////////////////////////////////////////////////

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>

#include "eventman.h"

using namespace std;

// Constructor

EventMan::EventMan(int argc, char **argv)
{
  _argc = argc;
  _argv = argv;
  isOK = true;

  // set programe name

  _progName = "EventMan";
  ucopyright( _progName);

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (gd.params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName,
		gd.params.instance,
		84300);
  
  return;

}

// destructor

EventMan::~EventMan()
{
  // unregister process
  PMU_auto_unregister();
}

//////////////////////////////////////////////////
// SAVEFILE

int EventMan::SaveFile (FILE *ofile)
{
	for(int i = 0; i < gd.Wev.size(); i++) {
		gd.Wev[i]->toAscii(ofile);
	}

	char t_buf1[128];
	time_t now = time(0);
	if(gd.params.use_localtime) {
	   strftime(t_buf1,128,"#\n#Last Update: %Y/%m/%d %T %Z",localtime(&now));
	} else {
	   strftime(t_buf1,128,"#\n#Last Update: %Y/%m/%d %T %Z",gmtime(&now));
	}
	fprintf(ofile,"%s\n",t_buf1);
}

#define EVMAN_BUF_SIZE 32768
#define EVMAN_LINE_SIZE 1024
//////////////////////////////////////////////////
// LOADFILE

int EventMan::LoadFile (FILE *infile)
{
	char buf[EVMAN_BUF_SIZE];
	char line[EVMAN_LINE_SIZE];
	char *ptr;
	
	memset(buf,0,EVMAN_BUF_SIZE);
	memset(line,0,EVMAN_LINE_SIZE);
	ptr = buf;
	while((fgets(line,EVMAN_LINE_SIZE,infile)) != NULL) {
		strncpy(ptr,line,EVMAN_BUF_SIZE - strlen(buf));

		if((ptr + strlen(line)) > (buf + EVMAN_BUF_SIZE)) {
			fprintf(stderr,"Problem Lines: - Too Big,  Truncating... Oh Dear!\n");
		}
		ptr += strlen(line);
		if(ptr > (buf + EVMAN_BUF_SIZE)) ptr = buf + EVMAN_BUF_SIZE;
		//
		// If we have a valid start - end line - Process the Buffer.
		if(strstr(line,"start") != NULL && strstr(line,"end") != NULL ) { 

			// Instantiate a new Weather Event Object.
			Wevent *W = new Wevent(buf);

			// Add it to our vector of Wether Events.
			gd.Wev.push_back(W);

			memset(buf,0,EVMAN_BUF_SIZE);
			ptr = buf;
		} 
	    memset(line,0,EVMAN_LINE_SIZE);
	}
}

//
//////////////////////////////////////////////////
// Run

int EventMan::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  FILE *evfile;

  if((evfile = fopen(gd.params.event_list_file,"rw")) == NULL) {
	  fprintf(stderr,"Problems opening %s\n",gd.params.event_list_file);
	  perror("EventMan");
	  exit(-1);
  }

  LoadFile(evfile);

  gd.cur_event = 0;

  fclose(evfile);

  init_xview(_argc, _argv);

  modify_xview_objects();

  /* Turn control over to XView.  */
  xv_main_loop(gd.Eventman_em_bw->em_bw);

  return 0;
}

