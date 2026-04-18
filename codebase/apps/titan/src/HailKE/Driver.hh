////////////////////////////////////////////////////////////////////////////////
//
//  Driver for HailKE application class
//
//  Terri L. Betancourt, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//  October 2001
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _HAILKE_DRIVER_INC_
#define _HAILKE_DRIVER_INC_

#include <string>
#include <vector>
#include <toolsa/Path.hh>
#include <toolsa/MsgLog.hh>

/*------------------------------------------------------------------
 * Temporarily use old DsInputPath until we get ahold of libs/dsdata
 *------------------------------------------------------------------
#include <dsdata/DsTrigger.hh>
--------------------------------------------------------------------*/
#include <didss/DsInputPath.hh>

#include "Params.hh"
#include "DataMgr.hh"
using namespace std;

class Driver
{
public:
   Driver();
  ~Driver();

   //
   // Initialization
   //
   static const string version;
   int                 init( int argc, char **argv );
   const string&       getProgramName(){ return program.getFile(); }
   const string&       getVersion(){ return version; }

   //
   // Messaging
   //
   MsgLog&             getMsgLog(){ return msgLog; }

   //
   // Execution
   //
   int                 run();

private:

   //
   // Command line processing
   //
   Path               program;
   void               usage();
   int                processArgs( int argc, char **argv );

   //
   // Execution triggering
   //

/*------------------------------------------------------------------
 * Temporarily use old DsInputPath until we get ahold of libs/dsdata
 *------------------------------------------------------------------
   DsTrigger         *dsTrigger;
--------------------------------------------------------------------*/
   DsInputPath       *dsInputPath;

   vector< string >  *inputFileList;
   time_t             startTime;
   time_t             endTime;
   time_t             oneTime;

   //
   // Parameter processing
   //
   Params             params;
   char              *paramPath;
   tdrp_override_t    tdrpOverride;

   int                readParams(  int argc, char **argv );
   int                processParams();

   //
   // Messaging
   //
   MsgLog             msgLog;

   //
   // Data management
   //

   DataMgr            dataMgr;
};

//
// Make one instance global
//
#ifdef _HAILKE_MAIN_
          Driver *driver;
#else
   extern Driver *driver;
#endif

// 
// Macros for message logging 
// 
#define POSTMSG          driver->getMsgLog().postMsg
#define DEBUG_ENABLED    driver->getMsgLog().isEnabled( DEBUG )
   
//
// Macro for easy access to application name
//
#define PROGRAM_NAME driver->getProgramName().c_str()

//
// Prototypes for asynchronous handlers
//
void   dieGracefully( int signal );

#endif
