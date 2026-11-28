//=============================================================================
//
//  (c) Copyright, 2008 University Corporation for Atmospheric Research (UCAR).
//      All rights reserved. 
//
//      File: $RCSfile: Main.cc,v $
//      Version: $Revision: 1.1 $  Dated: $Date: 2014/10/31 18:25:45 $
//
//=============================================================================

/**
 *
 * @file Main.cc
 *
 * Main function.
 *  
 * @date 10/30/2008
 *
 */

#include <stdio.h>

#include <toolsa/port.h>
#include <toolsa/umisc.h>

#include "GpmL3Hdf2Mdv.hh"

using namespace std;

// Prototypes for static functions

static void tidy_and_exit(int sig);


// Global variables

GpmL3Hdf2Mdv *Prog = (GpmL3Hdf2Mdv *)NULL;


/*********************************************************************
 * main()
 */

int main(int argc, char **argv)
{
  // Create program object.

  Prog = GpmL3Hdf2Mdv::Inst(argc, argv);
  if (!Prog->okay)
    return -1;

  if (!Prog->init())
    return -1;
  
  // Register function to trap termination and interrupts.

  PORTsignal(SIGQUIT, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGINT, tidy_and_exit);

  // Run the program.

  Prog->run();

  // clean up

  tidy_and_exit(0);
  return 0;
}

/*********************************************************************
 * tidy_and_exit()
 */

static void tidy_and_exit(int sig)
{
  // Delete the program object.

  if (Prog != (GpmL3Hdf2Mdv *)NULL)
    delete Prog;

  // Now exit the program.

  exit(sig);
}
