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
/////////////////////////////////////////////////
// ProcviewWindow.cc
//
// Mike Dixon
// January 1997
//
/////////////////////////////////////////////////

#include <MotifApp/Application.h>
#include <MotifApp/Timer.h>
#include <MotifApp/QuitCmd.h>
#include <MotifApp/RadioSet.h>
#include <MotifApp/UndoCmd.h>
#include <MotifApp/NoOpCmd.h>
#include <MotifApp/ExecuteFuncCmd.h>
#include <MotifApp/MenuList.h>
#include <MotifApp/SelectStrCmd.h>
#include <MotifApp/PromptStrCmd.h>
#include "Args.h"
#include "Procinfo.h"
#include "ProcviewWindow.h"
#include "Schematic.h"
#include "Legend.h"
#include "RunCmd.h"
#include "EditCmd.h"
#include "AddSymbolCmd.h"
#include "PrintProcsCmd.h"
#include <MotifApp/MenuBar.h>
#include <Xm/Form.h>
#include <Xm/Separator.h>
#include <stream.h>

ProcviewWindow::ProcviewWindow ( const char *name ) :
  MenuWindow ( name )
{
  _timer = NULL;
  _schematic = NULL;
  _legend = NULL;

  cerr << "procmap host is " << Args::Inst()->procmapHost << "\n";
  cerr << "procmap interval is " <<
    Args::Inst()->params.procmapInterval << " secs \n";
}

ProcviewWindow::~ProcviewWindow ()
{
  delete _timer;
  delete _schematic;
  delete _legend;
}

Widget ProcviewWindow::createWorkArea ( Widget parent )
  
{

  // set up the timer
  
  _timer = new Timer (Procinfo::Inst()->queryMapper,
		      Args::Inst()->params.procmapInterval * 1000);
  
  // The ProcviewWindow work area is implemented as a form widget
  // that contains the other components of the interface
    
  Widget form =  XtCreateWidget ( "workArea", xmFormWidgetClass,
				  parent, NULL, 0 );
  
  // Create each major component of the app window
  
  _schematic = new Schematic ( "schematic", form );
  _legend = new Legend ( "legend", form );

  // Set up the attachments
  
  XtVaSetValues ( _legend->baseWidget(), 
		  XmNtopAttachment,    XmATTACH_NONE,
		  XmNleftAttachment,   XmATTACH_FORM,
		  XmNrightAttachment,  XmATTACH_FORM,
		  XmNbottomAttachment, XmATTACH_FORM,
		  NULL );
  
  Widget sep =  
    XtVaCreateManagedWidget ( "sep", 
			      xmSeparatorWidgetClass,
			      form,
			      XmNleftAttachment,   XmATTACH_FORM,
			      XmNrightAttachment,  XmATTACH_FORM,
			      XmNtopAttachment,    XmATTACH_NONE,
			      XmNbottomWidget, _legend->baseWidget(),
			      XmNbottomAttachment, XmATTACH_WIDGET,
			      NULL );
  
  XtVaSetValues ( _schematic->baseWidget(),
		  XmNtopAttachment,    XmATTACH_FORM,
		  XmNleftAttachment,   XmATTACH_FORM,
		  XmNrightAttachment,  XmATTACH_FORM,
		  XmNbottomWidget,     sep,
		  XmNbottomAttachment, XmATTACH_WIDGET,
		  NULL );
  
  // Manage all child widgets and return the form
  
  _schematic->manage();
  _legend->manage();    
  
  return ( form );        

}

void ProcviewWindow::createMenuPanes ()

