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
/////////////////////////////////////////////////////////////
// Main for procview application.
//
// Mike Dixon
// RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
// January 1997
/////////////////////////////////////////////////////////////

#include <MotifApp/Application.h>
#include <MotifApp/QuestionDialogManager.h>
#include "ProcviewWindow.h"
#include "Args.h"
#include "Procinfo.h"
#include <toolsa/port.h>
#include <signal.h>
#include <stream.h>

static void tidy_and_exit(int sig);
static void handle_sigpipe(int sig);

static MainWindow *Mwindow = NULL;

/////////////////////////////////////////////////////////////
// main()
/////////////////////////////////////////////////////////////


int main ( int argc, char **argv )

{

  /*
   * set signal handling
   */
  
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGPIPE, handle_sigpipe);

  // parse the command line args
  
  Args *args = Args::Inst(argc, argv);
  if (!args->OK) {
    tidy_and_exit(-1);
  }

  // create the procmap info object
  
  Procinfo::Inst();

  // instantiate an Application object
  
  Application *app = Application::Inst("procviewApp", &argc, argv);
  
  // create the main window
  
  Mwindow = new ProcviewWindow ( "procview");

  // initialize the application

  app->initialize ();

  // enter event loop

  app->handleEvents();

  // free up

  delete (app);

  tidy_and_exit(0);

}

// ignore SIGPIPE

static void handle_sigpipe(int sig)
{
  int i;
  i = sig;
  return;
}

// exit

void tidy_and_exit (int sig)
{

  if (Mwindow != NULL) {
    delete (Mwindow);
  }
  exit(sig);

}
