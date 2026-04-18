////////////////////////////////////////////////////////////////////////////////
//
//  Exit point for the application
//
//  Terri L. Betancourt, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//  October 2001
//
////////////////////////////////////////////////////////////////////////////////

#include <toolsa/pmu.h>

#include "Driver.hh"                        
using namespace std;

void dieGracefully( int sig )
{
   POSTMSG( DEBUG,  "Exiting application with signal %d", sig );

   //
   // Remove the top-level application class                                   
   // 
   delete driver;

   //
   // Unregister with process mapper
   //
   PMU_auto_unregister();
   exit( sig );
}