{

  //////////////////////////////////////////////////////////////////
  // Create the main application menu
    
  MenuList *appList = new MenuList ( "Application" , 'A');
  appList->setTearOff();

  RunCmd *run = new RunCmd ("Run", 'R', FALSE, _schematic, _timer);
  EditCmd *edit = new EditCmd ("Edit", 'E', TRUE, _schematic, _timer);
  Cmd *hardCopy = new NoOpCmd("Hard Copy", 'C', FALSE);
  Cmd *refresh = new NoOpCmd("Refresh", 'f', FALSE);
  Cmd *quit = new QuitCmd ( "Quit", 'Q');

  appList->vaAdd (run, edit, 
		  UndoCmd::Inst(),
		  hardCopy, refresh,
		  quit, NULL);
  
  _menuBar->addCommands ( appList );
    
  // Create schematics menu

  MenuList *schList = new MenuList ( "Schematic", 'S', FALSE);
  schList->setTearOff();
    
  Cmd *schLoad = new NoOpCmd ( "Load", 'L', FALSE);
  Cmd *schSave = new NoOpCmd ( "Save", 'S' , FALSE);
  Cmd *schSaveAs = new NoOpCmd ( "Save As", 'A' , FALSE);
  Cmd *schPrint = new NoOpCmd ( "Print", 'P' , FALSE);

  schList->vaAdd (schLoad, schSave, schSaveAs, schPrint, NULL);
  _menuBar->addCommands ( schList );

  ////////////////////////////////////////////////////////////////
  // Create procmap menu

  MenuList *procmapList = new MenuList ( "Procmap", 'P');
  procmapList->setTearOff();
    
  Cmd *procmapPrint = new PrintProcsCmd( "Print", 'P', TRUE);

  Cmd  *procmapSetHost;

  if (Args::Inst()->params.hostList.len > 0) {
    procmapSetHost = 
      new SelectStrCmd ( "SetHost", 'H', TRUE, 
			 &Args::Inst()->setHostCallback,
			 (void *) Args::Inst(),
			 "Procmap Host",
			 Args::Inst()->params.hostList.val,
			 Args::Inst()->params.hostList.len,
			 _w);
  } else {
    procmapSetHost = 
      new PromptStrCmd ( "SetHost", 'H', TRUE, 
			 &Args::Inst()->setHostCallback,
			 (void *) Args::Inst(),
			 "Procmap Host",
			 _w);
  }

  ExecuteFuncCmd *procmapQuery = new ExecuteFuncCmd
    ("Query", 'Q', TRUE,
     Procinfo::Inst()->queryMapper);

  procmapList->vaAdd (procmapPrint, procmapSetHost, procmapQuery, NULL);
  _menuBar->addCommands ( procmapList );
  
  //////////////////////////////////////////////////////////////////
  // Create a menu for adding symbols to the schematic
  
  MenuList *addList = new MenuList ( "Add", 'd', FALSE);
  addList->setTearOff();

  RadioCmd *addProcess = new AddSymbolCmd("Process", 'P', TRUE,
					  _schematic, Symbol::Process);
  
  RadioCmd *addStore = new AddSymbolCmd("Store", 'S', TRUE,
					_schematic, Symbol::Store);
  
  RadioCmd *addArrow = new AddSymbolCmd("Arrow", 'A', TRUE,
					_schematic, Symbol::Arrow);
  
  RadioCmd *addDblArrow = new AddSymbolCmd("DblArrow", 'D', TRUE,
					   _schematic, Symbol::DblArrow);
  
  RadioCmd *addNone = new AddSymbolCmd("None", 'N', TRUE,
				       _schematic, Symbol::NoAdd);
  
  addList->vaAdd (addProcess, addStore, addArrow, addDblArrow,
		  addNone, NULL);
  
  _menuBar->addCommands ( addList );
  
  // create the set of radio buttons for add commands

  {
    RadioSet rset;
    rset.vaAdd(addProcess, addStore, addArrow, addDblArrow, addNone, NULL);
    rset.activate(4);
  }

  //////////////////////////////////////////////////////////////////////////
  // Create edit menu

  MenuList *editList = new MenuList ( "Edit", 'E', FALSE);
  editList->setTearOff();
    
  Cmd *editDelete = new NoOpCmd ( "Delete", 'D', FALSE);
  Cmd *editResize = new NoOpCmd ( "Resize", 'z' , FALSE);
  Cmd *editReshape = new NoOpCmd ( "Reshape", 'R' , FALSE);
  Cmd *editConnect = new NoOpCmd ( "Connect", 'C' , FALSE);

  editList->vaAdd (editDelete, editResize, editReshape,
		   editConnect, NULL);
  _menuBar->addCommands ( editList );

  //////////////////////////////////////////////////////////////////////////
  // Create help menu

  MenuList *helpList = new MenuList ( "Help", 'H');
  helpList->setTearOff();
    
  Cmd *onContext = new NoOpCmd ( "On Context", 'C', FALSE);
  Cmd *onHelp = new NoOpCmd ( "On Help", 'H' , FALSE);
  Cmd *onWindow = new NoOpCmd ( "On Window", 'W' , FALSE);
  Cmd *onKeys = new NoOpCmd ( "Keys", 'K' , FALSE);
  Cmd *index = new NoOpCmd ( "Index", 'I' , FALSE);
  Cmd *tutorial = new NoOpCmd ( "On Tutorial", 'T' , FALSE);
  Cmd *onVersion = new NoOpCmd ( "On Version", 'V' , FALSE);

  helpList->vaAdd (onContext, onHelp, onWindow, onKeys,
		   index, tutorial, onVersion, NULL);
  _menuBar->addCommands ( helpList );

  /////////////////////////////////////////////////////////////////////////
  // set up activation and deactivation lists
  
  run->addToActivationList(edit);
  run->vaAddToDeactivationList(run, schList, addList, editList, NULL);
  
  edit->addToDeactivationList(edit);
  edit->vaAddToActivationList(run, schList, addList, editList, NULL);
  
  // set running

  run->setRunning();
    
}

