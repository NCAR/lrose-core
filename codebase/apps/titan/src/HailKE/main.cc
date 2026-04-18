////////////////////////////////////////////////////////////////////////////////
// 
//  Entry point for the application
//
//  Terri L. Betancourt, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//  October 2001
//
////////////////////////////////////////////////////////////////////////////////
#include <toolsa/port.h>
#include <toolsa/umisc.h>

#define _HAILKE_MAIN_
#include "Driver.hh"
using namespace std;

int main( int argc, char **argv )
{
   int status;

   //        
   // Trap signals for a clean exit
   //                                                                        
   PORTsignal( SIGHUP, dieGracefully );
   PORTsignal( SIGINT,  dieGracefully );                    
   PORTsignal( SIGTERM, dieGracefully );
   PORTsignal( SIGQUIT, dieGracefully );      
   PORTsignal( SIGKILL, dieGracefully );      
   PORTsignal( SIGPIPE, (PORTsigfunc)SIG_IGN );
   
   //                                                        
   // Instantiate and initialize the top-level application class
   //                              
   driver = new Driver();
   status = driver->init( argc, argv );
   if ( status != 0 ) { 
      POSTMSG( ERROR, "Application initialization failed." );
      dieGracefully( status );
   }

   //
   // Startup the application -- the end.
   //
   status = driver->run();
   dieGracefully( status );
}
