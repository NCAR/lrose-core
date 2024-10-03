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
// main for Lucid
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2023
//
///////////////////////////////////////////////////////////////

#include "Lucid.hh"
#include <QApplication>
#include <toolsa/uusleep.h>
#include <toolsa/LogStream.hh>
#include <QIcon>

// file scope

static void tidy_and_exit (int sig);
static Lucid *Prog;
static QApplication *app;

// main

int main(int argc, char **argv)

{

  // create program object

  try {

    // The following line may fix the problem of the Boundary Editor
    // not appearing in the menu bar when Lucid is built using
    // linux, run on a Mac.                                                                                                                       
    QCoreApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
    
    app = new QApplication(argc, argv);
    app->setWindowIcon(QIcon("://LucidCycloneIcon.icns"));
    LOG(DEBUG_VERBOSE) << "After setting Window Icon\n";
    Lucid *Prog;
    Prog = new Lucid(argc, argv);
    if (!Prog->OK) {
      return(-1);
    }
    
    // run it
    
    int iret = Prog->Run(*app);
    
    // clean up
    
    tidy_and_exit(iret);
    return (iret);
    
  } catch (std::bad_alloc &a) {
    cerr << ">>>>> bad alloc: " << a.what() << endl;
  }

}

// tidy up on exit

static void tidy_and_exit (int sig)

{
  app->exit();
  delete(Prog);
  umsleep(1000);
  exit(sig);
}
